#include "GraphicManager.h"

#include "DirectXController.h"
#include "Camera.h"

#include "Grid.h"
#include "GraphicRigidBody.h"
#include "GraphicParticleSystem.h"
#include "GraphicParticleData.h"

#include "GraphicData.h"
#include "GeneralSimulationData.h"

#include "SingletonHolder.h"
#include "IWindowsManager.h"
#include "ITimeManager.h"
#include "IInputManager.h"
#include "IConfigManager.h"
#include "IThreadManager.h"

#include "ThreadHelper.h"
#include "ThreadEnumeration.h"

#include "SpecialKey.h"

namespace
{
#if false
	const float g_defaultColor[4] = { 0.f, 0.5f, 0.0f, 1.f };
#else
	const float g_defaultColor[4] = { 0.f, 0.f, 0.f, 1.f };
#endif

#if _WIN32
	struct VectorHijacker
	{
		std::size_t _newSize;
	};

	using VectorHijackerMakeBelieve = const VectorHijacker &;

	using HijackedType = Storm::GraphicParticleData;
#endif
}

#if _WIN32
// (   - _ - |||)
// Ugly but super efficient...
template<>
template<>
decltype(auto) std::vector<HijackedType, std::allocator<HijackedType>>::emplace_back<VectorHijackerMakeBelieve>(VectorHijackerMakeBelieve first)
{
	// We're modifying directly the size of the vector without passing by the extra initialization.
	_Mypair._Myval2._Mylast = _Mypair._Myval2._Myfirst + first._newSize;
}
#endif

namespace
{
#if _WIN32
	void setNumUninitialized_hijack(std::vector<HijackedType> &hijackedVector, VectorHijackerMakeBelieve hijacker)
	{
		hijackedVector.emplace_back<VectorHijackerMakeBelieve>(hijacker);
	}
#endif

	std::vector<Storm::GraphicParticleData> fastOptimizedTransCopy(const std::vector<Storm::Vector3> &particlePosData)
	{
		std::vector<Storm::GraphicParticleData> dxParticlePosDataTmp;

		const DirectX::XMVECTOR defaultColor{ 0.3f, 0.5f, 0.7f, 1.f };

#if _WIN32

		const VectorHijackerMakeBelieve hijacker{ particlePosData.size() };
		dxParticlePosDataTmp.reserve(hijacker._newSize);

		// Huge optimization that completely destroys resize method... Cannot be much faster than this, it is like Unreal technology (TArray provides a SetNumUninitialized).
		// (Except that Unreal implemented their own TArray instead of using std::vector. Since I'm stuck with this, I didn't have much choice than to hijack... Note that this code isn't portable because it relies heavily on how Microsoft implemented std::vector (to find out the breach in the armor, we must know whose armor it is ;) )).
		setNumUninitialized_hijack(dxParticlePosDataTmp, hijacker);

		for (std::size_t iter = 0; iter < hijacker._newSize; ++iter)
		{
			Storm::GraphicParticleData &current = dxParticlePosDataTmp[iter];
			memcpy(&current._pos, &particlePosData[iter], sizeof(Storm::Vector3));
			reinterpret_cast<float*>(&current._pos)[3] = 1.f;

			current._color = defaultColor;
		}

#else
		dxParticlePosDataTmp.reserve(hijacker._newSize);

		for (const Storm::Vector3 &pos : particlePosData)
		{
			dxParticlePosDataTmp.emplace_back(pos, defaultColor);
		}

#endif

		return dxParticlePosDataTmp;
	}
}

Storm::GraphicManager::GraphicManager() :
	_renderCounter{ 0 },
	_directXController{ std::make_unique<Storm::DirectXController>() }
{

}

Storm::GraphicManager::~GraphicManager() = default;

bool Storm::GraphicManager::initialize_Implementation()
{
	LOG_COMMENT << "Starting to initialize the Graphic Manager. We would evaluate if Windows is created. If not, we will suspend initialization and come back later.";

	Storm::IWindowsManager &windowsMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IWindowsManager>();

	HWND hwnd = static_cast<HWND>(windowsMgr.getWindowHandle());
	if (hwnd != nullptr)
	{
		this->initialize_Implementation(hwnd);
		return true;
	}
	else
	{
		bool initRes = false;
		windowsMgr.bindFinishInitializeCallback([this, res = &initRes](void* hwndOnceReady, bool calledAtBindingTime)
		{
			if (calledAtBindingTime)
			{
				// If this was called at binding time, then this is on the same thread so it is okay to still reference res (we haven't left initialize_Implementation yet).
				this->initialize_Implementation(hwndOnceReady);
				*res = true;
			}
			else
			{
				this->initialize(hwndOnceReady);
			}
		});

		LOG_WARNING << "HWND not valid, Graphic initialization will be suspended and done asynchronously later.";
		return initRes;
	}
}

