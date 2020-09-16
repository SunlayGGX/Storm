#pragma once


namespace Storm
{
	bool isSimulationThread();
	bool isLoadingThread();
	bool isTimeThread();
	bool isGraphicThread();
	bool isSpaceThread();
	bool isInputThread();
	bool isLoggerThread();
}
