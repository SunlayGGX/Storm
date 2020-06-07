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

		IncreaseCameraX,
		IncreaseCameraY,
		IncreaseCameraZ,
		DecreaseCameraX,
		DecreaseCameraY,
		DecreaseCameraZ,

		RotatePosCameraX,
		RotatePosCameraY,
		RotateNegCameraX,
		RotateNegCameraY,
		// No roll !

		NearPlaneMoveUp,
		NearPlaneMoveBack,
		FarPlaneMoveUp,
		FarPlaneMoveBack,

		IncreaseCameraSpeed,
		DecreaseCameraSpeed,

		ResetCamera,
	};
}
