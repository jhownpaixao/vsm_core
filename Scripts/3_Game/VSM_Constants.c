
class VSM_StorageVersion
{
    //! @brief Versões de compatibilidade do VSM
    static const int V_2504 = 100;

    //!@note: utilize para a versão mais recente
    static const int COMPATIBILITY_VERSION = V_2504;
    static const int CURRENT_VERSION = V_2504;
};

class VSM_Constants
{
    static bool m_IsNew;
    static bool m_IsNewInitialized = false;

    static const string METADATA_FILENAME = "meta.json";
    static const string VIRTUAL_FILENAME = "main.bin";

}

enum VSM_RPCTypes
{
    INVALID = 8830,
    MENUCTX_CLOSE_CONTAINER,
    MENUCTX_OPEN_CONTAINER,
    COUNT
};