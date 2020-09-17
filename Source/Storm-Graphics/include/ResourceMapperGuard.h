#pragma once


namespace Storm
{
	class ResourceMapperGuard
	{
	public:
		ResourceMapperGuard(const ComPtr<ID3D11DeviceContext> &immediateContext, ID3D11Resource* resource, UINT subresource, D3D11_MAP mapType, UINT mapFlags, D3D11_MAPPED_SUBRESOURCE &mappedResource);
		~ResourceMapperGuard();

	private:
		const ComPtr<ID3D11DeviceContext> &_immediateContext;
		ID3D11Resource* _resource;
		UINT _subresource;
	};
}
