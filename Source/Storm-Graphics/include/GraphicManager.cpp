#include "GraphicManager.h"

#include "DirectXController.h"
#include "GraphicsAction.h"
#include "Camera.h"

#include "Grid.h"
#include "GraphicRigidBody.h"
#include "GraphicData.h"

#include "SingletonHolder.h"
#include "IWindowsManager.h"
#include "ITimeManager.h"
#include "IInputManager.h"
#include "IConfigManager.h"

#include "ThreadHelper.h"

#include "SpecialKey.h"

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

	for (auto &meshesPair : _meshesMap)
	{
		meshesPair.second->initializeRendering(device);
	}

	Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();
	inputMgr.bindKey(Storm::SpecialKey::KC_UP, [this]() { this->executeActionAsync(GraphicsAction::IncreaseCameraY); });
	inputMgr.bindKey(Storm::SpecialKey::KC_DOWN, [this]() { this->executeActionAsync(GraphicsAction::DecreaseCameraY); });
	inputMgr.bindKey(Storm::SpecialKey::KC_LEFT, [this]() { this->executeActionAsync(GraphicsAction::IncreaseCameraX); });
	inputMgr.bindKey(Storm::SpecialKey::KC_RIGHT, [this]() { this->executeActionAsync(GraphicsAction::DecreaseCameraX); });
	inputMgr.bindKey(Storm::SpecialKey::KC_8, [this]() { this->executeActionAsync(GraphicsAction::IncreaseCameraZ); });
	inputMgr.bindKey(Storm::SpecialKey::KC_2, [this]() { this->executeActionAsync(GraphicsAction::DecreaseCameraZ); });
	inputMgr.bindKey(Storm::SpecialKey::KC_S, [this]() { this->executeActionAsync(GraphicsAction::RotatePosCameraX); });
	inputMgr.bindKey(Storm::SpecialKey::KC_W, [this]() { this->executeActionAsync(GraphicsAction::RotateNegCameraX); });
	inputMgr.bindKey(Storm::SpecialKey::KC_D, [this]() { this->executeActionAsync(GraphicsAction::RotatePosCameraY); });
	inputMgr.bindKey(Storm::SpecialKey::KC_A, [this]() { this->executeActionAsync(GraphicsAction::RotateNegCameraY); });

	_renderThread = std::thread([this]()
	{
		Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();
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
		this->internalExecuteActions();

		_directXController->clearView(g_defaultColor);
		_directXController->initView();

		_directXController->renderElements(this->getCamera(), _renderedElements, _meshesMap);

		_directXController->unbindTargetView();
		_directXController->presentToDisplay();
	}
}

void Storm::GraphicManager::executeActionAsync(Storm::GraphicsAction actionToExecute)
{
	std::lock_guard<std::mutex> lock{ _actionMutex };
	_actionsToBeExecuted.emplace_back(actionToExecute);
}

void Storm::GraphicManager::internalExecuteActions()
{
	std::vector<Storm::GraphicsAction> backBuffer;
	{
		std::lock_guard<std::mutex> lock{ _actionMutex };
		if (_actionsToBeExecuted.empty())
		{
			return;
		}

		std::swap(backBuffer, _actionsToBeExecuted);
	}

	for (Storm::GraphicsAction actionToExecute : backBuffer)
	{
		this->internalExecuteActionElement(actionToExecute);
	}
}

void Storm::GraphicManager::internalExecuteActionElement(Storm::GraphicsAction action)
{
	switch (action)
	{
	case Storm::GraphicsAction::IncreaseCameraX: _camera->positiveMoveXAxis(); break;
	case Storm::GraphicsAction::IncreaseCameraY: _camera->positiveMoveYAxis(); break;
	case Storm::GraphicsAction::IncreaseCameraZ: _camera->positiveMoveZAxis(); break;
	case Storm::GraphicsAction::DecreaseCameraX: _camera->negativeMoveXAxis(); break;
	case Storm::GraphicsAction::DecreaseCameraY: _camera->negativeMoveYAxis(); break;
	case Storm::GraphicsAction::DecreaseCameraZ: _camera->negativeMoveZAxis(); break;
	case Storm::GraphicsAction::RotatePosCameraX: _camera->positiveRotateXAxis(); break;
	case Storm::GraphicsAction::RotatePosCameraY: _camera->positiveRotateYAxis(); break;
	case Storm::GraphicsAction::RotateNegCameraX: _camera->negativeRotateXAxis(); break;
	case Storm::GraphicsAction::RotateNegCameraY: _camera->negativeRotateYAxis(); break;

	case Storm::GraphicsAction::NearPlaneMoveUp: _camera->increaseNearPlane(); break;
	case Storm::GraphicsAction::NearPlaneMoveBack: _camera->decreaseNearPlane(); break;
	case Storm::GraphicsAction::FarPlaneMoveUp: _camera->increaseFarPlane(); break;
	case Storm::GraphicsAction::FarPlaneMoveBack: _camera->decreaseFarPlane(); break;

	case Storm::GraphicsAction::IncreaseCameraSpeed: _camera->increaseCameraSpeed(); break;
	case Storm::GraphicsAction::DecreaseCameraSpeed: _camera->decreaseCameraSpeed(); break;
	case Storm::GraphicsAction::ResetCamera: _camera->reset(); break;


	// Those are controller actions.
	case Storm::GraphicsAction::ShowWireframe:
	case Storm::GraphicsAction::ShowSolidFrameWithCulling:
	case Storm::GraphicsAction::ShowSolidFrameNoCulling:
	case Storm::GraphicsAction::EnableZBuffer:
	case Storm::GraphicsAction::DisableZBuffer:
	case Storm::GraphicsAction::EnableBlendAlpha:
	case Storm::GraphicsAction::DisableBlendAlpha:
	default:
		_directXController->executeAction(action);
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

const Storm::Camera& Storm::GraphicManager::getCamera() const
{
	return *_camera;
}
