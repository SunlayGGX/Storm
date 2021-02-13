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
			_value{ 0 }
		{
			_value._dividedInternals._major = major;
			_value._dividedInternals._minor = minor;
			_value._dividedInternals._subminor = subminor;
		}

		Version(const std::string_view &versionStr);

	public:
		constexpr bool operator==(const Version &other) const noexcept
		{
			return _value._bunk == other._value._bunk;
		}

		constexpr bool operator<(const Version &other) const noexcept
		{
			return _value._bunk < other._value._bunk;
		}

		bool operator==(const std::string_view &versionStr) const;
		bool operator<(const std::string_view &versionStr) const;

		void serialize(Storm::SerializePackage &package);
		static std::size_t getSizeInSerializePacket();

		explicit operator std::string() const;

	public:
		static constexpr Version retrieveCurrentStormVersion()
		{
			return Version{ STORM_MAJOR, STORM_MINOR, STORM_SUBMINOR };
		}

	private:
		union
		{
			struct DividedData
			{
				VersionNumber _major;
				VersionNumber _minor;
				VersionNumber _subminor;
			} _dividedInternals;

			int32_t _bunk;
		} _value;
	};
}
