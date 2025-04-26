class VSM_VirtualizeQueue extends BatchQueue_Base
{
	const string VIRTUAL_FILENAME_BKP = ".bckp";
    protected ref array<EntityAI> m_Items;
    protected ref FileSerializer m_Serialize;
    protected ref VirtualStorageFile m_VirtualStorageFile;
	protected ref CF_File m_CFVirtualFile;

    protected ref array<ref VirtualObject> m_ProcessedItems;

    override bool OnInit()
    {
        m_Container.VSM_SetIsProcessing(true);
        m_Items = new array<EntityAI>;
        LoadItems();

        m_ItemCount = m_Items.Count();
        m_Serialize = new FileSerializer();
        m_VirtualStorageFile = new VirtualStorageFile();
        m_ProcessedItems = new array<ref VirtualObject>();
        //debug

        if (!m_Serialize.Open(m_VirtualFilePath, FileMode.WRITE))
		{
			VSM_Error("OnSaveVirtualStore", "Não foi possível abrir o arquivo virtual para escrever: " + m_VirtualFilePath);
            m_Container.VSM_SetIsProcessing(false);
            Stop();
            return false;
		}

        m_Metadata.SetVersion(VSM_StorageVersion.CURRENT_VERSION); //!save lastversion
		m_CFVirtualFile = new CF_File(m_VirtualFilePath);
        CreateBackup();
        
        return super.OnInit();
    }

	override void OnStart()
	{
		VSM_Info("OnStart", m_Container.GetType() + " iniciando processo de virtualização de itens items: " + m_ItemCount);

        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZING);
        m_Container.VSM_OnBeforeContainerVirtualize();

        CreateVirtualizedItems();

        //! gravar arquivo virtual
        if (!m_Serialize.Write(m_VirtualStorageFile))
		{
			VSM_Error("OnSaveVirtualStore", "Não foi possível salvar o arquivo virtual: " + m_VirtualFilePath);
            m_Metadata.SetLastOperationState(VirtualStorageState.RESTORED);
            Stop();
            return;
        }

        m_Container.VSM_SetHasItems(m_ItemCount != 1); //!fix enquanto os itens são deletados aparece a opção de "pegar na mão" para itens como barril
        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZED);
	}
	
    override void OnTick(int idx)
    {
        ItemBase item = ItemBase.Cast(m_Items.Get(idx));

        if(item && item.VSM_CanVirtualize())
        {
            VSM_Debug("OnAfterBatchProcess", "Excluindo %1", item.GetType());
            item.VSM_OnAfterVirtualize();
            item.Delete();
        };
    }

    override void OnComplete()
    {
        super.OnComplete();

        m_Container.VSM_OnAfterContainerVirtualize();
        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZED);
        DeleteBackup();

        foreach (VirtualObject obj : m_ProcessedItems) {
            if (obj) 
            obj.OnVirtualizeComplete();
        }
    
        m_Serialize.Close();
        m_Container.VSM_SetIsProcessing(false);
        VSM_Info("OnComplete", m_Container.GetType() + " processo de virtualização completo");
    }

    override void OnRestart()
    {
		RestoreBackup();
        m_Container.VSM_Close();
        Start();
    }

    override void OnSync()
    {
        m_Container.VSM_SetIsProcessing(true);


        FileSerializer serializer = new FileSerializer();
		VirtualStorageFile virtualStorage = new VirtualStorageFile();

        if (serializer.Open(m_VirtualFilePath, FileMode.READ))
        {
            if (serializer.Read(virtualStorage))
            {
                ClearInventory();

                if(m_Container.VSM_IsOpen())
                    m_Container.VSM_Close();

                bool hasItems = virtualStorage.storedItems.Count() > 0;
                m_Container.VSM_SetHasItems(hasItems);
            }
            serializer.Close();
        }
        
        m_Container.VSM_SetIsProcessing(false);
        Stop();
    }
	
	void CreateBackup()
	{
        //TODO: em estado experimental
        return;
		DeleteBackup();
        string backpFile = m_VirtualFilePath + VIRTUAL_FILENAME_BKP;
		m_CFVirtualFile.Copy(backpFile);
	}
	
	void RestoreBackup()
	{
        //TODO: em estado experimental
        return;
        
        string backpFile = m_VirtualFilePath + VIRTUAL_FILENAME_BKP;
        if(!FileExist(backpFile))
            return;
        
        if(FileExist(m_VirtualFilePath))
			DeleteFile(m_VirtualFilePath);

        CF_File file = new CF_File(backpFile);
		file.Copy(m_VirtualFilePath);
        m_CFVirtualFile = new CF_File(m_VirtualFilePath);
	}

    void DeleteBackup()
    {
        string backpFile = m_VirtualFilePath + VIRTUAL_FILENAME_BKP;
		if(FileExist(backpFile))
			DeleteFile(backpFile);
    }

    void CreateVirtualizedItems()
    {
        foreach (EntityAI entity : m_Items) {
			ItemBase item = ItemBase.Cast(entity);
            if (!item || !item.VSM_CanVirtualize())
                continue;

            string fileName = item.VSM_GetId() + ".bin";
            string virtualFile = m_VirtualContextDirectory + fileName;
            
            VSM_Debug("OnStart(virtualizing)", "Processando item: " + item + " | virtualFile: " + virtualFile);

            
            
            if(FileExist(virtualFile))
                DeleteFile(virtualFile);
                  
            FileSerializer ctx = new FileSerializer();
            if (!ctx.Open(virtualFile, FileMode.WRITE))
            {
                VSM_Error("OnStart", "Não foi possível abrir o arquivo virtual para escrever: " + virtualFile);
                continue;
            }
            
            // processo de virtualização
            item.VSM_OnBeforeVirtualize();
            VirtualObject virtualObj = new VirtualObject(m_VirtualContextDirectory, m_Container); 
            virtualObj.OnVirtualize(ctx, item );
            m_ProcessedItems.Insert(virtualObj);
            ctx.Close();

            // para salvamento
            VirtualObjectContext obj = new VirtualObjectContext();
            obj.virtualId = item.VSM_GetId();
            obj.contextFile = fileName;
            m_VirtualStorageFile.storedItems.Insert(obj);
		}
    }

    void LoadItems()
    {
		m_Container.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, m_Items);
    }
}