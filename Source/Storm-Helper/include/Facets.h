#pragma once


#include "CRTPHierarchy.h"
#include "TypeIdGenerator.h"
#include "StaticAssertionsMacros.h"


namespace Storm
{
	template<class GroupType>
	struct FacetBase
	{
	public:
		struct ID : private Storm::FullHierarchisable<ID>
		{
		public:
			using UnderlyingType = unsigned int;

		private:
			ID(UnderlyingType idValue) : _id{ idValue }{}

		public:
			template<class NativeType>
			friend auto operator<=>(const NativeType left, const ID &right)
			{
				STORM_STATIC_ASSERT(std::is_integral_v<NativeType>, "left must be an integer type!");
				return left <=> right._id;
			}

			bool operator==(const ID &other) const
			{
				return _id == other._id;
			}

			bool operator<(const ID &other) const
			{
				return _id < other._id;
			}

			operator UnderlyingType() const
			{
				return _id;
			}

			template<class Type>
			static ID produceID()
			{
				return { s_idGenerator.produceID<Type>() };
			}

		private:
			static inline Storm::TypeIdGenerator s_idGenerator;
			const UnderlyingType _id;
		};

	protected:
		virtual ~FacetBase() = default;
	};

	template<class ChildType, class GroupType>
	struct Facet :
		public FacetBase<GroupType>
	{
	private:
		friend ChildType;
		Facet() = default;

	public:
		virtual ~Facet() = default;

	public:
		static inline const FacetBase<GroupType>::template ID ID = FacetBase<GroupType>::template ID::produceID<ChildType>();
	};
}

