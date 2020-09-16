#pragma once


namespace Storm
{
	// Those methods are to help debugging and should be removed in final code.

	// Will wait (freeze execution) until a debugger attached to the executable.
	void waitForDebuggerToAttach(bool breakAfter = false);

	// This will make it that the assert macro (cassert) will pop the windows with the full information instead of popping the useless windows with no info.
	void setupFullAssertionBox();
}
