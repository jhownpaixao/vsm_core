class VSMVirtualItem : VSMBase
{
    int     m_EntityId;
    string  m_Classname;
    string  m_Type;
    int     m_InvRow;
    int     m_InvCol;
    int     m_InvType;
    int     m_InvIdX;
    int     m_InvSlotId;
    bool    m_InvFliped;
    bool    m_Delete;

    ref array<ref VSMVirtualItem>   m_Cargo;
    ref array<ref VSMVirtualItem>   m_Attachments;

    ref VSMHealthComponent          m_HealthComponent;
    ref VSMQuantityComponent        m_QuantityComponent;
    ref VSMAmmunitionComponent      m_AmmunitionComponent;
    ref VSMMagazineComponent        m_MagazineComponent;
    ref VSMWeaponComponent          m_WeaponComponent;
    ref VSMStoreComponent           m_StoreComponent;

    // Operational
    [NonSerialized()]
    int         m_Version;
    [NonSerialized()]
    ItemBase    m_Item;

    void VSMVirtualItem()
    {
        m_Cargo         = new array<ref VSMVirtualItem>;
        m_Attachments   = new array<ref VSMVirtualItem>;

        m_HealthComponent       = new VSMHealthComponent();
        m_QuantityComponent     = new VSMQuantityComponent();
        m_AmmunitionComponent   = new VSMAmmunitionComponent();
        m_MagazineComponent     = new VSMMagazineComponent();
        m_WeaponComponent       = new VSMWeaponComponent();
        m_StoreComponent        = new VSMStoreComponent();
    }

    void Virtualize(ItemBase item, ParamsWriteContext ctx, ParamsWriteContext ctxStore, int version)
    {
        m_Item = item;
        m_Version = version;

        VSM_Debug("Virtualize", "Iniciando virtualização do item: " + item.GetType() + " com versão: " + m_Version.ToString());

        //# preparação
        m_Item.VSM_OnBeforeVirtualize();

        m_EntityId = m_Item.GetID();
        m_Classname = m_Item.GetType();
        m_Type = m_Item.GetType();
        m_Delete = m_Item.VSM_IsVirtualizable();

        CreateInvVars(m_Item);
        CreateVirtualChildren(m_Item, ctx, ctxStore);
        if(m_Delete)
            SaveComponents(m_Item, ctx, ctxStore);

        m_Item.VSM_OnAfterVirtualize();
    }

    ItemBase Restore(int version, ItemBase container, ParamsReadContext ctx, ParamsReadContext storeCtx, bool grounded = false)
    {
        m_Version = version;
        ItemBase restored;

        VSM_Debug("Restore", "Iniciando restauração do item: " + m_Classname + " com versão: " + m_Version.ToString() + " | ID: " + m_EntityId.ToString() + " | Deletável: " + m_Delete.ToString());

        if(m_Delete)
        {
            restored = ItemBase.Cast(SpawnEntity(container, grounded));
            if(!restored)
                return null; 
        }
        else
        {
            array<EntityAI> entities = new array<EntityAI>;
            container.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, entities);

            VSM_Debug("Restore", "Procurando item não virtualizável no inventário: " + m_Classname + " | ID: " + m_EntityId.ToString() + " | Itens atuais: " + entities.Count().ToString());

            for (int i = 0; i < entities.Count(); i++)
            {
                ItemBase currentItem = ItemBase.Cast(entities.Get(i));
                if(!currentItem) 
                    continue;

                VSM_Debug("Restore", "Verificando item: " + currentItem.GetType() + " | ID: " + currentItem.GetID().ToString());

                if(currentItem.GetID() == m_EntityId)
                {
                    restored = currentItem;
                    VSM_Debug("Restore", "Item encontrado no inventário: " + m_Classname + " | ID: " + m_EntityId.ToString());
                    break;
                }
            }
        }

        if(!restored)
        {
            VSM_Error("Restore", "Falha ao restaurar item: " + m_Classname + " em: " + container.GetType() + " | ID: " + m_EntityId.ToString());
            return null;
        }

        restored.VSM_OnBeforeRestore();

        RestoreVirtualChildren(restored, ctx, storeCtx);

        if(m_Delete)
        {
            RestoreComponents(restored, ctx, storeCtx);
        }
            
        restored.VSM_OnAfterRestore();
        return restored;
    }

    protected void CreateInvVars(ItemBase item)
    {
        InventoryLocation invLocation = new InventoryLocation();
        item.GetInventory().GetCurrentInventoryLocation(invLocation);

        m_InvRow = invLocation.GetRow();
        m_InvCol = invLocation.GetCol();
        m_InvIdX = invLocation.GetIdx();
        m_InvSlotId = invLocation.GetSlot();
        m_InvType = invLocation.GetType();
        m_InvFliped = invLocation.GetFlip();
    }

    protected void CreateVirtualChildren(ItemBase container, ParamsWriteContext ctx, ParamsWriteContext storeCtx)
    {
        VSM_Debug("CreateVirtualChildren", "Iniciando criação de filhos virtuais para: " + container.GetType());
        container.VSM_OnBeforeVirtualizeChildren();
        GameInventory inv   = container.GetInventory();
        CargoBase cargo     = inv.GetCargo();
        ItemBase item;
        VSMVirtualItem vItem;

        int count = inv.AttachmentCount();
        for (int i = 0; i < count; i++)
		{
            VSM_Debug("CreateVirtualChildren", "Criando filho virtual para: " + inv.GetAttachmentFromIndex(i).GetType());
			item = ItemBase.Cast(inv.GetAttachmentFromIndex(i));
            vItem = new VSMVirtualItem();
            vItem.Virtualize(item, ctx, storeCtx, m_Version);
            VSM_Debug("CreateVirtualChildren", "Filho virtual criado: " + item.GetType() + " | ID: " + item.GetID().ToString() + " | Deletável: " + vItem.m_Delete.ToString());
            if(m_Delete && !vItem.m_Delete)
            {
                VSM_Debug("CreateVirtualChildren", "O item não pode ser deletado: " + item.GetType() + " | ID: " + item.GetID().ToString());
                m_Delete = false; // se algum filho não for deletado, o pai também não será
            }

            m_Attachments.Insert(vItem);
		}
  
        if(cargo)
        {
            count = cargo.GetItemCount();
            for (int j = 0; j < count; j++)
            {
                VSM_Debug("CreateVirtualChildren", "Criando filho virtual para: " + cargo.GetItem(j).GetType());
                item = ItemBase.Cast(cargo.GetItem(j));
                vItem = new VSMVirtualItem();
                vItem.Virtualize(item, ctx, storeCtx, m_Version);
                VSM_Debug("CreateVirtualChildren", "Filho virtual criado: " + item.GetType() + " | ID: " + item.GetID().ToString() + " | Deletável: " + vItem.m_Delete.ToString());
                if(m_Delete && !vItem.m_Delete)
                {
                    VSM_Debug("CreateVirtualChildren", "O item não pode ser deletado: " + item.GetType() + " | ID: " + item.GetID().ToString());
                    m_Delete = false; // se algum filho não for deletado, o pai também não será
                }

                m_Cargo.Insert(vItem);
            }
        }

        container.VSM_OnAfterVirtualizeChildren();
        VSM_Debug("CreateVirtualChildren", "%1 terminado, attachments: %2, cargo: %3",container.GetType(), m_Attachments.Count().ToString(), m_Cargo.Count().ToString());
    }

    protected void RestoreVirtualChildren(ItemBase container, ParamsReadContext ctx, ParamsReadContext storeCtx, bool grounded = false)
    {
		VSMVirtualItem vItem;

        container.VSM_OnBeforeRestoreChildren();

        for (int i = 0; i < m_Attachments.Count(); i++)
        {
            vItem = m_Attachments.Get(i);
            vItem.Restore(m_Version, container, ctx, storeCtx, grounded);
        }

        for (int j = 0; j < m_Cargo.Count(); j++)
        {
            vItem = m_Cargo.Get(j);
            vItem.Restore(m_Version, container, ctx, storeCtx, grounded);
        }

        container.VSM_OnAfterRestoreChildren();
    } 

    protected void SaveComponents(ItemBase item, ParamsWriteContext ctx, ParamsWriteContext storeCtx)
    {
        VSM_Debug("SaveComponents", "Salvando componentes para: " + item.GetType());   
        
        array<VSMObjectComponent> components = new array<VSMObjectComponent>;
        
        if (m_HealthComponent.CanHandler(item))
            components.Insert(m_HealthComponent);

        if (m_QuantityComponent.CanHandler(item))
            components.Insert(m_QuantityComponent);

        if (m_AmmunitionComponent.CanHandler(item))
            components.Insert(m_AmmunitionComponent);

        if (m_MagazineComponent.CanHandler(item))
            components.Insert(m_MagazineComponent);

        if (m_WeaponComponent.CanHandler(item))
            components.Insert(m_WeaponComponent);

        if (m_StoreComponent.CanHandler(item))
            components.Insert(m_StoreComponent);

        for (int i = 0; i < components.Count(); i++)
        {
            VSMObjectComponent component = components.Get(i);
            component.OnBeforeVirtualize();
            component.Virtualize(ctx, storeCtx, item);
        }

        components.Clear();
        VSM_Debug("SaveComponents", "Componentes salvos para: " + item.GetType());
    }

    protected void RestoreComponents(ItemBase item, ParamsReadContext ctx, ParamsReadContext storeCtx)
    {
        VSM_Debug("RestoreComponents", "Restaurando componentes para: " + item.GetType());
        array<VSMObjectComponent> components = new array<VSMObjectComponent>;
        
        if (m_HealthComponent.CanHandler(item))
            components.Insert(m_HealthComponent);

        if (m_QuantityComponent.CanHandler(item))
            components.Insert(m_QuantityComponent);

        if (m_AmmunitionComponent.CanHandler(item))
            components.Insert(m_AmmunitionComponent);

        if (m_MagazineComponent.CanHandler(item))
            components.Insert(m_MagazineComponent);

        if (m_WeaponComponent.CanHandler(item))
            components.Insert(m_WeaponComponent);

        if (m_StoreComponent.CanHandler(item))
            components.Insert(m_StoreComponent);

        for (int i = 0; i < components.Count(); i++)
        {
            VSMObjectComponent component = components.Get(i);
            component.OnBeforeRestore();
            component.Restore(ctx, storeCtx, m_Version, item);
        }

        components.Clear();

        VSM_Debug("RestoreComponents", "Componentes restaurados para: " + item.GetType());
    }

    /*
     * Utilitários
     */
    protected EntityAI SpawnEntity(ItemBase container, bool grounded = false)
    {
        if(grounded)
            return SpawnItemOnGround(m_Classname, container.GetPosition());
        
        switch (m_InvType)
        {
            case InventoryLocationType.ATTACHMENT:
                return SpawnItemOnSlot(m_Classname, container, m_InvSlotId);

            case InventoryLocationType.CARGO:
            case InventoryLocationType.PROXYCARGO:
                return SpawnItemOnCargo(m_Classname, container, m_InvIdX, m_InvRow, m_InvCol, m_InvFliped);

            default:
                return SpawnItemOnGround(m_Classname, container.GetPosition());
        }

        if (VSM_Settings.GetSettings().m_ForceSpawnOnError)
            return SpawnItemOnGround(m_Classname, container.GetPosition());

        return null;
    }

    protected EntityAI SpawnItemOnGround(string classname, vector pos)
    {
        VSM_Trace("SpawnItemOnGround", "classname: %1, pos: %2", classname, pos.ToString());
        ItemBase item = ItemBase.Cast(GetGame().CreateObject(m_Classname, pos, false, false, true));
        return item;
    }

    protected EntityAI SpawnItemOnSlot(string classname, ItemBase container, int slotId)
    {
        ItemBase item;

        VSM_Trace("SpawnItemOnSlot", "classname: %1, container: %2, slotId: %3", classname, container.GetType(), slotId.ToString());
        container.VSM_SetForceReceiveItems(true);
        container.GetInventory().SetSlotLock(slotId, false);
        VSM_Trace("SpawnItemOnSlot", "Manipule? " + container.VSM_CanManipule());

        if (container.IsWeapon() && GetGame().IsKindOf(classname, "Magazine"))
        {
            Weapon_Base weapon = Weapon_Base.Cast(container);
            item = weapon.SpawnAttachedMagazine(classname, WeaponWithAmmoFlags.NONE);
        }
        else
        {
            InventoryLocation location = new InventoryLocation();
            EntityAI attachedItem = container.GetInventory().FindAttachment(slotId);
            if (!attachedItem)
            {
                location.SetAttachment(container, null, slotId);
                item = ItemBase.Cast(GameInventory.LocationCreateEntity(location, classname, ECE_IN_INVENTORY, RF_DEFAULT));
            }
            else if(attachedItem.GetType() == classname)
            {
                item = ItemBase.Cast(attachedItem);
            }
            else
            {
                VSM_Debug("SpawnItemOnSlot", "Item já existe no slot: " + classname + " " + attachedItem.GetType() + " " + slotId.ToString());
            }

            // item = ItemBase.Cast(container.GetInventory().CreateAttachmentEx(classname, slotId));
        }

        container.VSM_SetForceReceiveItems(false);
        return item;
    }

    protected EntityAI SpawnItemOnCargo(string classname, ItemBase container, int idx, int row, int col, bool isFliped)
    {
        container.VSM_SetForceReceiveItems(true);
        VSM_Trace("SpawnItemOnCargo", "classname: %1, container: %2, idx: %3, row: %4, col: %5, isFliped: %6", classname, container.GetType(), idx.ToString(), row.ToString(), col.ToString(), isFliped.ToString());
        VSM_Trace("SpawnItemOnCargo", "Manipule? " + container.VSM_CanManipule());

        GameInventory parentInv = GameInventory.Cast(container.GetInventory());
        ItemBase item = ItemBase.Cast(parentInv.CreateEntityInCargoEx(classname, idx, row, col, isFliped));
        container.VSM_SetForceReceiveItems(false);
        return item;
    }
}