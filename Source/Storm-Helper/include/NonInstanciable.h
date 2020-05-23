#pragma once

namespace Storm
{
	class NonInstanciable
	{
	public:
		~NonInstanciable() = delete;
	};
}