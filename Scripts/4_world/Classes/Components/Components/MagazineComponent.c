/**
 * @brief Classe para manipulação de carregadores de armas
 * @note Esta classe é usada para lidar com características específicas de carregadores, como quantidade de munição e perfis de cartuchos
 */
class VSMMagazineComponent extends VSMObjectComponent
{
    int m_Quantity = 0;
    ref array<ref VSM_MagazineCartridgeProfile> m_Cartridges = { };

    override bool CanHandler(ItemBase item)
    {
        if(!item) return false;

        Magazine magazine = Magazine.Cast(item);
        if(!magazine || !magazine.IsMagazine()) return false;

        return true;
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    {
        if(version < VSM_StorageVersion.V_0510)
        {
            if (!ctx.Read(m_Quantity)) return false;
            if (!ctx.Read(m_Cartridges)) return false;
        }

        return true;
    }

    override void OnStoreSave(ParamsWriteContext ctx) 
    {
        // ctx.Write(m_Quantity);
        // ctx.Write(m_Cartridges);
    }
    
    override void OnVirtualize(ItemBase virtualize) 
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

        VSM_Trace("OnVirtualize", virtualize.GetType() + "virtualizando m_Quantity=" + m_Quantity);
    }

    override bool OnRestore(ItemBase restored) 
    {
        Magazine magazine = Magazine.Cast(restored);
        if(!magazine) 
            return false;
        
        magazine.ServerSetAmmoCount(0);

        for (int i = 0; i < m_Cartridges.Count(); i++)
		{
			VSM_MagazineCartridgeProfile cartrige = m_Cartridges[i];
            if(!cartrige) continue;

            VSM_Trace("VSMMagazineComponent","cartrige =" + cartrige + "restaurando " + cartrige.m_Classname);
            magazine.ServerStoreCartridge(cartrige.m_Damage, cartrige.m_Classname);
        }

        magazine.SetSynchDirty();
        magazine.Update();

        VSM_Trace("OnRestore", restored.GetType() + " restaurando m_Quantity=" + m_Quantity);
        return true;
    }
}

class VSM_MagazineCartridgeProfile
{
    string m_Classname;
    float m_Damage;
}