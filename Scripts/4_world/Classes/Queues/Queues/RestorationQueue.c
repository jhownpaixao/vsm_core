/** 
 * @brief Classe de fila para o processo de restauração de itens
 * @note Esta classe é usada para gerenciar o processo de restauração de itens, quando um storage é aberto no mundo
 */
class VSMRestorationQueue extends VSMBulkProcessingQueue
{
    protected ref VSMVirtualContainer           m_vContainer;
    protected bool                              m_ForceSpawnOnGround;
    protected ref array<ItemBase>               m_SpawnedItems; // versão spawnda do item (para fins de controle)
    override bool OnInit()
    {
        if(!super.OnInit()) return false;

        if (!m_Ctx.Open(m_VirtualFilePath, FileMode.READ))
		{
			VSM_Error("OnInit", "Não foi possível abrir o arquivo virtual" + m_VirtualFilePath);
            m_Container.VSM_SetIsProcessing(false);
            Stop();
			return false;
		}

        if (!m_StoreCtx.Open(m_VirtualCtxFilePath, FileMode.READ))
		{
			VSM_Error("OnInit", "Não foi possível abrir o arquivo virtual para escrever: " + m_VirtualCtxFilePath);
            m_Container.VSM_SetIsProcessing(false);
            Stop();
            return false;
		}

        m_vContainer = new VSMVirtualContainer();
        if(!m_vContainer.Load(m_Ctx))
        {
            VSM_Error("OnInit", "Não foi possível carregar o container virtual: " + m_Container.GetType());
            m_Container.VSM_SetIsProcessing(false);
            Stop();
            return false;
        }

        m_Container.VSM_SetIsProcessing(true);

        m_ItemCount             = m_vContainer.m_Cargo.Count();
        m_ForceSpawnOnGround    = false;
        m_SpawnedItems          = new array<ItemBase>;

        return true;
    }

    override void OnStart()
    {
        super.OnStart();

        Backup();
        m_Metadata.SetLastOperationState(VirtualStorageState.RESTORING);
        m_Container.VSM_OnBeforeContainerRestore();

        VSM_Info("OnStart", m_Container.GetType() + " iniciando processo de restauração de itens items: " + m_ItemCount);
    }

    override void OnTick(int idx)
    {
        VSMVirtualItem vItem = m_vContainer.m_Cargo.Get(idx);

        //! restauração
        ItemBase restored = vItem.Restore(m_vContainer.m_Version, m_Container, m_Ctx, m_StoreCtx, m_ForceSpawnOnGround);
        if(restored) m_SpawnedItems.Insert(restored);
    }

    override void OnComplete()
    {
        super.OnComplete();

        m_Metadata.SetLastOperationState(VirtualStorageState.RESTORED);
        m_Container.VSM_OnAfterContainerRestore();
        m_Container.VSM_SetHasItems(false);

        m_Ctx.Close();
        m_StoreCtx.Close();

        m_Container.VSM_SetIsProcessing(false);
        m_Container.VSM_SetIgnoreVirtualization(false);
        if( m_Container.VSM_IsLoaded())
		    m_Container.VSM_SetVirtualLoaded(true);

        VSM_Info("OnComplete", m_Container.GetType() + " processo de restauração de itens concluido");
    }

