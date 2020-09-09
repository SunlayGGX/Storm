#include "PhysicsManager.h"

#include "PhysXHandler.h"

#include "PhysicsDynamicRigidBody.h"
#include "PhysicsStaticsRigidBody.h"

#include "PhysicsConstraint.h"

#include "RigidBodySceneData.h"
#include "ConstraintData.h"

#include "SingletonHolder.h"
#include "IGraphicsManager.h"
#include "IThreadManager.h"

#include "ThreadEnumeration.h"

#include "ThrowException.h"
#include "SearchAlgo.h"


Storm::PhysicsManager::PhysicsManager() = default;
Storm::PhysicsManager::~PhysicsManager() = default;

void Storm::PhysicsManager::initialize_Implementation()
{
	LOG_COMMENT << "PhysX initialization started";

	_physXHandler = std::make_unique<Storm::PhysXHandler>();

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

void Storm::PhysicsManager::update(float deltaTime)
{
	_physXHandler->update(_simulationMutex, deltaTime);

	this->pushPhysicsVisualizationData();
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
	Storm::SearchAlgo::executeOnObjectInContainer(constraintData._rigidBodyId1, [&](const auto &rb1)
	{
		Storm::SearchAlgo::executeOnObjectInContainer(constraintData._rigidBodyId2, [&](const auto &rb2)
		{
			_constraints.emplace_back(std::make_unique<Storm::PhysicsConstraint>(constraintData, rb1.getInternalPhysicsPointer(), rb2.getInternalPhysicsPointer()));
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
