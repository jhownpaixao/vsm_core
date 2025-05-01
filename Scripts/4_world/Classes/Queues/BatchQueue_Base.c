class BatchQueue_Base : VSM_Base
{
    protected int m_TickInterval;
    protected int m_BatchSize;
    protected int m_CurrentIdx;
    protected string m_VirtualContextDirectory;
    protected string m_VirtualFilePath;
    protected ref VirtualMetadata m_Metadata;
    protected ItemBase m_Container;
    protected int m_ItemCount;
    
    void BatchQueue_Base(ItemBase container)
    {
        m_TickInterval = CfgGameplayHandler.GetVSM_BatchInterval() * 1000;
        m_BatchSize = CfgGameplayHandler.GetVSM_BatchSize();
        m_Container = container;
        m_CurrentIdx = 0;
        m_VirtualContextDirectory = VirtualStorageModule.GetModule().GetVirtualContextDirectory(m_Container);
        m_VirtualFilePath = VirtualStorageModule.GetModule().GetVirtualFile(container);
        m_Metadata = new VirtualMetadata(VirtualStorageModule.GetModule().GetVirtualMetadataFile(container));
    }

    bool OnInit()
    {
        VSM_Debug("OnInit", m_Container.GetType() + " proceso iniciado");
        m_Metadata.OnInit();
        return true;
    }

    void OnStart() { }

    void OnTick(int idx) { }

    void OnAfterBatchProcess(int startIdx, int endIdx) { }

    void OnStop()
    {
        VSM_Debug("OnStop", m_Container.GetType() + " proceso parado");
    }

    void OnComplete()
    {
        VSM_Debug("OnComplete", m_Container.GetType() + " proceso completado");
        m_Metadata.Save();
        VirtualStorageModule.GetModule().RemoveActiveQueue(m_Container);
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
            Stop();
            OnComplete();
        }
    }

    void Start()
    {
        if(!OnInit())
            return;

        OnStart();

        Tick(); // tick inicial
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Tick, m_TickInterval, true);
    }

    void Stop()
    {
        OnStop();
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

        foreach (EntityAI entity : items) {
            ItemBase item = ItemBase.Cast(entity);
            if (item && item.VSM_CanVirtualize()) {
                item.Delete();
            }
        }
    }
}