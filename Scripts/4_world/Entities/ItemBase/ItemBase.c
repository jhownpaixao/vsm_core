modded class ItemBase
{
    protected bool m_VSM_HasVirtualItems; // verifica se existem item no virtual
    protected bool m_VSM_ProcessingItems;
    protected bool m_VSM_VirtualStorageLoaded;

    void ItemBase()
    {
        m_VSM_HasVirtualItems = false;
        m_VSM_VirtualStorageLoaded = false;
        RegisterNetSyncVariableBool("m_VSM_HasVirtualItems");
    }

    override bool CanReleaseCargo(EntityAI cargo)
    {
        if (VSM_IsProcessing() || VSM_IsOpen())
            return false;

        return super.CanReleaseCargo(cargo);
    }

    override bool CanDisplayCargo()
    {
        if (VSM_IsProcessing() || VSM_IsOpen())
            return false;

        return super.CanDisplayCargo();
    }

    override bool CanPutInCargo(EntityAI parent)
    {
        if (VSM_IsProcessing())
            return false;

        return super.CanPutInCargo(parent);
    }

    override bool CanReceiveItemIntoCargo(EntityAI item)
    {
        if (VSM_IsProcessing() || VSM_IsOpen())
            return false;

        return super.CanReceiveItemIntoCargo(item);
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

    bool VSM_IsProcessing()
    {
        return m_VSM_ProcessingItems;
    }

    void VSM_SetIsProcessing(bool processing)
    {
        m_VSM_ProcessingItems = processing;
    }

    // pode ser virtualizado?
    bool VSM_CanVirtualize()
    {
        bool includeDecayItems = CfgGameplayHandler.GetVSM_IncludeDecayItems();
        if (!includeDecayItems && CanDecay())
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("VSM_CanVirtualize ignorado: " + GetType() + " CanDecay");
            return false;
        }

        ref TStringArray ignoredItems = CfgGameplayHandler.GetVSM_IgnoredItems();
        if (ignoredItems.Find(GetType()) > -1)
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("VSM_CanVirtualize ignorado: " + GetType() + " ignoredItems");
            return false;
        }

        return true;
    }

    // deletar os arquivos ao excluir este item?
    bool VSM_CanDeleteVirtualFile()
    {
        return true;
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
    void VSM_Open() { }

    // fechar o storage
    void VSM_Close() { }

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
    }

    // antes de iniciar a restauração deste storage
    void VSM_OnBeforeContainerRestore()
    {
        VSM_SetIsProcessing(true);
    }

    // após o término da restauração deste storage
    void VSM_OnAfterContainerRestore()
    {
        VSM_SetIsProcessing(false);
        VSM_StartAutoClose();
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

    // inicia o autoclose
    void VSM_StartAutoClose()
    {
        bool enableAutoClose = CfgGameplayHandler.GetVSM_AutoCloseEnable();
        if (!enableAutoClose || !VSM_CanVirtualize() || !VSM_CanAutoClose() || !VSM_IsOpen())
            return;

        VSM_StopAutoClose();

        int timeToClose = CfgGameplayHandler.GetVSM_AutoCloseInterval() * 1000;
        if (VirtualStorageModule.GetModule().m_Debug)
            Print("Starting auto close: " + GetType() + " for seconds:" + timeToClose);

        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.VSM_OnAutoClose, timeToClose, false, false);
    }

    // para o autoclose
    void VSM_StopAutoClose()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.VSM_OnAutoClose);
    }

    void VSM_OnAutoClose()
    {
        bool ignorePlayersNearby = CfgGameplayHandler.GetVSM_AutoCloseIgnorePlayerNearby();
        float playerDistance = CfgGameplayHandler.GetVSM_AutoCloseIgnorePlayerNearby();

        if (!ignorePlayersNearby && VirtualUtils.IsPlayerNearby(GetPosition(), playerDistance))
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("VSM_OnAutoClose: " + GetType() + " ignorando, players proximos");

            VSM_StartAutoClose();
            return;
        }


        VSM_Close();
    }
}