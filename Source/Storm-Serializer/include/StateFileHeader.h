#pragma once

#include "Version.h"


namespace Storm
{
	class SerializePackage;

	class StateFileHeader
	{
	public:
		StateFileHeader();
		virtual ~StateFileHeader();

	public:
		bool isValid() const;

	public:
		void serialize(Storm::SerializePackage &package);

		void endSerialize(Storm::SerializePackage &package);

	private:
		void serializePreheader(Storm::SerializePackage &package, bool end);
		void serializeHeaderPart(Storm::SerializePackage &package);

	protected:
		// Preheader
		uint64_t _checksumMagicWord;
		Storm::Version _stateFileVersion;
		uint64_t _stateFileSize;

		// Header


		// Additional variable
		std::size_t _headerStreamPosition;
		std::size_t _bodyStreamPosition;
	};
}
