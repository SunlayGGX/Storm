#pragma once

namespace Storm
{
	enum class SerializePackageCreationModality
	{
		Loading,
		LoadingManual,
		SavingNew,
		SavingAppend,
		SavingAppendPreheaderProvidedAfter,
	};
}
