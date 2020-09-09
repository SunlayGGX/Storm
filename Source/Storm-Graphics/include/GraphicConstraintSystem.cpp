#include "GraphicConstraintSystem.h"

#include "ConstraintShader.h"



Storm::GraphicConstraintSystem::GraphicConstraintSystem(const ComPtr<ID3D11Device> &device) :
	_shader{ std::make_unique<Storm::ConstraintShader>(device) },
	_constraintVertexCount{ 0 }
{

}

Storm::GraphicConstraintSystem::~GraphicConstraintSystem() = default;

