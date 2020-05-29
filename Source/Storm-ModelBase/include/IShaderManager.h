#pragma once

#include "SingletonHeldInterfaceBase.h"

namespace Storm
{
	class IShaderManager : public Storm::ISingletonHeldInterface<IShaderManager>
	{
	public:
		virtual ~IShaderManager() = default;

	public:
		// In fact, this is an ID3DBlob*... But prototyping it requires to include Windows.h to access "interface" macro that declare a lot of declspec...
		// So no gain to prototyping it (and I don't want to make big include like this)... So void* is okay.
		virtual void* requestCompiledShaderBlobs(const std::string &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target) = 0;
	};
}
