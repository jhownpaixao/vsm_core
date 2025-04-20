class VSM_WeaponHander_Legacy extends VirtualObjectHandler_Base_Legacy
{
    bool m_IsWeapon;

    ref array<VSM_WeaponCartridgeProfile> m_Barrels = new array<VSM_WeaponCartridgeProfile>;
    ref array<VSM_WeaponCartridgeProfile> m_Magazines = new array<VSM_WeaponCartridgeProfile>;

    override bool CanHandler(ItemBase item)
    {
        return item.IsWeapon();
    }

    override void OnVirtualize(string virtualPath, ItemBase virtualize, ItemBase parent)
    {
        Weapon_Base weapon = Weapon_Base.Cast(virtualize);
        OnVirtualizeMuzzles(weapon);
    }

    override void OnRestore(string virtualPath, ItemBase restored, ItemBase parent)
    {
        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_StoreHandler: OnRestore " + restored.GetType() + " restaurando");

        Weapon_Base weapon = Weapon_Base.Cast(restored);
        OnRestoreMuzzles(weapon);
    }


    void OnVirtualizeMuzzles(Weapon_Base weapon)
    {
        float damage;
        string type;

        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_WeaponHander: OnVirtualizeMuzzles " + weapon.GetType() + " iniciando");

        for (int i = 0; i < weapon.GetMuzzleCount(); ++i)
        {
            if (!weapon.IsChamberEmpty(i))
            {
                if (VirtualStorageModule.GetModule().m_Debug)
                Print("VSM_WeaponHander: OnVirtualizeMuzzles " + weapon.GetType() + " iniciando");

                if (weapon.GetCartridgeInfo(i, damage, type))
                {
                    VSM_WeaponCartridgeProfile barrel = new VSM_WeaponCartridgeProfile();
                    barrel.m_Idx = i;
                    barrel.m_Damage = damage;
                    barrel.m_Classname = type;

                    if (VirtualStorageModule.GetModule().m_Debug)
                    Print("VSM_WeaponHander: OnVirtualizeMuzzles " + weapon.GetType() + " salvando muzzle: id " + barrel.m_Idx + " damage="  + barrel.m_Damage + " classname=" + barrel.m_Classname);
                    m_Barrels.Insert(barrel);
                }
            }

            for (int a = 0; a < weapon.GetInternalMagazineCartridgeCount(i); ++a)
            {
                if (weapon.GetInternalMagazineCartridgeInfo(i, a, damage, type))
                {
                    VSM_WeaponCartridgeProfile mag = new VSM_WeaponCartridgeProfile();
                    mag.m_Idx = a;
                    mag.m_Damage = damage;
                    mag.m_Classname = type;

                    if (VirtualStorageModule.GetModule().m_Debug)
                    Print("VSM_WeaponHander: OnVirtualizeMuzzles " + weapon.GetType() + " salvando magazine: id " + mag.m_Idx + " damage="  + mag.m_Damage + " classname=" + mag.m_Classname);

                    m_Magazines.Insert(mag);
                }
            }

            if (VirtualStorageModule.GetModule().m_Debug)
            Print("VSM_WeaponHander: OnVirtualizeMuzzles " + weapon.GetType() + " concluídos, muzzles=" + m_Barrels.Count() + " magazines=" + m_Magazines.Count());

        }
    }

    void OnRestoreMuzzles(Weapon_Base weapon) 
    {
        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_WeaponHander: OnRestoreMuzzles " + weapon.GetType() + " barrels="+m_Barrels.Count()+" magazines=" + m_Magazines.Count());

        bool needAction = false;
        foreach (VSM_WeaponCartridgeProfile barrel : m_Barrels) {
            if(!barrel)
                continue;
                
            if (weapon.PushCartridgeToChamber(barrel.m_Idx, barrel.m_Damage, barrel.m_Classname))
            {
                if (VirtualStorageModule.GetModule().m_Debug)
                Print("VSM_WeaponHander: OnRestoreMuzzles " + weapon.GetType() + " restaurando muzzle= " + barrel.m_Classname);
                needAction = true;
            }
                
        }

        foreach (VSM_WeaponCartridgeProfile magazine : m_Magazines) {
            if (weapon.PushCartridgeToChamber(magazine.m_Idx, magazine.m_Damage, magazine.m_Classname))
            {
                if (VirtualStorageModule.GetModule().m_Debug)
                Print("VSM_WeaponHander: OnRestoreMuzzles " + weapon.GetType() + " restaurando magazine= " + magazine.m_Classname);
                needAction = true;
            }
                
        }

        if (needAction)
        {
            if (GetGame().IsServer())
            {
                weapon.RandomizeFSMState();
            }

            weapon.Synchronize();
        }

        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_WeaponHander: OnRestoreMuzzles " + weapon.GetType() + " restauração concluída ");
    }
}