    override void OnRollback()
    {
        VSM_Info("OnRollback", m_Container.GetType() + " revertendo processo de restauração");

        m_Container.VSM_SetIsProcessing(true);

        for (int i = 0; i < m_SpawnedItems.Count(); i++)
		{
			ItemBase item = m_SpawnedItems[i];
            if(!item) continue;

            item.Delete();
		}

        m_Container.VSM_Close();
        m_Container.VSM_SetIsProcessing(false);
        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZED);
    }

    override void OnStop()
    {
        super.OnStop();

        if(!m_Completed)
        {
            m_Container.VSM_SetIsProcessing(false);
            m_Container.VSM_SetIgnoreVirtualization(false);

            if( m_Container.VSM_IsLoaded())
			    m_Container.VSM_SetVirtualLoaded(true);
        }
    }
    
    /* após a queda do servidor em restauração, apenas devolver o estado */
    override void OnRestart()
    {
        VSM_Info("OnRestart", m_Container.GetType() + " restaurando estado virtualizado");

        // limpa todos items possivelmente restaurados antes;
        ClearInventory(); 

        // restaura estado fechado
        if(m_Container.VSM_IsOpen())
        {
            m_Container.VSM_SetIgnoreVirtualization(true);
            m_Container.VSM_Close();
            m_Container.SetSynchDirty();
            m_Container.VSM_SetIgnoreVirtualization(false);
        }
            
        Stop();
    }

    override void OnSync()
    {
        // 1 - buscar ultimo backup (se não houver, ignora)
        // 2 - verificar se a quantidade de itens bate com com o que esta no container (conteinarItems < itemsSalvos)
        // 3 - como isto é chamado apenas na abertura, então limpar o container e subistituir arquivos com os do backup e devolver o estado para fechado.
        VSM_Info("OnSync", m_Container.GetType() + " sincronizando processo de restauração");
        

        if(m_Container.VSM_IsOpen())
        {
            //TODO: verificar se os itens restaurados ainda estão no inventário do container.
            //TODO: verificar se a quantidade no container bate com a previamente virtualizada no backup
            // if(HasBackup())
            // {
                
            // }
        }
        else
        {
            if(HasBackup())
            {
                RestoreBackup();
                ClearInventory();
                
                m_Container.VSM_SetIgnoreVirtualization(true);
                m_Container.VSM_Open();
                m_Container.SetSynchDirty();
                m_Container.VSM_SetIgnoreVirtualization(false);
            }
            else
            {
                VSM_Warn("OnSync", m_Container.GetType() + " o storage esta fechado (deveria esta aberto), não foi possível sincronizar, pois não há backup");
            }
        } 
       
        Stop();
    }

    void Backup()
    {
        string bckpPath = m_VirtualContextDirectory + "backup/";
        if (!FileExist(bckpPath)) MakeDirectory(bckpPath);

        DeleteBackup();

        CF_File mainBkp = new CF_File(m_VirtualFilePath);
        mainBkp.Copy(bckpPath + mainBkp.GetFileName());

        CF_File ctxBkp = new CF_File(m_VirtualCtxFilePath);
        ctxBkp.Copy(bckpPath + ctxBkp.GetFileName());

        CF_File metaBkp = new CF_File(m_Metadata.GetPath());
        metaBkp.Copy(bckpPath + metaBkp.GetFileName());
    }

    void RestoreBackup()
    {
        string bckpPath = m_VirtualContextDirectory + "backup/";
        if(!FileExist(bckpPath))
        {
            VSM_Warn("RestoreBackup", "Não foi possível restaurar o backup, pois não existe o diretório de backup: " + bckpPath);
            return;
        }

        CF_File main = new CF_File(m_VirtualFilePath);
        CF_File ctx = new CF_File(m_VirtualCtxFilePath);
        CF_File meta = new CF_File(m_Metadata.GetPath());

        string mainBkpPath = bckpPath + main.GetFileName();
        string ctxBkpPath = bckpPath + ctx.GetFileName();
        string metaBkpPath = bckpPath + meta.GetFileName();

        if(FileExist(mainBkpPath))
        {
            CF_File mainBkp = new CF_File(mainBkpPath);
            main.Delete();
            mainBkp.Copy(main.GetFullPath());
        }

        if(FileExist(ctxBkpPath))
        {
            CF_File ctxBkp = new CF_File(ctxBkpPath);
            ctx.Delete();
            ctxBkp.Copy(ctx.GetFullPath());
        }

        if(FileExist(metaBkpPath))
        {
            CF_File metaBkp = new CF_File(metaBkpPath);
            meta.Delete();
            metaBkp.Copy(meta.GetFullPath());
        }

        VSM_Info("RestoreBackup", "Backup restaurado com sucesso para o container: " + m_Container.GetType()); 
    }

    void DeleteBackup()
    {
        string bckpPath = m_VirtualContextDirectory + "backup/";
        if (!FileExist(bckpPath)) return;

        CF_File main = new CF_File(m_VirtualFilePath);
        CF_File ctx = new CF_File(m_VirtualCtxFilePath);
        CF_File meta = new CF_File(m_Metadata.GetPath());

        string mainBkpPath = bckpPath + main.GetFileName();
        string ctxBkpPath = bckpPath + ctx.GetFileName();
        string metaBkpPath = bckpPath + meta.GetFileName();

        if(FileExist(mainBkpPath))
        {
            CF_File mainBkp = new CF_File(mainBkpPath);
            mainBkp.Delete();
        }

        if(FileExist(ctxBkpPath))
        {
            CF_File ctxBkp = new CF_File(ctxBkpPath);
            ctxBkp.Delete();
        }

        if(FileExist(metaBkpPath))
        {
            CF_File metaBkp = new CF_File(metaBkpPath);
            metaBkp.Delete();
        }
    }

    bool HasBackup()
    {
        string bckpPath = m_VirtualContextDirectory + "backup/";
        if (!FileExist(bckpPath)) return false;

        bool hasMain =  FileExist(bckpPath + CF_Path.GetFileName(m_VirtualFilePath));
        bool hasCtx =  FileExist(bckpPath + CF_Path.GetFileName(m_VirtualCtxFilePath));
        bool hasMeta =  FileExist(bckpPath + CF_Path.GetFileName(m_Metadata.GetPath()));

        return hasMain && hasCtx && hasMeta;
    }

}
