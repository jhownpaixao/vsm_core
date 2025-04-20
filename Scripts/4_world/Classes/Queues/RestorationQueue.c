class VSM_RestorationQueue extends BatchQueue_Base
{
    protected bool m_ForceSpawnOnGround;
    protected ref array<ref VirtualObjectContext> m_Items;
    protected ref array<ItemBase> m_SpawnedItems;

    protected ref array<ref VirtualObject> m_ProcessedItems;

    void OnInit( array<ref VirtualObjectContext> items)
    {
        Print("VSM_RestorationQueue item:" + items);
        m_Items = items;
        m_ItemCount = m_Items.Count();
        m_ForceSpawnOnGround = false;
        m_SpawnedItems = new array<ItemBase>;
        super.OnInit(); //debug

        m_Metadata.SetLastOperationState(VirtualStorageState.RESTORING);
    }

    override void OnStart()
    {
        super.OnStart();
        m_Container.VSM_OnBeforeContainerRestore(); //prepare
    }

    override void OnTick(int idx)
    {
        VirtualObjectContext item = m_Items.Get(idx);
        string virtualFile = m_VirtualContextDirectory + item.contextFile;
        if (!FileExist(virtualFile))
        {
            VSM_Error("OnTick", "O arquivo virtual não existe " + virtualFile);
            return;
        }

        FileSerializer ctx = new FileSerializer();
		VirtualStorageFile virtualStorage = new VirtualStorageFile();
        if (!ctx.Open(virtualFile, FileMode.READ))
        {
            VSM_Error("OnTick", "Não foi possível abrir o arquivo virtual" + virtualFile);
            return;
        }

        VirtualObject obj = new VirtualObject(m_VirtualContextDirectory, m_Container);
        ItemBase restored = obj.OnRestore(ctx, m_ForceSpawnOnGround);

        if(restored)
            m_SpawnedItems.Insert(restored);
    }

    override void OnComplete()
    {
        super.OnComplete();
        m_Container.VSM_OnAfterContainerRestore();
        m_Container.VSM_SetHasItems(false);
        m_Container.VSM_SetIsProcessing(false);
        m_Metadata.SetLastOperationState(VirtualStorageState.RESTORED);

        foreach (VirtualObject obj : m_ProcessedItems) {
            if (obj) obj.OnRestoreComplete();
        }
    }

    override void OnRollback()
    {
        m_Container.VSM_SetIsProcessing(true);
        foreach (ItemBase item : m_SpawnedItems) {
			if(item)
                item.Delete();
		}
        m_Container.VSM_Close();
        m_Container.VSM_SetIsProcessing(false);
        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZED);
    }
    
    override void OnRestart()
    {
        ClearInventory();
        m_Container.VSM_Open();
    }

    override void OnSync()
    {
        m_Container.VSM_SetIsProcessing(true);
        m_Container.VSM_Open();
        m_Container.VSM_SetIsProcessing(false);
    }
}
