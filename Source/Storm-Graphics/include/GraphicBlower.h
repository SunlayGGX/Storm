#pragma once

#include "IRenderedElement.h"
#include "GraphicAreaBaseHolder.h"


namespace Storm
{
	enum class BlowerState;
	struct SceneBlowerConfig;

	class GraphicBlower :
		public Storm::IRenderedElement,
		private Storm::GraphicAreaBaseHolder
	{
	public:
		GraphicBlower(const ComPtr<ID3D11Device> &device, const Storm::SceneBlowerConfig &blowerConfig, const std::vector<Storm::Vector3> &vertexes, const std::vector<unsigned int> &indexes);

	public:
		void render(const ComPtr<ID3D11Device> &device, const ComPtr<ID3D11DeviceContext> &deviceContext, const Storm::Camera &currentCamera) final override;

	public:
		void setBlowerState(const Storm::BlowerState newState) noexcept;

	public:
		std::size_t getId() const noexcept;

	private:
		std::size_t _id;

		Storm::BlowerState _blowerState;
	};
}
