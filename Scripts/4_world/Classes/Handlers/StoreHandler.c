class VSM_StoreHandler extends VirtualObjectHandler_Base
{
    string m_ContextId;

    override void OnVirtualize(string virtualPath, ItemBase virtualize, ItemBase parent) {

        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_StoreHandler: OnVirtualize " + virtualize.GetType() + "salvando contextoId=" + m_ContextId);
        
        if (!virtualize)
            return;

        FileSerializer ctx = new FileSerializer();
        m_ContextId = VirtualStorageModule.GetModule().GetPersistentId(virtualize);
        string virtualFile = m_ContextId + ".bin";
        string filename = virtualPath + virtualFile;

        if (ctx.Open(filename, FileMode.WRITE))
        {
            virtualize.VSM_OnVirtualStoreSave(ctx);
            if (VirtualStorageModule.GetModule().m_Debug)
            Print("VSM_StoreHandler: OnVirtualize " + virtualize.GetType() + " salvando arquivo de contexto m_ContextId=" + m_ContextId);

        }
        else
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("Não foi possível abrir o arquivo de contexto: " + filename);
        }

        ctx.Close();
        Print("VSM_StoreHandler: OnVirtualize " + virtualize.GetType() + " salvamento concluído");

    }

    override void OnRestore(string virtualPath, ItemBase restored, ItemBase parent) 
    {
        FileSerializer ctx = new FileSerializer();
        string virtualFile = m_ContextId + ".bin";
        string filename = virtualPath + virtualFile;

        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_StoreHandler: OnRestore " + restored.GetType() + " iniciando restauração, m_ContextId=" + m_ContextId + " filename " + filename );

        if (!ctx.Open(filename, FileMode.READ))
        {
            if (VirtualStorageModule.GetModule().m_Debug)
                Print("Não foi possível abrir o arquivo de contexto: " + filename);
            return;
        }

        // bool success = restored.VSM_OnVirtualStoreLoad(ctx, GAME_STORAGE_VERSION);
        if(restored.VSM_OnVirtualStoreLoad(ctx, 9999))
        {
            if (VirtualStorageModule.GetModule().m_Debug)
            Print("VSM_StoreHandler: OnRestore " + restored.GetType() + " leitura concluida");
        }
        else
        {
            if (VirtualStorageModule.GetModule().m_Debug)
            Print("VSM_StoreHandler: OnRestore " + restored.GetType() + " falha na restauração...");
        }
        

        ctx.Close();

        DeleteFile(filename);
        if (VirtualStorageModule.GetModule().m_Debug)
        Print("VSM_StoreHandler: OnRestore " + restored.GetType() + " terminada");

    }

}