#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class BlowerState;
	struct BlowerData;
	class IRigidBody;
	struct PushedParticleSystemDataParameter;

	class IGraphicsManager : public Storm::ISingletonHeldInterface<IGraphicsManager>
	{
	public:
		virtual ~IGraphicsManager() = default;

	public:
		virtual void update() = 0;

	public:
		virtual void addMesh(unsigned int meshId, const std::vector<Storm::Vector3> &vertexes, const std::vector<Storm::Vector3> &normals, const std::vector<unsigned int> &indexes) = 0;
		virtual void bindParentRbToMesh(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const = 0;

	public:
		virtual void loadBlower(const Storm::BlowerData &blowerData, const std::vector<Storm::Vector3> &vertexes, const std::vector<unsigned int> &indexes) = 0;

	public:
		// Warning : the caller makes the copy (a copy of the data will be made). It cannot be avoided to prevent data races but this is the most efficient way I thought...
		// The caller (Simulation thread) will copy the data it has and push it by move to a staging location waiting for the graphic thread to retrieve it.
		// When the Graphic thread begins its loop, it will move the staging data into its own data (no copy) and continue with it. No lock while copying is made so no bottleneck.
		virtual void pushParticlesData(const Storm::PushedParticleSystemDataParameter &param) = 0;
		virtual void pushConstraintData(const std::vector<Storm::Vector3> &constraintsVisuData) = 0;
		virtual void pushParticleSelectionForceData(const Storm::Vector3 &selectedParticlePos, const Storm::Vector3 &selectedParticleForce) = 0;

	public:
		virtual void createGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValueStr) = 0;
		virtual void updateGraphicsField(std::vector<std::pair<std::wstring_view, std::wstring>> &&rawFields) = 0;
		virtual void updateGraphicsField(const std::wstring_view &fieldName, std::wstring &&fieldValue) = 0;

	public:
		// For picking algorithm. Transform a 2D position into a 3D ray vector. This method isn't thread safe, therefore should only be executed inside the Graphic thread.
		virtual void convertScreenPositionToRay(const Storm::Vector2 &screenPos, Storm::Vector3 &outRayOrigin, Storm::Vector3 &outRayDirection) const = 0;
		virtual void getClippingPlaneValues(float &outZNear, float &outZFar) const = 0;

		// This is a faster raycast using the Z-buffer.
		virtual Storm::Vector3 get3DPosOfScreenPixel(const Storm::Vector2 &screenPos) const = 0;

	public:
		virtual void safeSetSelectedParticle(unsigned int particleSystemId, std::size_t particleIndex) = 0;
		virtual void safeClearSelectedParticle() = 0;

	public:
		virtual void setTargetPositionTo(const Storm::Vector3 &newTargetPosition) = 0;

	public:
		virtual void changeBlowerState(const std::size_t blowerId, const Storm::BlowerState newState) = 0;

	public:
		virtual void cycleColoredSetting() = 0;
		virtual void setColorSettingMinMaxValue(float minValue, float maxValue) = 0;
	};
}
