#pragma once


#include "Singleton.h"
#include "IShaderManager.h"


namespace Storm
{
	class ShaderManager :
		private Storm::Singleton<ShaderManager>,
		public Storm::IShaderManager
	{
		STORM_DECLARE_SINGLETON(ShaderManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void* requestCompiledShaderBlobs(const std::string &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target, const Storm::ShaderMacroContainer &shaderMacros) final override;
		void* requestCompiledShaderBlobs(const std::string &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target) final override;

	private:
		void flushCache() const;

	private:
		mutable std::mutex _mutex;
		std::map<std::string, std::filesystem::file_time_type> _compiledShaderMapCache;
	};
}
