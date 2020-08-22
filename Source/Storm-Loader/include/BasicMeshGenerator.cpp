#include "BasicMeshGenerator.h"


#define STORM_REGISTER_TRIANGLE_INDEX(index1, index2, index3)\
inOutIndexes.emplace_back(index1);\
inOutIndexes.emplace_back(index2);\
inOutIndexes.emplace_back(index3);

namespace
{
	template<class IndexContainer>
	void addIndexesOfQuadIndex(IndexContainer &inOutIndexes, const uint32_t p0, const uint32_t p1, const uint32_t p2, const uint32_t p3)
	{
		STORM_REGISTER_TRIANGLE_INDEX(p0, p2, p1);
		STORM_REGISTER_TRIANGLE_INDEX(p1, p2, p3);
	}

	// LUT means "Look Up Table"
	template<std::size_t division, bool hemisphere>
	struct SinCosLUT
	{
	public:
		SinCosLUT() :
			_divAngleRad{ static_cast<float>((hemisphere ? M_PI : 2.0 * M_PI) / static_cast<double>(division)) }
		{
			for (std::size_t iter = 0; iter < division; ++iter)
			{
				const float angleRad = _divAngleRad * static_cast<float>(iter);
				_cosLUT[iter] = std::cosf(angleRad);
				_sinLUT[iter] = std::sinf(angleRad);
			}
		}

	public:
		inline float cosInLUT(std::size_t index) const
		{
			return _cosLUT[index];
		}

		inline float sinInLUT(std::size_t index) const
		{
			return _sinLUT[index];
		}

	public:
		const float _divAngleRad;

	private:
		float _cosLUT[division];
		float _sinLUT[division];
	};
}

void Storm::BasicMeshGenerator::generateCube(const Storm::Vector3 &position, const Storm::Vector3 &dimension, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes)
{
	enum : std::size_t
	{
		k_summitCount = 8, // Eight summits in a cube
		k_faceCount = 6, // Six faces in a cube

		// Since there are no normals, we can optimize the memory consumption by not duplicating points.
		k_vertexCount = k_summitCount,

		k_triangleCount = k_faceCount * 2,

		// 3 points per triangle
		k_indexCount = k_triangleCount * 3
	};

	// Increase the vertex count to avoid reallocation. The increase is from the former size of those buffers since maybe, they contains a scene we want to render in one batch... 
	inOutVertexes.reserve(inOutVertexes.size() + k_summitCount);
	inOutIndexes.reserve(inOutIndexes.size() + k_indexCount);

	// Summits
	const Storm::Vector3 midScale = dimension / 2.f;
	const Storm::Vector3 upCorner = position + midScale;
	const Storm::Vector3 downCorner = position - midScale;

	inOutVertexes.emplace_back(upCorner.x(), upCorner.y(), upCorner.z());
	inOutVertexes.emplace_back(upCorner.x(), upCorner.y(), downCorner.z());
	inOutVertexes.emplace_back(upCorner.x(), downCorner.y(), upCorner.z());
	inOutVertexes.emplace_back(upCorner.x(), downCorner.y(), downCorner.z());
	inOutVertexes.emplace_back(downCorner.x(), upCorner.y(), upCorner.z());
	inOutVertexes.emplace_back(downCorner.x(), upCorner.y(), downCorner.z());
	inOutVertexes.emplace_back(downCorner.x(), downCorner.y(), upCorner.z());
	inOutVertexes.emplace_back(downCorner.x(), downCorner.y(), downCorner.z());

	// indexes
	STORM_REGISTER_TRIANGLE_INDEX(0, 2, 1);
	STORM_REGISTER_TRIANGLE_INDEX(1, 2, 3);
	STORM_REGISTER_TRIANGLE_INDEX(0, 1, 4);
	STORM_REGISTER_TRIANGLE_INDEX(1, 5, 4);
	STORM_REGISTER_TRIANGLE_INDEX(0, 4, 2);
	STORM_REGISTER_TRIANGLE_INDEX(2, 4, 6);
	STORM_REGISTER_TRIANGLE_INDEX(1, 3, 5);
	STORM_REGISTER_TRIANGLE_INDEX(3, 7, 5);
	STORM_REGISTER_TRIANGLE_INDEX(2, 7, 3);
	STORM_REGISTER_TRIANGLE_INDEX(2, 6, 7);
	STORM_REGISTER_TRIANGLE_INDEX(4, 5, 7);
	STORM_REGISTER_TRIANGLE_INDEX(4, 7, 6);
}

