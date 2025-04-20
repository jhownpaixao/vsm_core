class VirtualObject_Legacy
{
    string m_Classname;
    string m_Type;

    //inventory
    int m_InvRow;
    int m_InvCol;
    int m_InvType;
    int m_InvIdX;
    int m_InvSlotId;
    bool m_InvFliped;

    int m_Quantity = 0;
    int m_LiquidType = 0;
    float m_Health;
    ref array<ref VirtualObject_Legacy> m_Children = { };

    //handlers
    ref VSM_AmmunitionHandler_Legacy m_AmmoHandler;
    ref VSM_MagazineHandler_Legacy m_MagazineHandler;
    ref VSM_WeaponHander_Legacy m_WeaponHandler;
    ref VSM_StoreHandler_Legacy m_StoreHandler;

    void RegisterObjectHandler()
    {
        m_AmmoHandler = new VSM_AmmunitionHandler_Legacy();
        m_MagazineHandler = new VSM_MagazineHandler_Legacy();
        m_WeaponHandler = new VSM_WeaponHander_Legacy();
        m_StoreHandler = new VSM_StoreHandler_Legacy();

        if (VirtualStorageModule.GetModule().m_Debug)
            Print("RegisterObjectHandler: " + m_Classname);
    }

    void OnVirtualize(string virtualPath, ItemBase item, ItemBase parent)
    {
        m_Classname = item.GetType();
        if (VirtualStorageModule.GetModule().m_Debug)
            Print("OnVirtualize: " + m_Classname + " iniciando");

        //inventory
        InventoryLocation invLocation = new InventoryLocation();
        item.GetInventory().GetCurrentInventoryLocation(invLocation);

        m_InvRow = invLocation.GetRow();
        m_InvCol = invLocation.GetCol();
        m_InvIdX = invLocation.GetIdx();
        m_InvSlotId = invLocation.GetSlot();
        m_InvType = invLocation.GetType();
        m_InvFliped = invLocation.GetFlip();

        if (VirtualStorageModule.GetModule().m_Debug)
            Print("OnVirtualize: " + m_Classname + " m_InvRow=" + m_InvRow + " m_InvCol=" + m_InvCol + " m_InvIdX=" + m_InvIdX + " m_InvSlotId=" + m_InvSlotId + " m_InvType=" + m_InvType + " m_InvFliped=" + m_InvFliped);
        // condition
        m_Health = item.GetHealth();

        //foodstage
        //temp
        //wetness


        //quantity
        if (item.HasQuantity())
        {
            m_Quantity = item.GetQuantity();
            if (item.IsLiquidContainer())
                m_LiquidType = item.GetLiquidType();

            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_Quantity=" + m_Quantity);
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_LiquidType=" + m_LiquidType);

        }
        if (VirtualStorageModule.GetModule().m_Debug)
            Print("OnVirtualize: " + m_Classname + " init handlers");

        RegisterObjectHandler();
        if (m_AmmoHandler.CanHandler(item))
        {
            m_AmmoHandler.OnVirtualize(virtualPath, item, parent);
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_AmmoHandler handler ok ");
        }
        else
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_AmmoHandler handler recused ");
        }

        if (m_MagazineHandler.CanHandler(item))
        {
            m_MagazineHandler.OnVirtualize(virtualPath, item, parent);
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_MagazineHandler handler ok ");
        }
        else
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_MagazineHandler handler recused ");
        }

        if (m_WeaponHandler.CanHandler(item))
        {
            m_WeaponHandler.OnVirtualize(virtualPath, item, parent);
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_WeaponHandler handler ok ");
        }
        else
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_WeaponHandler handler recused ");
        }

        if (m_StoreHandler.CanHandler(item))
        {
            m_StoreHandler.OnVirtualize(virtualPath, item, parent);
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_StoreHandler handler ok ");
        }
        else
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnVirtualize: " + m_Classname + " m_StoreHandler handler recused ");
        }

        VirtualizeChildren(virtualPath, item);
    }

    void OnRestore(string virtualPath, ItemBase parent)
    {
        if (VirtualStorageModule.GetModule().m_Debug)
            Print("OnRestore: " + m_Classname + " m_Children=" + m_Children.Count());

        ItemBase restoredObject;

        if (m_InvSlotId != -1)
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnRestore: " + m_Classname + " spawn attachment");

            restoredObject = ItemBase.Cast(parent.GetInventory().CreateAttachmentEx(m_Classname, m_InvSlotId));
        }
        else
        {
            GameInventory parentInv = GameInventory.Cast(parent.GetInventory());
            if (!parentInv)
            {
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " inventory parent fail");
                return;
            }

            parentInv.UnlockInventory(HIDE_INV_FROM_SCRIPT);
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnRestore: " + m_Classname + " unloking inv");


            if (IsValidSpawnInInv())
            {
                restoredObject = ItemBase.Cast(parentInv.CreateEntityInCargoEx(m_Classname, m_InvIdX, m_InvRow, m_InvCol, m_InvFliped));
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " spawn on inv, m_InvIdX=" + m_InvIdX + " m_InvRow=" + m_InvRow + " m_InvCol=" + m_InvCol + " m_InvFliped=" + m_InvFliped);
            }
            else
            {
                restoredObject = ItemBase.Cast(GetGame().CreateObject(m_Classname, parent.GetPosition(), false, false, true));
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " spawn on ground");

            }
        }

        if (restoredObject)
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnRestore: " + m_Classname + " restoredObject is valid");

            restoredObject.SetQuantity(m_Quantity);
            restoredObject.SetHealth(m_Health);


            restoredObject.VSM_OnBeforeRestoreChildren();
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnRestore: " + m_Classname + " starting spawn children");

            foreach (VirtualObject_Legacy childObj : m_Children) {
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " restoring child " + childObj.m_Classname);
                childObj.OnRestore(virtualPath, restoredObject);
            }
            restoredObject.VSM_OnAfterRestoreChildren();
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnRestore: " + m_Classname + " spawn children ok");

            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnRestore: " + m_Classname + " starting handlers");

            if (m_AmmoHandler.CanHandler(restoredObject))
            {
                m_AmmoHandler.OnRestore(virtualPath, restoredObject, parent);
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " m_AmmoHandler handler ok ");
            }
            else
            {
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " m_AmmoHandler handler recused ");
            }

            if (m_MagazineHandler.CanHandler(restoredObject))
            {
                m_MagazineHandler.OnRestore(virtualPath, restoredObject, parent);
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " m_MagazineHandler handler ok ");
            }
            else
            {
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " m_MagazineHandler handler recused ");
            }

            if (m_WeaponHandler.CanHandler(restoredObject))
            {
                m_WeaponHandler.OnRestore(virtualPath, restoredObject, parent);
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " m_WeaponHandler handler ok ");
            }
            else
            {
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " m_WeaponHandler handler recused ");
            }

            if (m_StoreHandler.CanHandler(restoredObject))
            {
                m_StoreHandler.OnRestore(virtualPath, restoredObject, parent);
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " m_StoreHandler handler ok ");
            }
            else
            {
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("OnRestore: " + m_Classname + " m_StoreHandler handler recused ");
            }

            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnRestore: " + m_Classname + " handlers ok");
            restoredObject.VSM_OnAfterRestore();
        }
        else
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("OnRestore: " + m_Classname + " restoredObject is not valid!!!!");
        }

        if (VirtualStorageModule.GetModule().m_Debug)
            Print("OnRestore: " + m_Classname + " terminated");

    }

    void VirtualizeChildren(string virtualPath, ItemBase item)
    {
        if (VirtualStorageModule.GetModule().m_Debug)
            Print("VirtualizeChildren: " + item.GetType() + " init");
        array<EntityAI> items = new array<EntityAI>;
        item.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);

        int total = items.Count();
        for (int i = 0; i < total; i++)
        {
            ItemBase child = ItemBase.Cast(items.Get(i));
            if (child)
            {
                if (VirtualStorageModule.GetModule().m_Debug)
                    Print("VirtualizeChildren: " + item.GetType() + " saving " + child.GetType());

                VirtualObject_Legacy childObj = new VirtualObject_Legacy();
                childObj.OnVirtualize(virtualPath, child, item);
                m_Children.Insert(childObj);
            }
        }

        if (VirtualStorageModule.GetModule().m_Debug)
            Print("VirtualizeChildren: " + item.GetType() + " terminated ");

    }

    bool IsValidSpawnInInv()
    {
        return m_InvIdX != -1 && m_InvRow != -1 && m_InvRow != -1;
    }
}