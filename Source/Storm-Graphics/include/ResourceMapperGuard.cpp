#include "ResourceMapperGuard.h"


Storm::ResourceMapperGuard::ResourceMapperGuard(const ComPtr<ID3D11DeviceContext> &immediateContext, ID3D11Resource* resource, UINT subresource, D3D11_MAP mapType, UINT mapFlags, D3D11_MAPPED_SUBRESOURCE &mappedResource) :
	_immediateContext{ immediateContext },
	_resource{ resource },
	_subresource{ subresource }
{
	Storm::throwIfFailed(_immediateContext->Map(resource, subresource, mapType, mapFlags, &mappedResource));
}

Storm::ResourceMapperGuard::~ResourceMapperGuard()
{
	_immediateContext->Unmap(_resource, _subresource);
}