void Storm::BasicMeshGenerator::generateSphere(const Storm::Vector3 &position, const float radius, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes)
{
	enum : std::size_t
	{
		k_ringsX = 36,
		k_ringsY = 36,

		// The summit doesn't need to have a concentration of k_ringsX vertexes. In other words, the rings rotation made redundancy at 2 specific location.
		k_ringsYLean = k_ringsY - 2,

		k_ringsYIndexLine = k_ringsYLean - 1,

		k_ringsXIteration = k_ringsX - 1,
		k_ringsYIteration = k_ringsYLean + 1,

		k_vertexCount = k_ringsX * k_ringsYLean + 2,

		// Every batch of 4 vertexes make 2 triangles : 3 indexes per triangle, therefore 6 indexes per batch of 4 vertexes.
		k_indexCount = (k_ringsYLean * k_ringsXIteration + 1) * 6 + k_ringsX * 6
	};

	// Increase the vertex count to avoid reallocation. The increase is from the former size of those buffers since maybe, they contains a scene we want to render in one batch... 
	inOutVertexes.reserve(inOutVertexes.size() + k_vertexCount);
	inOutIndexes.reserve(inOutIndexes.size() + k_indexCount);

	const float diameter = 2.f * radius;

	const Storm::Vector3 upVect{ 0.f, radius, 0.f };
	const Storm::Vector3 upCorner = position + upVect;
	const Storm::Vector3 downCorner = position - upVect;

	static const SinCosLUT<k_ringsX, false> k_sinCosXRingLUT;
	static const SinCosLUT<k_ringsY, true> k_sinCosYRingLUT;

	// Vertexes
	for (std::size_t iter = 1; iter < k_ringsYIteration; ++iter)
	{
		const float currentCutRadius = radius * k_sinCosYRingLUT.sinInLUT(iter);
		const float yCutPos = downCorner.y() + radius * (1.f - k_sinCosYRingLUT.cosInLUT(iter));

		for (std::size_t jiter = 0; jiter < k_ringsX; ++jiter)
		{
			inOutVertexes.emplace_back(downCorner.x() + currentCutRadius * k_sinCosXRingLUT.cosInLUT(jiter), yCutPos, downCorner.z() + currentCutRadius * k_sinCosXRingLUT.sinInLUT(jiter));
		}
	}

	// The 2 special location where all points in the same ring converges.
	inOutVertexes.emplace_back(downCorner);
	inOutVertexes.emplace_back(upCorner);

	// Indexes
	for (std::size_t iter = 0; iter < k_ringsYIndexLine; ++iter)
	{
		const uint32_t pointStartOffset = static_cast<uint32_t>(iter * k_ringsX);

		// Goes to the point 0 of the ring to the one just before the last... We handle the last separately since it should loop back to the first.
		for (std::size_t jiter = 0; jiter < k_ringsXIteration; ++jiter)
		{
			// The Quad defined by the current ring and the next ring
			const uint32_t currentP0 = static_cast<uint32_t>(pointStartOffset + jiter);

			addIndexesOfQuadIndex(inOutIndexes,
				currentP0,
				currentP0 + 1,
				currentP0 + static_cast<uint32_t>(k_ringsX),
				currentP0 + static_cast<uint32_t>(k_ringsX) + 1
			);
		}

		// The last point of the ring that loops back.
		addIndexesOfQuadIndex(inOutIndexes,
			pointStartOffset + static_cast<uint32_t>(k_ringsXIteration),
			pointStartOffset,
			pointStartOffset + static_cast<uint32_t>(k_ringsX) + static_cast<uint32_t>(k_ringsXIteration),
			pointStartOffset + static_cast<uint32_t>(k_ringsX)
		);
	}

	// For the 2 specific points that converges
	const uint32_t downVertexIndex = static_cast<uint32_t>(k_vertexCount - 2);
	const uint32_t upVertexIndex = static_cast<uint32_t>(downVertexIndex + 1);
	const uint32_t lastRingStartOffset = static_cast<uint32_t>(downVertexIndex - k_ringsX);
	for (std::size_t jiter = 0; jiter < k_ringsXIteration; ++jiter)
	{
		STORM_REGISTER_TRIANGLE_INDEX(downVertexIndex, static_cast<uint32_t>(jiter), static_cast<uint32_t>(jiter + 1));
		STORM_REGISTER_TRIANGLE_INDEX(upVertexIndex, static_cast<uint32_t>(lastRingStartOffset + jiter + 1), static_cast<uint32_t>(lastRingStartOffset + jiter));
	}

	STORM_REGISTER_TRIANGLE_INDEX(downVertexIndex, static_cast<uint32_t>(k_ringsXIteration), 0);
	STORM_REGISTER_TRIANGLE_INDEX(upVertexIndex, static_cast<uint32_t>(lastRingStartOffset), static_cast<uint32_t>(lastRingStartOffset + k_ringsXIteration));
}
