#include "UniformSampler.h"

#include "GeometryType.h"


namespace
{
	std::vector<Storm::Vector3> sampleUniformSphere(const float separationDistance, const float radius)
	{
		std::vector<Storm::Vector3> result;
		result.reserve(static_cast<std::size_t>(std::ceilf((static_cast<float>(4.0 * M_PI) * radius * radius) / (separationDistance * separationDistance) + 0.000001f)));

		result.emplace_back(0.f, -radius, 0.f);

		if (radius > 0.f)
		{
			float deltaRad = std::asin(separationDistance / radius);

			for (float tetha = static_cast<float>(-M_PI / 2) + deltaRad; tetha < static_cast<float>(M_PI / 2); tetha += deltaRad)
			{
				const float currentY = std::cos(tetha) * radius;

				const float littleCircleRadius = std::sinf(tetha) * radius;
				const float anglePhi = std::asin(separationDistance / littleCircleRadius);
				for (float phi = static_cast<float>(-M_PI); phi < static_cast<float>(M_PI); phi += anglePhi)
				{
					result.emplace_back(std::cos(phi) * littleCircleRadius, currentY, std::sin(phi) * littleCircleRadius);
				}
			}

			result.emplace_back(0.f, radius, 0.f);
		}

		return result;
	}

	std::vector<Storm::Vector3> sampleUniformCube(const float separationDistance, const Storm::Vector3 &dimensions)
	{
		std::vector<Storm::Vector3> result;
		result.reserve(static_cast<std::size_t>(std::ceilf(2.f * (dimensions.x() * dimensions.y() + dimensions.y() * dimensions.z() + dimensions.z() * dimensions.x()) / (separationDistance * separationDistance)) + 0.000001f));

		const float midSepDist = separationDistance / 2.f;

		const Storm::Vector3 begin = dimensions / -2.f;
		const Storm::Vector3 end = dimensions / 2.f;


		float currentX = begin.x() + separationDistance;
		float endX = dimensions.x() / 2.f;

		// 1st face
		for (float y = begin.y() + midSepDist; y <= end.y(); y += separationDistance)
		{
			for (float z = begin.z() + midSepDist; z <= end.z(); z += separationDistance)
			{
				result.emplace_back(currentX, y, z);
			}
		}
		currentX += separationDistance;

		const float sepDistanceSquared = separationDistance * separationDistance;
		// Scan plane
		for (; currentX < endX; currentX += separationDistance)
		{
			std::size_t firstPScanlineIdx = result.size();

			for (float y = begin.y(); y < end.y(); y += separationDistance)
			{
				result.emplace_back(currentX, y, begin.z());
			}

			float last = end.y() - result.back().y();
			float offset = std::sqrtf(sepDistanceSquared - last * last);

			for (float z = begin.z() /*+ offset*/; z < end.z(); z += separationDistance)
			{
				result.emplace_back(currentX, end.y(), z);
			}

			last = end.z() - result.back().z();
			offset = std::sqrtf(sepDistanceSquared - last * last);

			for (float y = end.y() /*- offset*/; y > begin.y(); y -= separationDistance)
			{
				result.emplace_back(currentX, y, end.z());
			}

			last = begin.y() - result.back().y();
			offset = std::sqrtf(sepDistanceSquared - last * last);
			const Storm::Vector3 &firstP = result[firstPScanlineIdx];

			for (float z = end.z() /*- offset*/; z > firstP.z(); z -= separationDistance)
			{
				result.emplace_back(currentX, begin.y(), z);
			}
		}

		for (float y = begin.y() + midSepDist; y <= end.y(); y += separationDistance)
		{
			for (float z = begin.z() + midSepDist; z <= end.z(); z += separationDistance)
			{
				result.emplace_back(currentX, y, z);
			}
		}

		return result;
	}
}



std::vector<Storm::Vector3> Storm::UniformSampler::process(const Storm::GeometryType geometry, const float separationDistance, const void*const dimensions)
{
	if (separationDistance <= 0.f)
	{
		Storm::throwException<Storm::Exception>("Spearation distance should be greater than 0!");
	}

	switch (geometry)
	{
	case Storm::GeometryType::Sphere:
		return sampleUniformSphere(separationDistance , *reinterpret_cast<const float*const>(dimensions));

	case Storm::GeometryType::Cube:
		return sampleUniformCube(separationDistance, *reinterpret_cast<const Storm::Vector3*const>(dimensions));

	case Storm::GeometryType::None:
	default:
		Storm::throwException<Storm::Exception>("Uniform Sampler only work on simple geometries! Custom or unknown geometry aren't handled!");
	}
}
