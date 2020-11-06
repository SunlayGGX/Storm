#include "ShaderManager.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "ShaderMacroItem.h"

#include <d3dcompiler.h>

#include <fstream>
#include <comdef.h>

#include <boost\lexical_cast.hpp>


namespace
{
	std::string compileShader(const std::filesystem::path &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target, const Storm::ShaderMacroContainer &shaderMacros, ComPtr<ID3DBlob> &blobRes)
	{
		std::string compileErrorMsg;

		UINT flag1 = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined(_DEBUG)
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

	std::string getShaderIdentifier(const std::string &shaderFilePath, const std::string_view &shaderFuncName)
	{
		return shaderFilePath + "<?>" + shaderFuncName;
	}

	std::filesystem::path getShaderBlobCacheFilePath(const std::filesystem::path &tmpPath, const std::string &shaderFilePath, const std::string_view &shaderFuncName, const Storm::ShaderMacroContainer &shaderMacros)
	{
		return tmpPath / (std::filesystem::path{ shaderFilePath }.replace_extension("").string() + '_' + shaderFuncName + shaderMacros.toCachedName() + ".stormShader");
	}

	constexpr static std::string_view k_shaderCacheFileName = "shader.cache";
	constexpr static std::string_view k_shaderCacheRelativeFilePathToTmp = "Shaders";
	constexpr static std::string_view k_shaderCacheSeparator = ">>>>>>>";

	std::filesystem::path getShadersTemporaryPath(const std::filesystem::path &tmpPath)
	{
		return tmpPath / k_shaderCacheRelativeFilePathToTmp;
	}

	std::filesystem::path getTemporaryFinalShaderCacheToRemovePath(const std::filesystem::path &finalShaderCache)
	{
		return finalShaderCache.string() + ".last.remov";
	}

	std::filesystem::path getTemporaryFinalShaderCacheToFlush(const std::filesystem::path &tmpPath)
	{
		return getShadersTemporaryPath(tmpPath) / (k_shaderCacheFileName + ".tmp");
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

	const std::filesystem::path finalShaderCache = shaderTmpPath / k_shaderCacheFileName;
	const std::filesystem::path finalShaderCacheToRemove = getTemporaryFinalShaderCacheToRemovePath(finalShaderCache);
	const std::filesystem::path toFlushFilePath = getTemporaryFinalShaderCacheToFlush(tmpPath);

	if (std::filesystem::exists(toFlushFilePath))
	{
		LOG_WARNING << "Cache of written shaders was not flushed correctly last time. We will try to recover it!";

		if (std::filesystem::exists(finalShaderCacheToRemove))
		{
			LOG_COMMENT <<
				finalShaderCacheToRemove << 
				" was found, which means that the last state is still here so we could recover from it but it also means that it is possible we might recompile some shaders again!";
			std::filesystem::rename(finalShaderCacheToRemove, finalShaderCache);
		}
		else
		{
			LOG_WARNING << "We cannot recover the shader state file. It means that we would consider that no shader was compiled before. We will rebuild them all.";
		}

		std::filesystem::remove_all(toFlushFilePath);
	}
	else if (!std::filesystem::exists(finalShaderCache) && std::filesystem::exists(finalShaderCacheToRemove))
	{
		LOG_WARNING << "Something went wrong last time we flushed the shaders cache file. We will recover it from last state!";
		std::filesystem::rename(finalShaderCacheToRemove, finalShaderCache);
	}

	// This file should never exist at the end (it is the last state of our cache to not lose the cache state while writing the new cached state...
	// So it is a byproduct of an operation that was forcefully stopped in the middle for whatever reasons).
	std::filesystem::remove_all(finalShaderCacheToRemove);

	if (std::filesystem::exists(finalShaderCache))
	{
		std::ifstream shaderCacheFile{ finalShaderCache.string() };
		std::string line;
		while (std::getline(shaderCacheFile, line))
		{
			if (!line.empty())
			{
				bool added = false;

				const std::size_t shaderCacheSeparatorPos = line.find(k_shaderCacheSeparator);
				if (shaderCacheSeparatorPos != std::string::npos)
				{
					std::string shaderCacheId = line.substr(0, shaderCacheSeparatorPos);
					std::string shaderCacheTimestamp = line.substr(shaderCacheSeparatorPos + k_shaderCacheSeparator.size());

					if (!shaderCacheTimestamp.empty())
					{
						std::filesystem::file_time_type::rep readRawTimestampValue = boost::lexical_cast<std::filesystem::file_time_type::rep>(shaderCacheTimestamp);
						
						const std::filesystem::file_time_type reconvertedTimestamp{ std::filesystem::file_time_type::duration{ readRawTimestampValue } };
						_compiledShaderMapCache.emplace(shaderCacheId, reconvertedTimestamp);
						added = true;
					}
				}

				if (!added)
				{
					LOG_ERROR <<
						"'" << line << "' in shader cache was not parsed successfully. This may be due to a corruption in shader cache file.\n"
						"We will ignore this entry but it means that we may be forced to recompile a shader.";
				}
			}
		}
	}
}

void Storm::ShaderManager::cleanUp_Implementation()
{
	std::lock_guard<std::mutex> lock{ _mutex };
	this->flushCache();
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
		std::lock_guard<std::mutex> lock{ _mutex };
		if (const auto cachedShaderIt = _compiledShaderMapCache.find(getShaderIdentifier(shaderFilePath, shaderFuncName)); cachedShaderIt != std::end(_compiledShaderMapCache))
		{
			hasCachedBlobs = cachedShaderIt->second == shaderFoundTimestamp;
			if (!hasCachedBlobs)
			{
				LOG_DEBUG << "Shader " << shaderFilePath << " (for function : " << shaderFuncName << ") cache mismatches, therefore should be invalidated.";
			}
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
			Storm::throwException<std::exception>(fullError);
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
			Storm::throwException<std::exception>("Shader compilation errors detected for '" + shaderFilePath + "' (compiling method " + shaderFuncName + ") : " + outCompileErrorMsg);
		}

		// Save the blob file
		std::ofstream{ expectedShaderBlobFilePath.string(), std::ios_base::out | std::ios_base::binary }.write(
			static_cast<const char*>(shaderBlob->GetBufferPointer()),
			shaderBlob->GetBufferSize()
		);

		std::filesystem::last_write_time(expectedShaderBlobFilePath, shaderFoundTimestamp);

		std::lock_guard<std::mutex> lock{ _mutex };
		_compiledShaderMapCache[getShaderIdentifier(shaderFilePath, shaderFuncName)] = std::filesystem::last_write_time(expectedShaderBlobFilePath);

		this->flushCache();
	}

	// Detach because we want transfer the ownership to the client code.
	// Don't forget to use Attach afterwards.
	return shaderBlob.Detach();
}

void* Storm::ShaderManager::requestCompiledShaderBlobs(const std::string &shaderFilePath, const std::string_view &shaderFuncName, const std::string_view &target)
{
	return this->requestCompiledShaderBlobs(shaderFilePath, shaderFuncName, target, Storm::ShaderMacroContainer{});
}

void Storm::ShaderManager::flushCache() const
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const std::filesystem::path tmpPath = configMgr.getTemporaryPath();

	const std::filesystem::path finalShaderCache = getShadersTemporaryPath(tmpPath) / k_shaderCacheFileName;
	const std::filesystem::path finalShaderCacheToRemove = getTemporaryFinalShaderCacheToRemovePath(finalShaderCache);
	const std::filesystem::path toFlushFilePath = getTemporaryFinalShaderCacheToFlush(tmpPath);
	
	{
		std::ofstream fileStream{ toFlushFilePath.string() };
		for (const auto &shaderCache : _compiledShaderMapCache)
		{
			std::string shaderLastCompiledFileTimeStamp = std::to_string(shaderCache.second.time_since_epoch().count());
			fileStream << shaderCache.first << k_shaderCacheSeparator << shaderLastCompiledFileTimeStamp << '\n';
		}
	}

	// The 1st time we run Storm, this file was not created yet.
	if (std::filesystem::exists(finalShaderCache))
	{
		std::filesystem::rename(finalShaderCache, finalShaderCacheToRemove);
	}

	std::filesystem::rename(toFlushFilePath, finalShaderCache);
	std::filesystem::remove_all(finalShaderCacheToRemove);
}
