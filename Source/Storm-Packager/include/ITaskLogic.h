#pragma once


namespace StormPackager
{
	class ITaskLogic
	{
	public:
		virtual ~ITaskLogic() = default;

	public:
		virtual std::string_view getName() const = 0;

		virtual std::string prepare() = 0;
		virtual std::string execute() = 0;
		virtual std::string cleanUp() = 0;
	};
}
