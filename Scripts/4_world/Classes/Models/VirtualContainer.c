class VSMVirtualContainer : VSMBase
{
    int     m_Version;
    string  m_Classname;
    string  m_ID;

    ref array<ref VSMVirtualItem> m_Cargo;

    void VSMVirtualContainer()
    {
        m_Cargo         = new array<ref VSMVirtualItem>;
    }

    void Virtualize(ItemBase container, ParamsWriteContext ctx, ParamsWriteContext ctxStore)
    {
        m_Version       = VSM_StorageVersion.CURRENT_VERSION; //! força para última versão
        VSM_Debug("Virtualize", "Iniciando virtualização de container: " + container.GetType() + " com versão: " + m_Version.ToString());

        m_Classname = container.GetType();
        m_ID = container.GetID().ToString();

        CreateVirtualChildren(container, ctx, ctxStore);
        Save(ctx);

        VSM_Debug("Virtualize", "Arquivo de virtualização preenchido: " + m_Classname + " com itens: " + m_Cargo.Count().ToString());
    }

    bool Restore(ParamsReadContext ctx, ParamsReadContext storeCtx, ItemBase container, bool grounded = false)
    {
        if(!Load(ctx))
            return false;

        VSM_Debug("Restore", "Arquivo virtual carregado: " + m_Classname + " com versão: " + m_Version.ToString());
        return true;
    }

    protected void Save(ParamsWriteContext ctx)
    {
        ctx.Write(m_Version);
        ctx.Write(m_Classname);
        ctx.Write(m_ID);
        ctx.Write(m_Cargo);
    }

    bool Load(ParamsReadContext ctx)
    {   
        if(!ctx.Read(m_Version))
            return false;
        if(!ctx.Read(m_Classname))
            return false;
        if(!ctx.Read(m_ID))
            return false;
        if(!ctx.Read(m_Cargo))
            return false;

        return true;
    }

    void CreateVirtualChildren(ItemBase container, ParamsReadContext ctx, ParamsReadContext storeCtx)
    {
        VSM_Debug("CreateVirtualChildren", "Iniciando criação de filhos virtuais para: " + container.GetType());
        GameInventory inv   = container.GetInventory();
        CargoBase cargo     = inv.GetCargo();
        ItemBase item;
		VSMVirtualItem vItem;
        int count;

        count = inv.AttachmentCount();
        for (int i = 0; i < count; i++)
		{
            VSM_Debug("CreateVirtualChildren", "Criando filho virtual para: " + inv.GetAttachmentFromIndex(i).GetType());
			item = ItemBase.Cast(inv.GetAttachmentFromIndex(i));

            vItem = new VSMVirtualItem();
            vItem.Virtualize(item, ctx, storeCtx, m_Version);

            m_Cargo.Insert(vItem);
            VSM_Debug("CreateVirtualChildren", "Filho virtual criado: " + item.GetType());
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

                m_Cargo.Insert(vItem);
                VSM_Debug("CreateVirtualChildren", "Filho virtual criado: " + item.GetType());
            }
        }

        VSM_Debug("CreateVirtualChildren", "Criação dos filhos concluída: " + container.GetType() + " com itens virtuais: " + m_Cargo.Count().ToString());
    }
}