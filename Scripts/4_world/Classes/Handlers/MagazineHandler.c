class VSM_MagazineHandler extends VirtualObjectHandler_Base
{
    int m_Quantity = 0;
    ref array<ref VSM_MagazineCartridgeProfile> m_Cartridges = { };

    override bool CanHandler(ItemBase item)
    {
        return item.IsMagazine();
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    {
        if (!ctx.Read(m_Quantity)) return false;
        if (!ctx.Read(m_Cartridges)) return false;

        return true;
    }

    override void OnStoreSave(ParamsWriteContext ctx) 
    {
        ctx.Write(m_Quantity);
        ctx.Write(m_Cartridges);
    }
    
    override void OnVirtualize(ItemBase virtualize, ItemBase parent) 
    {
        Magazine magazine = Magazine.Cast(virtualize);

        m_Quantity = magazine.GetAmmoCount();

        // handler per item in magazine
        for (int i = 0; i < m_Quantity; i++)
        {
            VSM_MagazineCartridgeProfile cartridge = new VSM_MagazineCartridgeProfile();

            float damage;
            string classname;
            magazine.GetCartridgeAtIndex(i, damage, classname);

            cartridge.m_Classname = classname;
            cartridge.m_Damage = damage;
            m_Cartridges.Insert(cartridge);
        }

        VSM_Debug("OnVirtualize", virtualize.GetType() + "virtualizando m_Quantity=" + m_Quantity);
    }

    override bool OnRestore(ItemBase restored, ItemBase parent) 
    {
        Magazine magazine = Magazine.Cast(restored);
        if(!magazine) 
            return false;
        
        magazine.ServerSetAmmoCount(0);

        foreach (VSM_MagazineCartridgeProfile cartrige : m_Cartridges) {
            VSM_Debug("VSM_MagazineHandler","cartrige =" + cartrige + "restaurando " + cartrige.m_Classname);
            magazine.ServerStoreCartridge(cartrige.m_Damage, cartrige.m_Classname);
        }

        magazine.SetSynchDirty();
        magazine.Update();

        return true;
    }
}

class VSM_MagazineCartridgeProfile
{
    string m_Classname;
    float m_Damage;
}