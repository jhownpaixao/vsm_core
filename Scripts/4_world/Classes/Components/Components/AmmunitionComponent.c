/**
 * @brief Classe para manipulação de munições
 * @note Esta classe é usada para lidar com características específicas de munições
 */
class VSMAmmunitionComponent extends VSMObjectComponent
{
    int m_Quantity;

    override bool CanHandler(ItemBase item)
    {
        if(!item) return false;

        Ammunition_Base ammo = Ammunition_Base.Cast(item);
        if(!ammo || !ammo.IsAmmoPile()) return false;

        return true;
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    {
        if(version < VSM_StorageVersion.V_0510)
        {
            if (!ctx.Read(m_Quantity)) return false;
        }
       
        return true;
    }

    override void OnStoreSave(ParamsWriteContext ctx) 
    {
        // ctx.Write(m_Quantity); statico, salvo automaticamente
    }

    override void OnVirtualize(ItemBase virtualize) 
    {
        Ammunition_Base ammo = Ammunition_Base.Cast(virtualize);
        m_Quantity = ammo.GetAmmoCount();
        VSM_Debug("OnVirtualize", "Virtualizando componente de munição para: " + virtualize.GetType() + ", quantidade: " + m_Quantity.ToString());
    }

    override bool OnRestore(ItemBase restored) 
    {
        Ammunition_Base ammo = Ammunition_Base.Cast(restored);
        ammo.ServerSetAmmoCount(m_Quantity);

        VSM_Debug("OnRestore", "Restaurando componente de munição para: " + restored.GetType() + ", quantidade: " + m_Quantity.ToString());
        return true;
    }
}