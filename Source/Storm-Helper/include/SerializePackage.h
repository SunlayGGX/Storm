#pragma once

#include "TraitsHelper.h"
#include "StaticAssertionsMacros.h"

#include <fstream>


namespace Storm
{
	enum class SerializePackageCreationModality
	{
		Loading,
		SavingNew,
		SavingAppend,
		SavingAppendPreheaderProvidedAfter,
	};

	class SerializePackage
	{
	public:
		SerializePackage(Storm::SerializePackageCreationModality modality, const std::string &packageFilePath);

	private:
		template<class Type>
		auto doSerialize(Type &other, int) -> decltype(other.serialize(std::declval<SerializePackage>()), static_cast<SerializePackage*>(nullptr))
		{
			other.serialize(*this);
			return this;
		}

		// For later to be defined Vector3
		template<class Vector3Type>
		auto doSerialize(Vector3Type &vect3, int) -> decltype(vect3.x(), vect3.y(), vect3.z(), static_cast<SerializePackage*>(nullptr))
		{
			this->operator <<(vect3.x());
			this->operator <<(vect3.y());
			this->operator <<(vect3.z());
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
			if constexpr (std::is_const_v<Type>)
			{
				STORM_COMPILE_ERROR("Object Type to be serialized shouldn't be const since we know only if we write or read at runtime");
			}
			else
			{
				STORM_COMPILE_ERROR("Type isn't defined to be a serializable type. Put a public method named 'serialize' inside that takes a SerializePackage reference as argument!");
			}
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

		template<class Type>
		SerializePackage& operator<<(std::atomic<Type> &other)
		{
			Type val;
			this->operator<<(val);
			other = val;
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

		const std::fstream& getUnderlyingStream() const noexcept;
		std::fstream& getUnderlyingStream() noexcept;

		void seekAbsolute(std::size_t newPos);

		const std::string& getFilePath() const noexcept;

	private:
		bool _isSaving;
		std::fstream _file;
		const std::string _filePath;
	};
}
