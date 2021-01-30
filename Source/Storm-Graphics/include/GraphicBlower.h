#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	enum class BlowerType;
	enum class BlowerState;
	struct SceneBlowerConfig;
	class AreaShader;

	class GraphicBlower : public Storm::IRenderedElement
	{
	public:
		GraphicBlower(const ComPtr<ID3D11Device> &device, const Storm::SceneBlowerConfig &blowerConfig, const std::vector<Storm::Vector3> &vertexes, const std::vector<unsigned int> &indexes);
		~GraphicBlower();

	protected:
		void instantiateShader(const ComPtr<ID3D11Device> &device, const std::vector<Storm::Vector3> &vertexes, const std::vector<uint32_t> &indexes);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	private:
		void setupBlower(const ComPtr<ID3D11DeviceContext> &deviceContext);

	public:
		void setBlowerState(const Storm::BlowerState newState);

	public:
		std::size_t getId() const;
		Storm::BlowerType getType() const;

	protected:
		std::size_t _id;
		Storm::BlowerType _type;

		uint32_t _indexCount;
		Storm::BlowerState _blowerState;

		ComPtr<ID3D11Buffer> _vertexBuffer;
		ComPtr<ID3D11Buffer> _indexBuffer;

		std::unique_ptr<Storm::AreaShader> _blowerShader;
	};
}
