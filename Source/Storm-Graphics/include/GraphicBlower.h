#pragma once


namespace Storm
{
	enum class BlowerType;

	template<Storm::BlowerType type, class MeshGeneratorType>
	class GraphicBlower :
		public Storm::GraphicBlowerBase,
		private MeshGeneratorType
	{

	};
}
