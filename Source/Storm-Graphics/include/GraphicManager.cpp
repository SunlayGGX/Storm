#include "GraphicManager.h"

#include "DirectXController.h"
#include "Camera.h"

#include "GeneralReadOnlyUIDisplay.h"

#include "Grid.h"
#include "GraphicCoordinateSystem.h"
#include "GraphicRigidBody.h"
#include "GraphicParticleSystem.h"
#include "GraphicParticleData.h"
#include "GraphicConstraintSystem.h"
#include "GraphicBlower.h"
#include "ParticleForceRenderer.h"
#include "GraphicKernelEffectArea.h"

#include "PushedParticleSystemData.h"
#include "GraphicPipe.h"

#include "SceneBlowerConfig.h"
#include "SceneGraphicConfig.h"
#include "SceneSimulationConfig.h"

#include "SingletonHolder.h"
#include "IWindowsManager.h"
#include "ITimeManager.h"
#include "IInputManager.h"
#include "IConfigManager.h"
#include "IThreadManager.h"

#include "ThreadHelper.h"
#include "ThreadEnumeration.h"
#include "ThreadingSafety.h"
#include "ThreadFlaggerObject.h"

#include "SpecialKey.h"

#include "RunnerHelper.h"


namespace
{
#if false
	const float g_defaultColor[4] = { 0.f, 0.5f, 0.0f, 1.f };
#else
	const float g_defaultColor[4] = { 0.f, 0.f, 0.f, 1.f };
#endif
}


Storm::GraphicManager::GraphicManager() :
	_renderCounter{ 0 },
	_directXController{ std::make_unique<Storm::DirectXController>() },
	_selectedParticle{ std::numeric_limits<decltype(_selectedParticle.first)>::max(), 0 },
	_hasUI{ false },
	_dirty{ true },
	_watchedRbNonOwningPtr{ nullptr }
{

}

Storm::GraphicManager::~GraphicManager() = default;

bool Storm::GraphicManager::initialize_Implementation(const Storm::WithUI &)
{
	LOG_COMMENT << "Starting to initialize the Graphic Manager. We would evaluate if Windows is created. If not, we will suspend initialization and come back later.";

	_hasUI = true;

	_pipe = std::make_unique<Storm::GraphicPipe>();

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::IWindowsManager &windowsMgr = singletonHolder.getSingleton<Storm::IWindowsManager>();

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
				// It is important to call initialize here and not initialize_Implementation because it locks the initialization mutex. It prevents tocttou.
				this->initialize(hwndOnceReady);
			}
		});

		LOG_WARNING << "HWND not valid, Graphic initialization will be suspended and done asynchronously later.";
		return initRes;
	}
}

void Storm::GraphicManager::initialize_Implementation(const Storm::NoUI &)
{
	LOG_DEBUG << "No UI requested. Graphic Manager will be left uninitialized.";
	_hasUI = false;
}

