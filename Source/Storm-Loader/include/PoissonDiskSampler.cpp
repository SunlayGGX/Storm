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

			_normals[0] = _vect01.cross(_vect02);
			const float norm = _normals[0].norm();
			_normals[0] /= norm;
			_normals[1] = _normals[0];
			_normals[2] = _normals[1];

			_area = norm / 2.f;

			auto normalJunctionInitializer = [this](const std::size_t index)
			{
				_normalsJunction[index].first = _normals[index];
				_normalsJunction[index].second = 1;
			};
			normalJunctionInitializer(0);
			normalJunctionInitializer(1);
			normalJunctionInitializer(2);
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

		void producePoint(Storm::IRandomManager &randMgr, Storm::Vector3 &outPoint, Storm::Vector3 &outNormals) const
		{
			do
			{
				outPoint = *_v[0] + randMgr.randomizeFloat() * _vect01 + randMgr.randomizeFloat() * _vect02;
			} while (!this->isPointInside(outPoint));

			outNormals = _normals[0];
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

		// For debugging purpose
		std::string toStdString() const
		{
			struct CommaSeparatedPolicy { static constexpr const char separator() { return ','; } };

			std::string result;

			const std::string positions = Storm::toStdString<CommaSeparatedPolicy>(_v);
			const std::string v01Str = Storm::toStdString(_vect01);
			const std::string v02Str = Storm::toStdString(_vect02);
			const std::string v12Str = Storm::toStdString(_vect12);
			const std::string normalsStr = Storm::toStdString<CommaSeparatedPolicy>(_normals);
			const std::string areaStr = Storm::toStdString(_area);

			result.reserve(v01Str.size() + v02Str.size() + v12Str.size() + positions.size() + normalsStr.size() + areaStr.size() + 41);

			result += "pos=";
			result += positions;
			result += "; edge=01:";
			result += v01Str;
			result += ", 02:";
			result += v02Str;
			result += ", 12:";
			result += v12Str;
			result += "; normals=";
			result += normalsStr;
			result += "; area=";
			result += areaStr;

			return result;
		}

	public:
		const Storm::Vector3* _v[3];
		Storm::Vector3 _vect01;
		Storm::Vector3 _vect02;
		Storm::Vector3 _vect12;
		Storm::Vector3 _normalizedVect01;
		Storm::Vector3 _normalizedVect02;
		Storm::Vector3 _normals[3];
		std::pair<Storm::Vector3, std::size_t> _normalsJunction[3];
		float _area;
	};

	std::size_t computeExpectedSampleCount(const float diskRadius, const float totalArea)
	{
		const float minDistSquared = diskRadius * diskRadius;

		// DiskArea = pi * r�
		const float personalParticleSpaceArea = static_cast<float>(M_PI) * minDistSquared;

		constexpr float epsilon = 0.000001f;
		return static_cast<std::size_t>(std::max(std::ceilf(totalArea / personalParticleSpaceArea) + epsilon, 0.f));
	}

	Storm::SamplingResult produceRandomPointsAllOverMesh(Storm::IRandomManager &randMgr, const std::vector<Triangle> &allTriangles, const float maxArea, const std::size_t expectedSampleFinalCount)
	{
		enum : std::size_t
		{
			k_allParticleCluteringCoefficient = 45
		};

		Storm::SamplingResult denseSamplingResult;

		if (!allTriangles.empty())
		{
			const Storm::VectorHijacker expectedPopulationCount{ k_allParticleCluteringCoefficient * expectedSampleFinalCount };

			Storm::setNumUninitialized_safeHijack(denseSamplingResult._position, expectedPopulationCount);
			Storm::setNumUninitialized_safeHijack(denseSamplingResult._normals, expectedPopulationCount);

			// OpenMP is bugged. So we force it to NOT use openMP here.
			Storm::runParallel<false>(denseSamplingResult._position, [&randMgr, &allTriangles, &maxArea, &normals = denseSamplingResult._normals, triangleLastIndex = static_cast<int64_t>(allTriangles.size() - 1)](Storm::Vector3 &currentPointSample, const std::size_t currentPIndex)
			{
				std::size_t selectedTriangleIndex;
				do
				{
					selectedTriangleIndex = randMgr.randomizeInteger(triangleLastIndex);

					const Triangle &triangle = allTriangles[selectedTriangleIndex];
					if (randMgr.randomizeFloat() < (triangle._area / maxArea))
					{
						triangle.producePoint(randMgr, currentPointSample, normals[currentPIndex]);
						return;
					}

				} while (true);
			});
		}

		return denseSamplingResult;
	}
}

