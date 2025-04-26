class VSM_DropQueue : VSM_RestorationQueue
{
    
    override bool OnInit()
    {
        bool can = super.OnInit();
        m_ForceSpawnOnGround = true;
        return can;
    }

    override void OnStart()
    {
        super.OnStart();
        VSM_Info("OnStart", m_Container.GetType() + " iniciando processo de drop de itens items: " + m_ItemCount);
    }

    override void OnComplete()
    {
        super.OnComplete();
        VirtualStorageModule.GetModule().OnDeleteContainer(m_Container);
        VSM_Info("OnComplete", m_Container.GetType() + " drop de itens concluido");
    }
}