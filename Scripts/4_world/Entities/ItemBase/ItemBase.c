modded class ItemBase
{
    protected bool m_VSM_Virtualized; // esta sendo virtualizado (ao deletar não excluir os arquivos)
    protected bool m_VSM_HasVirtualItems; // verifica se existem item no virtual
    protected bool m_VSM_Restoring;
    protected bool m_VSM_VirtualStorageLoaded;

    void ItemBase()
    {
        m_VSM_HasVirtualItems = false;
        m_VSM_VirtualStorageLoaded = false;
        m_VSM_Virtualized = false;
        RegisterNetSyncVariableBool("m_VSM_HasVirtualItems");
    }

    // inicia o autoclose
    void VSM_StartAutoClose()
    {
        if (!CfgGameplayHandler.GetVSM_EnableAutoClose())
            return;

        if (VirtualStorageModule.GetModule().m_Debug)
            Print("Init auto close: " + GetType());
            
        if (VSM_CanVirtualize() && VSM_IsOpen() &&)
        {
            VSM_StopAutoClose();
            int timeToClose = CfgGameplayHandler.GetVSM_TimeToAutoClose() * 1000;
            
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("Starting auto close: " + GetType() + " for seconds:" + timeToClose);

            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.VSM_Close, timeToClose, false, false);
        }
    }

    // para o autoclose
    void VSM_StopAutoClose()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.VSM_Close);
    }

    bool VSM_CanAutoClose()
    {
        return true;
    }

    // existe itens virtuais?
    bool VSM_HasVirtualItems()
    {
        return m_VSM_HasVirtualItems;
    }

    // defini o estado dos itens (auto)
    void VSM_SetHasItems(bool has)
    {
        m_VSM_HasVirtualItems = has;
        SetSynchDirty();
    }

    // esta carregado? (auto)
    bool VSM_IsLoaded()
    {
        return m_VSM_VirtualStorageLoaded;
    }

    // defini o estado de carregado (auto)
    void VSM_SetVirtualLoaded(bool loaded)
    {
        m_VSM_VirtualStorageLoaded = loaded;
    }

    bool VSM_IsRestoring()
    {
        return m_VSM_Restoring;
    }

    void VSM_SetIsRestoring(bool restoring)
    {
        m_VSM_Restoring = restoring;
    }

    // pode ser virtualizado?
    bool VSM_CanVirtualize()
    {
        return !CfgGameplayHandler.GetVSM_IncludeDecayItems();
    }

    // deletar os arquivos ao excluir este item?
    bool VSM_CanDeleteVirtualFile()
    {
        return m_VSM_Virtualized;
    }

    // retorna o atual estado de virtualização do storage (true= fechado/virtualizado, false= aberto/restaurado)
    bool VSM_IsVirtualized()
    {
        return m_VSM_Virtualized;
    }

    void VSM_SetVirtualized(bool virtualized)
    {
        m_VSM_Virtualized = virtualized;
    }

    // esta aberto ?
    bool VSM_IsOpen()
    {
        return false;
    }

    // id de virtualização
    string VSM_GetId()
    {

        int persistent1;
        int persistent2;
        int persistent3;
        int persistent4;
        string id;

        GetPersistentID(persistent1, persistent2, persistent3, persistent4);
        id = GetType() + "_" + VSM_NormalizeSingleId(persistent1) + VSM_NormalizeSingleId(persistent2) + VSM_NormalizeSingleId(persistent3) + VSM_NormalizeSingleId(persistent4);
        return id;
    }

    string VSM_NormalizeSingleId(int id)
    {
        string strId = id.ToString();
        strId.Replace("-", "");
        return strId;
    }

    // abrir o storage
    void VSM_Open()
    {
        if (VSM_IsOpen())
            VSM_StartAutoClose();
    }

    // fechar o storage
    void VSM_Close()
    {
        if (!VSM_IsOpen())
            VSM_StopAutoClose();
    }

    // ao carregar o contexto de variaveis (igual à OnStoreLoad())
    bool VSM_OnVirtualStoreLoad(ParamsReadContext ctx, int version)
    {
        return OnStoreLoad(ctx, version);
    }

    // ao salvar o contexto de variaveis (igual à OnStoreSave())
    void VSM_OnVirtualStoreSave(ParamsWriteContext ctx)
    {
        OnStoreSave(ctx);
    }

    // apos o carregamento de todas as variaveis de contexto (igual à AfterStoreLoad())
    void VSM_AfterVirtualStoreLoad()
    {
        AfterStoreLoad();
    }

    // antes do item ser virtualizado
    void VSM_OnBeforeVirtualize() { }

    // após do item ser virtualizado
    void VSM_OnAfterVirtualize() { }

    // antes dos itens serem restaurados
    void VSM_OnBeforeRestoreChildren() { }

    // antes dos itens serem restaurados
    void VSM_OnAfterRestoreChildren() { }

    // após o item ser restaurado
    void VSM_OnAfterRestore()
    {
        SetSynchDirty();
        Update();
    }

    // antes de iniciar a virtualização deste storage
    void VSM_OnBeforeContainerVirtualize() { }

    // após o término da virtualização deste storage
    void VSM_OnAfterContainerVirtualize()
    {
        VSM_StopAutoClose();
        VSM_SetVirtualized(true);
    }

    // antes de iniciar a restauração deste storage
    void VSM_OnBeforeContainerRestore()
    {
        VSM_SetIsRestoring(true);
    }

    // após o término da restauração deste storage
    void VSM_OnAfterContainerRestore()
    {
        VSM_SetIsRestoring(false);
        VSM_StartAutoClose();
        VSM_SetVirtualized(false);
    }
    

    // se estiver em um carro
    bool VSM_IsAttachedOnVehicle()
    {
        EntityAI parent = EntityAI.Cast(GetHierarchyParent());
        if (parent && parent.IsInherited(CarScript))
        {
            InventoryLocation location = new InventoryLocation();
            if (GetInventory().GetCurrentInventoryLocation(location))
            {
                return location.GetType() == InventoryLocationType.ATTACHMENT;
            }
        }

        return false;
    }
}