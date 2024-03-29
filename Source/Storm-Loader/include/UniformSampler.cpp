#include "UniformSampler.h"

#include "GeometryType.h"
#include "GeometryConfig.h"

#include "BitField.h"


namespace
{
	template<bool useAlKashi>
	double computeAngleStep(const double separationDistance, const double circleRadius)
	{
		if constexpr (useAlKashi)
		{
			// Cosine law (Al Kashi Theorem)
			//	   c�=a�+b�-2ab cos(tetha)
			// <=> tetha = acos((a�+b�-c�) / (2ab))
			// <=> tetha = acos((2r� - sep�) / (2r�)) since a=b=r and c=sep=separationDistance

			const double circleRadiusSquaredCoeff = 2.0 * circleRadius * circleRadius;
			const double res = std::acos((circleRadiusSquaredCoeff - (separationDistance * separationDistance)) / circleRadiusSquaredCoeff);
			if (!std::isnan(res))
			{
				return std::abs(res);
			}
		}
		
		// From a rule of thumbs.
		// If we travel 2pi*radius, then we have swept an angle of 2pi.
		// So to travel separationDistance, we need to sweep tetha = 2pi * sepDist / (2pi* radius) <=> tetha = sepDist / radius.
		//
		// Note that is formula is only correct (performs even better for our algorithm than the exact Al Kashi theorem) if separationDistance is not much
		// i.e. tetha is little (if tetha is too big, then we cannot approximate that the distance between 2 point which must be separationDistance is equal to the distance traveled along the perimeter of the circle).
		return std::abs(separationDistance / circleRadius);
	}

#define STORM_USE_COMPLEXE_BUT_EXACT_ALGO false

