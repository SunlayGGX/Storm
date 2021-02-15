
namespace Storm
{
#if __cplusplus
    enum class
#else
    enum
#endif
        NetworkMessageType
    {
        None,
        Script,
    }
#if __cplusplus
    ;
#endif
}
