class VSM_DropQueue : VSM_RestorationQueue
{
    override void OnInit( array<ref VirtualObjectContext> items)
    {
        Print("VSM_DropQueue: " + items);
        super.OnInit(items);
        m_ForceSpawnOnGround = true;
    }

    override void OnComplete()
    {
       super.OnComplete();
       VirtualStorageModule.GetModule().OnDeleteContainer(m_Container);
    }
}