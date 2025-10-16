#ifdef EXPANSIONMODBASEBUILDING
modded class ExpansionCodeLock
{
    override bool VSM_IsVirtualizable()
    {
        if(super.VSM_IsVirtualizable())
        {
            return !GetInventory().IsAttachment();
        }
        
        return true;
    }
}
#endif