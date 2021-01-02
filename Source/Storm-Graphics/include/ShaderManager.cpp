#include "ShaderManager.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "ShaderMacroItem.h"

#include <d3dcompiler.h>

#include <fstream>
#include <comdef.h>

#include <boost\lexical_cast.hpp>


#define STORM_DEBUG_SHADERS false


namespace
{
	std::string compileShader(const std::filesystem::path &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target, const Storm::ShaderMacroContainer &shaderMacros, ComPtr<ID3DBlob> &blobRes)
	{
		std::string compileErrorMsg;

		UINT flag1 = D3D10_SHADER_ENABLE_STRICTNESS;

#if STORM_DEBUG_SHADERS
		flag1 |= D3DCOMPILE_DEBUG;
		flag1 |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		flag1 |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		std::unique_ptr<D3D_SHADER_MACRO[]> shaderMacrosPtr;

		const std::vector<Storm::ShaderMacroItem> &shaderMacroItems = shaderMacros.getMacros();
		const std::size_t shaderMacroCount = shaderMacroItems.size();
		if (shaderMacroCount > 0)
		{
			shaderMacrosPtr = std::make_unique<D3D_SHADER_MACRO[]>(shaderMacroCount + 1);
			
			std::size_t iter = 0;
			for (; iter < shaderMacroCount; ++iter)
			{
				const Storm::ShaderMacroItem &currentSrcShaderMacro = shaderMacroItems[iter];
				D3D_SHADER_MACRO &currentDstShaderMacro = shaderMacrosPtr[iter];

				currentDstShaderMacro.Name = currentSrcShaderMacro.getMacroName().data();
				currentDstShaderMacro.Definition = currentSrcShaderMacro.getMacroValue().c_str();
			}

			D3D_SHADER_MACRO &last = shaderMacrosPtr[iter];
			last.Name = NULL;
			last.Definition = NULL;
		}
		else
		{
			shaderMacrosPtr = nullptr;
		}

		ComPtr<ID3DBlob> errorMsg;
		HRESULT res = D3DCompileFromFile(
			shaderFilePath.wstring().c_str(),
			shaderMacrosPtr.get(),
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			shaderFuncName.data(),
			target.data(),
			flag1,
			0,
			&blobRes,
			&errorMsg
		);
		
		if (FAILED(res))
		{
			compileErrorMsg += "Compilation failed! Reason : " + Storm::toStdString(_com_error{ res }) + '\n';

			if (errorMsg)
			{
				const std::size_t errorBufferSize = errorMsg->GetBufferSize();
				if (errorBufferSize > 0)
				{
					compileErrorMsg += "Compile error are ";
					compileErrorMsg.append(static_cast<const char*>(errorMsg->GetBufferPointer()), errorBufferSize);

					while (compileErrorMsg.back() == '\0')
					{
						compileErrorMsg.pop_back();
					}
				}
			}
			else
			{
				compileErrorMsg += "Error blob null => Shader " + shaderFilePath.string() + " was missing!";
			}
		}

		return compileErrorMsg;
	}

	std::filesystem::path getShaderBlobCacheFilePath(const std::filesystem::path &tmpPath, const std::string &shaderFilePath, const std::string_view &shaderFuncName, const Storm::ShaderMacroContainer &shaderMacros)
	{
		constexpr std::string_view shaderCacheExtension = ".stormShader";
		const std::string macroCacheName = shaderMacros.toCachedName();
		const std::string shaderCacheBaseFileName = std::filesystem::path{ shaderFilePath }.replace_extension("").string();
#if STORM_DEBUG_SHADERS
		constexpr std::string_view shaderConfigPrefix = "_d";
#else
		constexpr std::string_view shaderConfigPrefix = "_r";
#endif

		std::string shaderCacheFileName;
		shaderCacheFileName.reserve(
			shaderCacheBaseFileName.size() +
			shaderFuncName.size() +
			macroCacheName.size() +
			shaderConfigPrefix.size() +
			shaderCacheExtension.size() +
			2
		);

		shaderCacheFileName += shaderCacheBaseFileName;
		shaderCacheFileName += '_';
		shaderCacheFileName += shaderFuncName;
		shaderCacheFileName += macroCacheName;
		shaderCacheFileName += shaderConfigPrefix;
		shaderCacheFileName += shaderCacheExtension;

		return tmpPath / shaderCacheFileName;
	}

