#pragma once

#include "Average.h"


namespace Storm
{
	class SpeedProfileData
	{
	public:
		void startTime();
		void stopTime();

		const float& computationSpeed() const;

	private:
		std::chrono::high_resolution_clock::time_point _lastStartTime;

		Storm::Average<float, Storm::MovingAverageTraits<11>> _averageComputationSpeed;
	};
}
