#pragma once


#include "Facets.h"


namespace Storm
{
	struct FacetContainerIsOwner {};
	struct FacetContainerIsNotOwner {};

	struct FacetContainerUseMapStorage {};
	template<std::size_t staticSize = 1> struct FacetContainerUseContiguousStaticStorage {};
	struct FacetContainerUseContiguousDynamicStorage {};

	template<class FacetGroup, class ShouldBeOwner = FacetContainerIsOwner, class StorageTraits = FacetContainerUseMapStorage>
	class FacetContainer
	{
	public:
		using FacetBase = Storm::FacetBase<FacetGroup>;
		using IdType = typename FacetBase::ID;
		using ParentType = FacetContainer<FacetGroup, ShouldBeOwner, StorageTraits>;

	private:
		class FacetContainerInternalImplBase
		{
		protected:
			template<class PtrType>
			static auto extractRawPtr(const PtrType &facetPtr, int) noexcept -> decltype(facetPtr.get())
			{
				return facetPtr.get();
			}

			template<class PtrType>
			static PtrType extractRawPtr(const PtrType &facetPtr, void*) noexcept
			{
				return facetPtr;
			}
		};

	private:
		template<class ElementType, class StorageTraits> class FacetContainerStorage;

		template<class ElementType>
		class FacetContainerStorage<ElementType, FacetContainerUseMapStorage> :
			protected Storm::FacetContainer<FacetGroup, ShouldBeOwner, StorageTraits>::FacetContainerInternalImplBase
		{
		public:
			template<class Facet, class ItemType>
			void registerFacet(ItemType &&item)
			{
				const IdType &id = Facet::ID;
				const auto found = _map.find(id);
				if (found == std::end(_map))
				{
					_map.emplace_hint(found, Facet::ID, std::forward<ItemType>(item));
				}
				else
				{
					Storm::throwException<Storm::StormException>("A facet was already found in the facet map position (" + toStdString(Facet::ID) + ")!");
				}
			}

			template<class Facet>
			Facet* getFacet() const
			{
				const IdType &id = Facet::ID;
				auto facetFound = _map.find(id);
				if (facetFound != std::end(_map))
				{
					return static_cast<Facet*>(FacetContainerInternalImplBase::extractRawPtr(facetFound->second, 0));
				}

				return nullptr;
			}

			template<class Facet>
			void unregisterFacet()
			{
				const IdType &id = Facet::ID;
				const auto facetFound = _map.find(id);
				if (facetFound != std::end(_map))
				{
					_map.erase(facetFound);
				}
				else
				{
					assert(false && "Facet asked to remove didn't exists in the internal facet map container!");
				}
			}

		private:
			std::map<IdType, ElementType> _map;
		};

		template<class ElementType, std::size_t staticSize>
		class FacetContainerStorage<ElementType, FacetContainerUseContiguousStaticStorage<staticSize>> :
			protected Storm::FacetContainer<FacetGroup, ShouldBeOwner, StorageTraits>::FacetContainerInternalImplBase
		{
		public:
			constexpr FacetContainerStorage()
			{
				for (std::size_t iter = 0; iter < staticSize; ++iter)
				{
					_array[iter] = nullptr;
				}
			}

		public:
			template<class Facet, class ItemType>
			void registerFacet(ItemType &&item)
			{
				const IdType &index = Facet::ID;
				assert(staticSize > index && "Facet ID is too big for the static array! Please increase its compile time size!");
				if (ElementType &itemPosition = _array[index]; itemPosition == nullptr)
				{
					itemPosition = std::forward<ItemType>(item);
				}
				else
				{
					Storm::throwException<Storm::StormException>("A facet was already found in the facet contiguous container position (" + toStdString(index) + ")!");
				}
			}

			template<class Facet>
			Facet* getFacet() const
			{
				const IdType &index = Facet::ID;
				assert(staticSize > index && "Facet ID is too big for the static array! Please increase its compile time size!");
				return static_cast<Facet*>(FacetContainerInternalImplBase::extractRawPtr(_array[index], 0));
			}

			template<class Facet>
			void unregisterFacet()
			{
				const IdType &index = Facet::ID;
				assert(staticSize > index && "Facet ID is too big for the static array! Please increase its compile time size!");
				_array[index] = nullptr;
			}

		private:
			ElementType _array[staticSize];
		};

		template<class ElementType>
		class FacetContainerStorage<ElementType, FacetContainerUseContiguousDynamicStorage> :
			protected Storm::FacetContainer<FacetGroup, ShouldBeOwner, StorageTraits>::FacetContainerInternalImplBase
		{
		public:
			FacetContainerStorage()
			{
				_array.resize(16, nullptr);
			}

		public:
			template<class Facet, class ItemType>
			void registerFacet(ItemType &&item)
			{
				const IdType &index = Facet::ID;
				if (_array.size() <= index)
				{
					_array.resize(index + 1, nullptr);
				}
				else if (ElementType &itemPosition = _array[index]; itemPosition != nullptr)
				{
					Storm::throwException<Storm::StormException>("A facet was already found in the facet contiguous container position (" + toStdString(index) + ")!");
				}

				_array[index] = std::forward<ItemType>(item);
			}

			template<class Facet>
			Facet* getFacet() const
			{
				const IdType &index = Facet::ID;
				assert(_array.size() > index && "Facet should have been registered before!");
				return static_cast<Facet*>(FacetContainerInternalImplBase::extractRawPtr(_array[index], 0));
			}

			template<class Facet>
			void unregisterFacet()
			{
				const IdType &index = Facet::ID;
				assert(_array.size() > index && "Facet should have been registered before!");
				_array[index] = nullptr;
			}

		private:
			std::vector<ElementType> _array;
		};


	private:
		template<class StorageTrait, class ShouldBeOwner> class InternalImpl;

		template<class StorageTrait> class InternalImpl<StorageTrait, FacetContainerIsOwner> :
			public FacetContainerStorage<std::unique_ptr<FacetBase>, StorageTrait>
		{};

		template<class StorageTrait> class InternalImpl<StorageTrait, FacetContainerIsNotOwner> :
			public FacetContainerStorage<FacetBase*, StorageTrait>
		{};


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
		InternalImpl<StorageTraits, ShouldBeOwner> _internal;
	};
}
