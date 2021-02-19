#pragma once


namespace Storm
{
	class UIFieldContainer;

	class GeneralReadOnlyUIDisplay
	{
	public:
		GeneralReadOnlyUIDisplay();
		~GeneralReadOnlyUIDisplay();

	private:
		std::unique_ptr<Storm::UIFieldContainer> _readOnlyFields;
	};
}
