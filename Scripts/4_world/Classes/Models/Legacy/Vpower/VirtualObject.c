typedef VSMObjectComponent TObjComponent;

class VSMVirtualObject : VSMBase
{
    int     m_Version;
    string  m_Classname;
    string  m_Type;
    int     m_InvRow;
    int     m_InvCol;
    int     m_InvType;
    int     m_InvIdX;
    int     m_InvSlotId;
    bool    m_InvFliped;
    int     m_Quantity;

    /* Operacional */
    ref array<ref VSMMetadataItem>              m_Children;
    protected string                            m_VirtualContextDirectory;
    protected ItemBase                          m_Parent;
    protected ref array<ref TObjComponent>      m_ObjComponents;
    protected ref array<ref VSMVirtualObject>   m_ProcessedItems; // itens processados

    void VSMVirtualObject(string virtualPath, ItemBase parent, VSMMetadataItem vItem = null)
    {
        m_Version                   = VSM_StorageVersion.CURRENT_VERSION; //! força para última versão
        m_VirtualContextDirectory   = virtualPath;
        m_Parent                    = parent;
        m_ProcessedItems            = new array<ref VSMVirtualObject>;
        m_ObjComponents             = new array<ref TObjComponent>;

        if(vItem)
            m_Children              = vItem.m_Children;
        else
            m_Children              = new array<ref VSMMetadataItem>;
    }

    void OnRegisterObjectHandler()
    {   
        VSMObjectComponentConstructor constructor = new VSMObjectComponentConstructor();
        constructor.ConstructComponents(m_Version, m_ObjComponents);
    }

    //! corrigidos erros de perda de itens
    /**
     * Bloco de restauração
     */
    ItemBase Restore(ParamsReadContext ctx, ParamsReadContext storeCtx, bool grounded = false)
    {
        string header;
        ItemBase rObj;

        if (!ctx.Read(m_Version)) return null;
        
        VSM_Debug("Restore", "Iniciando restauração com versão: " + m_Version.ToString());

        if (m_Version >= VSM_StorageVersion.V_1409)
		{   
            if (!ctx.Read(header))
            {
                VSM_Error("Restore", "Falha ao ler o cabeçalho do item - tentando liberar o contexto");
                ReleaseCtx(ctx);
                return null;
            }

            if (header != VSM_HeaderType.ITEM_INIT)
			{
				VSM_Error("Restore", "Cabeçalho inválido: " + header + " - tentando liberar o contexto");
				ReleaseCtx(ctx);
                return null;
			}
		}

        //![Phase 1]: carregar o contexto deste item
        if (!OnStoreLoad(ctx))
        {
            VSM_Error("Restore", "Falha ao carregar o contexto do item: " + m_Classname + " - tentando liberar o contexto");
            
            if (m_Version >= VSM_StorageVersion.V_1409) ReleaseCtx(ctx);
            return null;
        };

        
        //# spawnar item com as informações do contexto
        rObj = OnSpawn(grounded);
        if(!rObj)
        {
            VSM_Warn("Restore", "Não foi possível spawnar o item: " + m_Classname);
            if (m_Version >= VSM_StorageVersion.V_1409) ReleaseCtx(ctx);
            return null;
        }

        rObj.VSM_OnBeforeRestore();

        //# restaurar variaveis
        OnSetVariables(rObj);

        //![Phase 2]: restaurar os items filhos
        OnRestoreChildren(ctx, storeCtx, rObj);

        //![Phase 3]: restaurar handlers
        OnRestoreHandlers(ctx, storeCtx, rObj);

        //# conclusão
        rObj.VSM_OnAfterRestore();

        if(m_Version >= VSM_StorageVersion.V_1409)
		{
			if (!ctx.Read(header) || header != VSM_HeaderType.ITEM_EOF) 
			{
				VSM_Warn("StoreLoad", "Fim de bloco inválido - tentando liberar o contexto");
                ReleaseCtx(ctx);
			}
		}

        VSM_Debug("Restore", m_Classname + " restaurado com sucesso, childrens: " + m_Children.Count().ToString() + ", handlers: " + m_ObjComponents.Count().ToString());
        return rObj;
    }

    protected bool OnStoreLoad(ParamsReadContext ctx)
    {
        if (!ctx.Read(m_Classname)) return false;
        if (!ctx.Read(m_Type))      return false;
        if (!ctx.Read(m_InvRow))    return false;
        if (!ctx.Read(m_InvCol))    return false;
        if (!ctx.Read(m_InvType))   return false;
        if (!ctx.Read(m_InvIdX))    return false;
        if (!ctx.Read(m_InvSlotId)) return false;
        if (!ctx.Read(m_InvFliped)) return false;
        if (!ctx.Read(m_Quantity))  return false;

        return true;
    }

    protected void OnSetVariables(ItemBase rObj)
    {
        rObj.SetQuantity(m_Quantity);
    }

