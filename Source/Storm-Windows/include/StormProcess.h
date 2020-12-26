#pragma once



namespace Storm
{
	namespace details
	{
		class IStormProcessImpl;
	}

	struct StormProcessStartup;

	class StormProcess
	{
	public:
		StormProcess(Storm::StormProcessStartup &&startupParam);
		StormProcess(Storm::StormProcess &&other);
		~StormProcess();

	public:
		void prepareDestroy();
		void release();
		void close();
		int waitForCompletion();

		int32_t getExitCode(bool &outHasExited, bool &outFailure) const;

	private:
		std::unique_ptr<Storm::details::IStormProcessImpl> _processImpl;
	};
}