	constexpr static std::string_view k_shaderCacheFileName = "shader.cache";
	constexpr static std::string_view k_shaderCacheRelativeFilePathToTmp = "Shaders";

	std::filesystem::path getShadersTemporaryPath(const std::filesystem::path &tmpPath)
	{
		return tmpPath / k_shaderCacheRelativeFilePathToTmp;
	}
}


Storm::ShaderManager::ShaderManager() = default;
Storm::ShaderManager::~ShaderManager() = default;

void Storm::ShaderManager::initialize_Implementation()
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const std::filesystem::path tmpPath = configMgr.getTemporaryPath();

	const std::filesystem::path shaderTmpPath = getShadersTemporaryPath(tmpPath);
	std::filesystem::create_directories(shaderTmpPath);
}

void* Storm::ShaderManager::requestCompiledShaderBlobs(const std::string &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target, const Storm::ShaderMacroContainer &shaderMacros)
{
	ComPtr<ID3DBlob> shaderBlob;

	bool hasCachedBlobs = false;

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const std::string &tmpPath = configMgr.getTemporaryPath();

	const auto shaderFoundTimestamp = std::filesystem::last_write_time(shaderFilePath);

	const std::filesystem::path expectedShaderBlobFilePath = getShaderBlobCacheFilePath(tmpPath, shaderFilePath, shaderFuncName, shaderMacros);
	if (std::filesystem::exists(expectedShaderBlobFilePath))
	{
		hasCachedBlobs = std::filesystem::last_write_time(expectedShaderBlobFilePath) == shaderFoundTimestamp;
		if (!hasCachedBlobs)
		{
			LOG_DEBUG << "Shader " << shaderFilePath << " (for function : " << shaderFuncName << ") cache mismatches, therefore should be invalidated.";
		}
	}
	
	if (hasCachedBlobs)
	{
		LOG_DEBUG << "Shader " << shaderFilePath << " (for function : " << shaderFuncName << ") has already been compiled and its cache is up to date. We will load the cached data instead of compiling it anew.";

		std::size_t blobSize = std::filesystem::file_size(expectedShaderBlobFilePath);
		
		HRESULT res = D3DCreateBlob(blobSize, &shaderBlob);
		if (FAILED(res))
		{
			std::string fullError = "Blob creation failed : " + Storm::toStdString(_com_error{ res });
			LOG_ERROR << fullError;
			Storm::throwException<Storm::Exception>(fullError);
		}

		char* blobData = static_cast<char*>(shaderBlob->GetBufferPointer());
		std::ifstream{ expectedShaderBlobFilePath.string(), std::ios_base::in | std::ios_base::binary }.read(blobData, blobSize);
	}
	else
	{
		LOG_COMMENT << "Compiling Shader " << shaderFilePath << " (for function : " << shaderFuncName << ") and generate its cache for a next reloading.";

		std::string outCompileErrorMsg = compileShader(shaderFilePath, shaderFuncName, target, shaderMacros, shaderBlob);
		if (!outCompileErrorMsg.empty())
		{
			std::string fullError = "Shader compilation failure for '" + shaderFilePath + "' (compiling method " + shaderFuncName + ") : " + outCompileErrorMsg;
			LOG_ERROR << fullError;
			Storm::throwException<Storm::Exception>("Shader compilation errors detected for '" + shaderFilePath + "' (compiling method " + shaderFuncName + ") : " + outCompileErrorMsg);
		}

		// Save the blob file
		std::ofstream{ expectedShaderBlobFilePath.string(), std::ios_base::out | std::ios_base::binary }.write(
			static_cast<const char*>(shaderBlob->GetBufferPointer()),
			shaderBlob->GetBufferSize()
		);

		std::filesystem::last_write_time(expectedShaderBlobFilePath, shaderFoundTimestamp);
	}

	// Detach because we want transfer the ownership to the client code.
	// Don't forget to use Attach afterwards.
	return shaderBlob.Detach();
}

void* Storm::ShaderManager::requestCompiledShaderBlobs(const std::string &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target)
{
	return this->requestCompiledShaderBlobs(shaderFilePath, shaderFuncName, target, Storm::ShaderMacroContainer{});
}
