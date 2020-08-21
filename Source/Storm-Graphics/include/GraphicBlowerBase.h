#pragma once

#include "IRenderedElement.h"


namespace Storm
{
	struct BlowerData;
	class GraphicBlowerBase : public Storm::IRenderedElement
	{
	protected:
		GraphicBlowerBase(const std::size_t index, const Storm::BlowerData &blowerData);
		virtual ~GraphicBlowerBase();

	public:
		std::size_t getIndex() const;

	protected:
		std::size_t _index;
	};
}
