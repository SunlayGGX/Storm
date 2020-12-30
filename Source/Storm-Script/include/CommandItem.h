#pragma once


namespace Storm
{
	struct CommandLogicSupport;
	enum class CommandKeyword;

	class CommandItem
	{
	protected:
		CommandItem(const Storm::CommandKeyword keyword);

	public:
		virtual ~CommandItem();

	public:
		virtual void handleLogic(Storm::CommandLogicSupport &inOutParam) = 0;
		Storm::CommandKeyword getKeyword() const noexcept;

		void baseDirectHandlePostLogic(Storm::CommandLogicSupport &inOutParam) const;

	private:
		const Storm::CommandKeyword _keyword;
	};
}
