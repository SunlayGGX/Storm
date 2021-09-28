#if !__cplusplus
using System;
#endif

namespace Storm
{
#if !__cplusplus
	public
#endif
		class MacroTags
    {
#if __cplusplus
    public:
        using string = std::string_view;
#endif


#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormExe = "StormExe";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormFolderExe = "StormFolderExe";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormRoot = "StormRoot";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormConfig = "StormConfig";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormResource = "StormResource";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormIntermediate = "StormIntermediate";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormRecord = "StormRecord";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormStates = "StormStates";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormScripts = "StormScripts";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormDebug = "StormDebug";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormArchive = "StormArchive";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_StormTmp = "StormTmp";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_DateTime = "DateTime";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_Date = "Date";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_PID = "PID";

#if __cplusplus
        constexpr static
#else
		public
#endif
			const string k_builtInMacroKey_ComputerName = "ComputerName";
	}
#if __cplusplus
    ;
#endif
}
