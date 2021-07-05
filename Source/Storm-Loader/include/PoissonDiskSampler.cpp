#include "PoissonDiskSampler.h"

#include "SingletonHolder.h"
#include "IRandomManager.h"
#include "ISpacePartitionerManager.h"

#include "IDistanceSpacePartitionProxy.h"

#include "RunnerHelper.h"

#define STORM_HIJACKED_TYPE Storm::Vector3
#include "VectHijack.h"


namespace
{
	struct Triangle
	{
	public:
		Triangle(const Storm::Vector3 &v0, const Storm::Vector3 &v1, const Storm::Vector3 &v2)
		{
			_v[0] = &v0;
			_v[1] = &v1;
			_v[2] = &v2;

			_vect01 = v1 - v0;
			_vect02 = v2 - v0;
			_vect12 = v2 - v1;
			_normalizedVect01 = _vect01.normalized();
			_normalizedVect02 = _vect02.normalized();

			Storm::Vector3 perpVect = _vect01.cross(_vect02);

			_area = perpVect.norm() / 2.f;
		}

	public:
		bool isPointInside(const Storm::Vector3 &ptToCheck) const
		{
			constexpr const float k_epsilon = 0.000001f;

			// If a point is inside a Triangle (in 2D), then total area A == A0 + A1 + A2 with Ax the area of the triangle made by replacing the point x of the triangle by the point we check.
			const float A2 = computeTriangleArea(ptToCheck - *_v[0], _vect01);
			const float A0 = computeTriangleArea(ptToCheck - *_v[1], _vect12);
			const float A1 = computeTriangleArea(ptToCheck - *_v[2], _vect02);

			return fabs(_area - A2 - A0 - A1) < k_epsilon;
		}


		static float computeTriangleArea(const Storm::Vector3 &v12, const Storm::Vector3 &v13)
		{
			return v12.cross(v13).norm() / 2.f;
		}

		void producePoint(Storm::IRandomManager &randMgr, Storm::Vector3 &outPoint) const
		{
			do
			{
				outPoint = *_v[0] + randMgr.randomizeFloat() * _vect01 + (randMgr.randomizeFloat() * _vect02);
			} while (!this->isPointInside(outPoint));
		}

		bool producePoint(Storm::IRandomManager &randMgr, const std::vector<Storm::Vector3> &registeredPts, int kTry, float minDist, float maxDist, const std::size_t activePtIndex, Storm::Vector3 &outPoint) const
		{
			const Storm::Vector3 &activePt = registeredPts[activePtIndex];

			const float minDistSquared = minDist * minDist;
			const float maxDistSquared = maxDist * maxDist;

			while (kTry > 0)
			{
				do
				{
					outPoint = activePt;

					bool xPositive = randMgr.randomizeInteger(1);
					bool yPositive = randMgr.randomizeInteger(1);
					const float randomX = randMgr.randomizeFloat(minDist, maxDist);
					const float randomY = randMgr.randomizeFloat(minDist, maxDist);

					if (xPositive)
					{
						outPoint += randomX * _normalizedVect01;
					}
					else
					{
						outPoint -= randomX * _normalizedVect01;
					}

					if (yPositive)
					{
						outPoint += randomY * _normalizedVect02;
					}
					else
					{
						outPoint -= randomY * _normalizedVect02;
					}

				} while (((activePt - outPoint).squaredNorm() > maxDistSquared) || !this->isPointInside(outPoint));

				bool success = true;
				for (const Storm::Vector3 &pt : registeredPts)
				{
					if ((pt - outPoint).squaredNorm() < minDistSquared)
					{
						success = false;
						break;
					}
				}

				if (success)
				{
					return true;
				}

				--kTry;
			}

			return false;
		}

	public:
		const Storm::Vector3* _v[3];
		Storm::Vector3 _vect01;
		Storm::Vector3 _vect02;
		Storm::Vector3 _vect12;
		Storm::Vector3 _normalizedVect01;
		Storm::Vector3 _normalizedVect02;
		float _area;
	};

	std::size_t computeExpectedSampleCount(const float diskRadius, const float totalArea)
	{
		const float minDistSquared = diskRadius * diskRadius;

		// DiskArea = pi * r²
		const float personalParticleSpaceArea = static_cast<float>(M_PI) * minDistSquared;

		constexpr float epsilon = 0.000001f;
		return static_cast<std::size_t>(std::max(std::ceilf(totalArea / personalParticleSpaceArea) + epsilon, 0.f));
	}

