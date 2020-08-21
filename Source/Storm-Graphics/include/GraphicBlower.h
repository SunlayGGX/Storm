#pragma once


namespace Storm
{
	enum class BlowerType;

	template<Storm::BlowerType type, class MeshMaker>
	class GraphicBlower :
		public Storm::GraphicBlowerBase
	{

	};
}
