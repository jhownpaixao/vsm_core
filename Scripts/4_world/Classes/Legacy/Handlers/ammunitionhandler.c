class VSM_AmmunitionHandler_Legacy extends VirtualObjectHandler_Base_Legacy
{
    bool m_IsAmmo;
    int m_Quantity;

    override bool CanHandler(ItemBase item)
    {
        
        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_AmmunitionHandler CanHandler=" + item.GetType());

        Ammunition_Base ammo = Ammunition_Base.Cast(item);
        if (ammo && ammo.IsAmmoPile())
            return true;
        
        return false;
    }
    
    
    override void OnVirtualize(string virtualPath, ItemBase virtualize, ItemBase parent) 
    {
        
        Ammunition_Base ammo = Ammunition_Base.Cast(virtualize);
        m_IsAmmo = true;
        m_Quantity = ammo.GetAmmoCount();
        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_AmmunitionHandler virtualizando quantity="+m_Quantity);
    }

    override void OnRestore(string virtualPath, ItemBase restored, ItemBase parent) 
    {
        Ammunition_Base ammo = Ammunition_Base.Cast(restored);
        ammo.ServerSetAmmoCount(m_Quantity);
        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_AmmunitionHandler restaurando quantity="+m_Quantity);
    }
}