	template<bool internalLayer, bool useAlKashi>
	Storm::SamplingResult sampleUniformSphere(const float separationDistance, int layerCount, float radius)
	{
		Storm::SamplingResult result;

		const std::size_t expectedCount = static_cast<std::size_t>(std::ceilf((static_cast<float>(4.0 * M_PI) * radius * radius) / (separationDistance * separationDistance) + 0.000001f)) * layerCount;

		result._position.reserve(expectedCount);
		result._normals.reserve(expectedCount);

		constexpr double twoPi = 2.0 * M_PI;

		do
		{
#if !STORM_USE_COMPLEXE_BUT_EXACT_ALGO
			{
				const Storm::Vector3 &pos = result._position.emplace_back(0.f, -radius, 0.f);
				result._normals.emplace_back(pos.normalized());
			}
#endif

			if (radius > 0.f)
			{
				// I think this implementation is more exact than the other part of the #if #else #endif... But the visual result are hard to compare and both looks the same to me... Therefore I prefer to keep the simpler one as the active code path (easier to troubleshoot if an issue arise).
#if STORM_USE_COMPLEXE_BUT_EXACT_ALGO
				double deltaRad = computeAngleStep<useAlKashi>(separationDistance, radius);
				if (!std::isnan(deltaRad))
				{
					const std::size_t circleRowCount = static_cast<std::size_t>(M_PI / deltaRad + 0.000000001);
					if (circleRowCount > 0)
					{
						deltaRad = M_PI / static_cast<double>(circleRowCount);

						double offset = (M_PI - static_cast<double>(circleRowCount - 1) * deltaRad) / 2.0;

						for (std::size_t tethaIter = 0; tethaIter < circleRowCount; ++tethaIter)
						{
							double tetha = (static_cast<double>(tethaIter) * deltaRad) - M_PI_2 + offset;
							if (tetha > M_PI) STORM_UNLIKELY
							{
								break;
							}

							const double currentY = std::sin(tetha) * radius;

							const double littleCircleRadius = std::cos(tetha) * radius;
							double anglePhi = computeAngleStep<useAlKashi>(separationDistance, littleCircleRadius);
							if (!std::isnan(anglePhi))
							{
								const std::size_t particleCountOnCircle = static_cast<std::size_t>(static_cast<double>(twoPi) / std::abs(anglePhi) + 0.000000001);
								if (particleCountOnCircle != 0)
								{
									anglePhi = static_cast<double>((std::signbit(anglePhi) ? -twoPi : twoPi) / static_cast<double>(particleCountOnCircle));

									for (std::size_t iter = 0; iter < particleCountOnCircle; ++iter)
									{
										const double phi = static_cast<double>(iter) * anglePhi;
										const Storm::Vector3 &pos = result._position.emplace_back(
											static_cast<float>(std::cos(phi) * littleCircleRadius),
											static_cast<float>(currentY),
											static_cast<float>(std::sin(phi) * littleCircleRadius)
										);

										result._normals.emplace_back(pos.normalized());
									}
								}
							}
						}
					}
				}
#else
				double deltaRad = computeAngleStep<useAlKashi>(separationDistance, radius);

				std::size_t indexLastCircle = 0;

				for (double tetha = -M_PI_2 + deltaRad; tetha <= M_PI_2; tetha += deltaRad)
				{
					const double currentY = std::sin(tetha) * radius;

					const double littleCircleRadius = std::cos(tetha) * radius;
					double anglePhi = computeAngleStep<useAlKashi>(separationDistance, littleCircleRadius);
					if (!std::isnan(anglePhi))
					{
						const std::size_t particleCountOnCircle = static_cast<std::size_t>(twoPi / std::abs(anglePhi));
						if (particleCountOnCircle != 0)
						{
							anglePhi = (std::signbit(anglePhi) ? -twoPi : twoPi) / static_cast<double>(particleCountOnCircle);

							indexLastCircle = result._position.size();

							for (std::size_t iter = 0; iter < particleCountOnCircle; ++iter)
							{
								const double phi = static_cast<double>(iter) * anglePhi;
								const Storm::Vector3 &pos = result._position.emplace_back(
									static_cast<float>(std::cos(phi) * littleCircleRadius),
									static_cast<float>(currentY),
									static_cast<float>(std::sin(phi) * littleCircleRadius)
								);

								result._normals.emplace_back(pos.normalized());
							}
						}
					}
				}

				// Just a failsafe to try to fill the hole if one appeared for whatever reason, but in normal time, we should not use it.
				const Storm::Vector3 last{ 0.f, radius, 0.f };
				if (std::find_if(std::begin(result._position) + indexLastCircle, std::end(result._position), [&last, sepDistSquared = separationDistance * separationDistance](const Storm::Vector3 &pos) 
				{
					return (last - pos).squaredNorm() < sepDistSquared; 
				}) == std::end(result._position))
				{
					result._position.emplace_back(last);
					result._normals.emplace_back(last.normalized());
				}
#endif
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

#undef STORM_USE_COMPLEXE_BUT_EXACT_ALGO


	// How to generate equidistributed points on the surface of a sphere
	// Markus Deserno - September 28, 2004
	template<bool internalLayer>
	Storm::SamplingResult sampleUniformEquiSphere_MarkusDeserno(const float separationDistance, int layerCount, std::pair<float, std::size_t> data)
	{
		Storm::SamplingResult result;

		double radius = static_cast<double>(data.first);
		const std::size_t N = data.second;

		const std::size_t expectedCount = static_cast<std::size_t>(std::ceilf(static_cast<float>(4.0 * M_PI * radius * radius) / (separationDistance * separationDistance) + 0.000001f)) * layerCount;

		result._position.reserve(expectedCount);
		result._normals.reserve(expectedCount);

		constexpr float twoPi = static_cast<float>(2.0 * M_PI);

		const int srcLayerCount = layerCount;

		do
		{
			std::size_t nCount = result._position.size();
			double a = 4.0 * M_PI * radius * radius / static_cast<double>(N);
			double d = std::sqrt(a);
			double mTetha = std::round(M_PI / d);
			double dTetha = M_PI / mTetha;
			double dPhi = a / dTetha;

			std::size_t mTethaSz = static_cast<std::size_t>(mTetha + 0.00000001);
			for (std::size_t m = 0; m < mTethaSz; ++m)
			{
				double tetha = M_PI * (m + 0.5) / mTetha;
				double mPhi = std::round(twoPi * std::sin(tetha) / dPhi);

				std::size_t mPhiSz = static_cast<std::size_t>(mPhi + 0.00000001);

				for (std::size_t n = 0; n < mPhiSz; ++n)
				{
					double phi = twoPi * static_cast<double>(n) / mPhi;
					const Storm::Vector3 &pos = result._position.emplace_back(
						static_cast<float>(radius * std::sin(tetha) * std::cos(phi)),
						static_cast<float>(radius * std::sin(tetha) * std::sin(phi)),
						static_cast<float>(radius * std::cos(tetha))
					);

					result._normals.emplace_back(pos.normalized());
				}
			}

			nCount = result._position.size() - nCount;

			LOG_DEBUG << "Created " << nCount << " samples using M. Deserno Algorithm for sphere layer " << srcLayerCount - layerCount;

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

		if (result._position.empty())
		{
			Storm::throwException<Storm::Exception>("No samples were created with Markus Deserno algorithm. Maybe the sample count was too much.");
		}
		else if (result._normals.size() != result._position.size())
		{
			Storm::throwException<Storm::Exception>("Mismatch between generated normals and positions while sampling using Uniform sampler.");
		}

		return result;
	}

	template<bool internalLayer>
	Storm::SamplingResult sampleUniformCube(const float separationDistance, int layerCount, const std::pair<const Storm::Vector3* const, Storm::GeometryConfig::HoleModality> &data)
	{
		Storm::SamplingResult result;

		Storm::Vector3 dimensions = *data.first;

		const bool xLeftClosed = !STORM_IS_BIT_ENABLED(data.second, Storm::GeometryConfig::HoleModality::OpenedXLeft);
		const bool xRightClosed = !STORM_IS_BIT_ENABLED(data.second, Storm::GeometryConfig::HoleModality::OpenedXRight);
		const bool yUpClosed = !STORM_IS_BIT_ENABLED(data.second, Storm::GeometryConfig::HoleModality::OpenedYUp);
		const bool yDownClosed = !STORM_IS_BIT_ENABLED(data.second, Storm::GeometryConfig::HoleModality::OpenedYDown);
		const bool zBackClosed = !STORM_IS_BIT_ENABLED(data.second, Storm::GeometryConfig::HoleModality::OpenedZBack);
		const bool zFrontClosed = !STORM_IS_BIT_ENABLED(data.second, Storm::GeometryConfig::HoleModality::OpenedZFront);

		const std::size_t expectedCount = static_cast<std::size_t>(std::ceilf(2.f * (dimensions.x() * dimensions.y() + dimensions.y() * dimensions.z() + dimensions.z() * dimensions.x()) / (separationDistance * separationDistance)) + 0.000001f) * layerCount;
		result._position.reserve(expectedCount);
		result._normals.reserve(expectedCount);
		
		const float offsetLayerCoord = separationDistance * 2.f;

		const bool shouldScanPlaneXZ = yUpClosed || yDownClosed || zBackClosed || zFrontClosed;

		do
		{
			const Storm::Vector3 begin = dimensions / -2.f;
			const Storm::Vector3 end = dimensions / 2.f;

			float currentX = begin.x();
			const float endX = dimensions.x() / 2.f;

			// 1st face
			if (xLeftClosed)
			{
				for (float y = begin.y(); y <= end.y(); y += separationDistance)
				{
					for (float z = begin.z(); z <= end.z(); z += separationDistance)
					{
						result._position.emplace_back(currentX, y, z);
						result._normals.emplace_back(-1.f, 0.f, 0.f);
					}
				}
				currentX += separationDistance;
			}

			// Scan plane
			if (shouldScanPlaneXZ)
			{
				const float sepDistanceSquared = separationDistance * separationDistance;
				for (; currentX < endX; currentX += separationDistance)
				{
					float last;
					float offset;

					if (zFrontClosed)
					{
						for (float y = begin.y(); y < end.y(); y += separationDistance)
						{
							result._position.emplace_back(currentX, y, begin.z());
							result._normals.emplace_back(0.f, 0.f, -1.f);
						}

						last = end.y() - result._position.back().y();
						offset = std::sqrtf(sepDistanceSquared - last * last);
					}
					else
					{
						last = 0.f;
						offset = 0.f;
					}

					if (yUpClosed)
					{
						for (float z = begin.z() /*+ offset*/; z < end.z(); z += separationDistance)
						{
							result._position.emplace_back(currentX, end.y(), z);
							result._normals.emplace_back(0.f, 1.f, 0.f);
						}

						last = end.z() - result._position.back().z();
						offset = std::sqrtf(sepDistanceSquared - last * last);
					}
					else
					{
						last = 0.f;
						offset = 0.f;
					}

					if (zBackClosed)
					{
						for (float y = end.y() /*- offset*/; y > begin.y(); y -= separationDistance)
						{
							result._position.emplace_back(currentX, y, end.z());
							result._normals.emplace_back(0.f, 0.f, 1.f);
						}

						last = begin.y() - result._position.back().y();
						offset = std::sqrtf(sepDistanceSquared - last * last);
					}
					else
					{
						last = 0.f;
						offset = 0.f;
					}

					if (yDownClosed)
					{
						for (float z = end.z() /*- offset*/; z > begin.z(); z -= separationDistance)
						{
							result._position.emplace_back(currentX, begin.y(), z);
							result._normals.emplace_back(0.f, -1.f, 0.f);
						}
					}
				}
			}
			else
			{
				currentX = endX;
			}

			if (xRightClosed)
			{
				for (float y = begin.y(); y <= end.y(); y += separationDistance)
				{
					for (float z = begin.z(); z <= end.z(); z += separationDistance)
					{
						result._position.emplace_back(currentX, y, z);
						result._normals.emplace_back(1.f, 0.f, 0.f);
					}
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
Storm::SamplingResult Storm::UniformSampler::process(const Storm::GeometryConfig &geometryConfig, const float separationDistance, const int layerCount, const void*const samplerData)
{
	if (separationDistance <= 0.f)
	{
		Storm::throwException<Storm::Exception>("Separation distance should be greater than 0!");
	}
	else if (layerCount < 1)
	{
		Storm::throwException<Storm::Exception>("Layer count should be greater than 0!");
	}

	switch (geometryConfig._type)
	{
	case Storm::GeometryType::Sphere:
		return sampleUniformSphere<internalLayer, false>(separationDistance, layerCount, *reinterpret_cast<const float*const>(samplerData));

	case Storm::GeometryType::Sphere_AlKashi:
		return sampleUniformSphere<internalLayer, true>(separationDistance, layerCount, *reinterpret_cast<const float*const>(samplerData));

	case Storm::GeometryType::EquiSphere_MarkusDeserno:
		return sampleUniformEquiSphere_MarkusDeserno<internalLayer>(separationDistance, layerCount, *reinterpret_cast<const std::pair<float, std::size_t>*const>(samplerData));

	case Storm::GeometryType::Cube:
	{
		std::pair<const Storm::Vector3*const, Storm::GeometryConfig::HoleModality> finalData{
			reinterpret_cast<const Storm::Vector3*const>(samplerData),
			geometryConfig._holeModality
		};
		return sampleUniformCube<internalLayer>(separationDistance, layerCount, finalData);
	}

	case Storm::GeometryType::None:
	default:
		Storm::throwException<Storm::Exception>("Uniform Sampler only work on simple geometries! Custom or unknown geometry aren't handled!");
	}
}

template Storm::SamplingResult Storm::UniformSampler::process<true>(const Storm::GeometryConfig &, const float, const int, const void*const);
template Storm::SamplingResult Storm::UniformSampler::process<false>(const Storm::GeometryConfig &, const float, const int, const void*const);
