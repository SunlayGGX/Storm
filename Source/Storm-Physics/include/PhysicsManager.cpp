#include "PhysicsManager.h"

#include "PhysXHandler.h"

#include "PhysicsDynamicRigidBody.h"
#include "PhysicsStaticsRigidBody.h"

#include "PhysicsConstraint.h"

#include "RigidBodySceneData.h"
#include "ConstraintData.h"
#include "GeneralSimulationData.h"
#include "RecordConfigData.h"

#include "SingletonHolder.h"
#include "IGraphicsManager.h"
#include "IThreadManager.h"
#include "IConfigManager.h"
#include "IInputManager.h"

#include "ThreadEnumeration.h"
#include "SpecialKey.h"

#include "SerializeConstraintLayout.h"
#include "SerializeRecordContraintsData.h"

#include "RecordMode.h"

#include "ThrowException.h"
#include "SearchAlgo.h"

namespace
{
	template<class RbType, class ConstraintType>
	auto registerConstraint(RbType &rb, const ConstraintType &constraint, int) -> decltype(rb.registerConstraint(constraint))
	{
		return rb.registerConstraint(constraint);
	}

	template<class ConstraintType, class RbType>
	constexpr void registerConstraint(RbType &, const ConstraintType &, void*)
	{

	}
}


Storm::PhysicsManager::PhysicsManager() :
	_rigidBodiesFixated{ false }
{

}

Storm::PhysicsManager::~PhysicsManager() = default;

void Storm::PhysicsManager::initialize_Implementation()
{
	LOG_COMMENT << "PhysX initialization started";

	_physXHandler = std::make_unique<Storm::PhysXHandler>();

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	_rigidBodiesFixated = configMgr.getGeneralSimulationData()._fixRigidBodyAtStartTime;

	Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();
	inputMgr.bindKey(Storm::SpecialKey::KC_L, [this]() { _rigidBodiesFixated = !_rigidBodiesFixated; });

	LOG_COMMENT << "PhysX initialization ended";
}

void Storm::PhysicsManager::cleanUp_Implementation()
{
	LOG_COMMENT << "PhysX cleanup requested";

	_constraints.clear();

	_staticsRbMap.clear();
	_dynamicsRbMap.clear();

	_physXHandler.reset();

	LOG_COMMENT << "PhysX cleanup finished successfully";
}

void Storm::PhysicsManager::notifyIterationStart()
{
	Storm::SearchAlgo::executeOnContainer([](auto &rb) 
	{
		rb.onIterationStart();
	}, _dynamicsRbMap, _staticsRbMap);
}

void Storm::PhysicsManager::update(float deltaTime)
{
	if (!_rigidBodiesFixated)
	{
		_physXHandler->update(_simulationMutex, deltaTime);

		this->pushPhysicsVisualizationData();
	}
}

void Storm::PhysicsManager::addPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertexes, const std::vector<uint32_t> &indexes)
{
	if (rbSceneData._static)
	{
		_staticsRbMap[rbSceneData._rigidBodyID] = std::make_unique<Storm::PhysicsStaticsRigidBody>(rbSceneData, vertexes, indexes);
	}
	else
	{
		_dynamicsRbMap[rbSceneData._rigidBodyID] = std::make_unique<Storm::PhysicsDynamicRigidBody>(rbSceneData, vertexes, indexes);
	}
}

void Storm::PhysicsManager::bindParentRbToPhysicalBody(const bool isStatic, const unsigned int rbId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const
{
	if (isStatic)
	{
		if (const auto found = _staticsRbMap.find(rbId); found != std::end(_staticsRbMap))
		{
			found->second->setRbParent(parentRb);
		}
		else
		{
			Storm::throwException<std::exception>("Cannot find static physics rigid body " + std::to_string(rbId));
		}
	}
	else
	{
		if (const auto found = _dynamicsRbMap.find(rbId); found != std::end(_dynamicsRbMap))
		{
			found->second->setRbParent(parentRb);
		}
		else
		{
			Storm::throwException<std::exception>("Cannot find dynamic physics rigid body " + std::to_string(rbId));
		}
	}
}

void Storm::PhysicsManager::bindParentRbToPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::shared_ptr<Storm::IRigidBody> &parentRb) const
{
	this->bindParentRbToPhysicalBody(rbSceneData._static, rbSceneData._rigidBodyID, parentRb);
}

