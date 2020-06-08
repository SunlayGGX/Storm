#include "PoissonDiskSampler.h"

#include "SingletonHolder.h"
#include "IRandomManager.h"



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
			constexpr const float k_epsilon = 0.001f;

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
	const float minDistSquared = diskRadius * diskRadius;
	const float maxDist = 2.f * diskRadius;
	const float personalParticleSpace = static_cast<float>(M_PI) * minDistSquared;

	// No optimization for now... I'll think about it later.
	constexpr float epsilon = 0.0001f;
	samplingResult.reserve(static_cast<std::size_t>(ceilf(totalArea / personalParticleSpace) + epsilon));
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
