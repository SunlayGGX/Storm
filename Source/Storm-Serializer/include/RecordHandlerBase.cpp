#include "RecordHandlerBase.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "SceneRecordConfig.h"

#include "RecordPreHeaderSerializer.h"

#include "SerializePackageCreationModality.h"

#include "SerializeConstraintLayout.h"
#include "SerializeParticleSystemLayout.h"

#include "Version.h"


namespace
{
	inline std::string retrieveRecordFilePath()
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
		return configMgr.getSceneRecordConfig()._recordFilePath;
	}
}

// Reading
Storm::RecordHandlerBase::RecordHandlerBase(Storm::SerializeRecordHeader &&header) :
	_header{ std::move(header) },
	_package{ Storm::SerializePackageCreationModality::LoadingManual, retrieveRecordFilePath() },
	_movingSystemCount{ 0 },
	_preheaderSerializer{ std::make_unique<Storm::RecordPreHeaderSerializer>() }
{
	_package << *_preheaderSerializer;
}

// Writing
Storm::RecordHandlerBase::RecordHandlerBase(Storm::SerializeRecordHeader &&header, Storm::Version &recordVersion) :
	_header{ std::move(header) },
	_package{ Storm::SerializePackageCreationModality::SavingAppendPreheaderProvidedAfter, retrieveRecordFilePath() },
	_movingSystemCount{ 0 },
	_preheaderSerializer{ std::make_unique<Storm::RecordPreHeaderSerializer>(recordVersion) }
{
	_package << *_preheaderSerializer;
}

Storm::RecordHandlerBase::~RecordHandlerBase() = default;

void Storm::RecordHandlerBase::serializeHeader()
{
	const Storm::Version &currentVersion = _preheaderSerializer->getRecordVersion();

	_package << _header._recordFrameRate << _header._frameCount;

#define XMACRO_STORM_SERIALIZE_VECTOR3_TYPE			\
	STORM_SERIALIZE_VECTOR3_TYPE_SPECIFIC(float)	\
	STORM_SERIALIZE_VECTOR3_TYPE_SPECIFIC(double)

	std::string vect3ScalarTypeStr;

	// Write/Read the type of vector3 since if we read a recording with a Vector3 data that is different than when we wrote it. It will gives us unexpected results.
	if (_package.isSerializing())
	{
#define STORM_SERIALIZE_VECTOR3_TYPE_SPECIFIC(Type) 				  \
		if constexpr(std::is_same_v<Storm::Vector3::Scalar, Type>)	  \
		{															  \
			vect3ScalarTypeStr = #Type;								  \
		}

		XMACRO_STORM_SERIALIZE_VECTOR3_TYPE
#undef STORM_SERIALIZE_VECTOR3_TYPE_SPECIFIC

		if (vect3ScalarTypeStr.empty())
		{
			// We shouldn't come here.
			Storm::throwException<Storm::StormException>("Unknown vector 3 scalar type!");
		}

		_package << vect3ScalarTypeStr;
	}
	else
	{
		_package << vect3ScalarTypeStr;

#define STORM_SERIALIZE_VECTOR3_TYPE_SPECIFIC(Type)																				   \
		if constexpr(std::is_same_v<Storm::Vector3::Scalar, Type>)																   \
		{																														   \
			if (vect3ScalarTypeStr != #Type)																					   \
			{																													   \
				Storm::throwException<Storm::StormException>(																			   \
					"Current recording to be read was written for a Vector3 with the internal layout type of " #Type ".\n"		   \
					"But we're trying to read the record with another layout (" + vect3ScalarTypeStr + "). This isn't allowed!"	   \
				);																												   \
			}																													   \
		}

		XMACRO_STORM_SERIALIZE_VECTOR3_TYPE
#undef STORM_SERIALIZE_VECTOR3_TYPE_SPECIFIC
	}

#undef XMACRO_STORM_SERIALIZE_VECTOR3_TYPE

	uint32_t particleSystemCount = static_cast<uint32_t>(_header._particleSystemLayouts.size());
	_package << particleSystemCount;

	if (!_package.isSerializing())
	{
		_header._particleSystemLayouts.resize(particleSystemCount);
	}

	for (auto &particleSystemLayout : _header._particleSystemLayouts)
	{
		_package <<
			particleSystemLayout._particleSystemId <<
			particleSystemLayout._particlesCount <<
			particleSystemLayout._isFluid <<
			particleSystemLayout._isStatic
			;

		if (!particleSystemLayout._isStatic)
		{
			++_movingSystemCount;
		}
	}

	if (currentVersion < Storm::Version{ 1, 1 })
	{
		return;
	}
	// The part after should only be executed from version 1.1 onwards.

	uint32_t constraintsCount = static_cast<uint32_t>(_header._contraintLayouts.size());
	_package << constraintsCount;

	if (!_package.isSerializing())
	{
		_header._contraintLayouts.resize(constraintsCount);
	}

	for (auto &constraintLayout : _header._contraintLayouts)
	{
		_package << constraintLayout._id;
	}
}

const Storm::SerializeRecordHeader& Storm::RecordHandlerBase::getHeader() const noexcept
{
	return _header;
}

void Storm::RecordHandlerBase::endWriteHeader(uint64_t headerPos, uint64_t frameCount)
{
	_package.seekAbsolute(headerPos + sizeof(_header._recordFrameRate));
	_package << frameCount;

	_preheaderSerializer->endSerializing(_package);
	_preheaderSerializer.reset();
}
