#ifdef EXPANSIONMODBASEBUILDING
modded class ExpansionCodeLock
{
    override bool VSM_CanVirtualize()
    {
        ItemBase parent = ItemBase.Cast(GetHierarchyParent());
        if (parent)
        {
            InventoryLocation location = new InventoryLocation();
            GetInventory().GetCurrentInventoryLocation(location);
            
            int slotID = location.GetSlot();
            if (slotID != -1)
                return false;
        }

        return true;
    }
}
#endif