class VirtualObjectHandler_Base : VSM_Base
{
	string m_VirtualPath;

	void VirtualObjectHandler_Base(string virtualPath) 
	{ 
		m_VirtualPath = virtualPath;
	}

	bool OnStoreLoad(ParamsReadContext ctx, int version);

	void OnStoreSave(ParamsWriteContext ctx);

	bool OnRestore(ItemBase restored, ItemBase parent);

	void OnRestoreComplete();

	void OnVirtualize(ItemBase virtualize, ItemBase parent);

	void OnVirtualizeComplete();

	void Virtualize(ParamsWriteContext ctx, ItemBase virtualize, ItemBase parent)
	{
		OnVirtualize(virtualize, parent);
		StoreSave(ctx);
	}

	bool Restore(ParamsReadContext ctx, int version, ItemBase restored, ItemBase parent)
	{
		if (StoreLoad(ctx, version))
			return OnRestore(restored, parent);
		
		return false;
	}

	bool StoreLoad(ParamsReadContext ctx, int version)
	{
		//TODO - implementar carregamento de valores bases
		return OnStoreLoad(ctx, version);
	}

	void StoreSave(ParamsWriteContext ctx)
	{
		//TODO - implementar salvamento de valores bases
		OnStoreSave(ctx);
	}

	bool CanHandler(ItemBase item)
	{
		return item != null;
	}

}