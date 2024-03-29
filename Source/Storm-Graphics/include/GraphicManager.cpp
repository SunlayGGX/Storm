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
#include "GraphicGravity.h"
#include "ParticleForceRenderer.h"
#include "GraphicKernelEffectArea.h"
#include "GraphicNormals.h"
#include "GraphicSmokes.h"

#include "RenderedElementProxy.h"

#include "PushedParticleSystemData.h"
#include "PushedParticleEmitterData.h"
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
#include "ThreadFlaggerObject.h"
#include "ThreadingSafety.h"

#include "SpecialKey.h"

#include "GraphicCutMode.h"

#include "UIFieldBase.h"
#include "UIField.h"
#include "UIFieldContainer.h"

#include "FuncMovePass.h"


namespace
{
#if false
	const float g_defaultColor[4] = { 0.f, 0.5f, 0.0f, 1.f };
#else
	const float g_defaultColor[4] = { 0.f, 0.f, 0.f, 1.f };
#endif
}

#define STORM_WATCHED_RB_POSITION "Rb position"

Storm::GraphicManager::GraphicManager() :
	_renderCounter{ 0 },
	_directXController{ std::make_unique<Storm::DirectXController>() },
	_selectedParticle{ std::numeric_limits<decltype(_selectedParticle.first)>::max(), 0 },
	_hasUI{ false },
	_dirty{ true },
	_watchedRbNonOwningPtr{ nullptr },
	_displayNormals{ false },
	_userMovedCameraThisFrame{ true },
	_rbNoNearPlaneCut{ false }
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
	_fields = std::make_unique<Storm::UIFieldContainer>();

	_directXController->initialize(static_cast<HWND>(hwnd));

	_camera = std::make_unique<Storm::Camera>(_directXController->getViewportWidth(), _directXController->getViewportHeight());

	const auto &device = _directXController->getDirectXDevice();
	const Storm::SceneGraphicConfig &sceneGraphicConfig = configMgr.getSceneGraphicConfig();

	_rbNoNearPlaneCut = sceneGraphicConfig._displayRbInFull;

	_gridNonOwningPtr = static_cast<Storm::Grid*>(_renderedElements.emplace_back(std::make_unique<Storm::Grid>(device, sceneGraphicConfig._grid, sceneGraphicConfig._showGridFloor)).get());
	_coordSystemNonOwningPtr = static_cast<Storm::GraphicCoordinateSystem*>(_renderedElements.emplace_back(std::make_unique<Storm::GraphicCoordinateSystem>(device, sceneGraphicConfig._showCoordinateAxis)).get());
	_gravityNonOwningPtr = static_cast<Storm::GraphicGravity*>(_renderedElements.emplace_back(std::make_unique<Storm::GraphicGravity>(device, _directXController->getUIRenderTarget())).get());

	_graphicParticlesSystem = std::make_unique<Storm::GraphicParticleSystem>(device);

	_graphicConstraintsSystem = std::make_unique<Storm::GraphicConstraintSystem>(device);

	_forceRenderer = std::make_unique<Storm::ParticleForceRenderer>(device);

	_kernelEffectArea = std::make_unique<Storm::GraphicKernelEffectArea>(device);

	_graphicNormals = std::make_unique<Storm::GraphicNormals>(device);

	if (const auto &smokeEmitterCfgs = configMgr.getSceneSmokeEmittersConfig();
		!smokeEmitterCfgs.empty())
	{
		_graphicSmokes = std::make_unique<Storm::GraphicSmokes>(device, smokeEmitterCfgs);
	}

	for (auto &meshesPair : _meshesMap)
	{
		meshesPair.second->initializeRendering(device);
	}

	if (sceneGraphicConfig._rbWatchId != std::numeric_limits<decltype(sceneGraphicConfig._rbWatchId)>::max())
	{
		_watchedRbNonOwningPtr = _meshesMap.find(static_cast<unsigned int>(sceneGraphicConfig._rbWatchId))->second.get();
		_fields->bindField(STORM_WATCHED_RB_POSITION, _watchedRbNonOwningPtr->getRbPosition());
	}

	_shouldTrackRbTranslation = sceneGraphicConfig._trackTranslation;

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
			inputMgr.bindKey(Storm::SpecialKey::KC_UP, [this]() { _camera->positiveMoveYAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_LEFT, [this]() { _camera->positiveMoveXAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_NUMPAD8, [this]() { _camera->positiveMoveZAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_RIGHT, [this]() { _camera->negativeMoveXAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_DOWN, [this]() { _camera->negativeMoveYAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_NUMPAD2, [this]() { _camera->negativeMoveZAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_S, [this]() { _camera->positiveRotateXAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_D, [this]() { _camera->positiveRotateYAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_W, [this]() { _camera->negativeRotateXAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_A, [this]() { _camera->negativeRotateYAxis(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_NUMPAD0, [this]() { _camera->reset(); _dirty = true; _userMovedCameraThisFrame = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_ADD, [this]() { this->checkUserCanChangeNearPlane(); _camera->increaseNearPlane(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_SUBTRACT, [this]() { this->checkUserCanChangeNearPlane(); _camera->decreaseNearPlane(); _dirty = true;});
			inputMgr.bindKey(Storm::SpecialKey::KC_MULTIPLY, [this]() { _camera->increaseFarPlane(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_DIVIDE, [this]() { _camera->decreaseFarPlane(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F2, [this]() { _coordSystemNonOwningPtr->switchShow(); _dirty = true; });
			inputMgr.bindKey(Storm::SpecialKey::KC_F2, [this]() { _gravityNonOwningPtr->switchShow(); _dirty = true; });
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
	_graphicSmokes.reset();
	_meshesMap.clear();
	_renderedElements.clear();
	_fieldsMap.clear();

	_directXController->cleanUp();
	_directXController.reset();
}

void Storm::GraphicManager::cleanUp_Implementation(const Storm::NoUI &)
{
	LOG_COMMENT << "No UI requested. Graphic Manager clean up is trivial.";
}

void Storm::GraphicManager::update()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().processCurrentThreadActions();

	if (_renderCounter++ % 2 == 0)
	{
		Storm::Camera &currentCamera = *_camera;
		currentCamera.update();

		if (_dirty)
		{
			const bool isSimulationPaused = singletonHolder.getSingleton<Storm::ITimeManager>().simulationIsPaused();
			if (_watchedRbNonOwningPtr != nullptr && (_watchedRbNonOwningPtr->positionDirty() || _userMovedCameraThisFrame))
			{
				bool shouldTrackTranslation = _shouldTrackRbTranslation;
				if (shouldTrackTranslation)
				{
					if (isSimulationPaused)
					{
						shouldTrackTranslation = false;
					}
					else if(_userMovedCameraThisFrame)
					{
						shouldTrackTranslation = true;
						_userMovedCameraThisFrame = false;
					}
				}

				currentCamera.updateWatchedRb(_watchedRbNonOwningPtr->getRbPosition(), shouldTrackTranslation);
				_fields->pushField(STORM_WATCHED_RB_POSITION);
			}

			_directXController->clearView(g_defaultColor);
			_directXController->initView();

			_directXController->renderElements(currentCamera, Storm::RenderedElementProxy{
				._renderedElementArrays = _renderedElements,
				._rbElementArrays = _meshesMap,
				._particleSystem = *_graphicParticlesSystem,
				._blowersMap = _blowersMap,
				._constraintSystem = *_graphicConstraintsSystem,
				._selectedParticleForce = *_forceRenderer,
				._kernelEffectArea = *_kernelEffectArea,
				._graphicNormals = _displayNormals ? _graphicNormals.get() : nullptr,
				._graphicSmokesOptional = _graphicSmokes.get(),
				._multiPass = _rbNoNearPlaneCut
			});

			_directXController->drawUI(_renderedElements, _fieldsMap);

			_directXController->unbindTargetView();
			_directXController->presentToDisplay();

			_directXController->reportDeviceMessages();

			_dirty = false;
			if (!isSimulationPaused)
			{
				_userMovedCameraThisFrame = false;
			}
		}
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

void Storm::GraphicManager::registerRigidbodiesParticleSystem(unsigned int rbId, const std::size_t pCount)
{
	if (this->isActive())
	{
		assert(Storm::isSimulationThread() && "This method should be called from simulation thread!");
		_pipe->registerRb(rbId, pCount);

		LOG_DEBUG << "Rb particle system " << rbId << " registered to graphic pipe with " << pCount << " particles.";
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
			[this, particleSystemId = param._particleSystemId, particlePosDataCopy = FuncMovePass<decltype(_pipe->fastOptimizedTransCopy(param))>{ _pipe->fastOptimizedTransCopy(param) }, isFluids = param._isFluids, isWall = param._isWall, pos = param._position]() mutable
		{
			if (_forceRenderer->prepareData(particleSystemId, particlePosDataCopy._object, _selectedParticle))
			{
				if (this->hasSelectedParticle() && particleSystemId == _selectedParticle.first)
				{
					_kernelEffectArea->setAreaPosition(particlePosDataCopy._object[_selectedParticle.second]);
				}

				_graphicParticlesSystem->refreshParticleSystemData(_directXController->getDirectXDevice(), particleSystemId, std::move(particlePosDataCopy._object), isFluids, isWall);

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
			_forceRenderer->updateForceData(_directXController->getDirectXDevice(), _parameters, selectedParticlePos, selectedParticleForce);
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::pushNormalsData(const std::vector<Storm::Vector3>& positions, const std::vector<Storm::Vector3>& normals)
{
	if (this->isActive())
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(ThreadEnumeration::GraphicsThread,
			[this, positions, normals]()
		{
			_graphicNormals->updateNormalsData(_directXController->getDirectXDevice(), _parameters, positions, normals);
			_displayNormals = true;
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::pushSmokeEmittedData(std::vector<Storm::PushedParticleEmitterData> &&param)
{
	if (this->isActive())
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(ThreadEnumeration::GraphicsThread,
			[this, dataParam = Storm::FuncMovePass{ std::move(param) }]()
		{
			if (_graphicSmokes)
			{
				_graphicSmokes->updateData(_directXController->getDirectXDevice(), dataParam._object);
				_dirty = true;
			}
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

void Storm::GraphicManager::removeGraphicsField(const std::wstring_view &fieldName)
{
	if (this->isActive())
	{
		if (const auto found = _fieldsMap.find(fieldName); found != std::end(_fieldsMap))
		{
			_fieldsMap.erase(found);

			_directXController->notifyFieldCount(_fieldsMap.size());

			_dirty = true;
		}
		else
		{
			assert(false && "We shouldn't remove a field that doesn't exist!");
		}
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

void Storm::GraphicManager::clearNormalsData()
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(ThreadEnumeration::GraphicsThread, [this]()
		{
			_displayNormals = false;
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
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, shouldShow]()
		{
			_coordSystemNonOwningPtr->show(shouldShow);
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::showGridVisibility(const bool shouldShow)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, shouldShow]()
		{
			if (_gridNonOwningPtr->setVisibility(shouldShow))
			{
				_dirty = true;
			}
		});
	}
}

void Storm::GraphicManager::setKernelAreaRadius(const float radius)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, radius]()
		{
			_kernelEffectArea->setAreaRadius(radius);
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::lockNearPlaneOnWatchedRb(unsigned int watchedRbId)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, watchedRbId]()
		{
			if (const auto found = _meshesMap.find(static_cast<unsigned int>(watchedRbId)); found != std::end(_meshesMap))
			{
				const auto* old = _watchedRbNonOwningPtr;
				_watchedRbNonOwningPtr = found->second.get();
				if (old != nullptr)
				{
					_fields->deleteField(STORM_WATCHED_RB_POSITION);
				}

				_fields->bindField(STORM_WATCHED_RB_POSITION, _watchedRbNonOwningPtr->getRbPosition());
				_dirty = true;
			}
			else
			{
				Storm::throwException<Storm::Exception>("Cannot find the rigid body to lock the near plane on. Requested id was " + std::to_string(watchedRbId));
			}
		});
	}
}

void Storm::GraphicManager::unlockNearPlaneOnWatchedRb()
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this]()
		{
			if (_watchedRbNonOwningPtr)
			{
				_watchedRbNonOwningPtr = nullptr;
				_fields->deleteField(STORM_WATCHED_RB_POSITION);
			}
		});
	}
}

void Storm::GraphicManager::stopTrackingTranslationOnWatchedRb()
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this]()
		{
			_shouldTrackRbTranslation = false;
		});
	}
}

void Storm::GraphicManager::checkUserCanChangeNearPlane() const
{
	if (_watchedRbNonOwningPtr != nullptr)
	{
		Storm::throwException<Storm::Exception>("User cannot change the near plane value because it is locked on a rigid body!");
	}
}

void Storm::GraphicManager::setVectMultiplicatorCoeff(const float newCoeff)
{
	if (this->isActive())
	{
		if (newCoeff <= 0.f)
		{
			Storm::throwException<Storm::Exception>("Vectors multiplication coefficient must be strictly greater than 0! Received value was " + std::to_string(newCoeff) + ".");
		}

		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, newCoeff]()
		{
			if (_parameters._vectNormMultiplicator != newCoeff)
			{
				const float oldCoeff = _parameters._vectNormMultiplicator;
				_parameters._vectNormMultiplicator = newCoeff;

				const auto &device = _directXController->getDirectXDevice();
				_forceRenderer->refreshForceData(device, _parameters);
				_graphicNormals->refreshNormalsData(device, _parameters, oldCoeff);

				_dirty = true;
			}
		});
	}
}

void Storm::GraphicManager::makeCutAroundWatchedRb(const Storm::GraphicCutMode cutMode)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, cutMode]()
		{
			if (_watchedRbNonOwningPtr)
			{
				this->makeCutAroundRigidbody(_watchedRbNonOwningPtr->getID(), cutMode);
			}
			else
			{
				LOG_ERROR << "No watched rb. Skipping cut.";
			}
		});
	}
}

void Storm::GraphicManager::makeCutAroundRigidbody(const unsigned int rbId, const Storm::GraphicCutMode cutMode)
{
	if (this->isActive())
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

		switch (cutMode)
		{
		case Storm::GraphicCutMode::Kernel:
			threadMgr.executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, rbId]()
			{
				if (_watchedRbNonOwningPtr)
				{
					if (_watchedRbNonOwningPtr->getID() != rbId)
					{
						Storm::throwException<Storm::Exception>("We have a watched rb locked but we requested to set the cut on another rb, this is forbidden!");
					}

					_camera->makeCut(_watchedRbNonOwningPtr->getRbPosition(), _kernelEffectArea->getAreaRadius());
				}
				else if (const auto meshesFound = _meshesMap.find(rbId); meshesFound != std::end(_meshesMap))
				{
					_camera->makeCut(meshesFound->second->getRbPosition(), _kernelEffectArea->getAreaRadius());
				}
				else
				{
					LOG_ERROR << "Cannot find the graphic mesh specified by id " << rbId;
				}
			});
			break;

		case Storm::GraphicCutMode::Particle:
		{
			const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
			threadMgr.executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, rbId, particleRadius = configMgr.getSceneSimulationConfig()._particleRadius]()
			{
				if (_watchedRbNonOwningPtr)
				{
					if (_watchedRbNonOwningPtr->getID() != rbId)
					{
						Storm::throwException<Storm::Exception>("We have a watched rb locked but we requested to set the cut on another rb, this is forbidden!");
					}

					_camera->makeCut(_watchedRbNonOwningPtr->getRbPosition(), particleRadius);
				}
				else if (const auto meshesFound = _meshesMap.find(rbId); meshesFound != std::end(_meshesMap))
				{
					_camera->makeCut(meshesFound->second->getRbPosition(), particleRadius);
				}
				else
				{
					LOG_ERROR << "Cannot find the graphic mesh specified by id " << rbId;
				}
			});
			break;
		}

		default:
			Storm::throwException<Storm::Exception>("Unknown cut mode (" + Storm::toStdString(cutMode) + ")");
		}
	}
}

void Storm::GraphicManager::makeCutAroundSelectedParticle(const Storm::GraphicCutMode cutMode)
{
	if (this->isActive())
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

		switch (cutMode)
		{
		case Storm::GraphicCutMode::Kernel:
			threadMgr.executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this]()
			{
				if (_watchedRbNonOwningPtr)
				{
					Storm::throwException<Storm::Exception>("We have a watched rb locked but we requested to make a cut, this is forbidden (except a cut on the watched rb center)!");
				}

				if (this->hasSelectedParticle())
				{
					_camera->makeCut(_kernelEffectArea->getAreaPosition(), _kernelEffectArea->getAreaRadius());
				}
				else
				{
					LOG_ERROR << "No selected particle. Skipping cut.";
				}
			});
			break;

		case Storm::GraphicCutMode::Particle:
			threadMgr.executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, &singletonHolder]()
			{
				if (_watchedRbNonOwningPtr)
				{
					Storm::throwException<Storm::Exception>("We have a watched rb locked but we requested to make a cut, this is forbidden (except a cut on the watched rb center)!");
				}

				if (this->hasSelectedParticle())
				{
					const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
					_camera->makeCut(_kernelEffectArea->getAreaPosition(), configMgr.getSceneSimulationConfig()._particleRadius);
				}
				else
				{
					LOG_ERROR << "No selected particle. Skipping cut.";
				}
			});
			break;
		}
	}
}

void Storm::GraphicManager::displayDynamicRbInFull(bool enable)
{
	if (this->isActive())
	{
		Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, enable]()
		{
			_rbNoNearPlaneCut = enable;
			_dirty = true;
		});
	}
}

void Storm::GraphicManager::getViewportTextureResolution(float &outWidth, float &outHeight) const
{
	outWidth = _directXController->getViewportWidth();
	outHeight = _directXController->getViewportHeight();
}
