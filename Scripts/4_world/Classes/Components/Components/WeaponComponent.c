/**
 * @brief Classe para manipulação de armas e seus componentes
 * @note Esta classe é usada para lidar com características específicas de armas, como munições no cano, carregadores e estados
 */
class VSMWeaponComponent extends VSMObjectComponent
{
    ref array<ref VSM_WeaponCartridgeProfile> m_Barrels;
    ref array<ref VSM_WeaponCartridgeProfile> m_Magazines;

    override bool CanHandler(ItemBase item)
    {
        if(!item || !item.IsWeapon()) return false;

        return true;
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    {
        if(version < VSM_StorageVersion.V_0510)
        {
            if (!ctx.Read(m_Barrels)) return false;
            if (!ctx.Read(m_Magazines)) return false;
        }

        return true;
    }

    override void OnStoreSave(ParamsWriteContext ctx) 
    {
        // ctx.Write(m_Barrels);
        // ctx.Write(m_Magazines);
    }

    override void OnVirtualize(ItemBase virtualize)
    {
        Weapon_Base weapon = Weapon_Base.Cast(virtualize);
        m_Barrels = new array<ref VSM_WeaponCartridgeProfile>;
        m_Magazines = new array<ref VSM_WeaponCartridgeProfile>;
        
        OnVirtualizeMuzzles(weapon);

        VSM_Debug("OnVirtualize", "Virtualizando componente de arma para: " + virtualize.GetType() + ", canos: " + m_Barrels.Count().ToString() + ", carregadores: " + m_Magazines.Count().ToString());
    }

    override bool OnRestore(ItemBase restored)
    {
        Weapon_Base weapon = Weapon_Base.Cast(restored);
        OnRestoreMuzzles(weapon);
        weapon.SetSynchDirty();
        weapon.Update();

        VSM_Debug("OnRestore", "Restaurando componente de arma para: " + restored.GetType() + ", canos: " + m_Barrels.Count().ToString() + ", carregadores: " + m_Magazines.Count().ToString());
        return true;
    }

    void OnVirtualizeMuzzles(Weapon_Base weapon)
    {
        float damage;
        string type;

        for (int i = 0; i < weapon.GetMuzzleCount(); ++i)
        {
            // para munições na agulha
            if (!weapon.IsChamberEmpty(i))
            {
                if (weapon.GetCartridgeInfo(i, damage, type))
                {
                    VSM_WeaponCartridgeProfile barrel = new VSM_WeaponCartridgeProfile();
                    barrel.m_Idx = i;
                    barrel.m_Damage = damage;
                    barrel.m_Classname = type;
                    
                    m_Barrels.Insert(barrel);
                }
            }

            // para munições no cano (armas sem carregadores, ex: MP133)
            for (int a = 0; a < weapon.GetInternalMagazineCartridgeCount(i); ++a)
            {
                if (weapon.GetInternalMagazineCartridgeInfo(i, a, damage, type))
                {
                    VSM_WeaponCartridgeProfile mag = new VSM_WeaponCartridgeProfile();
                    mag.m_Idx = i; //id do cano e não da munição
                    mag.m_Damage = damage;
                    mag.m_Classname = type;

                    m_Magazines.Insert(mag);
                }
            }
        }
    }

    void OnRestoreMuzzles(Weapon_Base weapon) 
    {
        int i;
        for (i = 0; i < m_Barrels.Count(); i++)
		{
			VSM_WeaponCartridgeProfile barrel = m_Barrels[i];
            if(barrel)
            {
                if (!weapon.PushCartridgeToChamber(barrel.m_Idx, barrel.m_Damage, barrel.m_Classname))
                {
                    VSM_Warn("OnRestoreMuzzles", weapon.GetType() + " não foi possível restaurar a agulha ID: "+ barrel.m_Idx + " tipo:" + barrel.m_Classname);
                }
            };
        }
       
        for (i = 0; i < m_Magazines.Count(); i++)
		{
			VSM_WeaponCartridgeProfile magazine = m_Magazines[i];
            if(magazine)
            {
                if (!weapon.PushCartridgeToInternalMagazine(magazine.m_Idx, magazine.m_Damage, magazine.m_Classname))
                {
                    VSM_Warn("OnRestoreMuzzles", weapon.GetType() + " não foi possível restaurar munição interna ID: "+ magazine.m_Idx + " tipo:" + magazine.m_Classname);
                }
            };
        }

        weapon.Synchronize();
    }
}

class VSM_WeaponCartridgeProfile
{
    int m_Idx;
    float m_Damage;
    string m_Classname;
}