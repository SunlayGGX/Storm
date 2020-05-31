#pragma once


namespace Storm
{
	enum class GraphicsAction
	{
		ShowWireframe,
		ShowSolidFrameWithCulling,
		ShowSolidFrameNoCulling,

		EnableZBuffer,
		DisableZBuffer,

		EnableBlendAlpha,
		DisableBlendAlpha,
	};
}
