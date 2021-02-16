

namespace Storm
{
#if __cplusplus
    enum class
#else
    public enum
#endif
        NetworkApplication
    {
        Unknown,

        Storm,
        Storm_ScriptSender,
    }
#if __cplusplus
    ;
#endif
}
