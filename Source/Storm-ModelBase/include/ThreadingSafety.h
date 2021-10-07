#pragma once


namespace Storm
{
	bool isMainThread();
	bool isSimulationThread();
	bool isLoadingThread();
	bool isGraphicThread();
	bool isRaycastThread();
	bool isInputThread();
	bool isLoggerThread();
	bool isSpaceThread();
	bool isSerializerThread();
	bool isWindowsThread();
	bool isTimeThread();
	bool isScriptThread();
	bool isPhysicsThread();
	bool isAnimationThread();
	bool isNetworkThread();
	bool isSafetyThread();
}
