#pragma once


namespace Storm
{
	// The reason I'm not using any std::unique_ptr or anything here is because of Dll import and incompatibility stuff with STL.
	// This class is here to provide make the id shared between dynamic libs and static libs (when I'll need it, for example, to embed the SingletonHolder into a dll).
	class TypeIdGenerator
	{
	private:
#if false
		using KeyType = std::string;
#else
		using KeyType = std::string_view;
#endif

	public:
		TypeIdGenerator();
		~TypeIdGenerator();

	private:
		unsigned int produceID(const std::string_view func);

	public:
		template<class Type>
		unsigned int produceID()
		{
			return this->produceID(__FUNCDNAME__);
		}

	private:
		std::mutex* _mutex;
		std::size_t _capacity;
		Storm::TypeIdGenerator::KeyType* _registeredKeysType;
		unsigned int* _ids;
		unsigned int _idsGen;
	};
}
