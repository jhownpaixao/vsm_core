class VSM_WeaponHander extends VirtualObjectHandler_Base
{
    ref array<ref VSM_WeaponCartridgeProfile> m_Barrels;
    ref array<ref VSM_WeaponCartridgeProfile> m_Magazines;

    override bool CanHandler(ItemBase item)
    {
        return item.IsWeapon();
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    {
        if (!ctx.Read(m_Barrels)) return false;
        if (!ctx.Read(m_Magazines)) return false;

        return true;
    }

    override void OnStoreSave(ParamsWriteContext ctx) 
    {
        ctx.Write(m_Barrels);
        ctx.Write(m_Magazines);
    }

    override void OnVirtualize(ItemBase virtualize, ItemBase parent)
    {
        Weapon_Base weapon = Weapon_Base.Cast(virtualize);
        m_Barrels = new array<ref VSM_WeaponCartridgeProfile>;
        m_Magazines = new array<ref VSM_WeaponCartridgeProfile>;
        
        OnVirtualizeMuzzles(weapon);
    }

    override bool OnRestore(ItemBase restored, ItemBase parent)
    {
        
        VSM_Debug("OnRestore ", restored.GetType() + " restaurando");

        Weapon_Base weapon = Weapon_Base.Cast(restored);
        OnRestoreMuzzles(weapon);
        weapon.SetSynchDirty();
        weapon.Update();
        return true;
    }


    void OnVirtualizeMuzzles(Weapon_Base weapon)
    {
        float damage;
        string type;

        VSM_Debug("OnVirtualizeMuzzles", weapon.GetType() + " iniciando");

        for (int i = 0; i < weapon.GetMuzzleCount(); ++i)
        {
            // para munições na agulha
            if (!weapon.IsChamberEmpty(i))
            {
                
                VSM_Debug("OnVirtualizeMuzzles", weapon.GetType() + " iniciando");

                if (weapon.GetCartridgeInfo(i, damage, type))
                {
                    VSM_WeaponCartridgeProfile barrel = new VSM_WeaponCartridgeProfile();
                    barrel.m_Idx = i;
                    barrel.m_Damage = damage;
                    barrel.m_Classname = type;

                    VSM_Debug("OnVirtualizeMuzzles", weapon.GetType() + " salvando agulha, cano: id " + barrel.m_Idx + " damage="  + barrel.m_Damage + " classname=" + barrel.m_Classname);
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

                    VSM_Debug("OnVirtualizeMuzzles", weapon.GetType() + " salvando munição do cano: id " + mag.m_Idx + " damage="  + mag.m_Damage + " classname=" + mag.m_Classname);
                    m_Magazines.Insert(mag);
                }
            }
  
            VSM_Debug("OnVirtualizeMuzzles", weapon.GetType() + " concluídos, muzzles=" + m_Barrels.Count() + " magazines=" + m_Magazines.Count());
        }
    }

    void OnRestoreMuzzles(Weapon_Base weapon) 
    {
        
        VSM_Debug("OnRestoreMuzzles", weapon.GetType() + " barrels="+m_Barrels.Count()+" magazines=" + m_Magazines.Count());

        bool needAction = false;

        VSM_Debug("OnRestoreMuzzles", weapon.GetType() + " INICIANDO RESTAURAÇÃO DE barrels="+m_Barrels.Count());
        foreach (VSM_WeaponCartridgeProfile barrel : m_Barrels) {

            if(!barrel)
                continue;

            VSM_Debug("OnRestoreMuzzles", weapon.GetType() + " restaurando agulha ID: "+ barrel.m_Idx + " tipo:" + barrel.m_Classname);
            if (weapon.PushCartridgeToChamber(barrel.m_Idx, barrel.m_Damage, barrel.m_Classname))
            {
                VSM_Debug("OnRestoreMuzzles", weapon.GetType() + " agulha restaurada ID: "+ barrel.m_Idx + " tipo:" + barrel.m_Classname);
            } else
            {
               VSM_Warn("OnRestoreMuzzles", weapon.GetType() + " não foi possível restaurar a agulha ID: "+ barrel.m_Idx + " tipo:" + barrel.m_Classname);
            } 
        }
       
        VSM_Debug("OnRestoreMuzzles", weapon.GetType() + " INICIANDO RESTAURAÇÃO DE magazines="+m_Magazines.Count());
        foreach (VSM_WeaponCartridgeProfile magazine : m_Magazines) {

            if(!magazine)
                continue;

            VSM_Debug("OnRestoreMuzzles", weapon.GetType() + " restaurando munição interna ID: "+ magazine.m_Idx + " tipo:" + magazine.m_Classname);
            if (weapon.PushCartridgeToInternalMagazine(magazine.m_Idx, magazine.m_Damage, magazine.m_Classname))
            {
                VSM_Debug("OnRestoreMuzzles", weapon.GetType() + "  munição interna restaurada ID: "+ magazine.m_Idx + " tipo:" + magazine.m_Classname);
            }   
            else
            {
                VSM_Warn("OnRestoreMuzzles", weapon.GetType() + " não foi possível restaurar munição interna ID: "+ magazine.m_Idx + " tipo:" + magazine.m_Classname);
            }
        }
        weapon.Synchronize();
        VSM_Debug("OnRestoreMuzzles", weapon.GetType() + " restauração concluída ");
    }
}

class VSM_WeaponCartridgeProfile
{
    int m_Idx;
    float m_Damage;
    string m_Classname;
}