#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IRandomManager : public Storm::ISingletonHeldInterface<IRandomManager>
	{
	public:
		virtual ~IRandomManager() = default;

	public:
		// Return a random float between 0 and 1 (both included)
		virtual float randomizeFloat() = 0;

		// Return a random float between min and max (both included)
		virtual float randomizeFloat(float min, float max) = 0;
		
		// Return a random float between 0 and max (both included)
		virtual float randomizeFloat(float max) = 0;


		// Return a random 32 bits integer between min and max (both included)
		virtual int32_t randomizeInteger(int32_t min, int32_t max) = 0;

		// Return a random 32 bits integer between 0 and max (both included)
		virtual int32_t randomizeInteger(int32_t max) = 0;

		// Return a random 64 bits integer between min and max (both included)
		virtual int64_t randomizeInteger(int64_t min, int64_t max) = 0;

		// Return a random 64 bits integer between 0 and max (both included)
		virtual int64_t randomizeInteger(int64_t max) = 0;

		// Shuffle container from 0 index to endIndex index (not included).
		virtual void shuffle(std::vector<int> &container, std::size_t endIndex) = 0;
	};
}
