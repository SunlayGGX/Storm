# ifndef __cplusplus
using System;
#endif


namespace Storm
{
    class NetworkConstants
    {
# ifdef __cplusplus
    public:
        using string = std::string_view;
        using UInt32 = uint32_t;
        using UInt16 = uint16_t;

#   pragma push_macro("const")
#   define const constexpr

#   pragma push_macro("public")
#   define public
#endif

        public static const string k_networkVersion = "1.0.0";

        public static const string k_networkSeparator = "/|||/";
        public static const UInt32 k_magicKeyword = 0xFABC770C;

        public static const UInt16 k_defaultScriptSenderPort = 24139;

        public static const string k_endOfMessageCommand = "++EOM_COM++";

    }
#ifdef __cplusplus
    ;

#   undef const
#   pragma pop_macro("const")

#   undef public
#   pragma pop_macro("public")
#endif

}
