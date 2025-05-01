class VirtualObject : VSM_Base
{
    //!v100 não alterar
    int m_Version;

    //* V_2504
    string m_Classname;
    string m_Type;

    int m_InvRow;
    int m_InvCol;
    int m_InvType;
    int m_InvIdX;
    int m_InvSlotId;
    bool m_InvFliped;
    
    int m_Quantity;

    ref array<ref VirtualObjectContext> m_Children = { };

    /* Operacional */
    protected string m_VirtualContextDirectory;
    protected ItemBase m_Parent;
    protected ref array<ref VirtualObjectHandler_Base> m_Handlers = { };
    protected ref array<string> m_FilesToDelete; // arquivos a serem deletados após o processamento
    protected ref array<ref VirtualObject> m_ProcessedItems; // itens processados

    void VirtualObject(string virtualPath, ItemBase parent)
    {
        m_VirtualContextDirectory = virtualPath;
        m_Parent = parent;
        m_Version = VSM_StorageVersion.CURRENT_VERSION; //! força para última versão
        m_ProcessedItems = new array<ref VirtualObject>;
        m_FilesToDelete = new array<string>;
    }

    bool OnStoreLoad(ParamsReadContext ctx)
    {
        if (!ctx.Read(m_Version)) return false;

        if (!ctx.Read(m_Classname)) return false;
        if (!ctx.Read(m_Type)) return false;

        if (!ctx.Read(m_InvRow)) return false;
        if (!ctx.Read(m_InvCol)) return false;
        if (!ctx.Read(m_InvType)) return false;
        if (!ctx.Read(m_InvIdX)) return false;
        if (!ctx.Read(m_InvSlotId)) return false;
        if (!ctx.Read(m_InvFliped)) return false;

        if (!ctx.Read(m_Quantity)) return false;

        if (!ctx.Read(m_Children))  return false;

        return true;
    }

    void OnStoreSave(ParamsWriteContext ctx)
    {
        ctx.Write(m_Version);

        ctx.Write(m_Classname);
        ctx.Write(m_Type);

        ctx.Write(m_InvRow);
        ctx.Write(m_InvCol);
        ctx.Write(m_InvType);
        ctx.Write(m_InvIdX);
        ctx.Write(m_InvSlotId);
        ctx.Write(m_InvFliped);

        ctx.Write(m_Quantity);

        ctx.Write(m_Children);
    }

    void OnRegisterObjectHandler()
    {   
        //* V_2504
        m_Handlers.Insert(new VSM_HealthHandler(m_VirtualContextDirectory));
        m_Handlers.Insert(new VSM_AmmunitionHandler(m_VirtualContextDirectory));
        m_Handlers.Insert(new VSM_MagazineHandler(m_VirtualContextDirectory));
        m_Handlers.Insert(new VSM_WeaponHander(m_VirtualContextDirectory));
        m_Handlers.Insert(new VSM_StoreHandler(m_VirtualContextDirectory));
    }

    ItemBase OnRestore(ParamsReadContext ctx, bool grounded = false)
    {
        ItemBase restoredObject;

        if (!OnStoreLoad(ctx))
        {
            VSM_Warn("OnRestore", " Não foi possível ler o contexto do item.");
            return restoredObject;
        }

        VSM_Debug("OnRestore", "%1 iniciando. VERSÃO %2", m_Classname, m_Version.ToString());
        OnRegisterObjectHandler();

        VSM_Trace("OnRestore", "%1 grounded: %2, m_InvType: %3, m_InvIdX: %4, m_InvRow: %5, m_InvCol: %6, m_InvFliped: %7", m_Classname, grounded.ToString(), m_InvType.ToString(), m_InvIdX.ToString(), m_InvRow.ToString(), m_InvCol.ToString(), m_InvFliped.ToString());
        if (!grounded)
        {
            switch (m_InvType)
            {
                case InventoryLocationType.ATTACHMENT:
                    restoredObject = SpawnItemOnSlot(m_Classname, m_Parent, m_InvSlotId);
                    break;

                case InventoryLocationType.CARGO:
                case InventoryLocationType.PROXYCARGO:
                    restoredObject = SpawnItemOnCargo(m_Classname, m_Parent, m_InvIdX, m_InvRow, m_InvCol, m_InvFliped);
                    break;

                default:
                    restoredObject = SpawnItemOnGround(m_Classname, m_Parent.GetPosition());
                    break;
            }
        }
        else
        {
            restoredObject = SpawnItemOnGround(m_Classname, m_Parent.GetPosition());
        }
        
        //! se ja estiver no inventário ele vai duplicar aqui.
        if (!restoredObject && CfgGameplayHandler.GetVSM_ForceSpawnOnError())
            restoredObject = SpawnItemOnGround(m_Classname, m_Parent.GetPosition()); 


        if (!restoredObject)
        {
            VSM_Warn("OnRestore", m_Classname + " item não spawnado");
            return restoredObject;
        }

        RestoreChildren(restoredObject);

        //!commons
        restoredObject.SetQuantity(m_Quantity);

        foreach (VirtualObjectHandler_Base handler : m_Handlers)
        {
            if (handler.CanHandler(restoredObject))
                handler.Restore(ctx, m_Version, restoredObject, m_Parent);
        }

        restoredObject.VSM_OnAfterRestore();
        return restoredObject;
    }

    void RestoreChildren(ItemBase parent)
    {
        parent.VSM_OnBeforeRestoreChildren();
        foreach (VirtualObjectContext childObj : m_Children) {
            string virtualFile = m_VirtualContextDirectory + childObj.contextFile;
            
            if (!FileExist(virtualFile))
            {
                VSM_Error("RestoreChildren", "O arquivo virtual não existe " + virtualFile);
                return;
            }

            FileSerializer ctx = new FileSerializer();
            VirtualStorageFile virtualStorage = new VirtualStorageFile();
            if (!ctx.Open(virtualFile, FileMode.READ))
            {
                VSM_Error("RestoreChildren", "Não foi possível abrir o arquivo virtual" + virtualFile);
                return;
            }

            VirtualObject obj = new VirtualObject(m_VirtualContextDirectory, parent);
            ItemBase restored = obj.OnRestore(ctx);
            m_ProcessedItems.Insert(obj);
            m_FilesToDelete.Insert(virtualFile);
            ctx.Close();
        } 
        parent.VSM_OnAfterRestoreChildren();      
    }

    void OnVirtualize(ParamsWriteContext ctx, ItemBase childItem)
    {
        m_Classname = childItem.GetType();
        InventoryLocation invLocation = new InventoryLocation();
        childItem.GetInventory().GetCurrentInventoryLocation(invLocation);

        m_InvRow = invLocation.GetRow();
        m_InvCol = invLocation.GetCol();
        m_InvIdX = invLocation.GetIdx();
        m_InvSlotId = invLocation.GetSlot();
        m_InvType = invLocation.GetType();
        m_InvFliped = invLocation.GetFlip();

        VSM_Debug("OnVirtualize", m_Classname + " m_InvRow=" + m_InvRow + " m_InvCol=" + m_InvCol + " m_InvIdX=" + m_InvIdX + " m_InvSlotId=" + m_InvSlotId + " m_InvType=" + m_InvType + " m_InvFliped=" + m_InvFliped);
        
        if (childItem.HasQuantity())
            m_Quantity = childItem.GetQuantity();

        VirtualizeChildren(childItem);
        OnStoreSave(ctx); //! salvar este contexto antes dos handers
        
        // handlers
        OnRegisterObjectHandler();
        foreach (VirtualObjectHandler_Base handler : m_Handlers)
        {
            if (handler.CanHandler(childItem))
                handler.Virtualize(ctx, childItem, m_Parent);
        }    
    }

    void VirtualizeChildren(ItemBase parent)
    {
        VSM_Debug("VirtualizeChildren", parent.GetType() + " init");
        parent.VSM_OnBeforeVirtualizeChildren();
        array<EntityAI> items = new array<EntityAI>;
        parent.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);

        int total = items.Count();
        for (int i = 0; i < total; i++)
        {
            ItemBase child = ItemBase.Cast(items.Get(i));
            if (child && child.VSM_CanVirtualize())
            {

                VSM_Debug("VirtualizeChildren", child.GetType() + " salvando child" + child.GetType());

                FileSerializer ctx = new FileSerializer();
                string fileName = child.VSM_GetId() + ".bin";
                string virtualFile = m_VirtualContextDirectory + fileName;
                
                if (FileExist(virtualFile))
                    DeleteFile(virtualFile);

                if (!ctx.Open(virtualFile, FileMode.WRITE))
                {
                    VSM_Error("OnStart", "Não foi possível abrir o arquivo virtual para escrever: " + virtualFile);
                    return;
                }

                child.VSM_OnBeforeVirtualize();
                VirtualObject virtualObj = new VirtualObject(m_VirtualContextDirectory, parent); 
                virtualObj.OnVirtualize(ctx, child );

                ctx.Close();

                VirtualObjectContext obj = new VirtualObjectContext();
                obj.virtualId = child.VSM_GetId();
                obj.contextFile = fileName;

                m_Children.Insert(obj);  
            }
        }

        parent.VSM_OnAfterVirtualizeChildren();
        VSM_Debug("VirtualizeChildren", "%1 terminado, childrens: %2",parent.GetType(), m_Children.Count().ToString());
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
        return item;
    }

    ItemBase SpawnItemOnCargo(string classname, ItemBase container, int idx, int row, int col, bool isFliped)
    {
        VSM_Trace("SpawnItemOnCargo", "classname: %1, container: %2, idx: %3, row: %4, col: %5, isFliped: %6", classname, container.GetType(), idx.ToString(), row.ToString(), col.ToString(), isFliped.ToString());
        GameInventory parentInv = GameInventory.Cast(container.GetInventory());
        ItemBase item = ItemBase.Cast(parentInv.CreateEntityInCargoEx(classname, idx, row, col, isFliped));
        return item;
    }

    bool IsValidSpawnInInv()
    {
        return m_InvIdX != -1 && m_InvRow != -1 && m_InvRow != -1;
    }

    int GetVersion()
    {
        return m_Version;
    }

    void SetVersion(int version)
    {
        m_Version = version;
    }
    
    void OnRestoreComplete()
    {
        foreach (VirtualObjectHandler_Base handler : m_Handlers)
        {
            handler.OnRestoreComplete();
        }

        foreach (VirtualObject obj : m_ProcessedItems) {
            if (obj) obj.OnRestoreComplete();
        }

        VSM_Debug("OnComplete", "Iniciando delete de arquivos" + m_FilesToDelete.Count());
        foreach (string file : m_FilesToDelete) {
            if (FileExist(file)) {
                DeleteFile(file);
                VSM_Debug("OnComplete", "Arquivo de contexto deletado: " + file);
            }
            else 
            {
                VSM_Debug("OnComplete", "Arquivo de contexto não existe: " + file);
            }
        }

    }

    void OnVirtualizeComplete()
    {
        foreach (VirtualObjectHandler_Base handler : m_Handlers)
        {
            handler.OnVirtualizeComplete();
        }
    }
}