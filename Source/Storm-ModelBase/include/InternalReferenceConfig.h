#pragma once


namespace Storm
{
	enum class PaperType
	{
		Unknown,
		Article,
		Misc
	};

	struct InternalReferenceConfig
	{
	public:
		InternalReferenceConfig();

	public:
		Storm::PaperType _type;
		std::size_t _id;
		std::string _authors;
		std::string _serialNumber;
		std::string _name;
		std::string _date;
		std::string _url;
		std::string _bibTexLink;
	};
}