    protected void OnRestoreChildren(ParamsReadContext ctx, ParamsReadContext storeCtx, ItemBase parent)
    {
        parent.VSM_OnBeforeRestoreChildren();

        for (int i = 0; i < m_Children.Count(); i++)
        {
            VSMMetadataItem vItem = m_Children.Get(i);
            VSMVirtualObject obj = new VSMVirtualObject(m_VirtualContextDirectory, parent, vItem);
            ItemBase restored = obj.Restore(ctx, storeCtx);
            m_ProcessedItems.Insert(obj);
        }

        parent.VSM_OnAfterRestoreChildren();      
    }

    protected void OnRestoreHandlers(ParamsReadContext ctx, ParamsReadContext storeCtx, ItemBase rObj)
    {
        OnRegisterObjectHandler();

        for (int i = 0; i < m_ObjComponents.Count(); i++)
        {
            TObjComponent handler = m_ObjComponents.Get(i);
            handler.OnBeforeRestore();

            if (handler.CanHandler(rObj))
                handler.Restore(ctx, storeCtx, m_Version, rObj);
        } 
    }

    void OnRestoreComplete()
    {
		int i;
        for (i = 0; i < m_ObjComponents.Count(); i++)
		{
			TObjComponent handler = m_ObjComponents[i];
            if(!handler) continue;

            handler.OnRestoreComplete();
        }

        for (i = 0; i < m_ProcessedItems.Count(); i++)
		{
			VSMVirtualObject obj = m_ProcessedItems[i];
            if(!obj) continue;

            obj.OnRestoreComplete();
        }

        VSM_Debug("OnComplete", m_Classname + " restaurado com sucesso, handlers: " + m_ObjComponents.Count().ToString() + ", processed: " + m_ProcessedItems.Count().ToString());
    }
    
    /**
     * Bloco de virtualização
     */
    VSMMetadataItem Virtualize(ParamsWriteContext ctx, ParamsWriteContext storeCtx, ItemBase item)
    {
        VSM_Debug("Virtualize", "Iniciando virtualização de: " + item.GetType() + " com versão: " + m_Version.ToString());
        
        //# preparação
        item.VSM_OnBeforeVirtualize();

        VSMMetadataItem vItem = new VSMMetadataItem();
        OnPrepareVirtualization(vItem, item);

        OnGetVariables(item);
        ctx.Write(m_Version);
        ctx.Write(VSM_HeaderType.ITEM_INIT);

        //![Phase 1]: salvar o contexto deste item
        OnStoreSave(ctx);

        //![Phase 2]: virtualizar os filhos deste item
        if(!OnVirtualizeChildren(ctx, storeCtx, item, vItem)) return null;

        //![Phase 3]: virtualizar handlers
        OnVirtualizeHandlers(ctx, storeCtx, item);

        //# conclusão
        item.VSM_OnAfterVirtualize();
        ctx.Write(VSM_HeaderType.ITEM_EOF);

        VSM_Debug("Virtualize", m_Classname + " virtualizado com sucesso, childrens: " + vItem.m_Children.Count().ToString());
        return vItem;  
    }

    protected void OnStoreSave(ParamsWriteContext ctx)
    {
        ctx.Write(m_Classname);
        ctx.Write(m_Type);
        ctx.Write(m_InvRow);
        ctx.Write(m_InvCol);
        ctx.Write(m_InvType);
        ctx.Write(m_InvIdX);
        ctx.Write(m_InvSlotId);
        ctx.Write(m_InvFliped);
        ctx.Write(m_Quantity);
    }

    protected void OnPrepareVirtualization(VSMMetadataItem vItem, ItemBase item)
    {
        m_Classname = item.GetType();
        vItem.m_Classname = m_Classname;

        InventoryLocation invLocation = new InventoryLocation();
        item.GetInventory().GetCurrentInventoryLocation(invLocation);

        m_InvRow = invLocation.GetRow();
        m_InvCol = invLocation.GetCol();
        m_InvIdX = invLocation.GetIdx();
        m_InvSlotId = invLocation.GetSlot();
        m_InvType = invLocation.GetType();
        m_InvFliped = invLocation.GetFlip();
    }

    protected void OnGetVariables(ItemBase item)
    {
        if (item.HasQuantity())
            m_Quantity = item.GetQuantity();
    }

    protected bool OnVirtualizeChildren(ParamsWriteContext ctx, ParamsWriteContext storeCtx, ItemBase parent, VSMMetadataItem vParent)
    {
        parent.VSM_OnBeforeVirtualizeChildren();

        array<EntityAI> children = new array<EntityAI>;
        parent.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, children);

        int i;
        int total = children.Count();
        ItemBase child;

        for (i = 0; i < total; i++)
        {
            child = ItemBase.Cast(children.Get(i));
            if(child && (!child.VSM_IsVirtualizable() || !CanVirtualizeChildren(child)))
                return false;
        }

