# if !__cplusplus
using System;
#endif


namespace Storm
{
#if !__cplusplus
    public
#endif
        class NetworkConstants
    {
#if __cplusplus
    public:
        using string = std::string_view;
        using Int32 = int32_t;
        using UInt16 = uint16_t;
#endif
        
        // ------------------- Network Version -------------------- //
#if __cplusplus
        constexpr static
#else
        public
#endif
            const string k_networkVersion = "1.0.0";


        // ------------------- Network Separator -------------------- //
#if __cplusplus
        constexpr static
#else
        public
#endif
            const string k_networkSeparator = "/|||/";


        // ------------------- Network Magic Keyword -------------------- //
#if __cplusplus
        constexpr static
#else
        public
#endif
            const Int32 k_magicKeyword = 0x7ABC7F0C;


        // ------------------- Network Default sender port -------------------- //
#if __cplusplus
        constexpr static
#else
        public
#endif
            const UInt16 k_defaultScriptSenderPort = 5007;


        // ------------------- Network End of message token -------------------- //
#if __cplusplus
        constexpr static
#else
        public
#endif
            const string k_endOfMessageCommand = "++EOM_COM++";

    }
#if __cplusplus
    ;
#endif

}
