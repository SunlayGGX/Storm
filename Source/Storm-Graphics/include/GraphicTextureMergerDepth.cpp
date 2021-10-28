#include "GraphicTextureMergerDepth.h"

#include "TextureMergerDepthShader.h"


Storm::GraphicTextureMergerDepth::GraphicTextureMergerDepth(const ComPtr<ID3D11Device> &device) :
	_shader{ std::make_unique<Storm::TextureMergerDepthShader>(device) }
{
	
}

Storm::GraphicTextureMergerDepth::~GraphicTextureMergerDepth() = default;
