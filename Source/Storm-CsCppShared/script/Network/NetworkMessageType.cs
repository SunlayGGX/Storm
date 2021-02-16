
namespace Storm
{
#if __cplusplus
    enum class
#else
    public enum
#endif
        NetworkMessageType
    {
        None,
        Script,
        Ping,
    }
#if __cplusplus
    ;
#endif
}