        for (i = 0; i < total; i++)
        {
            child = ItemBase.Cast(children.Get(i));
            if (child && child.VSM_IsVirtualizable())
            {
                VSM_Debug("VirtualizeChildren", child.GetType() + " salvando child" + child.GetType());

                VSMVirtualObject virtualObj = new VSMVirtualObject(m_VirtualContextDirectory, parent);
                VSMMetadataItem vItem = virtualObj.Virtualize(ctx, storeCtx, child);
                
                if(!vItem)
                {
                    VSM_Debug("VirtualizeChildren", "O item não pode ser virtualizado" + child);
                    continue;
                }

                vParent.AddChild(vItem);
            }
        }

        parent.VSM_OnAfterVirtualizeChildren();

        VSM_Debug("VirtualizeChildren", "%1 terminado, childrens: %2",parent.GetType(), vParent.m_Children.Count().ToString());
        return true;
    }
    
    protected void OnVirtualizeHandlers(ParamsWriteContext ctx, ParamsWriteContext storeCtx, ItemBase item)
    {
        OnRegisterObjectHandler();
        
        for (int i = 0; i < m_ObjComponents.Count(); i++)
        {
            TObjComponent handler = m_ObjComponents.Get(i);
            handler.OnBeforeVirtualize();

            if (handler.CanHandler(item))
                handler.Virtualize(ctx, storeCtx, item);
        }
    }

    void OnVirtualizeComplete()
    {
        for (int i = 0; i < m_ObjComponents.Count(); i++)
		{
			TObjComponent handler = m_ObjComponents[i];
            if(!handler) continue;

            handler.OnVirtualizeComplete();
        }
    }

    /*
     * Utilitários
     */
    ItemBase OnSpawn(bool grounded = false)
    {
        if(grounded)
            return SpawnItemOnGround(m_Classname, m_Parent.GetPosition());
        
        switch (m_InvType)
        {
            case InventoryLocationType.ATTACHMENT:
                return SpawnItemOnSlot(m_Classname, m_Parent, m_InvSlotId);

            case InventoryLocationType.CARGO:
            case InventoryLocationType.PROXYCARGO:
                return SpawnItemOnCargo(m_Classname, m_Parent, m_InvIdX, m_InvRow, m_InvCol, m_InvFliped);

            default:
                return SpawnItemOnGround(m_Classname, m_Parent.GetPosition());
        }

        if (VSM_Settings.GetSettings().m_ForceSpawnOnError)
            return SpawnItemOnGround(m_Classname, m_Parent.GetPosition());

        return null;
    }

    ItemBase SpawnItemOnGround(string classname, vector pos)
    {
        VSM_Trace("SpawnItemOnGround", "classname: %1, pos: %2", classname, pos.ToString());
        ItemBase item = ItemBase.Cast(GetGame().CreateObject(m_Classname, pos, false, false, true));
        return item;
    }

    ItemBase SpawnItemOnSlot(string classname, ItemBase container, int slotId)
    {
        ItemBase item;

        VSM_Trace("SpawnItemOnSlot", "classname: %1, container: %2, slotId: %3", classname, container.GetType(), slotId.ToString());
        container.VSM_SetForceReceiveItems(true);
        container.GetInventory().SetSlotLock(slotId, false);

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

    ItemBase SpawnItemOnCargo(string classname, ItemBase container, int idx, int row, int col, bool isFliped)
    {
        container.VSM_SetForceReceiveItems(true);
        VSM_Trace("SpawnItemOnCargo", "classname: %1, container: %2, idx: %3, row: %4, col: %5, isFliped: %6", classname, container.GetType(), idx.ToString(), row.ToString(), col.ToString(), isFliped.ToString());
        GameInventory parentInv = GameInventory.Cast(container.GetInventory());
        ItemBase item = ItemBase.Cast(parentInv.CreateEntityInCargoEx(classname, idx, row, col, isFliped));
        container.VSM_SetForceReceiveItems(false);
        return item;
    }

    //TODO: estatico pois é usado como utilitário por outras classes. Isso devia estar aqui?
    static bool CanVirtualizeChildren(ItemBase item)
    {
        array<EntityAI> items = new array<EntityAI>;
        item.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);
        
        for (int i = 0; i < items.Count(); i++)
        {
            ItemBase itemTest = ItemBase.Cast(items.Get(i));
            if(itemTest && (!itemTest.VSM_IsVirtualizable() || !CanVirtualizeChildren(itemTest)))
                return false;
        }
        
        return true;
    }

    private bool ReleaseCtx(ParamsReadContext ctx)
	{
        int attempts = 2000;
        string header;

        while (header != VSM_HeaderType.ITEM_EOF && attempts > 0)
		{
            ctx.Read(header);
            attempts--;
        }

        return true;
	}
}