	std::vector<Storm::Vector3> produceRandomPointsAllOverMesh(Storm::IRandomManager &randMgr, const std::vector<Triangle> &allTriangles, const float maxArea, const std::size_t expectedSampleFinalCount)
	{
		enum : std::size_t
		{
			k_allParticleCluteringCoefficient = 45
		};

		std::vector<Storm::Vector3> denseSamplingResult;

		if (!allTriangles.empty())
		{
			const Storm::VectorHijacker expectedPopulationCount{ k_allParticleCluteringCoefficient * expectedSampleFinalCount };

			denseSamplingResult.reserve(expectedPopulationCount._newSize);
			Storm::setNumUninitialized_hijack(denseSamplingResult, expectedPopulationCount);

			Storm::runParallel(denseSamplingResult, [&randMgr, &allTriangles, &maxArea, triangleLastIndex = static_cast<int64_t>(allTriangles.size() - 1)](Storm::Vector3 &currentPointSample)
			{
				std::size_t selectedTriangleIndex;
				do
				{
					selectedTriangleIndex = randMgr.randomizeInteger(triangleLastIndex);

					const Triangle &triangle = allTriangles[selectedTriangleIndex];
					if (randMgr.randomizeFloat() < (triangle._area / maxArea))
					{
						triangle.producePoint(randMgr, currentPointSample);
						return;
					}

				} while (true);
			});
		}

		return denseSamplingResult;
	}
}

std::vector<Storm::Vector3> Storm::PoissonDiskSampler::process(const int kTryConst, const float diskRadius, const std::vector<Storm::Vector3> &vertices)
{
	std::vector<Storm::Vector3> samplingResult;

	Storm::IRandomManager &randMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IRandomManager>();

	// Compute the triangles from the vertices.
	// Those triangle will be the base where we would sample on them (they would make 2D planes where we make 2D Poisson sampling)
	std::vector<Triangle> triangles;
	triangles.reserve(vertices.size() / 3);

	float totalArea = 0.f;

	for (std::size_t iter = 0; iter < vertices.size(); iter += 3)
	{
		const auto &addedTriangle = triangles.emplace_back(vertices[iter], vertices[iter + 1], vertices[iter + 2]);
		totalArea += addedTriangle._area;
	}

	// Now we would try to sample each triangle with Poisson Disk Sampling algorithm applied on 2D (because a space defined by a plane is 2D ;)).
	const float maxDist = 2.f * diskRadius;
	const std::size_t sampleCount = computeExpectedSampleCount(diskRadius, totalArea);

	// No optimization for now... I'll think about it later.
	samplingResult.reserve(sampleCount);
	for (const Triangle &currentTriangle : triangles)
	{
		std::size_t activeBeginPointIndex = samplingResult.size();

		// The first point to begin populating the triangle
		currentTriangle.producePoint(randMgr, samplingResult.emplace_back());

		do
		{
			Storm::Vector3 candidate;
			if (currentTriangle.producePoint(randMgr, samplingResult, kTryConst, diskRadius, maxDist, activeBeginPointIndex, candidate))
			{
				samplingResult.emplace_back(candidate);
			}
			else
			{
				++activeBeginPointIndex;
			}

		} while (samplingResult.size() != activeBeginPointIndex);
	}

	return samplingResult;
}

std::vector<Storm::Vector3> Storm::PoissonDiskSampler::process_v2(const int kTryConst, const float diskRadius, const std::vector<Storm::Vector3> &vertices, const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner)
{
	std::vector<Storm::Vector3> samplingResult;

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	Storm::IRandomManager &randMgr = singletonHolder.getSingleton<Storm::IRandomManager>();

	// Compute the triangles from the vertices.
	// Those triangle will be the base where we would sample on them (they would make 2D planes where we make 2D Poisson sampling)
	std::vector<Triangle> triangles;
	triangles.reserve(vertices.size() / 3);

	float totalArea = 0.f;
	float maxArea = 0.f;

	for (std::size_t iter = 0; iter < vertices.size(); iter += 3)
	{
		const auto &addedTriangle = triangles.emplace_back(vertices[iter], vertices[iter + 1], vertices[iter + 2]);
		if (addedTriangle._area > 0.f)
		{
			totalArea += addedTriangle._area;
			if (maxArea < addedTriangle._area)
			{
				maxArea = addedTriangle._area;
			}
		}
		else
		{
			// The triangle is invalid. Just scrap it.
			triangles.pop_back();
		}
	}

	if (totalArea == 0.f)
	{
		Storm::throwException<Storm::Exception>("A mesh to convert into particles must have a positive non zero area!");
	}

	// Produce a set of point sampling the mesh...
	const float maxDist = 2.f * diskRadius;
	std::size_t sampleCount = computeExpectedSampleCount(diskRadius, totalArea);
	std::vector<Storm::Vector3> allPossibleSamples = produceRandomPointsAllOverMesh(randMgr, triangles, maxArea, sampleCount);

	Storm::ISpacePartitionerManager &spacePartitionMgr = singletonHolder.getSingleton<Storm::ISpacePartitionerManager>();
	std::shared_ptr<Storm::IDistanceSpacePartitionProxy> distanceSearchProxy = spacePartitionMgr.makeDistancePartitionProxy(upCorner, downCorner, maxDist);

	const std::vector<Storm::Vector3>* containingBundlePtr;
	const std::vector<Storm::Vector3>* neighborBundlePtr[Storm::k_neighborLinkedBunkCount];

	// Now, remove the points that couldn't be in the final sample count...
	const float minDistSquared = diskRadius * diskRadius;
	for (const Storm::Vector3 &maybeSample : allPossibleSamples)
	{
		distanceSearchProxy->addDataIfDistanceUnique(maybeSample, minDistSquared, containingBundlePtr, neighborBundlePtr);
	}

	return distanceSearchProxy->getCompleteData();
}
