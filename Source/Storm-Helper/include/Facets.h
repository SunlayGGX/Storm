#pragma once


#include "CRTPHierarchy.h"


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

        public:
            ID() : _id{ ++s_idGenerator } {}

        public:
            bool operator==(const ID &other) const
            {
                return _id == other._id;
            }

            bool operator<(const ID &other) const
            {
                return _id < other._id;
            }

        private:
            static inline ID::UnderlyingType s_idGenerator = 0;
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
        static inline const FacetBase<GroupType>::template ID ID;
    };
}

