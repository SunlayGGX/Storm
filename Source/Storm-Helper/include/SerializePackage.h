#pragma once

#include "TraitsHelper.h"
#include "StaticAssertionsMacros.h"

#include <fstream>


namespace Storm
{
	class SerializePackage
	{
	public:
		SerializePackage(bool isSaving, const std::string &packageFilePath);

	private:
		template<class Type>
		auto doSerialize(Type &other, int) -> decltype(other.serialize(std::declval<SerializePackage>()), static_cast<SerializePackage*>(nullptr))
		{
			other.serialize(*this);
			return this;
		}

		template<class ContainerType>
		auto doSerialize(ContainerType &container, void*) -> decltype(std::end(container), std::declval<SerializePackage>() << *std::begin(container), static_cast<SerializePackage*>(nullptr))
		{
			for (auto &item : container)
			{
				this->operator <<(item);
			}
			return this;
		}

		template<class Type>
		SerializePackage* doSerialize(Type &, ...)
		{
			STORM_COMPILE_ERROR("Type isn't defined to be a serializable type. Put a public method named 'serialize' inside that takes a SerializePackage reference as argument!");
			return this;
		}

	public:
		template<class Type>
		SerializePackage& operator<<(Type &other)
		{
			return *doSerialize(other, 0);
		}

		template<class Type>
		SerializePackage& operator<<(Type* other)
		{
			if (other != nullptr)
			{
				return this->operator<<(*other);
			}
			else
			{
				return this->operator<<(0);
			}
		}

		template<class Type>
		SerializePackage& operator<<(std::shared_ptr<Type> &other)
		{
			Type val;
			this->operator<<(val);
			other = std::make_shared<Type>(std::move(val));
			return *this;
		}

		template<class Type>
		SerializePackage& operator<<(std::unique_ptr<Type> &other)
		{
			Type val;
			this->operator<<(val);
			other = std::make_unique<Type>(std::move(val));
			return *this;
		}

		SerializePackage& operator<<(char &other);
		SerializePackage& operator<<(int8_t &other);
		SerializePackage& operator<<(int16_t &other);
		SerializePackage& operator<<(int32_t &other);
		SerializePackage& operator<<(int64_t &other);
		SerializePackage& operator<<(uint8_t &other);
		SerializePackage& operator<<(uint16_t &other);
		SerializePackage& operator<<(uint32_t &other);
		SerializePackage& operator<<(uint64_t &other);
		SerializePackage& operator<<(float &other);
		SerializePackage& operator<<(double &other);
		SerializePackage& operator<<(bool &other);
		SerializePackage& operator<<(std::string &other);

	public:
		// true if serializing (saving/writing), false if deserializing (loading/reading)
		bool isSerializing() const noexcept;

	private:
		bool _isSaving;
		std::fstream _file;
	};
}