void Storm::GraphicManager::initialize_Implementation(void* hwnd)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	LOG_COMMENT << "HWND is valid so Windows was created, we can pursue the graphic initialization.";
	_directXController->initialize(static_cast<HWND>(hwnd));

	_camera = std::make_unique<Storm::Camera>(_directXController->getViewportWidth(), _directXController->getViewportHeight());

	const auto &device = _directXController->getDirectXDevice();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GraphicData &graphicData = configMgr.getGraphicData();
	_renderedElements.emplace_back(std::make_unique<Storm::Grid>(device, graphicData._grid));

	_graphicParticlesSystem = std::make_unique<Storm::GraphicParticleSystem>(device);

	for (auto &meshesPair : _meshesMap)
	{
		meshesPair.second->initializeRendering(device);
	}

	if (graphicData._displaySolidAsParticles)
	{
		_directXController->setAllParticleState();
	}

	bool registered = false;
	static std::condition_variable waiter;
	static std::mutex mut;
	std::unique_lock<std::mutex> lock{ mut };

	_renderThread = std::thread([this, registeredTmp = &registered]()
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

		STORM_REGISTER_THREAD(GraphicsThread);

		// The real moment we consider the thread as started is when it has registered. Not before...
		{
			std::lock_guard<std::mutex> lock{ mut };
			*registeredTmp = true;
			waiter.notify_all();
		}

		{
			// I have an Azerty keyboard, so if you have a Qwerty keyboard, you'll surely need to change those.

			Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();
			inputMgr.bindKey(Storm::SpecialKey::KC_UP, [this]() { _camera->positiveMoveYAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_LEFT, [this]() { _camera->positiveMoveXAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_NUMPAD8, [this]() { _camera->positiveMoveZAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_RIGHT, [this]() { _camera->negativeMoveXAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_DOWN, [this]() { _camera->negativeMoveYAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_NUMPAD2, [this]() { _camera->negativeMoveZAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_S, [this]() { _camera->positiveRotateXAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_D, [this]() { _camera->positiveRotateYAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_W, [this]() { _camera->negativeRotateXAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_A, [this]() { _camera->negativeRotateYAxis(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_NUMPAD0, [this]() { _camera->reset(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_ADD, [this]() { _camera->increaseNearPlane(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_SUBTRACT, [this]() { _camera->decreaseNearPlane(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_MULTIPLY, [this]() { _camera->increaseFarPlane(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_DIVIDE, [this]() { _camera->decreaseFarPlane(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_V, [this]() { _directXController->setWireFrameState(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_B, [this]() { _directXController->setSolidCullBackState(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_N, [this]() { _directXController->setSolidCullNoneState(); });
			inputMgr.bindKey(Storm::SpecialKey::KC_C, [this]() { _directXController->setAllParticleState(); });

			inputMgr.bindMouseWheel([this](int axisRelativeIncrement)
			{
				if (axisRelativeIncrement > 0)
				{
					_camera->increaseCameraSpeed();
				}
				else if (axisRelativeIncrement < 0)
				{
					_camera->decreaseCameraSpeed();
				}
			});
		}

		Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
		while (timeMgr.waitNextFrameOrExit())
		{
			this->update();
		}
	});

	waiter.wait(lock, [&registered]() { return registered; });
}

void Storm::GraphicManager::cleanUp_Implementation()
{
	LOG_COMMENT << "Starting to clean up the Graphic Manager.";
	Storm::join(_renderThread);

	_directXController->cleanUp();
}

void Storm::GraphicManager::update()
{
	if (_renderCounter++ % 2 == 0)
	{
		SingletonHolder::instance().getSingleton<Storm::IThreadManager>().processCurrentThreadActions();

		_directXController->clearView(g_defaultColor);
		_directXController->initView();

		_directXController->renderElements(this->getCamera(), _renderedElements, _meshesMap, *_graphicParticlesSystem);

		_directXController->unbindTargetView();
		_directXController->presentToDisplay();
	}
}

void Storm::GraphicManager::addMesh(unsigned int meshId, const std::vector<Storm::Vector3> &vertexes, const std::vector<Storm::Vector3> &normals, const std::vector<unsigned int> &indexes)
{
	_meshesMap[meshId] = std::make_unique<Storm::GraphicRigidBody>(vertexes, normals, indexes);
}

void Storm::GraphicManager::bindParentRbToMesh(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const
{
	if (const auto found = _meshesMap.find(meshId); found != std::end(_meshesMap))
	{
		found->second->setRbParent(parentRb);
	}
	else
	{
		Storm::throwException<std::exception>("Cannot find rb " + std::to_string(meshId) + " inside registered graphics meshes!");
	}
}

void Storm::GraphicManager::pushParticlesData(unsigned int particleSystemId, const std::vector<Storm::Vector3> &particlePosData, bool isFluids, bool isWall)
{
	assert(!(isFluids && isWall) && "Particle cannot be fluid AND wall at the same time!");

	this->callSequentialToInitCleanup([this, particleSystemId, &particlePosData, isFluids, isWall]()
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

		singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(ThreadEnumeration::GraphicsThread,
			[this, particleSystemId, particlePosDataCopy = fastOptimizedTransCopy(particlePosData), isFluids, isWall]() mutable
		{
			_graphicParticlesSystem->refreshParticleSystemData(particleSystemId, std::move(particlePosDataCopy), isFluids, isWall);
		});
	});
}

const Storm::Camera& Storm::GraphicManager::getCamera() const
{
	return *_camera;
}

const Storm::DirectXController& Storm::GraphicManager::getController() const
{
	return *_directXController;
}
