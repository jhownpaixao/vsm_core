
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
    static const string METADATA_FILENAME = "meta.json";
	static const string VIRTUAL_FILENAME = "main.bin";

}

bool VSM_IsNew()
{
    
	int instanceId = GetGame().ServerConfigGetInt("instanceId");
	string currentStorage = "storage_" + instanceId;
	string virtualPath = "$mission:" + currentStorage + "\\virtual\\";

    return !FileExist(virtualPath);
}