class VSM_RestorationQueue extends BatchQueue_Base
{
    protected bool m_ForceSpawnOnGround;
    protected ref array<ref VirtualObjectContext> m_Items; // dados dos items para processar

    protected ref array<ref VirtualObject> m_ProcessedItems; // itens processados
    protected ref array<ItemBase> m_SpawnedItems; // versão spawnda do item
    protected ref array<string> m_FilesToDelete; // arquivos a serem deletados após o processamento

    protected ref VirtualStorageFile m_VirtualStorageFile;

    override bool OnInit()
    {
        FileSerializer serializer = new FileSerializer();
		m_VirtualStorageFile = new VirtualStorageFile();

        if (!serializer.Open(m_VirtualFilePath, FileMode.READ) || !serializer.Read(m_VirtualStorageFile))
		{
			VSM_Error("OnInit", "Não foi possível abrir o arquivo virtual" + m_VirtualFilePath);
            Stop();
			return false;
		}

        m_Container.VSM_SetIsProcessing(true);
        m_Items = m_VirtualStorageFile.storedItems;
        m_ItemCount = m_Items.Count();
        m_ForceSpawnOnGround = false;
        m_SpawnedItems = new array<ItemBase>;
        m_FilesToDelete = new array<string>;
        m_ProcessedItems = new array<ref VirtualObject>;

        return super.OnInit();
    }

    override void OnStart()
    {
        super.OnStart();
        m_Metadata.SetLastOperationState(VirtualStorageState.RESTORING);
        m_Container.VSM_OnBeforeContainerRestore(); //prepare
        VSM_Info("OnStart", m_Container.GetType() + " iniciando processo de restauração de itens items: " + m_ItemCount);
    }

    override void OnTick(int idx)
    {
        VirtualObjectContext item = m_Items.Get(idx);
        string virtualFile = m_VirtualContextDirectory + item.contextFile;
        if (!FileExist(virtualFile))
        {
            VSM_Error("OnTick", "O arquivo virtual não existe " + virtualFile);
            return;
        }

        FileSerializer ctx = new FileSerializer();
		VirtualStorageFile virtualStorage = new VirtualStorageFile();
        if (!ctx.Open(virtualFile, FileMode.READ))
        {
            VSM_Error("OnTick", "Não foi possível abrir o arquivo virtual" + virtualFile);
            return;
        }

        VirtualObject obj = new VirtualObject(m_VirtualContextDirectory, m_Container);
        m_ProcessedItems.Insert(obj);

        ItemBase restored = obj.OnRestore(ctx, m_ForceSpawnOnGround);
        if(restored)
            m_SpawnedItems.Insert(restored);

        m_FilesToDelete.Insert(virtualFile);
    }

    override void OnComplete()
    {
        super.OnComplete();
        m_Container.VSM_OnAfterContainerRestore();
        m_Container.VSM_SetHasItems(false);
        m_Container.VSM_SetIsProcessing(false);
        m_Metadata.SetLastOperationState(VirtualStorageState.RESTORED);

        foreach (VirtualObject obj : m_ProcessedItems) {
            if (obj) obj.OnRestoreComplete();
        }

        VSM_Debug("OnComplete", "Iniciando delete de arquivos: " + m_FilesToDelete.Count());

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
       VSM_Info("OnComplete", m_Container.GetType() + " processo de restauração de itens concluido");
    }

    override void OnRollback()
    {
        m_Container.VSM_SetIsProcessing(true);
        foreach (ItemBase item : m_SpawnedItems) {
			if(item)
                item.Delete();
		}
        m_Container.VSM_Close();
        m_Container.VSM_SetIsProcessing(false);
        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZED);
    }
    
    override void OnRestart()
    {
        ClearInventory();
        m_Container.VSM_Open();
        m_Container.Update();
        Start();
    }

    override void OnSync()
    {
        m_Container.VSM_SetIsProcessing(true);
        m_Container.VSM_Open();
        m_Container.Update();
        m_Container.VSM_SetIsProcessing(false);
        Stop();
    }

    void LoadItems()
    {

    }
}
