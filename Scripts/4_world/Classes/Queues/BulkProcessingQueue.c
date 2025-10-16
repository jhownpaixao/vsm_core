class VSMBulkProcessingQueue : VSMBase
{
    protected int                   m_TickInterval;
    protected int                   m_BatchSize;
    protected int                   m_CurrentIdx;
    protected string                m_VirtualContextDirectory;
    protected string                m_VirtualFilePath;
    protected string                m_VirtualCtxFilePath;
    protected ref VSMMetadata       m_Metadata;
    protected ItemBase              m_Container;
    protected int                   m_ItemCount;

    protected ref FileSerializer    m_Ctx;
    protected ref FileSerializer    m_StoreCtx;
    protected bool                  m_Completed;
    protected bool                  m_Processing;

    void VSMBulkProcessingQueue(ItemBase container)
    {
        m_Container                 = container;

        m_TickInterval              = VSM_Settings.GetSettings().m_BatchInterval;
        m_BatchSize                 = VSM_Settings.GetSettings().m_BatchSize;
        m_CurrentIdx                = 0;
        m_Processing                = false;

        m_VirtualContextDirectory   = VirtualStorageModule.GetModule().GetVirtualContextDirectory(m_Container);
        m_VirtualFilePath           = VirtualStorageModule.GetModule().GetVirtualFile(container);
        m_VirtualCtxFilePath        = VirtualStorageModule.GetModule().GetVirtualCtxFile(container);

        VSM_Debug("VSMBulkProcessingQueue", "VirtualContextDirectory: " + m_VirtualContextDirectory);
        VSM_Debug("VSMBulkProcessingQueue", "VirtualFilePath: " + m_VirtualFilePath);
        VSM_Debug("VSMBulkProcessingQueue", "VirtualCtxFilePath: " + m_VirtualCtxFilePath);
        
        m_Metadata                  = VirtualStorageModule.GetModule().GetMetadata(container);
        m_Ctx                       = new FileSerializer();
        m_StoreCtx                  = new FileSerializer();
    }

    void ~VSMBulkProcessingQueue()
    {
        if (m_Ctx && m_Ctx.IsOpen())
            m_Ctx.Close();

        if (m_StoreCtx && m_StoreCtx.IsOpen())
            m_StoreCtx.Close();

        m_Container = null;
        m_Metadata = null;
        m_Ctx = null;
        m_StoreCtx = null;
    }

    bool OnInit()
    {
        VSM_Debug("OnInit", m_Container.GetType() + " processamento iniciado");
        m_Metadata.OnInit();

        return true;
    }

    
    void OnStart() { }
    void OnTick(int idx) { }
    void OnAfterBatchProcess(int startIdx, int endIdx) { }
    void OnStop()
    {
        VSM_Debug("OnStop", m_Container.GetType() + " processamento parado");
    }
    void OnComplete()
    {
        VSM_Debug("OnComplete", m_Container.GetType() + " processamento completado");
    }
    void OnCancel() { }
    void OnRollback() { }
    void OnRestart() { }
    void OnSync() { }

    void Tick()
    {
        if (!m_Container)
        {
            Stop();
            return;
        }

        int endIdx;

        if (VirtualStorageModule.GetModule().m_IsMissionFinishing)
            endIdx = m_ItemCount;
        else
            endIdx = Math.Min(m_CurrentIdx + m_BatchSize, m_ItemCount);

        for (int i = m_CurrentIdx; i < endIdx; i++)
        {
            OnTick(i);
        }

        OnAfterBatchProcess(m_CurrentIdx, endIdx);

        m_CurrentIdx = endIdx;
        if (m_CurrentIdx >= m_ItemCount)
        {
            m_Completed = true;
            Stop();
            OnComplete();
            m_Metadata.Save();
            return;
        }

        g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Tick, m_TickInterval, false);
    }

    void Start()
    {
        if(!OnInit()) return;

        OnStart();

        g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Call(this.Tick);
    }

    void Stop()
    {
        OnStop();
        
        VirtualStorageModule.GetModule().RemoveActiveQueue(m_Container);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.Tick);
    }

    void Cancel()
    {
        Stop();
        OnCancel();
        OnRollback();
    }

    void Restart()
    {
        OnRestart();
        Start();
    }

    void ClearInventory()
    {
        array<EntityAI> items = new array<EntityAI>;
		m_Container.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);
        int total = items.Count();

        if(total == 0) return;

        array<ItemBase> itemsToDelete = new array<ItemBase>;
        ItemBase item;
        for (int i = 0; i < total; i++)
        {
            EntityAI entity = items[i];
            if(!entity) continue;

            item = ItemBase.Cast(entity);
            if (item && item.VSM_IsVirtualizable()) 
                itemsToDelete.Insert(item);
        }

        //deletar em ordem reversa (de filho para pai)
        for (int j = itemsToDelete.Count() - 1; j >= 0; j--)
        {
            item = itemsToDelete[j];
            if(item)
                item.Delete();
        }
    }
}