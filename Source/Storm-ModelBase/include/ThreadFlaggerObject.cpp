#include "ThreadFlaggerObject.h"
#include "ThreadingFlagger.h"


const Storm::ThreadFlaggerObject& Storm::ThreadFlaggerObject::operator<<(const Storm::ThreadFlagEnum flag) const
{
	Storm::ThreadingFlagger::addThreadFlag(flag);
	return *this;
}