Storm::SamplingResult Storm::PoissonDiskSampler::process(const int kTryConst, const float diskRadius, const std::vector<Storm::Vector3> &vertices)
{
	Storm::SamplingResult samplingResult;

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
	samplingResult._position.reserve(sampleCount);
	samplingResult._normals.reserve(sampleCount);
	for (const Triangle &currentTriangle : triangles)
	{
		std::size_t activeBeginPointIndex = samplingResult._position.size();

		// The first point to begin populating the triangle
		currentTriangle.producePoint(randMgr, samplingResult._position.emplace_back(), samplingResult._normals.emplace_back());

		do
		{
			Storm::Vector3 candidate;
			if (currentTriangle.producePoint(randMgr, samplingResult._position, kTryConst, diskRadius, maxDist, activeBeginPointIndex, candidate))
			{
				samplingResult._position.emplace_back(candidate);

				// TODO : Smooth it, but since this is a retrocompatibility method, I don't care.
				samplingResult._normals.emplace_back(currentTriangle._normals[0]);
			}
			else
			{
				++activeBeginPointIndex;
			}

		} while (samplingResult._position.size() != activeBeginPointIndex);
	}

	return samplingResult;
}

Storm::SamplingResult Storm::PoissonDiskSampler::process_v2(const int /*kTryConst*/, const float diskRadius, const std::vector<Storm::Vector3> &vertices, const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, const bool /*smoothNormals*/)
{
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

#if false // To debug triangles
	DEBUG_LOG_UNTO_FILE("triangle.txt") << Storm::toStdString(triangles);
#endif


#if false
	// Smooth the triangles normals if specified
	if (smoothNormals)
	{
		const std::size_t triangleCount = triangles.size();
		for (std::size_t iter = 0; iter < triangleCount; ++iter)
		{
			Triangle &currentTriangle = triangles[iter];
			for (std::size_t jiter = iter + 1; jiter < triangleCount; ++jiter)
			{
				Triangle &triangleToCompare = triangles[jiter];

				const auto triangleRegisterLambda = [&currentTriangle, &triangleToCompare](const std::size_t currentIndex, const std::size_t indexToCompare)
				{
					const Storm::Vector3 &currentVertex = *currentTriangle._v[currentIndex];
					const Storm::Vector3 &vertexToCompare = *triangleToCompare._v[currentIndex];
					if (
						std::fabs(currentVertex.x() - vertexToCompare.x()) < 0.00000001f &&
						std::fabs(currentVertex.y() - vertexToCompare.y()) < 0.00000001f &&
						std::fabs(currentVertex.z() - vertexToCompare.z()) < 0.00000001f
						)
					{
						currentTriangle._normalsJunction[currentIndex].first += triangleToCompare._normals[indexToCompare];
						++currentTriangle._normalsJunction[currentIndex].second;
						triangleToCompare._normalsJunction[indexToCompare].first += currentTriangle._normals[currentIndex];
						++triangleToCompare._normalsJunction[indexToCompare].second;
					}
				};

				triangleRegisterLambda(0, 0);
				triangleRegisterLambda(0, 1);
				triangleRegisterLambda(0, 2);
				triangleRegisterLambda(1, 0);
				triangleRegisterLambda(1, 1);
				triangleRegisterLambda(1, 2);
				triangleRegisterLambda(2, 0);
				triangleRegisterLambda(2, 1);
				triangleRegisterLambda(2, 2);
			}
		}

		const auto smoother = [](const std::pair<Storm::Vector3, std::size_t> &normalJunction)
		{
			return (normalJunction.first / static_cast<float>(normalJunction.second)).normalized();
		};

		Storm::runParallel(triangles, [&smoother](Triangle &triangle)
		{
			triangle._normals[0] = smoother(triangle._normalsJunction[0]);
			triangle._normals[1] = smoother(triangle._normalsJunction[1]);
			triangle._normals[2] = smoother(triangle._normalsJunction[2]);
		});
	}
#endif

	// Produce a set of point sampling the mesh...
	const float maxDist = 2.f * diskRadius;
	std::size_t sampleCount = computeExpectedSampleCount(diskRadius, totalArea);

	Storm::SamplingResult allPossibleSamples = produceRandomPointsAllOverMesh(randMgr, triangles, maxArea, sampleCount);

	Storm::ISpacePartitionerManager &spacePartitionMgr = singletonHolder.getSingleton<Storm::ISpacePartitionerManager>();
	std::shared_ptr<Storm::IDistanceSpacePartitionProxy> distanceSearchProxy = spacePartitionMgr.makeDistancePartitionProxy(upCorner, downCorner, maxDist);

	const std::vector<Storm::Vector3>* containingBundlePtr;
	const std::vector<Storm::Vector3>* neighborBundlePtr[Storm::k_neighborLinkedBunkCount];

	// Now, remove the points that couldn't be in the final sample count...
	const float minDistSquared = diskRadius * diskRadius;

	const std::size_t allSampleCount = allPossibleSamples._position.size();

	Storm::SamplingResult result;
	result._position.reserve(allSampleCount);
	result._normals.reserve(allSampleCount);

	for (std::size_t iter = 0; iter < allSampleCount; ++iter)
	{
		const Storm::Vector3 &maybeSample = allPossibleSamples._position[iter];
		if (distanceSearchProxy->addDataIfDistanceUnique(maybeSample, minDistSquared, containingBundlePtr, neighborBundlePtr))
		{
			result._position.emplace_back(allPossibleSamples._position[iter]);
			result._normals.emplace_back(allPossibleSamples._normals[iter]);
		}
	}

	result._position.shrink_to_fit();
	result._normals.shrink_to_fit();

	return result;
}
