class VirtualMetadata : VSM_Base
{
    string m_Path;
    string m_VirtualId;
    string m_MetaFile;
    string m_LastDateOperation;
    int m_Version;
    private bool m_IsNew;

    VirtualStorageState m_LastOperationState;

    //container
    vector m_Position;

    void VirtualMetadata(string path)
    {
        m_Path = path;
        m_IsNew = true;
    }

    void OnInit()
    {
        Load();
    }

    bool IsRestored()
    {
        return m_LastOperationState == VirtualStorageState.RESTORED;
    }

    bool IsRestoring()
    {
        return m_LastOperationState == VirtualStorageState.RESTORING;
    }

    bool IsVirtualized()
    {
        return m_LastOperationState == VirtualStorageState.VIRTUALIZED;
    }

    bool IsVirtualizing()
    {
        return m_LastOperationState == VirtualStorageState.VIRTUALIZING;
    }

    void Save()
    {
        JsonFileLoader<VirtualMetadata>.JsonSaveFile(m_Path, this);
    }

    void Load()
    {
        if (!FileExist(m_Path))
        {
            VSM_Warn("OnInit", "O arquivo de metadata não existe");
            m_IsNew = true;
            return;
        }

         m_IsNew = false;
        JsonFileLoader<VirtualMetadata>.JsonLoadFile(m_Path, this);
    }

    void SetLastOperationState(VirtualStorageState state)
    {
        m_LastOperationState = state;
        Save();
    }

    VirtualStorageState GetLastState()
    {
        return m_LastOperationState;
    }

    bool IsNew()
    {
        return m_IsNew;
    }

    int GetVersion()
    {
        if(!m_Version)
            m_Version = VSM_StorageVersion.COMPATIBILITY_VERSION;
        
        return m_Version;
    }

    void SetVersion(int version)
    {
        m_Version = version;
    }

}

enum VirtualStorageState
{
    NONE,
    RESTORED,
    VIRTUALIZED,
    RESTORING,
    VIRTUALIZING
}