class VSM_VirtualizeQueue extends BatchQueue_Base
{
	const string VIRTUAL_FILENAME_BKP = ".bckp";
    protected ref array<EntityAI> m_Items;
    protected ref FileSerializer m_Serialize;
    protected ref VirtualStorageFile m_VirtualStorageFile;
	protected ref CF_File m_CFVirtualFile;

    protected ref array<ref VirtualObject> m_ProcessedItems;


    void OnInit(array<EntityAI> items)
    {
        m_Items = items;
        m_ItemCount = m_Items.Count();
        m_Serialize = new FileSerializer();
        m_VirtualStorageFile = new VirtualStorageFile();

        super.OnInit(); //debug

        m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZING);
        m_Metadata.SetVersion(VSM_STORAGE_VERSION); //!save lastversion
		m_CFVirtualFile = new CF_File(m_VirtualFilePath);
    }

	override void OnStart()
	{
		CreateBackup();

        foreach (EntityAI entity : m_Items) {
			ItemBase item = ItemBase.Cast(entity);
            VSM_Debug("OnStart(virtualizing)", "Processando item: " + item);

            if (!item || !item.VSM_CanVirtualize())
                return;
                
            FileSerializer ctx = new FileSerializer();
            string fileName = item.VSM_GetId() + ".bin";
            string virtualFile = m_VirtualContextDirectory + fileName;
            
            if (!ctx.Open(virtualFile, FileMode.WRITE))
            {
                VSM_Error("OnStart", "Não foi possível abrir o arquivo virtual para escrever: " + virtualFile);
            }
            
            item.VSM_OnBeforeVirtualize();
            VirtualObject virtualObj = new VirtualObject(m_VirtualContextDirectory, m_Container); 
            virtualObj.OnVirtualize(ctx, item );
            m_ProcessedItems.Insert(virtualObj);
            ctx.Close();

            VirtualObjectContext obj = new VirtualObjectContext();
            obj.virtualId = item.VSM_GetId();
            obj.contextFile = fileName;

            m_VirtualStorageFile.storedItems.Insert(obj);
		}

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
        }
        else
        {
            VSM_Warn("OnAfterBatchProcess", "Não foi possível deletar um item");
        }
    }

    override void OnComplete()
    {
        super.OnComplete();

        if (!m_Serialize.Open(m_VirtualFilePath, FileMode.WRITE))
		{
			VSM_Error("OnSaveVirtualStore", "Não foi possível abrir o arquivo virtual para escrever: " + m_VirtualFilePath);
		}
        else if (!m_Serialize.Write(m_VirtualStorageFile))
		{
			VSM_Error("OnSaveVirtualStore", "Não foi possível salvar o arquivo virtual: " + m_VirtualFilePath);
		}
        else
        {
            m_Container.VSM_SetHasItems(m_ItemCount != 1);
            m_Container.VSM_OnAfterContainerVirtualize();
			VSM_Info("OnSaveVirtualStore", m_Container.GetType() +" Virtualização completa");
            m_Metadata.SetLastOperationState(VirtualStorageState.VIRTUALIZED);
            DeleteBackup();

            foreach (VirtualObject obj : m_ProcessedItems) {
                if (obj) obj.OnVirtualizeComplete();
            }
        }
    
        m_Serialize.Close();
        m_Container.VSM_SetIsProcessing(false);
    }

    override void OnRestart()
    {
		RestoreBackup();
        m_Container.VSM_Close();
    }

    override void OnSync()
    {
        m_Container.VSM_SetIsProcessing(true);
        m_Container.VSM_Close();
        ClearInventory();

        FileSerializer serializer = new FileSerializer();
			VirtualStorageFile virtualStorage = new VirtualStorageFile();

        if (serializer.Open(m_VirtualFilePath, FileMode.READ)){
            if (serializer.Read(virtualStorage))
            {
                bool hasItems = virtualStorage.storedItems.Count() > 0;
                m_Container.VSM_SetHasItems(hasItems);
            }
        }

        m_Container.VSM_SetIsProcessing(false);
    }
	
	void CreateBackup()
	{
		DeleteBackup();
        string backpFile = m_VirtualFilePath + VIRTUAL_FILENAME_BKP;
		m_CFVirtualFile.Copy(backpFile);
	}
	
	void RestoreBackup()
	{
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
}