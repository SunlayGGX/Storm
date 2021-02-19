#include "StateWriter.h"

#include "StateSavingOrders.h"
#include "SystemSimulationStateObject.h"
#include "SimulationState.h"

#include "StateFileHeader.h"

#include "SerializePackageCreationModality.h"
#include "SerializePackage.h"


namespace
{
	void applySettings(Storm::StateSavingOrders &inOutSavingOrder)
	{
		Storm::StateSavingOrders::SavingSettings &settings = inOutSavingOrder._settings;
		const std::filesystem::path folderPath = settings._filePath.parent_path();

		std::filesystem::create_directories(folderPath);

		if (std::filesystem::exists(settings._filePath))
		{
			if (settings._overwrite)
			{
				LOG_WARNING << "'" << settings._filePath << "' already exists and overwriting was enabled, therefore this file will be cleaned before the state being saved.";
			}
			else if (settings._autoPathIfNoOwerwrite)
			{
				const std::filesystem::path fileExtension = settings._filePath.extension();
				const std::filesystem::path newBaseFilePath = folderPath / settings._filePath.stem();

				bool found = false;
				unsigned short iter = 0;
				do
				{
					std::filesystem::path newFilePath = newBaseFilePath;

					newFilePath += '_' + std::to_string(iter);
					newFilePath += fileExtension;

					if (!std::filesystem::exists(newFilePath))
					{
						LOG_WARNING << "'" << settings._filePath << "' already exists and we haven't specified to overwrite this, therefore we'll change it to '" << newFilePath << "'";
						settings._filePath = std::move(newFilePath);
						found = true;
						break;
					}

				} while (++iter < std::numeric_limits<decltype(iter)>::max());

				if (!found)
				{
					// We shouldn't come here. If we do, then it is really bad (we have more than 65535 state files inside the current folder).
					Storm::throwException<Storm::Exception>(
						"'" + Storm::toStdString(settings._filePath) + "' already exists and the limit to generate a new file has been exceeded.\n"
						"Please clean your folder!"
					);
				}
			}
			else
			{
				Storm::throwException<Storm::Exception>("'" + Storm::toStdString(settings._filePath) + "' already exists and we specified we shouldn't overwrite it and this file path cannot be changed.");
			}
		}
	}

	void validateOrders(const Storm::StateSavingOrders &savingOrder)
	{
		if (savingOrder._simulationState == nullptr)
		{
			Storm::throwException<Storm::Exception>("Simulation state object is null!");
		}

		const Storm::SimulationState &simulationState = *savingOrder._simulationState;

		if (simulationState._currentPhysicsTime < 0.f)
		{
			Storm::throwException<Storm::Exception>("Physics time cannot be negative (" + std::to_string(simulationState._currentPhysicsTime) + ")!");
		}

		for (const Storm::SystemSimulationStateObject &pState : simulationState._pSystemStates)
		{
			if (pState._isFluid && pState._isStatic)
			{
				Storm::throwException<Storm::Exception>("Particle system state " + std::to_string(pState._id) + " cannot be a fluid and static at the same time!");
			}

			const std::size_t positionsCount = pState._positions.size();
			const std::size_t velocitiesCount = pState._velocities.size();
			const std::size_t forcesCount = pState._forces.size();
			const std::size_t densitiesCount = pState._densities.size();
			const std::size_t pressuresCount = pState._pressures.size();
			const std::size_t massesCount = pState._masses.size();
			const std::size_t volumesCount = pState._volumes.size();

#define STORM_VALIDATE_CONTAINER_SIZE_NO_MISMATCH(memberName)																															\
			do if (positionsCount != pState.memberName.size())																															\
			{																																											\
				Storm::throwException<Storm::Exception>("Particle system state " + std::to_string(pState._id) + "'s " STORM_STRINGIFY(memberName) " array have its size that mismatch!"); \
			} while (false)


			STORM_VALIDATE_CONTAINER_SIZE_NO_MISMATCH(_velocities);
			STORM_VALIDATE_CONTAINER_SIZE_NO_MISMATCH(_forces);

			if (pState._isFluid)
			{
				STORM_VALIDATE_CONTAINER_SIZE_NO_MISMATCH(_densities);
				STORM_VALIDATE_CONTAINER_SIZE_NO_MISMATCH(_pressures);
				STORM_VALIDATE_CONTAINER_SIZE_NO_MISMATCH(_masses);
			}
			else
			{
				STORM_VALIDATE_CONTAINER_SIZE_NO_MISMATCH(_volumes);
			}

#undef STORM_VALIDATE_CONTAINER_SIZE_NO_MISMATCH
		}
	}

	class StateWriterImpl : public Storm::StateFileHeader
	{
	public:
		StateWriterImpl(Storm::StateSavingOrders &savingOrder) :
			_savingOrder{ savingOrder }
		{
			// Fill header
			_configFileNameUsed = savingOrder._simulationState->_configSceneName;
		}

	public:
		void serialize(Storm::SerializePackage &package);

	private:
		Storm::StateSavingOrders &_savingOrder;
	};

	void StateWriterImpl::serialize(Storm::SerializePackage &package)
	{
		Storm::StateFileHeader::serialize(package);

		Storm::SimulationState &simulationState = *_savingOrder._simulationState;
		
		package << simulationState._currentPhysicsTime;

		uint64_t pSystemCount = static_cast<uint64_t>(simulationState._pSystemStates.size());
		package << pSystemCount;

		for (Storm::SystemSimulationStateObject &pState : simulationState._pSystemStates)
		{
			package <<
				pState._id <<
				pState._isFluid <<
				pState._isStatic <<
				pState._positions <<
				pState._velocities <<
				pState._forces
				;

			if (pState._isFluid)
			{
				package <<
					pState._densities <<
					pState._pressures <<
					pState._masses
					;
			}
			else
			{
				package <<
					pState._globalPosition <<
					pState._volumes
					;
			}
		}
	}
}


void Storm::StateWriter::execute(Storm::StateSavingOrders &savingOrder)
{
	applySettings(savingOrder);
	validateOrders(savingOrder);

	StateWriterImpl toSerialize{ savingOrder };

	Storm::SerializePackage package{ Storm::SerializePackageCreationModality::SavingNewPreheaderProvidedAfter, savingOrder._settings._filePath.string() };
	package << toSerialize;

	toSerialize.endSerialize(package);

	LOG_DEBUG << "Saving request handled. Saving at " << savingOrder._settings._filePath;
}