void Storm::GraphicManager::initialize_Implementation(void* hwnd)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	if (!configMgr.withUI())
	{
		return;
	}

	Storm::IWindowsManager &windowsMgr = singletonHolder.getSingleton<Storm::IWindowsManager>();

	_windowsResizedCallbackId = windowsMgr.bindWindowsResizedCallback([this, &singletonHolder](int newWidth, int newHeight)
	{
		singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, newWidth, newHeight]()
		{
			this->notifyViewportRescaled(newWidth, newHeight);
			_dirty = true;
		});
	});

	_windowsMovedCallbackId = windowsMgr.bindWindowsMovedCallback([this, &singletonHolder](int newX, int newY)
	{
		singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, newX, newY]()
		{
			_dirty = true;
		});
	});

	LOG_COMMENT << "HWND is valid so Windows was created, we can pursue the graphic initialization.";
	_directXController->initialize(static_cast<HWND>(hwnd));

	_camera = std::make_unique<Storm::Camera>(_directXController->getViewportWidth(), _directXController->getViewportHeight());

	const auto &device = _directXController->getDirectXDevice();
	const Storm::SceneGraphicConfig &sceneGraphicConfig = configMgr.getSceneGraphicConfig();

	_renderedElements.emplace_back(std::make_unique<Storm::Grid>(device, sceneGraphicConfig._grid));
	_coordSystemNonOwningPtr = static_cast<Storm::GraphicCoordinateSystem*>(_renderedElements.emplace_back(std::make_unique<Storm::GraphicCoordinateSystem>(device)).get());

	_graphicParticlesSystem = std::make_unique<Storm::GraphicParticleSystem>(device);

	_graphicConstraintsSystem = std::make_unique<Storm::GraphicConstraintSystem>(device);

	_forceRenderer = std::make_unique<Storm::ParticleForceRenderer>(device);

	_kernelEffectArea = std::make_unique<Storm::GraphicKernelEffectArea>(device);

	for (auto &meshesPair : _meshesMap)
	{
		meshesPair.second->initializeRendering(device);
	}

	if (sceneGraphicConfig._rbWatchId != std::numeric_limits<decltype(sceneGraphicConfig._rbWatchId)>::max())
	{
		_watchedRbNonOwningPtr = _meshesMap.find(static_cast<unsigned int>(sceneGraphicConfig._rbWatchId))->second.get();
	}

	if (sceneGraphicConfig._displaySolidAsParticles)
	{
		_directXController->setAllParticleState();
	}

	_readOnlyFields = std::make_unique<Storm::GeneralReadOnlyUIDisplay>();

	_renderThread = std::thread([this]()
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

		STORM_REGISTER_THREAD(GraphicsThread);
		STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::GraphicThread;

		{
			// I have an Azerty keyboard, so if you have a Qwerty keyboard, you'll surely need to change those.

			Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();
			inputMgr.bindKey(Storm::SpecialKey::KC_UP, [this]() { _camera->positiveMoveYAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_LEFT, [this]() { _camera->positiveMoveXAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_NUMPAD8, [this]() { _camera->positiveMoveZAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_RIGHT, [this]() { _camera->negativeMoveXAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_DOWN, [this]() { _camera->negativeMoveYAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_NUMPAD2, [this]() { _camera->negativeMoveZAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_S, [this]() { _camera->positiveRotateXAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_D, [this]() { _camera->positiveRotateYAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_W, [this]() { _camera->negativeRotateXAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_A, [this]() { _camera->negativeRotateYAxis(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_NUMPAD0, [this]() { _camera->reset(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_ADD, [this]() { this->checkUserCanChangeNearPlane(); _camera->increaseNearPlane(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_SUBTRACT, [this]() { this->checkUserCanChangeNearPlane(); _camera->decreaseNearPlane(); _dirty = true;});
			inputMgr.bindKey(Storm::SpecialKey::KC_MULTIPLY, [this]() { _camera->increaseFarPlane(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_DIVIDE, [this]() { _camera->decreaseFarPlane(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F2, [this]() { _coordSystemNonOwningPtr->switchShow(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F3, [this]() { _directXController->setRenderSolidOnly(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F4, [this]() { _forceRenderer->tweekAlwaysOnTop(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F5, [this]() { _directXController->setWireFrameState(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F6, [this]() { _directXController->setSolidCullBackState(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F7, [this]() { _directXController->setSolidCullNoneState(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F8, [this]() { _directXController->setAllParticleState(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F9, [this]() { _directXController->setRenderNoWallParticle(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F12, [this]() { _directXController->setUIFieldDrawEnabled(!_directXController->getUIFieldDrawEnabled()); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F1, [this]()
			{
				if (this->hasSelectedParticle())
				{
					_kernelEffectArea->tweakEnabled();
					_dirty = true;
				}
			});

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

void Storm::GraphicManager::cleanUp_Implementation(const Storm::WithUI &)
{
	LOG_COMMENT << "Starting to clean up the Graphic Manager.";

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::IWindowsManager &windowsMgr = singletonHolder.getSingleton<Storm::IWindowsManager>();

	Storm::join(_renderThread);

	windowsMgr.unbindWindowsResizedCallback(_windowsResizedCallbackId);
	windowsMgr.unbindWindowsMovedCallback(_windowsMovedCallbackId);

	_selectedParticle.first = std::numeric_limits<decltype(_selectedParticle.first)>::max();

	_blowersMap.clear();
	_forceRenderer.reset();
	_graphicConstraintsSystem.reset();
	_graphicParticlesSystem.reset();
	_meshesMap.clear();
	_renderedElements.clear();
	_fieldsMap.clear();

	_directXController->cleanUp();
}

void Storm::GraphicManager::cleanUp_Implementation(const Storm::NoUI &)
{
	LOG_COMMENT << "No UI requested. Graphic Manager clean up is trivial.";
}

void Storm::GraphicManager::update()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().processCurrentThreadActions();

	if (_renderCounter++ % 2 == 0 && _dirty)
	{
		Storm::Camera &currentCamera = *_camera;

		if (_watchedRbNonOwningPtr != nullptr)
		{
			currentCamera.updateWatchedRb(_watchedRbNonOwningPtr->getRbPosition());
		}

		_directXController->clearView(g_defaultColor);
		_directXController->initView();

		_directXController->renderElements(currentCamera, _renderedElements, _meshesMap, *_graphicParticlesSystem, _blowersMap, *_graphicConstraintsSystem, *_forceRenderer, *_kernelEffectArea);

		_directXController->drawUI(_fieldsMap);

		_directXController->unbindTargetView();
		_directXController->presentToDisplay();

		_directXController->reportDeviceMessages();

		_dirty = false;
	}
}

bool Storm::GraphicManager::isActive() const noexcept
{
	return _hasUI || Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().withUI();
}

void Storm::GraphicManager::addMesh(unsigned int meshId, const std::vector<Storm::Vector3> &vertexes, const std::vector<Storm::Vector3> &normals, const std::vector<unsigned int> &indexes)
{
	if (this->isActive())
	{
		_meshesMap[meshId] = std::make_unique<Storm::GraphicRigidBody>(vertexes, normals, indexes);
	}
}

void Storm::GraphicManager::bindParentRbToMesh(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const
{
	if (this->isActive())
	{
		if (const auto found = _meshesMap.find(meshId); found != std::end(_meshesMap))
		{
			found->second->setRbParent(parentRb);
		}
		else
		{
			Storm::throwException<Storm::Exception>("Cannot find rb " + std::to_string(meshId) + " inside registered graphics meshes!");
		}
	}
}

void Storm::GraphicManager::loadBlower(const Storm::SceneBlowerConfig &blowerConfig, const std::vector<Storm::Vector3> &vertexes, const std::vector<unsigned int> &indexes)
{
	if (this->isActive())
	{
		const ComPtr<ID3D11Device> &currentDevice = _directXController->getDirectXDevice();

		std::unique_ptr<Storm::GraphicBlower> graphicBlower = std::make_unique<Storm::GraphicBlower>(currentDevice, blowerConfig, vertexes, indexes);
		_blowersMap[blowerConfig._blowerId] = std::move(graphicBlower);

		LOG_DEBUG << "Graphic blower " << blowerConfig._blowerId << " was created.";
	}
}

void Storm::GraphicManager::pushParticlesData(const Storm::PushedParticleSystemDataParameter &param)
{
	if (this->isActive())
	{
		assert(!(param._isFluids && param._isWall) && "Particle cannot be fluid AND wall at the same time!");

		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

		singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(ThreadEnumeration::GraphicsThread,
			[this, particleSystemId = param._particleSystemId, particlePosDataCopy = _pipe->fastOptimizedTransCopy(param), isFluids = param._isFluids, isWall = param._isWall, pos = param._position]() mutable
		{
			if (_forceRenderer->prepareData(particleSystemId, particlePosDataCopy, _selectedParticle))
			{
				_graphicParticlesSystem->refreshParticleSystemData(_directXController->getDirectXDevice(), particleSystemId, std::move(particlePosDataCopy), isFluids, isWall);

				if (this->hasSelectedParticle() && particleSystemId == _selectedParticle.first)
				{
					_kernelEffectArea->setAreaPosition(particlePosDataCopy[_selectedParticle.second]);
				}

				if (!isFluids && !isWall)
				{
					Storm::GraphicRigidBody &currentGraphicRb = *_meshesMap.find(particleSystemId)->second;
					currentGraphicRb.setRbPosition(pos);
				}

				_dirty = true;
			}
		});
	}
}

void Storm::GraphicManager::pushConstraintData(const std::vector<Storm::Vector3> &constraintsVisuData)
{
	if (this->isActive())
	{
		_graphicConstraintsSystem->refreshConstraintsData(_directXController->getDirectXDevice(), constraintsVisuData);
	}
}

void Storm::GraphicManager::pushParticleSelectionForceData(const Storm::Vector3 &selectedParticlePos, const Storm::Vector3 &selectedParticleForce)
{
	if (this->isActive())
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(ThreadEnumeration::GraphicsThread,
			[this, selectedParticlePos, selectedParticleForce]() mutable
		{
			_forceRenderer->refreshForceData(_directXController->getDirectXDevice(), selectedParticlePos, selectedParticleForce);
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::createGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValueStr)
{
	if (this->isActive())
	{
		assert(_fieldsMap.find(fieldName) == std::end(_fieldsMap) && "We shouldn't create another field with the same name!");
		_fieldsMap[fieldName] = std::move(fieldValueStr);

		_directXController->notifyFieldCount(_fieldsMap.size());

		_dirty = true;
	}
}

void Storm::GraphicManager::updateGraphicsField(std::vector<std::pair<std::wstring_view, std::wstring>> &&rawFields)
{
	if (this->isActive())
	{
		for (auto &field : rawFields)
		{
			this->updateGraphicsField(field.first, std::move(field.second));
		}
	}
}

void Storm::GraphicManager::updateGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValue)
{
	if (auto found = _fieldsMap.find(fieldName); found != std::end(_fieldsMap))
	{
		found->second = std::move(fieldValue);
		_dirty = true;
	}
}

std::size_t Storm::GraphicManager::getFieldCount() const
{
	return _fieldsMap.size();
}

void Storm::GraphicManager::convertScreenPositionToRay(const Storm::Vector2 &screenPos, Storm::Vector3 &outRayOrigin, Storm::Vector3 &outRayDirection) const
{
	assert(Storm::isGraphicThread() && "this method should only be executed on graphic thread.");

	if (this->isActive())
	{
		_camera->convertScreenPositionToRay(screenPos, outRayOrigin, outRayDirection);
	}
}

void Storm::GraphicManager::getClippingPlaneValues(float &outZNear, float &outZFar) const
{
	assert(Storm::isGraphicThread() && "this method should only be executed on graphic thread.");
	
	if (this->isActive())
	{
		outZNear = _camera->getNearPlane();
		outZFar = _camera->getFarPlane();
	}
	else
	{
		outZNear = 0.f;
		outZFar = 0.f;
	}
}

Storm::Vector3 Storm::GraphicManager::get3DPosOfScreenPixel(const Storm::Vector2 &screenPos) const
{
	// Screen pixel positions to 3D
	Storm::Vector3 vectClipSpace3DPos{
		screenPos.x(),
		screenPos.y(),
		0.f
	};

	// Transform the screen pixel positions to render target pixel texture position.
	_camera->rescaleScreenPosition(vectClipSpace3DPos.x(), vectClipSpace3DPos.y());

	// Apply the Z-buffer to the Z position, and we would have the 3D clip space position of the selected pixel.
	vectClipSpace3DPos.z() = _directXController->getDepthBufferAtPixel(static_cast<int>(vectClipSpace3DPos.x()), static_cast<int>(vectClipSpace3DPos.y()));

	return _camera->convertScreenPositionTo3DPosition(vectClipSpace3DPos);
}

void Storm::GraphicManager::safeSetSelectedParticle(unsigned int particleSystemId, std::size_t particleIndex)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(ThreadEnumeration::GraphicsThread, [this, particleSystemId, particleIndex]()
		{
			_selectedParticle.first = particleSystemId;
			_selectedParticle.second = particleIndex;
			_kernelEffectArea->setHasParticleHook(true);
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::safeClearSelectedParticle()
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(ThreadEnumeration::GraphicsThread, [this]()
		{
			_selectedParticle.first = std::numeric_limits<decltype(_selectedParticle.first)>::max();
			_kernelEffectArea->setHasParticleHook(false);
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::setUIFieldEnabled(bool enable)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, enable]()
	{
		_directXController->setUIFieldDrawEnabled(enable);
		_dirty = true;
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

void Storm::GraphicManager::setTargetPositionTo(const Storm::Vector3 &newTargetPosition)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, newTargetPosition]()
		{
			_camera->setTarget(newTargetPosition.x(), newTargetPosition.y(), newTargetPosition.z());
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::changeBlowerState(const std::size_t blowerId, const Storm::BlowerState newState)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, blowerId, newState]()
		{
			_blowersMap[blowerId]->setBlowerState(newState);
			_dirty = true;
		});
	}
}

bool Storm::GraphicManager::hasSelectedParticle() const
{
	return this->isActive() && _selectedParticle.first != std::numeric_limits<decltype(_selectedParticle.first)>::max();
}

void Storm::GraphicManager::notifyViewportRescaled(int newWidth, int newHeight)
{
	_camera->setRescaledDimension(static_cast<float>(newWidth), static_cast<float>(newHeight));
}

void Storm::GraphicManager::cycleColoredSetting()
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this]()
		{
			_pipe->cycleColoredSetting();
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::setColorSettingMinMaxValue(float minValue, float maxValue)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, minValue, maxValue]()
		{
			_pipe->setMinMaxColorationValue(minValue, maxValue);
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::setUseColorSetting(const Storm::ColoredSetting colorSetting)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, colorSetting]()
		{
			_pipe->setUsedColorSetting(colorSetting);
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::showCoordinateSystemAxis(const bool shouldShow)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, shouldShow]()
	{
		_coordSystemNonOwningPtr->show(shouldShow);
		_dirty = true;
	});
}

void Storm::GraphicManager::setKernelAreaRadius(const float radius)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, radius]()
	{
		_kernelEffectArea->setAreaRadius(radius);
		_dirty = true;
	});
}

void Storm::GraphicManager::lockNearPlaneOnWatchedRb(unsigned int watchedRbId)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, watchedRbId]()
	{
		if (const auto found = _meshesMap.find(static_cast<unsigned int>(watchedRbId)); found != std::end(_meshesMap))
		{
			_watchedRbNonOwningPtr = found->second.get();
			_dirty = true;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Cannot find the rigid body to lock the near plane on. Requested id was " + std::to_string(watchedRbId));
		}
	});
}

void Storm::GraphicManager::unlockNearPlaneOnWatchedRb()
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this]()
	{
		_watchedRbNonOwningPtr = nullptr;
	});
}

void Storm::GraphicManager::checkUserCanChangeNearPlane() const
{
	if (_watchedRbNonOwningPtr != nullptr)
	{
		Storm::throwException<Storm::Exception>("User cannot change the near plane value because it is locked on a rigid body!");
	}
}
