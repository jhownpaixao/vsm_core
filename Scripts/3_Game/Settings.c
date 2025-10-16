class VSM_Settings
{
    [NonSerialized()]
    static ref VSM_Settings     m_Instance;

    int                         m_Version;

    bool                        m_AutoCloseEnable;
	bool                        m_AutoCloseIgnorePlayerNearby;
	float                       m_AutoClosePlayerDistance;
	int                         m_AutoCloseInterval;

	int                         m_BatchSize;
	float                       m_BatchInterval;
	bool                        m_IncludeDecayItems;
	ref TStringArray            m_IgnoredItems;
	bool                        m_UseCfgIgnoreList;
	bool                        m_LogLevel;
	bool                        m_ForceSpawnOnError;
	bool                        m_IsRemovingModule;
    bool                        m_EnableAutoRestoreState;
    ref VSM_SettingsGameLabs    m_GameLabsIntegration;

    void VSM_Settings()
    {
        CheckPaths();

        if (FileExist(VSM_Constants.SETTING_FILE))
            LoadConfig();
        else
            OnDefaults();

        CheckMigration();
    }

    void CheckPaths()
    {
        if (!FileExist(VSM_Constants.ITZ_PROFILE_FOLDER_NAME))
            MakeDirectory(VSM_Constants.ITZ_PROFILE_FOLDER_NAME);
        
        if (!FileExist(VSM_Constants.GENERAL_PATH))
            MakeDirectory(VSM_Constants.GENERAL_PATH);
    }

    void LoadConfig()
    {
        JsonFileLoader<VSM_Settings>.JsonLoadFile(VSM_Constants.SETTING_FILE, this);
    }

    void SaveConfig()
    {
        JsonFileLoader<VSM_Settings>.JsonSaveFile(VSM_Constants.SETTING_FILE, this);
    }

    void OnDefaults()
    {
        Print("[ITZ][VSM] Nenhuma configuração encontrada, criando arquivo de configuração com valores padrão.");
        m_AutoCloseEnable = true;
        m_AutoCloseIgnorePlayerNearby = false;
        m_AutoClosePlayerDistance = 8.0; //mts
        m_AutoCloseInterval = 120; //s

	    m_BatchSize = 250;
	    m_BatchInterval = 500; //ms
	    m_IncludeDecayItems = false;

	    m_IgnoredItems = new TStringArray;

	    m_UseCfgIgnoreList = false;
	    m_LogLevel = VSM_LogLevel.ERROR;
	    m_ForceSpawnOnError = false;
	    m_IsRemovingModule = false;
        m_EnableAutoRestoreState = false;
        m_GameLabsIntegration = new VSM_SettingsGameLabs();

        SaveConfig();
    }

    void CheckMigration()
    {
        if(m_Version == VSM_ConfigVersion.CURRENT_VERSION)
        {
            return;
        }

        Print("[ITZ][VSM] Migrando configuração de: " + m_Version.ToString() + " para: " + VSM_ConfigVersion.CURRENT_VERSION.ToString());

        if (m_Version < VSM_ConfigVersion.V_2408)
        {
            m_EnableAutoRestoreState = false;
        }

        if (m_Version < VSM_ConfigVersion.V_0809)
        {
            m_GameLabsIntegration = new VSM_SettingsGameLabs();
        }

        if (m_Version < VSM_ConfigVersion.V_1409)
        {
            m_BatchSize = 250; //items
	        m_BatchInterval = 500; //ms
        }

        // update current version
        m_Version = VSM_ConfigVersion.CURRENT_VERSION;
        SaveConfig();
    }

    static VSM_Settings GetSettings()
    {
        if (!m_Instance)
        {
            m_Instance = new VSM_Settings();
        }
            
        return m_Instance;
    }
}

class VSM_SettingsGameLabs
{
    bool m_EnableIntegration           = false;
    bool m_LogStorageTakeAction        = true;
    bool m_LogStorageOpenAction        = true;
    bool m_LogStorageCloseAction       = true;
    bool m_LogStorageDestroyAction     = false;
} 