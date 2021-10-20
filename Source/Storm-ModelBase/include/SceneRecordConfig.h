#pragma once


namespace Storm
{
	enum class RecordMode;

	struct SceneRecordConfig
	{
	public:
		SceneRecordConfig();

	public:
		Storm::RecordMode _recordMode;
		float _recordFps;
		std::string _recordFilePath;
		bool _replayRealTime;
		unsigned int _leanStartJump;
	};
}
