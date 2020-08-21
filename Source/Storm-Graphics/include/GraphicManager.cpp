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

#include "BlowerData.h"
#include "BlowerType.h"
#include "GraphicBlower.h"

#include "MeshMaker.h"


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

	template<bool isFluids>
	constexpr DirectX::XMVECTOR getDefaultParticleColor()
	{
		if constexpr (isFluids)
		{
			return { 0.3f, 0.5f, 0.85f, 1.f };
		}
		else
		{
			return { 0.3f, 0.5f, 0.5f, 1.f };
		}
	}

	template<bool enableDifferentColoring>
	void fastTransCopyImpl(const std::vector<Storm::Vector3> &particlePosData, const std::vector<Storm::Vector3> &particleVelocityData, const float valueToMinColor, const float valueToMaxColor, std::vector<Storm::GraphicParticleData> &inOutDxParticlePosDataTmp)
	{
		const DirectX::XMVECTOR defaultColor = getDefaultParticleColor<enableDifferentColoring>();

		const float deltaColorRChan = 1.f - defaultColor.m128_f32[0];
		const float deltaValueForColor = valueToMaxColor - valueToMinColor;

		const std::size_t particleCount = particlePosData.size();
		for (std::size_t iter = 0; iter < particleCount; ++iter)
		{
			Storm::GraphicParticleData &current = inOutDxParticlePosDataTmp[iter];
			memcpy(&current._pos, &particlePosData[iter], sizeof(Storm::Vector3));
			reinterpret_cast<float*>(&current._pos)[3] = 1.f;

			if constexpr (enableDifferentColoring)
			{
				float coeff = particleVelocityData[iter].squaredNorm() - valueToMinColor;
				if (coeff < 0.f)
				{
					current._color = defaultColor;
				}
				else
				{
					coeff = coeff / deltaValueForColor;
					if (coeff > 1.f)
					{
						current._color.m128_f32[0] = 1.f;
					}
					else
					{
						current._color.m128_f32[0] = defaultColor.m128_f32[0] + deltaColorRChan * coeff;
					}
					memcpy(&current._color.m128_f32[1], &defaultColor.m128_f32[1], sizeof(float) * 3);
				}
			}
			else
			{
				current._color = defaultColor;
			}
		}
	}

	std::vector<Storm::GraphicParticleData> fastOptimizedTransCopy(const std::vector<Storm::Vector3> &particlePosData, const std::vector<Storm::Vector3> &particleVelocityData, const float valueToMinColor, const float valueToMaxColor, bool enableDifferentColoring)
	{
		std::vector<Storm::GraphicParticleData> dxParticlePosDataTmp;

#if _WIN32

		const VectorHijackerMakeBelieve hijacker{ particlePosData.size() };
		dxParticlePosDataTmp.reserve(hijacker._newSize);

		// Huge optimization that completely destroys resize method... Cannot be much faster than this, it is like Unreal technology (TArray provides a SetNumUninitialized).
		// (Except that Unreal implemented their own TArray instead of using std::vector. Since I'm stuck with this, I didn't have much choice than to hijack... Note that this code isn't portable because it relies heavily on how Microsoft implemented std::vector (to find out the breach in the armor, we must know whose armor it is ;) )).
		setNumUninitialized_hijack(dxParticlePosDataTmp, hijacker);

		if (enableDifferentColoring)
		{
			fastTransCopyImpl<true>(particlePosData, particleVelocityData, valueToMinColor, valueToMaxColor, dxParticlePosDataTmp);
		}
		else
		{
			fastTransCopyImpl<false>(particlePosData, particleVelocityData, valueToMinColor, valueToMaxColor, dxParticlePosDataTmp);
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

	void appendNewBlower(std::map<std::size_t, std::unique_ptr<Storm::GraphicBlowerBase>> &inOutGraphicBlowerContainer, const ComPtr<ID3D11Device> &device, const Storm::BlowerData &blowerDataConfig)
	{
		std::unique_ptr<Storm::GraphicBlowerBase> graphicBlower;
		
		switch (blowerDataConfig._blowerType)
		{
		case Storm::BlowerType::Cube:
			graphicBlower = std::make_unique<Storm::GraphicBlower<Storm::BlowerType::Cube, Storm::CubeMeshMaker>>(device, blowerDataConfig);
			break;

		case Storm::BlowerType::Sphere:
			graphicBlower = std::make_unique<Storm::GraphicBlower<Storm::BlowerType::Sphere, Storm::SphereMeshMaker>>(device, blowerDataConfig);
			break;

		default:
		case Storm::BlowerType::None:
			Storm::throwException<std::exception>("Unknown Graphic Blower to be created!");
			break;
		}

		inOutGraphicBlowerContainer[blowerDataConfig._id] = std::move(graphicBlower);

		LOG_DEBUG << "Graphic blower " << blowerDataConfig._id << " was created.";
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

	this->loadBlowers();

	for (auto &meshesPair : _meshesMap)
	{
		meshesPair.second->initializeRendering(device);
	}

	if (graphicData._displaySolidAsParticles)
	{
		_directXController->setAllParticleState();
	}

	_renderThread = std::thread([this]()
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

		STORM_REGISTER_THREAD(GraphicsThread);

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
			inputMgr.bindKey(Storm::SpecialKey::KC_X, [this]() { _directXController->setRenderNoWallParticle(); });

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

		_directXController->drawUI(_fieldsMap);

		_directXController->unbindTargetView();
		_directXController->presentToDisplay();

		_directXController->reportDeviceMessages();
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

void Storm::GraphicManager::loadBlowers()
{
	const ComPtr<ID3D11Device> &currentDevice = _directXController->getDirectXDevice();

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const std::vector<Storm::BlowerData> &allBlowersData = configMgr.getBlowersData();
	for (const Storm::BlowerData &blowerData : allBlowersData)
	{
		appendNewBlower(_blowersMap, currentDevice, blowerData);
	}
}

void Storm::GraphicManager::pushParticlesData(unsigned int particleSystemId, const std::vector<Storm::Vector3> &particlePosData, const std::vector<Storm::Vector3> &particleVelocityData, bool isFluids, bool isWall)
{
	assert(!(isFluids && isWall) && "Particle cannot be fluid AND wall at the same time!");

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GraphicData &graphicDataConfig = configMgr.getGraphicData();

	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(ThreadEnumeration::GraphicsThread,
		[this, particleSystemId, particlePosDataCopy = fastOptimizedTransCopy(particlePosData, particleVelocityData, graphicDataConfig._valueForMinColor, graphicDataConfig._valueForMaxColor, isFluids), isFluids, isWall]() mutable
	{
		_graphicParticlesSystem->refreshParticleSystemData(particleSystemId, std::move(particlePosDataCopy), isFluids, isWall);
	});
}

void Storm::GraphicManager::createGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValueStr)
{
	assert(_fieldsMap.find(fieldName) == std::end(_fieldsMap) && "We shouldn't create another field with the same name!");
	_fieldsMap[fieldName] = std::move(fieldValueStr);

	_directXController->notifyFieldCount(_fieldsMap.size());
}

void Storm::GraphicManager::updateGraphicsField(std::vector<std::pair<std::wstring_view, std::wstring>> &&rawFields)
{
	for (auto &field : rawFields)
	{
		this->updateGraphicsField(field.first, std::move(field.second));
	}
}

void Storm::GraphicManager::updateGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValue)
{
	if (auto found = _fieldsMap.find(fieldName); found != std::end(_fieldsMap))
	{
		found->second = std::move(fieldValue);
	}
}

const Storm::Camera& Storm::GraphicManager::getCamera() const
{
	return *_camera;
}

const Storm::DirectXController& Storm::GraphicManager::getController() const
{
	return *_directXController;
}

std::size_t Storm::GraphicManager::getFieldCount() const
{
	return _fieldsMap.size();
}
