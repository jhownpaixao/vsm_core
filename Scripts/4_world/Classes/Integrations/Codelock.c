#ifdef CodeLock
modded class CodeLock
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