class VSM_RestorationQueue
{
    protected bool m_Debug;
    protected int m_TickInterval;
    protected int m_BatchSize;
    protected int m_CurrentIdx;
    protected string m_VirtualContextDirectory;

    protected ItemBase m_Container;
    ref array<ref VirtualObject> m_Items;

    void VSM_RestorationQueue(ItemBase container, array<ref VirtualObject> items)
    {
        m_TickInterval = CfgGameplayHandler.GetVSM_BatchInterval() * 1000;
        m_BatchSize = CfgGameplayHandler.GetVSM_BatchSize();
        m_Container = container;
        m_Items = items;
        m_CurrentIdx = 0;
        m_VirtualContextDirectory = VirtualStorageModule.GetModule().GetVirtualContextDirectory(m_Container);
        m_Debug = VirtualStorageModule.GetModule().m_Debug;
    }

    void ~VSM_RestorationQueue()
    {
        Print("handle destruido");
    }

    void OnInit()
    {
        if (m_Debug)
        {
            Print("VSM_RestorationQueue: iniciando -----");
            Print(m_TickInterval);
            Print(m_BatchSize);
            Print(m_Container);
            Print(m_Items);
            Print(m_CurrentIdx);
            Print(m_VirtualContextDirectory);
            Print("-------------------------------------");
        }
    }

    void OnStart() { 
         if (m_Debug)
            Print("OnStart " + m_Container.GetType());
    }

    void OnTick()
    {
         
        if (!m_Container && !m_Container.IsAlive())
        {
            Stop();
            return;
        }

        int endIdx = Math.Min(m_CurrentIdx + m_BatchSize, m_Items.Count());
        
        if (m_Debug)
            Print("OnTick " + m_Container.GetType() + " m_CurrentIdx="+m_CurrentIdx+" endIdx="+endIdx);


        for (int i = m_CurrentIdx; i < endIdx; i++)
        {
            VirtualObject item = m_Items.Get(i);
            if (m_Debug)
					Print("OnTick " + m_Container.GetType() + " restoring " + item.m_Classname);
            item.OnRestore(m_VirtualContextDirectory, m_Container);
        }

        m_CurrentIdx = endIdx;

        if (m_CurrentIdx >= m_Items.Count())
        {
            if (m_Debug)
                Print("OnTick " + m_Container.GetType() + " PARANDO m_CurrentIdx="+m_CurrentIdx+" endIdx="+endIdx);
            
            Stop();
            OnComplete();
        }
    }

    void OnStop()
    {
        VirtualStorageModule.GetModule().RemoveActiveRestoration(this);
        m_Container.VSM_OnAfterContainerRestore();
		m_Container.VSM_SetIsProcessing(false);

        if (m_Debug)
            Print("VSM_RestorationQueue " + m_Container.GetType() + " carregamento parado");
    }

    void OnComplete()
    {
        m_Container.VSM_OnAfterContainerRestore();
        m_Container.VSM_SetHasItems(false);
		m_Container.VSM_SetIsProcessing(false);

        if (m_Debug)
            Print("VSM_RestorationQueue " + m_Container.GetType() + " carregamento concluído");
    }

    void Start()
    {
        if (m_Debug)
            Print("Start " + m_Container.GetType());

        if (!m_Items || m_Items.Count() == 0)
        {
            OnComplete();
            return;
        }
        OnStart();
        OnTick(); // tick inicial
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.OnTick, m_TickInterval, true);
    }

    void Stop()
    {
        if (m_Debug)
            Print("Stop " + m_Container.GetType());
            
        OnStop();
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.OnTick);
    }

    

}