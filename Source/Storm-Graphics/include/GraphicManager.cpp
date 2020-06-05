#include "GraphicManager.h"

#include "DirectXController.h"
#include "Camera.h"

#include "Grid.h"
#include "GraphicRigidBody.h"

#include "SingletonHolder.h"
#include "IWindowsManager.h"
#include "ITimeManager.h"

#include "ThreadHelper.h"

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
	LOG_COMMENT << "HWND is valid so Windows was created, we can pursue the graphic initialization.";
	_directXController->initialize(static_cast<HWND>(hwnd));

	_camera = std::make_unique<Storm::Camera>(_directXController->getViewportWidth(), _directXController->getViewportHeight());

	const auto &device = _directXController->getDirectXDevice();
	_renderedElements.emplace_back(std::make_unique<Storm::Grid>(device, Storm::Vector3{ 15.f, 0.f, 15.f }));

	for (auto &meshesPair : _meshesMap)
	{
		meshesPair.second->initializeRendering(device);
	}

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
	_directXController->executeAction(action);
}

void Storm::GraphicManager::addMesh(unsigned int meshId, const std::vector<Storm::Vector3> &vertexes, const std::vector<Storm::Vector3> &normals, const std::vector<unsigned int> &indexes)
{
	_meshesMap[meshId] = std::make_unique<Storm::GraphicRigidBody>(vertexes, normals, indexes);
}

void Storm::GraphicManager::bindParentRbToMesh(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb)
{
	_meshesMap[meshId]->setRbParent(parentRb);
}

const Storm::Camera& Storm::GraphicManager::getCamera() const
{
	return *_camera;
}
