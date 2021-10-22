#pragma once


namespace Storm
{
	class RecordArchiver
	{
	public:
		RecordArchiver();

	public:
		void preArchive();
		void execute();

	public:
		unsigned int _version;
		std::string _archivePath;
	};
}
