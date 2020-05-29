#include "GraphicManager.h"

#include "DirectXController.h"
#include "Camera.h"

#include "SingletonHolder.h"
#include "IWindowsManager.h"
#include "ITimeManager.h"

#include "ThreadHelper.h"

namespace
{
	const float g_defaultColor[4] = { 0.f, 1.f, 0.2f, 1.f };
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

	Storm::IWindowsManager* windowsMgr = Storm::SingletonHolder::instance().getFacet<Storm::IWindowsManager>();

	HWND hwnd = static_cast<HWND>(windowsMgr->getWindowHandle());
	if (hwnd != nullptr)
	{
		this->initialize_Implementation(hwnd);
		return true;
	}
	else
	{
		bool initRes = false;
		windowsMgr->bindFinishInitializeCallback([this, res = &initRes](void* hwndOnceReady, bool calledAtBindingTime)
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
	_renderThread = std::thread([this]()
	{
		Storm::ITimeManager* timeMgr = Storm::SingletonHolder::instance().getFacet<Storm::ITimeManager>();
		while (timeMgr->waitNextFrameOrExit())
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
		_directXController->clearRenderTarget(g_defaultColor);

		// TODO

		_directXController->drawRenderTarget();
	}
}

const Storm::Camera& Storm::GraphicManager::getCamera() const
{
	return *_camera;
}
