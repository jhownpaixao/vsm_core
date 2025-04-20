class VSM_AmmunitionHandler extends VirtualObjectHandler_Base
{
    int m_Quantity;

    override bool CanHandler(ItemBase item)
    {
        Ammunition_Base ammo = Ammunition_Base.Cast(item);
        return ammo && ammo.IsAmmoPile();
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    {
        if (!ctx.Read(m_Quantity)) return false;

        return true;
    }

    override void OnStoreSave(ParamsWriteContext ctx) 
    {
        ctx.Write(m_Quantity);
    }

    override void OnVirtualize(ItemBase virtualize, ItemBase parent) 
    {
        Ammunition_Base ammo = Ammunition_Base.Cast(virtualize);
        m_Quantity = ammo.GetAmmoCount();
    }

    override bool OnRestore(ItemBase restored, ItemBase parent) 
    {
        Ammunition_Base ammo = Ammunition_Base.Cast(restored);
        ammo.ServerSetAmmoCount(m_Quantity);
        return true;
    }
}