/**
 * @brief Classe de metadados virtuais
 * @note Esta classe é usada para armazenar metadados dos arquivos virtuais, como estado, versão e itens.
 */
class VSMMetadata : VSMBase
{
    int                         m_Version;
    string                      m_VirtualId;
    string                      m_LastDate;
    VirtualStorageState         m_State;
    vector                      m_Position;
    int                         m_ItemCount;
    ref array<ref VSMMetadataItem>  m_Items;

    [NonSerialized()]
    private bool m_IsNew;
    [NonSerialized()]
    private string m_Path;

    void VSMMetadata(string path)
    {
        m_Path = path;
        m_IsNew = true;
        m_Items = new array<ref VSMMetadataItem>;
        m_State = VirtualStorageState.NONE;

        CF_Date now = CF_Date.Now();
        m_LastDate = now.Format(CF_Date.DATETIME);
    }

    void OnInit()
    {
        Load();
    }

    bool IsRestored()
    {
        return m_State == VirtualStorageState.RESTORED;
    }

    bool IsRestoring()
    {
        return m_State == VirtualStorageState.RESTORING;
    }

    bool IsVirtualized()
    {
        return m_State == VirtualStorageState.VIRTUALIZED;
    }

    bool IsVirtualizing()
    {
        return m_State == VirtualStorageState.VIRTUALIZING;
    }

    void Save()
    {
        JsonFileLoader<VSMMetadata>.JsonSaveFile(m_Path, this);
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
        JsonFileLoader<VSMMetadata>.JsonLoadFile(m_Path, this);
    }

    void SetLastOperationState(VirtualStorageState state)
    {
        m_State = state;
        Save();
    }

    VirtualStorageState GetLastState()
    {
        return m_State;
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

    string GetVirtualId()
    {
        return m_VirtualId;
    }

    void SetVirtualId(string id)
    {
        m_VirtualId = id;
    }

    void SetPosition(vector position)
    {
        m_Position = position;
    }

    vector GetPosition()
    {
        return m_Position;
    }

    void AddItem(VSMMetadataItem item)
    {
        m_Items.Insert(item);
    }

    void ClearItems()
    {
        m_Items.Clear();
    }

    bool HasItem(VSMMetadataItem item)
    {
        return m_Items.Find(item) != -1;
    }

    int GetItemCount()
    {
        return m_ItemCount;
    }

    void SetItemCount(int count)
    {
        m_ItemCount = count;
    }

    bool IsOutdated()
    {
        return m_Version < VSM_StorageVersion.CURRENT_VERSION;
    }

    string GetPath()
    {
        return m_Path;
    }
}

class VSMMetadataItem
{
    string m_Classname;
    ref array<ref VSMMetadataItem> m_Children;

    void VSMMetadataItem()
    {
        m_Children = new array<ref VSMMetadataItem>;
    }

    void AddChild(VSMMetadataItem child)
    {
        m_Children.Insert(child);
    }
}