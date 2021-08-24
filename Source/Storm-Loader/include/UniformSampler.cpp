#include "UniformSampler.h"

#include "GeometryType.h"


namespace
{
	float computeAngleStep(const float separationDistance, const float circleRadius)
	{
		float res;
#if true
		// Cosine law (Al Kashi Theorem)
		//	   c²=a²+b²-2ab cos(tetha)
		// <=> tetha = acos((a²+b²-c²) / (2ab))
		// <=> tetha = acos((2r² - sep²) / (2r²)) since a=b=r and c=sep=separationDistance

		const float circleRadiusSquaredCoeff = 2.f * circleRadius * circleRadius;
		res = std::acosf((circleRadiusSquaredCoeff - (separationDistance * separationDistance)) / circleRadiusSquaredCoeff);
#else
		res = std::asin(separationDistance / radius);
#endif

		return std::fabs(res);
	}

	template<bool internalLayer>
	std::vector<Storm::Vector3> sampleUniformSphere(const float separationDistance, int layerCount, float radius)
	{
		std::vector<Storm::Vector3> result;
		result.reserve(static_cast<std::size_t>(std::ceilf((static_cast<float>(4.0 * M_PI) * radius * radius) / (separationDistance * separationDistance) + 0.000001f)) * layerCount);

		do
		{
			result.emplace_back(0.f, -radius, 0.f);

			if (radius > 0.f)
			{
				float deltaRad = computeAngleStep(separationDistance, radius);

				for (float tetha = static_cast<float>(0) + deltaRad; tetha < static_cast<float>(2.0 * M_PI); tetha += deltaRad)
				{
#if false
					const float currentY = std::cos(tetha) * radius;

					const float littleCircleRadius = std::sinf(tetha) * radius;
					const float anglePhi = computeAngleStep(separationDistance, littleCircleRadius);
					for (float phi = static_cast<float>(0); phi < static_cast<float>(M_PI); phi += anglePhi)
					{
						result.emplace_back(std::cos(phi) * littleCircleRadius, currentY, std::sin(phi) * littleCircleRadius);
					}
#else

					const float currentY = std::cos(tetha) * radius;

					const float littleCircleRadius = std::sinf(tetha) * radius;
					float anglePhi = computeAngleStep(separationDistance, littleCircleRadius);
					if (!std::isnan(anglePhi))
					{
						const std::size_t particleCountOnCircle = static_cast<std::size_t>(static_cast<float>(M_PI) / std::fabs(anglePhi));
						if (particleCountOnCircle != 0)
						{
							anglePhi = static_cast<float>((std::signbit(anglePhi) ? -M_PI : M_PI) / static_cast<double>(particleCountOnCircle));

							for (std::size_t iter = 0; iter < particleCountOnCircle; ++iter)
							{
								const float phi = static_cast<float>(iter) * anglePhi;
								result.emplace_back(std::cosf(phi) * littleCircleRadius, currentY, std::sinf(phi) * littleCircleRadius);
							}
						}
					}
#endif
				}

				result.emplace_back(0.f, radius, 0.f);
			}

			--layerCount;

			if constexpr (internalLayer)
			{
				radius -= separationDistance;
				if (radius <= 0.f)
				{
					if (layerCount > 0)
					{
						LOG_DEBUG_WARNING << "Too much layer to fit inside sphere geometry. Skipped " << layerCount << " layers";
					}
					break;
				}
			}
			else
			{
				radius += separationDistance;
			}

		} while (layerCount > 0);

		return result;
	}

	template<bool internalLayer>
	std::vector<Storm::Vector3> sampleUniformCube(const float separationDistance, int layerCount, Storm::Vector3 dimensions)
	{
		std::vector<Storm::Vector3> result;
		result.reserve(static_cast<std::size_t>(std::ceilf(2.f * (dimensions.x() * dimensions.y() + dimensions.y() * dimensions.z() + dimensions.z() * dimensions.x()) / (separationDistance * separationDistance)) + 0.000001f) * layerCount);

		const float midSepDist = separationDistance / 2.f;
		const float offsetLayerCoord = separationDistance * 2.f;

		do
		{
			const Storm::Vector3 begin = dimensions / -2.f;
			const Storm::Vector3 end = dimensions / 2.f;

			float currentX = begin.x();
			const float endX = dimensions.x() / 2.f;

			// 1st face
			for (float y = begin.y(); y <= end.y(); y += separationDistance)
			{
				for (float z = begin.z(); z <= end.z(); z += separationDistance)
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

			for (float y = begin.y(); y <= end.y(); y += separationDistance)
			{
				for (float z = begin.z(); z <= end.z(); z += separationDistance)
				{
					result.emplace_back(currentX, y, z);
				}
			}

			--layerCount;

			if constexpr (internalLayer)
			{
				dimensions.x() -= offsetLayerCoord;
				dimensions.y() -= offsetLayerCoord;
				dimensions.z() -= offsetLayerCoord;

				if (dimensions.x() <= 0.f || dimensions.y() <= 0.f || dimensions.z() <= 0.f)
				{
					if (layerCount > 0)
					{
						LOG_DEBUG_WARNING << "Too much layer to fit inside cube geometry. Skipped " << layerCount << " layers";
					}
					break;
				}
			}
			else
			{
				dimensions.x() += offsetLayerCoord;
				dimensions.y() += offsetLayerCoord;
				dimensions.z() += offsetLayerCoord;
			}

		} while (layerCount > 0);
		

		return result;
	}
}


template<bool internalLayer>
std::vector<Storm::Vector3> Storm::UniformSampler::process(const Storm::GeometryType geometry, const float separationDistance, const int layerCount, const void*const dimensions)
{
	if (separationDistance <= 0.f)
	{
		Storm::throwException<Storm::Exception>("Separation distance should be greater than 0!");
	}
	else if (layerCount < 1)
	{
		Storm::throwException<Storm::Exception>("Layer count should be greater than 0!");
	}

	switch (geometry)
	{
	case Storm::GeometryType::Sphere:
		return sampleUniformSphere<internalLayer>(separationDistance, layerCount, *reinterpret_cast<const float*const>(dimensions));

	case Storm::GeometryType::Cube:
		return sampleUniformCube<internalLayer>(separationDistance, layerCount, *reinterpret_cast<const Storm::Vector3*const>(dimensions));

	case Storm::GeometryType::None:
	default:
		Storm::throwException<Storm::Exception>("Uniform Sampler only work on simple geometries! Custom or unknown geometry aren't handled!");
	}
}

template std::vector<Storm::Vector3> Storm::UniformSampler::process<true>(const Storm::GeometryType, const float, const int, const void*const);
template std::vector<Storm::Vector3> Storm::UniformSampler::process<false>(const Storm::GeometryType, const float, const int, const void*const);
