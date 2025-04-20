class VirtualObject : VSM_Base
{
    //!v100 não alterar
    int m_Version;

    /* Caracteristicas */
    // v100 @{
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
    // v100 @}

    /* Operacional */
    protected string m_VirtualContextDirectory;
    protected ItemBase m_Parent;
    protected ref array<ref VirtualObjectHandler_Base> m_Handlers = { };

    void VirtualObject(string virtualPath, ItemBase parent)
    {
        m_VirtualContextDirectory = virtualPath;
        m_Parent = parent;
        m_Version = VSM_STORAGE_VERSION; //! força para última versão
    }

    bool OnStoreLoad(ParamsReadContext ctx)
    {
        if (!ctx.Read(m_Version))
        {
            VSM_Warn("OnStoreLoad", "não foi possível ler a versão do arquivo virtual");
            return false;
        }

        if (!ctx.Read(m_Classname))
        {
            VSM_Warn("OnStoreLoad", "não foi possível m_Classname");
            return false;
        }
        if (!ctx.Read(m_Type))
        {
            VSM_Warn("OnStoreLoad", "não foi possível m_Type");
            return false;
        }

        if (!ctx.Read(m_InvRow))
        {
            VSM_Warn("OnStoreLoad", "não foi possível ler m_InvRow");
            return false;
        }
        if (!ctx.Read(m_InvCol))
        {
            VSM_Warn("OnStoreLoad", "não foi possível ler m_InvCol");
            return false;
        }
        if (!ctx.Read(m_InvType))
        {
            VSM_Warn("OnStoreLoad", "não foi possível ler m_InvType");
            return false;
        }
        if (!ctx.Read(m_InvIdX))
        {
            VSM_Warn("OnStoreLoad", "não foi possível ler m_InvIdX");
            return false;
        }
        if (!ctx.Read(m_InvSlotId))
        {
            VSM_Warn("OnStoreLoad", "não foi possível ler m_InvSlotId");
            return false;
        }
        if (!ctx.Read(m_InvFliped))
        {
            VSM_Warn("OnStoreLoad", "não foi possível ler m_InvFliped");
            return false;
        }

        if (!ctx.Read(m_Quantity))
        {
            VSM_Warn("OnStoreLoad", "não foi possível m_Quantity");
            return false;
        }


        if (!ctx.Read(m_Children)) 
        {
            VSM_Warn("OnStoreLoad", "não foi possível ler m_Children");
            return false;
        };

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
            VSM_Error("OnRestore", " Não é foi possível ler o contexto do item.");
            return restoredObject;
        }

        VSM_Info("OnRestore", "%1 iniciando. VERSÃO %3", m_Classname, m_Version.ToString());
        OnRegisterObjectHandler();


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
        
        if (!restoredObject)
            restoredObject = SpawnItemOnGround(m_Classname, m_Parent.GetPosition());


        // if (grounded)
        // {
        //     VSM_Debug("OnRestore", m_Classname + " forçar spawn no chão");
        //     restoredObject = SpawnItemOnGround(m_Classname, m_Parent.GetPosition());
        // }
        // else if (m_InvSlotId != -1)
        // {
        //     VSM_Debug("OnRestore", m_Classname + " spawn no slot");
        //     restoredObject = SpawnItemOnSlot(m_Classname, m_Parent, m_InvSlotId);
        // }
        // else
        // {
        //     GameInventory parentInv = GameInventory.Cast(m_Parent.GetInventory());
        //     if (!parentInv)
        //     {
        //         VSM_Error("OnRestore", m_Classname + " não foi possível spawnar o item: inventário inacessível");
        //         return restoredObject;
        //     }

        //     parentInv.UnlockInventory(HIDE_INV_FROM_SCRIPT);
        //     if (IsValidSpawnInInv())
        //     {
        //         VSM_Debug("OnRestore", m_Classname + " spawn no inventário");
        //         restoredObject = SpawnItemOnCargo(m_Classname, m_Parent, m_InvIdX, m_InvRow, m_InvCol, m_InvFliped);
        //     }
        //     else
        //     {
        //         VSM_Debug("OnRestore", m_Classname + " spawn no chão");
        //         restoredObject = SpawnItemOnGround(m_Classname, m_Parent.GetPosition());
        //     }
        // }

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
        } 
        parent.VSM_OnAfterRestoreChildren();      
    }

    void OnVirtualize(ParamsWriteContext ctx, ItemBase childItem)
    {
        m_Classname = childItem.GetType();
        VSM_Info("OnVirtualize", m_Classname + " iniciando virtualização | VERSÃO %1", m_Version.ToString());

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
        VSM_Info("VirtualizeChildren", parent.GetType() + " init");
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
        ItemBase item = ItemBase.Cast(GetGame().CreateObject(m_Classname, pos, false, false, true));
        return item;
    }

    ItemBase SpawnItemOnSlot(string classname, ItemBase container, int slotId)
    {
        ItemBase item;
        if (container.IsWeapon() && GetGame().IsKindOf(classname, "Magazine"))
        {
            Weapon_Base weapon = Weapon_Base.Cast(container);
            item = weapon.SpawnAttachedMagazine(classname, WeaponWithAmmoFlags.NONE);
        }
        else
        {
            item = ItemBase.Cast(container.GetInventory().CreateAttachmentEx(classname, slotId));
        }
        return item;
    }

    ItemBase SpawnItemOnCargo(string classname, ItemBase container, int idx, int row, int col, bool isFliped)
    {
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
    }

    void OnVirtualizeComplete()
    {
        foreach (VirtualObjectHandler_Base handler : m_Handlers)
        {
            handler.OnVirtualizeComplete();
        }
    }
}