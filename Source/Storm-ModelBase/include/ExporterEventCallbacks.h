#pragma once


namespace Storm
{
	struct SerializeRecordHeader;
	struct SerializeRecordPendingData;

	struct ExporterEventCallbacks
	{
	public:
		std::function<bool(const SerializeRecordHeader &)> _onStartRecordRead;
		std::function<bool(const SerializeRecordPendingData &)> _onNewFrameReceive;
		std::function<void()> _onRecordClose;
	};
}