void Storm::PhysicsManager::addConstraint(const Storm::ConstraintData &constraintData)
{
	Storm::SearchAlgo::executeOnObjectInContainer(constraintData._rigidBodyId1, [&](auto &rb1)
	{
		std::shared_ptr<Storm::PhysicsConstraint> addedConstraint;
		Storm::SearchAlgo::executeOnObjectInContainer(constraintData._rigidBodyId2, [&](auto &rb2)
		{
			addedConstraint = std::make_shared<Storm::PhysicsConstraint>(constraintData, rb1.getInternalPhysicsPointer(), rb2.getInternalPhysicsPointer());
			
			registerConstraint(rb2, addedConstraint, 0);
			registerConstraint(rb1, addedConstraint, 0);

			_constraints.emplace_back(std::move(addedConstraint));

		}, _staticsRbMap, _dynamicsRbMap);
	}, _staticsRbMap, _dynamicsRbMap);

	LOG_DEBUG << "Constraint loaded";
}

void Storm::PhysicsManager::loadConstraints(const std::vector<Storm::ConstraintData> &constraintsToLoad)
{
	const std::size_t constraintsCount = constraintsToLoad.size();
	if (constraintsCount > 0)
	{
		LOG_DEBUG << "Loading " << constraintsCount << " constraints";

		_constraints.reserve(_constraints.size() + constraintsCount);

		for (const Storm::ConstraintData &constraint : constraintsToLoad)
		{
			this->addConstraint(constraint);
		}

		LOG_DEBUG << "Syncing constraint with Graphics modules.";
		this->pushConstraintsVisualizationData();
	}
	else
	{
		LOG_DEBUG << "No constraints to be load";
	}
}

void Storm::PhysicsManager::loadRecordedConstraint(const Storm::SerializeConstraintLayout &constraintsRecordLayout)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	if (configMgr.getRecordConfigData()._recordMode == Storm::RecordMode::Replay)
	{
		const std::vector<Storm::ConstraintData> &constraintDataArrays = configMgr.getConstraintsData();
		if (const auto found = std::find_if(std::begin(constraintDataArrays), std::end(constraintDataArrays), [id = constraintsRecordLayout._id](const Storm::ConstraintData &data)
		{
			return data._constraintId == id;
		}); found != std::end(constraintDataArrays))
		{
			const Storm::ConstraintData &constraintConfig = *found;
			_constraints.emplace_back(std::make_shared<Storm::PhysicsConstraint>(constraintConfig));

			LOG_DEBUG << "Recorded constraint loaded";
		}
		else
		{
			Storm::throwException<std::exception>("Cannot find constraint config data with index " + std::to_string(constraintsRecordLayout._id));
		}
	}
	else
	{
		Storm::throwException<std::exception>(__FUNCTION__ " should only be used when replaying!");
	}
}

void Storm::PhysicsManager::getConstraintsRecordLayoutData(std::vector<Storm::SerializeConstraintLayout> &outConstraintsRecordLayout) const
{
	outConstraintsRecordLayout.clear();
	outConstraintsRecordLayout.reserve(_constraints.size());

	for (const auto &constraint : _constraints)
	{
		Storm::SerializeConstraintLayout &layout = outConstraintsRecordLayout.emplace_back();
		layout._id = static_cast<uint32_t>(constraint->getID());
	}
}

void Storm::PhysicsManager::getConstraintsRecordFrameData(std::vector<Storm::SerializeRecordContraintsData> &outConstraintsRecordFrameData) const
{
	outConstraintsRecordFrameData.clear();
	outConstraintsRecordFrameData.reserve(_constraints.size());

	for (const auto &constraint : _constraints)
	{
		Storm::SerializeRecordContraintsData &data = outConstraintsRecordFrameData.emplace_back();
		data._id = static_cast<uint32_t>(constraint->getID());
		constraint->getCordPosition(data._position1, data._position2);
	}
}

