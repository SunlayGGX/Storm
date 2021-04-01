#pragma once
#include "CRTPHierarchy.h"
#include "VersionMacro.h"


namespace Storm
{
	class SerializePackage;

	class Version :
		private Storm::FullHierarchisable<Version>,
		private Storm::FullHierarchisable<Version, std::string_view>
	{
	public:
		using VersionNumber = uint8_t;

	public:
		constexpr Version(VersionNumber major = 0, VersionNumber minor = 0, VersionNumber subminor = 0) :
			_value{ major << 24 | minor << 16 | subminor << 8 }
		{}

		Version(const std::string_view &versionStr);

	public:
		constexpr bool operator==(const Version &other) const noexcept
		{
			return _value == other._value;
		}

		constexpr bool operator<(const Version &other) const noexcept
		{
			return _value < other._value;
		}

		bool operator==(const std::string_view &versionStr) const;
		bool operator<(const std::string_view &versionStr) const;

		void serialize(Storm::SerializePackage &package);
		static std::size_t getSizeInSerializePacket();

		explicit operator std::string() const;

		Version& operator=(const std::string_view &versionStr);

	public:
		static constexpr Version retrieveCurrentStormVersion()
		{
			return Version{ STORM_MAJOR, STORM_MINOR, STORM_SUBMINOR };
		}

	private:
		int32_t _value;
	};
}
