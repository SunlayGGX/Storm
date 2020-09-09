#pragma once


// STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(BlowerTypeName, BlowerTypeXmlName, EffectAreaType, MeshMakerType)


#define STORM_XMACRO_GENERATE_BLOWERS_CODE																							\
STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(Cube, "cube", BlowerCubeArea, BlowerCubeMeshMaker)											\
STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(Sphere, "sphere", BlowerSphereArea, BlowerSphereMeshMaker)									\
STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(Cylinder, "cylinder", BlowerCylinderArea, BlowerCylinderMeshMaker)							\
STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(Cone, "cone", BlowerConeArea, BlowerConeMeshMaker)							\
STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(RepulsionSphere, "repulsionsphere", BlowerRepulsionSphereArea, BlowerSphereMeshMaker)		\
STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(ExplosionSphere, "explosion", BlowerExplosionSphereArea, BlowerSphereMeshMaker)				\
STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(PulseExplosionSphere, "pulseexplosion", BlowerExplosionSphereArea, BlowerSphereMeshMaker)
