#pragma once


namespace Storm
{
	struct DefineDefaultInitAndCleanupImplementation
	{
	protected:
		template<class ... Args> constexpr void initialize_Implementation(Args &&... args) {}
		template<class ... Args> constexpr void cleanUp_Implementation(Args &&... args) {}
	};

	struct DefineDefaultInitImplementationOnly
	{
	protected:
		template<class ... Args> constexpr void initialize_Implementation(Args &&... args) {}
	};

	struct DefineDefaultCleanupImplementationOnly
	{
	protected:
		template<class ... Args> constexpr void cleanUp_Implementation(Args &&... args) {}
	};
}
