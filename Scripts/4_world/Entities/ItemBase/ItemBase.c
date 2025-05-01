modded class ItemBase
{
    protected bool m_VSM_HasVirtualItems; // verifica se existem item no virtual
    protected bool m_VSM_ProcessingItems; // utilizado para travar o container
    protected bool m_VSM_VirtualStorageLoaded;

    void ItemBase()
    {
        m_VSM_HasVirtualItems = false;
        m_VSM_VirtualStorageLoaded = false;
        RegisterNetSyncVariableBool("m_VSM_HasVirtualItems");
        RegisterNetSyncVariableBool("m_VSM_ProcessingItems");
        RegisterNetSyncVariableBool("m_VSM_VirtualStorageLoaded");
    }

    override bool CanReceiveItemIntoCargo(EntityAI item)
    {
        if (VSM_IsProcessing())
            return false;

        return super.CanReceiveItemIntoCargo(item);
    }

    override bool CanReleaseCargo(EntityAI cargo)
    {
        if (VSM_IsProcessing())
            return false;

        return super.CanReleaseCargo(cargo);
    }

    override bool CanReceiveAttachment(EntityAI attachment, int slotId) //impede o proprio container de restaurar os itens
    {
        //!desativar por enquanto, está impedindo a criação de attachments mesmo vindo do módulo de virtualização
        //TODO: formular um método de criação dos attachments apartir do módulo, ao mesmo tempo que não permite o player mexer...
        /* if (VSM_IsProcessing())
            return false; */

        return super.CanReceiveAttachment(attachment, slotId);
    }

    override bool CanReleaseAttachment( EntityAI attachment )
    {
        if (VSM_IsProcessing())
            return false;

        return super.CanReleaseAttachment(attachment);
    }

    override bool CanDisplayAttachmentSlot(int slot_id)
    {
         if (VSM_IsProcessing())
            return false;
        
         return super.CanDisplayAttachmentSlot(slot_id);
    }

    override bool CanDisplayCargo()
    {
        if (VSM_IsProcessing())
        {
            return false; 
        }
           
        return super.CanDisplayCargo();
    }

    override bool CanPutInCargo(EntityAI parent)
    {
        if (VSM_IsProcessing())
            return false;

        return super.CanPutInCargo(parent);
    }

    override bool CanDisplayAttachmentCategory( string category_name )
	{
        if (VSM_IsProcessing())
        {
            return false; 
        }

        return super.CanDisplayAttachmentCategory(category_name);
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
        SetSynchDirty();
    }

    bool VSM_IsProcessing()
    {
        return m_VSM_ProcessingItems;
    }

    void VSM_SetIsProcessing(bool processing)
    {
        m_VSM_ProcessingItems = processing;
        SetSynchDirty();
    }

    // pode ser virtualizado?
    bool VSM_CanVirtualize()
    {
        bool includeDecayItems = CfgGameplayHandler.GetVSM_IncludeDecayItems();
        if (!includeDecayItems && CanDecay())
            return false;
        
        if(VirtualStorageModule.GetModule().IsIgnoredItem(GetType()))
            return false;
    
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

    // antes da virtualização dos filhos
    void VSM_OnBeforeVirtualizeChildren() { }

    // após a virtualização dos filhos
    void VSM_OnAfterVirtualizeChildren() { }

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

    void VSM_StartAutoClose()
    {
        bool enableAutoClose = CfgGameplayHandler.GetVSM_AutoCloseEnable();
        if (!enableAutoClose || !VSM_CanVirtualize() || !VSM_CanAutoClose() || !VSM_IsOpen())
            return;

        VSM_StopAutoClose();

        int timeToClose = CfgGameplayHandler.GetVSM_AutoCloseInterval() * 1000;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.VSM_OnAutoClose, timeToClose, false, false);
    }

    void VSM_StopAutoClose()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.VSM_OnAutoClose);
    }

    void VSM_OnAutoClose()
    {
        bool ignorePlayersNearby = CfgGameplayHandler.GetVSM_AutoCloseIgnorePlayerNearby();
        float playerDistance = CfgGameplayHandler.GetVSM_AutoClosePlayerDistance();

        if (!ignorePlayersNearby && VirtualUtils.IsPlayerNearby(GetPosition(), playerDistance))
        {
            VSM_StartAutoClose();
            return;
        }

        VSM_Close();
    }

    bool VSM_CanManipule()
    {
        if (VSM_IsProcessing())
            return false;

        if (!VSM_IsOpen())
            return false;

        return true;
    }

    bool VSM_CanOpen()
    {
        return !VSM_IsProcessing() && !VSM_IsOpen();
    }

    bool VSM_CanClose()
    {
        return !VSM_IsProcessing() && VSM_IsOpen();
    }
}