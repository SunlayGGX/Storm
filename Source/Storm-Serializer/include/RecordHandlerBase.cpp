#include "RecordHandlerBase.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "RecordConfigData.h"

#include "ThrowException.h"


namespace
{
	inline std::string retrieveRecordFilePath()
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
		return configMgr.getRecordConfigData()._recordFilePath;
	}
}


Storm::RecordHandlerBase::RecordHandlerBase(Storm::SerializeRecordHeader &&header, Storm::SerializePackageCreationModality packageCreationModality) :
	_header{ std::move(header) },
	_package{ packageCreationModality, retrieveRecordFilePath() }
{

}

void Storm::RecordHandlerBase::serializeHeader()
{
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
			Storm::throwException<std::exception>("Unknown vector 3 scalar type!");
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
				Storm::throwException<std::exception>(																			   \
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
			particleSystemLayout._isFluid
			;
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
}
