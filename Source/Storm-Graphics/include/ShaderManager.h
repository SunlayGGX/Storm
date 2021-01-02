#pragma once


#include "Singleton.h"
#include "IShaderManager.h"
#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class ShaderManager final :
		private Storm::Singleton<ShaderManager, Storm::DefineDefaultCleanupImplementationOnly>,
		public Storm::IShaderManager
	{
		STORM_DECLARE_SINGLETON(ShaderManager);

	private:
		void initialize_Implementation();

	public:
		void* requestCompiledShaderBlobs(const std::string &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target, const Storm::ShaderMacroContainer &shaderMacros) final override;
		void* requestCompiledShaderBlobs(const std::string &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target) final override;
	};
}
