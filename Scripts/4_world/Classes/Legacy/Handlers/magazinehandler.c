class VSM_MagazineHandler_Legacy extends VirtualObjectHandler_Base_Legacy
{

    bool m_IsMagazine;
    int m_Quantity = 0;
    ref array<ref VSM_MagazineCartridgeProfile> m_Cartridges = new array<ref VSM_MagazineCartridgeProfile>;

    override bool CanHandler(ItemBase item)
    {
        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_MagazineHandler CanHandler=" + item.GetType());

        if (item.IsMagazine())
           return true;

        return false;
    }
    
    override void OnVirtualize(string virtualPath, ItemBase virtualize, ItemBase parent)
    {
        Magazine magazine = Magazine.Cast(virtualize);

        m_IsMagazine = true;
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

        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_MagazineHandler: " + virtualize.GetType() + "virtualizando m_Quantity=" + m_Quantity);

    }

    override void OnRestore(string virtualPath, ItemBase restored, ItemBase parent)
    {
        if(parent.IsWeapon())
        {
            Weapon_Base weapon = Weapon_Base.Cast(parent);
            // weapon.SpawnAmmo(restored.GetType(), 0); //! glitch, o item ja chegou como restored, aqui ele esta sendo adicionado novamente

            array<EntityAI> items = new array<EntityAI>;
			weapon.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);
            
            int total = items.Count();
            for (int i = 0; i < total; i++)
			{
                ItemBase itemInWeapon = ItemBase.Cast(items.Get(i));
                if(!itemInWeapon)
                    continue;
                
				Ammunition_Base ammo = Ammunition_Base.Cast(itemInWeapon);

                if (itemInWeapon.IsMagazine() && !(ammo && ammo.IsAmmoPile()))
				{
                    Magazine mag = Magazine.Cast(itemInWeapon);
                    OnRestoreMag(mag);
                }

            }
        }
        else
        {
            Magazine magazine = Magazine.Cast(restored);
            if(magazine)      
                OnRestoreMag(magazine);
        }
    }

    void OnRestoreMag(Magazine magazine)
    {
        magazine.ServerSetAmmoCount(0);

        foreach (VSM_MagazineCartridgeProfile cartrige : m_Cartridges) {
            if (VirtualStorageModule.GetModule().m_Debug)
            Print("VSM_MagazineHandler: cartrige =" + cartrige + "restaurando " + cartrige.m_Classname);
            magazine.ServerStoreCartridge(cartrige.m_Damage, cartrige.m_Classname);
        }
        magazine.SetSynchDirty();
        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_MagazineHandler: OnRestoreMag " + magazine.GetType() + "restaurando m_Quantity=" + m_Quantity);
    }
}