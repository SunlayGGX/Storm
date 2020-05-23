#pragma once


#include "Facets.h"
#include "ThrowException.h"


namespace Storm
{
	struct FacetContainerIsOwner {};
	struct FacetContainerIsNotOwner {};

	namespace details
	{
		class FacetContainerInternalImplBase
		{
		protected:
			template<class PtrType>
			static auto extractRawPtr(const PtrType &facetPtr, int) -> decltype(facetPtr.get())
			{
				return facetPtr.get();
			}

			template<class PtrType>
			static PtrType extractRawPtr(const PtrType &facetPtr, void*)
			{
				return facetPtr;
			}

			template<class Facet, class FacetInternalMap>
			static Facet* getFacet(const FacetInternalMap &internalMap)
			{
				auto facetFound = internalMap.find(Facet::ID);
				if (facetFound != std::end(internalMap))
				{
					return static_cast<Facet*>(details::FacetContainerInternalImplBase::extractRawPtr(facetFound->second, 0));
				}

				return nullptr;
			}

			template<class Facet, class FacetInternalMap>
			static void unregisterFacet(FacetInternalMap &internalMap)
			{
				const auto facetFound = internalMap.find(Facet::ID);
				if (facetFound != std::end(internalMap))
				{
					internalMap.erase(facetFound);
				}
				else
				{
					assert(false && "Facet asked to remove didn't exists in the internal facet map container!");
				}
			}
		};
	}

	template<class FacetGroup, class ShouldBeOwner = FacetContainerIsOwner>
	class FacetContainer
	{
	public:
		using FacetBase = Storm::FacetBase<FacetGroup>;

	private:
		template<class ShouldBeOwner> struct InternalImpl;

		template<> struct InternalImpl<FacetContainerIsOwner> : protected details::FacetContainerInternalImplBase
		{
		public:
			template<class Facet, class ... Args>
			void registerFacet(Args &&... args)
			{
				const auto facetId = Facet::ID;
				const auto facetFound = _facetMap.find(facetId);
				if (facetFound == std::end(_facetMap))
				{
					_facetMap.emplace_hint(facetFound, facetId, std::make_unique<Facet>(std::forward<Args>(args)...));
				}
				else
				{
					Storm::throwException<std::exception>("A facet was already found in the facet map position!");
				}
			}

			template<class Facet>
			Facet* getFacet() const
			{
				return details::FacetContainerInternalImplBase::getFacet<Facet>(_facetMap);
			}

			template<class Facet>
			void unregisterFacet()
			{
				details::FacetContainerInternalImplBase::unregisterFacet<Facet>(_facetMap);
			}

		public:
			std::map<typename FacetBase::ID, std::unique_ptr<FacetBase>> _facetMap;
		};

		template<> struct InternalImpl<FacetContainerIsNotOwner> : protected details::FacetContainerInternalImplBase
		{
		public:
			template<class Facet>
			void registerFacet(FacetBase* alreadyConstructedPtr)
			{
				if (alreadyConstructedPtr == nullptr)
				{
					assert(false && "You shouldn't pass a null pointer to be registered!");
				}

				_facetMap[Facet::ID] = alreadyConstructedPtr;
			}

			template<class Facet>
			Facet* getFacet() const
			{
				return details::FacetContainerInternalImplBase::getFacet<Facet>(_facetMap);
			}

			template<class Facet>
			void unregisterFacet()
			{
				details::FacetContainerInternalImplBase::unregisterFacet<Facet>(_facetMap);
			}

		public:
			std::map<typename FacetBase::ID, FacetBase*> _facetMap;
		};


	private:
		template<class Facet, class ... Args>
		auto registerFacetImpl(int, Args &&... args) -> decltype(Facet::ID, void())
		{
			_internal.registerFacet<Facet>(std::forward<Args>(args)...);
		}

		template<class Facet, class ... Args>
		constexpr void registerFacetImpl(void*, Args &&... args)
		{

		}

		template<class Facet>
		auto unregisterFacetImpl(int) -> decltype(Facet::ID, void())
		{
			_internal.unregisterFacet<Facet>();
		}

		template<class Facet>
		constexpr void unregisterFacetImpl(void*)
		{

		}

		template<class Facet>
		auto getFacetImpl(int) const -> decltype(Facet::ID, _internal.getFacet<Facet>())
		{
			return _internal.getFacet<Facet>();
		}

		template<class Facet>
		constexpr Facet* getFacetImpl(void*) const
		{
			return nullptr;
		}

	public:
		template<class Facet, class ... Args>
		FacetContainer& registerFacet(Args &&... args)
		{
			this->registerFacetImpl<Facet>(0, std::forward<Args>(args)...);
			return *this;
		}

		template<class Facet>
		FacetContainer& unregisterFacet()
		{
			this->unregisterFacetImpl<Facet>(0);
			return *this;
		}

		template<class Facet>
		Facet* getFacet() const
		{
			return this->getFacetImpl<Facet>(0);
		}

	private:
		InternalImpl<ShouldBeOwner> _internal;
	};
}
