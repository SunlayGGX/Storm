#pragma once


namespace Storm
{
	enum class RecordMode;

	struct RecordConfigData
	{
	public:
		RecordConfigData();

	public:
		Storm::RecordMode _recordMode;
		float _recordFps;
		std::string _recordFilePath;
		bool _replayRealTime;
	};
}
