#pragma once


#include <Eigen/Core>
#include <Eigen/Geometry>

namespace Storm
{
	using Vector3ui = Eigen::Matrix<unsigned int, 3, 1>;
	using Vector2 = Eigen::Vector2f;
	using Vector3 = Eigen::Vector3f;
	using Vector3d = Eigen::Vector3d;
	using Vector4 = Eigen::Vector4f;
	using Quaternion = Eigen::Quaternionf;
	using Rotation = Eigen::AngleAxisf;
}


extern bool operator==(const Storm::Rotation &left, const Storm::Rotation &right);
extern bool operator!=(const Storm::Rotation &left, const Storm::Rotation &right);
