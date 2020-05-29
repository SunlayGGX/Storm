#pragma once


namespace Storm
{
    class DirectXHardwareInfo
	{
	public:
		enum
		{
			CARD_NAME_MAX_LENGHT = 128
		};

    public:
		DirectXHardwareInfo(const DXGI_MODE_DESC &wantedDesc);

	public:
		int _width;
		int _height;
		uint64_t _memory;
		wchar_t _cardName[DirectXHardwareInfo::CARD_NAME_MAX_LENGHT];
		DXGI_MODE_DESC _mode;
    };
}
