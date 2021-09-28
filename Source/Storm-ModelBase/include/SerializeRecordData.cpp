#include "SerializeRecordHeader.h"
#include "SerializeConstraintLayout.h"
#include "SerializeParticleSystemLayout.h"
#include "SerializeSupportedFeatureLayout.h"

#include "SerializeRecordPendingData.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordContraintsData.h"


Storm::SerializeSupportedFeatureLayout::SerializeSupportedFeatureLayout()
{
	::memset(this, true, sizeof(*this));
}

Storm::SerializeRecordHeader::SerializeRecordHeader() :
	_supportedFeaturesLayout{ std::make_shared<Storm::SerializeSupportedFeatureLayout>() }
{}

Storm::SerializeRecordHeader::SerializeRecordHeader(Storm::SerializeRecordHeader &&) = default;
Storm::SerializeRecordHeader::~SerializeRecordHeader() = default;
Storm::SerializeRecordHeader& Storm::SerializeRecordHeader::operator=(Storm::SerializeRecordHeader &&) = default;
Storm::SerializeRecordPendingData::~SerializeRecordPendingData() = default;
