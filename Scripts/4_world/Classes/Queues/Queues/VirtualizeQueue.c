/**
 * @brief Classe de fila para o processo de virtualização de itens
 * @note Esta classe é usada para gerenciar o processo de virtualização de itens, quando um storage é fechado no mundo
 * @note Nesta classe a fila é usada apenas para deletar os itens do mundo, pois a virtualização é criada em OnStart
 */
class VSMVirtualizeQueue extends VSMBulkProcessingQueue
{
    protected ref array<ItemBase>               m_ItemsToDelete;
    protected ref VSMVirtualContainer           m_vContainer;

    override bool OnInit()
    {
        if(!super.OnInit()) return false;

        m_Container.VSM_SetIsProcessing(true);

        if (!m_Ctx.Open(m_VirtualFilePath, FileMode.WRITE))
		{
			VSM_Error("OnInit", "Não foi possível abrir o arquivo virtual para escrever: " + m_VirtualFilePath);
            m_Container.VSM_SetIsProcessing(false);
            Stop();
            return false;
		}

        if (!m_StoreCtx.Open(m_VirtualCtxFilePath, FileMode.WRITE))
		{
			VSM_Error("OnInit", "Não foi possível abrir o arquivo virtual para escrever: " + m_VirtualCtxFilePath);
            m_Container.VSM_SetIsProcessing(false);
            Stop();
            return false;
		}

        m_vContainer        = new VSMVirtualContainer();
        m_vContainer.Virtualize(m_Container, m_Ctx, m_StoreCtx);

        m_ItemCount         = m_vContainer.m_Cargo.Count();
        m_ItemsToDelete     = new array<ItemBase>();

        m_Metadata.SetVersion(VSM_StorageVersion.CURRENT_VERSION);
        m_Metadata.SetItemCount(m_ItemCount);
        m_Metadata.SetVirtualId(m_Container.VSM_GetId());
        return true;
    }

	override void OnStart()
	{
		VSM_Info("OnStart", m_Container.GetType() + " iniciando processo de virtualização de itens items: " + m_ItemCount);

        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZING);
        m_Container.VSM_OnBeforeContainerVirtualize();

        // bloqueia o container durante o processo (evitar players interagindo)
        m_Container.VSM_SetHasItems(m_ItemCount > 0); 
        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZED);

        // garantir integridade do arquivo, pois a fila é usada apenas para deletar os itens do mundo
        m_Ctx.Close();
        m_StoreCtx.Close();
	}
	
    override void OnTick(int idx)
    {
        VSMVirtualItem vItem = m_vContainer.m_Cargo.Get(idx);
        if(!vItem) return;

        DeleteItem(vItem);
    }

    private void DeleteItem(VSMVirtualItem vItem)
    {
        if(vItem.m_Item && vItem.m_Item.VSM_IsVirtualizable() && vItem.m_Delete)
        {
            VSM_Debug("OnTick", "Excluindo %1", vItem.m_Item.GetType());
            vItem.m_Item.Delete();
        };

        for (int i = 0; i < vItem.m_Attachments.Count(); i++)
        {
            DeleteItem(vItem.m_Attachments.Get(i));
        }

        for (int j = 0; j < vItem.m_Cargo.Count(); j++)
        {
            DeleteItem(vItem.m_Cargo.Get(j));
        }
    }

    override void OnComplete()
    {
        super.OnComplete();

        m_Container.VSM_OnAfterContainerVirtualize();
        m_Container.VSM_SetIsProcessing(false);
        m_Container.VSM_SetIgnoreVirtualization(false);
        if( m_Container.VSM_IsLoaded())
			    m_Container.VSM_SetVirtualLoaded(true);

        VSM_Info("OnComplete", m_Container.GetType() + " processo de virtualização completo");
    }

    override void OnStop()
    {
        super.OnStop();

        if(!m_Completed)
        {
            m_Container.VSM_SetIgnoreVirtualization(false);
            m_Container.VSM_SetIsProcessing(false);
            if( m_Container.VSM_IsLoaded())
			    m_Container.VSM_SetVirtualLoaded(true);
        }

    }

    /* apenas restaurar o estado do metadata e fechar o container */
    override void OnRestart()
    {
        VSM_Info("OnRestart", m_Container.GetType() + " restaurando estado fechado");

        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZED);
        ClearInventory();
        if(m_Container.VSM_IsOpen())
        {
            m_Container.VSM_SetIgnoreVirtualization(true);
            m_Container.VSM_Close();
            m_Container.SetSynchDirty();
            m_Container.VSM_SetIgnoreVirtualization(false);
        }

        Stop();
    }

    /* limpar possiveis itens restaurados e restaurar estado de fechado */
    override void OnSync()
    {
        VSM_Info("OnSync", m_Container.GetType() + " sincronizando processo de virtualização");

        ClearInventory();
        if(m_Container.VSM_IsOpen())
        {
            m_Container.VSM_SetIgnoreVirtualization(true);
            m_Container.VSM_Close();
            m_Container.SetSynchDirty();
            m_Container.VSM_SetIgnoreVirtualization(false);
        }

        Stop();
    }
}