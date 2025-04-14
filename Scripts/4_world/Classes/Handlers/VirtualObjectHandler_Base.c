class VirtualObjectHandler_Base
{
    void OnVirtualize(string virtualPath, ItemBase virtualize, ItemBase parent) { }

    void OnRestore(string virtualPath, ItemBase restored, ItemBase parent) { }

    bool CanHandler(ItemBase item)
    {
        return true;
    }
}