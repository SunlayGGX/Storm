#include "DirectXHardwareInfo.h"



Storm::DirectXHardwareInfo::DirectXHardwareInfo(const DXGI_MODE_DESC &wantedDesc)
{
	ComPtr<IDXGIFactory> factory = nullptr;
	ComPtr<IDXGIAdapter> adapter = nullptr;
	ComPtr<IDXGIOutput> output = nullptr;

	Storm::throwIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), &factory));
	Storm::throwIfFailed(factory->EnumAdapters(0, &adapter));
	Storm::throwIfFailed(adapter->EnumOutputs(0, &output));

	{ 
		//current state info
		DXGI_OUTPUT_DESC outDesc;
		output->GetDesc(&outDesc);

		_width = outDesc.DesktopCoordinates.right - outDesc.DesktopCoordinates.left;
		_height = outDesc.DesktopCoordinates.bottom - outDesc.DesktopCoordinates.top;

		output->FindClosestMatchingMode(&wantedDesc, &_mode, nullptr);
	}

	DXGI_ADAPTER_DESC desc;
	adapter->GetDesc(&desc);

	_memory = static_cast<uint64_t>(desc.DedicatedVideoMemory);

	wcscpy_s(_cardName, DirectXHardwareInfo::CARD_NAME_MAX_LENGHT, desc.Description);
}