void Storm::PhysicsManager::pushConstraintsRecordedFrame(const std::vector<Storm::SerializeRecordContraintsData> &constraintsRecordFrameData)
{
	const std::size_t constraintsCount = constraintsRecordFrameData.size();
	if (constraintsCount > 0)
	{
		std::vector<Storm::Vector3> constraintsPositionsTmp;
		constraintsPositionsTmp.reserve(constraintsCount * 2);

		const auto beginConstraintIter = std::begin(_constraints);
		const auto endConstraintIter = std::end(_constraints);

		for (const Storm::SerializeRecordContraintsData &constraintData : constraintsRecordFrameData)
		{
			constraintsPositionsTmp.emplace_back(constraintData._position1);
			constraintsPositionsTmp.emplace_back(constraintData._position2);

			if (auto found = std::find_if(beginConstraintIter, endConstraintIter, [id = constraintData._id](const std::shared_ptr<Storm::PhysicsConstraint> &constraint)
			{
				return constraint->getID() == id;
			}); found != endConstraintIter)
			{
				(*found)->setCordDistance((constraintData._position1 - constraintData._position2).norm());
			}
			else
			{
				Storm::throwException<std::exception>("Unknown constraint requested to be updated (requested id was " + std::to_string(constraintData._id) + ")");
			}
		}

		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		Storm::IGraphicsManager &graphicMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
		Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
		threadMgr.executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [&graphicMgr, constraintsPositions = std::move(constraintsPositionsTmp)]()
		{
			graphicMgr.pushConstraintData(constraintsPositions);
		});
	}
}

Storm::Vector3 Storm::PhysicsManager::getForceOnPhysicalBody(const unsigned int id) const
{
	Storm::Vector3 appliedForces;
	Storm::SearchAlgo::executeOnObjectInContainer(id, [&appliedForces](const auto &rbFound)
	{
		appliedForces = rbFound.getAppliedForce();
	}, _dynamicsRbMap, _staticsRbMap);

	return appliedForces;
}

void Storm::PhysicsManager::pushPhysicsVisualizationData() const
{
	this->pushConstraintsVisualizationData();
}

void Storm::PhysicsManager::pushConstraintsVisualizationData() const
{
	// Push constraints visualization settings.
	const std::size_t constraintCount = _constraints.size();
	if (constraintCount > 0)
	{
		std::vector<Storm::Vector3> constraintsPositionsTmp;
		constraintsPositionsTmp.reserve(constraintCount * 2);

		for (const auto &constraint : _constraints)
		{
			constraint->appendJointPositionToArray(constraintsPositionsTmp);
		}

		if (!constraintsPositionsTmp.empty())
		{
			const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
			Storm::IGraphicsManager &graphicMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
			Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

			threadMgr.executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [&graphicMgr, constraintsPositions = std::move(constraintsPositionsTmp)]()
			{
				graphicMgr.pushConstraintData(constraintsPositions);
			});
		}
	}
}

void Storm::PhysicsManager::getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const
{
	Storm::SearchAlgo::executeOnObjectInContainer(meshId, [&outTrans, &outRot](const auto &rbFound)
	{
		rbFound.getMeshTransform(outTrans, outRot);
	}, _staticsRbMap, _dynamicsRbMap);
}

void Storm::PhysicsManager::applyLocalForces(unsigned int particleSystemId, const std::vector<Storm::Vector3> &position, const std::vector<Storm::Vector3> &force)
{
	if (!_rigidBodiesFixated)
	{
		if (const auto dynamicFound = _dynamicsRbMap.find(particleSystemId); dynamicFound != std::end(_dynamicsRbMap))
		{
			Storm::PhysicsDynamicRigidBody &dynamicRb = *dynamicFound->second;
			const std::size_t applyCount = position.size();

			assert(applyCount == force.size() && "Mismatch detected between position and force apply count.");
			for (std::size_t iter = 0; iter < applyCount; ++iter)
			{
				dynamicRb.applyForce(position[iter], force[iter]);
			}
		}
		else
		{
			assert(_staticsRbMap.find(particleSystemId) != std::end(_staticsRbMap) && "Cannot find requested physics rigid body!");
		}
	}
}

void Storm::PhysicsManager::getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const
{
	Storm::SearchAlgo::executeOnObjectInContainer(meshId, [&outTrans, &outQuatRot](const auto &rbFound)
	{
		rbFound.getMeshTransform(outTrans, outQuatRot);
	}, _staticsRbMap, _dynamicsRbMap);
}

const Storm::PhysXHandler& Storm::PhysicsManager::getPhysXHandler() const
{
	return *_physXHandler;
}

Storm::PhysXHandler& Storm::PhysicsManager::getPhysXHandler()
{
	return *_physXHandler;
}
