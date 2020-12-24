#include "ScriptObject.h"


Storm::ScriptObject::ScriptObject(std::string &&script) :
	_script{ std::move(script) }
{

}

Storm::ScriptObject::~ScriptObject() = default;

const std::string& Storm::ScriptObject::getScript() const noexcept
{
	return _script;
}
