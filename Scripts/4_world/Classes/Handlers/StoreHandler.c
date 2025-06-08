class VSM_StoreHandler extends VirtualObjectHandler_Base
{
    string m_ContextId;
    int m_GameSaveVersion; //! não é o mesmo que o VSM_Version

    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    {
        if (!ctx.Read(m_ContextId)) return false;
        if (!ctx.Read(m_GameSaveVersion)) return false;

        return true;
    }

    override void OnStoreSave(ParamsWriteContext ctx) 
    {
        ctx.Write(m_ContextId);
        ctx.Write(m_GameSaveVersion);
    }

    override void OnVirtualize(ItemBase virtualize, ItemBase parent)
    {
        VSM_Debug("OnVirtualize ", "%1 iniciando virtualização de contexto",virtualize.GetType());
        
        m_ContextId = VirtualStorageModule.GetModule().GetPersistentId(virtualize);
        m_GameSaveVersion = GetGame().SaveVersion();
        string filename = GetCtxFileName();
        
        FileSerializer ctx = new FileSerializer();
        if (!ctx.Open(filename, FileMode.WRITE))
        {
           VSM_Debug("Não foi possível abrir o arquivo de contexto", filename);
           return;
        }

        virtualize.VSM_OnVirtualStoreSave(ctx);
        ctx.Close();

        VSM_Debug("OnVirtualize ","%1 salvamento concluído, id: %2",  virtualize.GetType(), m_ContextId);
    }

    override bool OnRestore(ItemBase restored, ItemBase parent) 
    {
        VSM_Debug("OnRestore", "%1 iniciando restauração, m_ContextId: %2",restored.GetType(), m_ContextId);
        
        string filename = GetCtxFileName();
        FileSerializer ctx = new FileSerializer();
        if (!ctx.Open(filename, FileMode.READ))
        {
            VSM_Error("OnRestore", "Não foi possível abrir o arquivo de contexto: " + filename);
            return false;
        }

        //TODO: preciso estudar mais sobre o versionamento do store vanilla. O 999 funciona com a maior parte dos itens.
        if(!restored.VSM_OnVirtualStoreLoad(ctx, m_GameSaveVersion))
            VSM_Debug("OnRestore","%1 falha na restauração do contexto,id: %1 | vesão: %3", restored.GetType(), m_ContextId, m_GameSaveVersion.ToString());
        
        restored.VSM_AfterVirtualStoreLoad();

        ctx.Close();

        DeleteFile(filename);
        VSM_Debug("OnRestore", restored.GetType() + " terminada");
        return true;
    }

    override void OnRestoreComplete() 
    {
        VSM_Debug("OnRestoreComplete","finalizando restauração, m_ContextId: %2", m_ContextId);
        
        string filename = GetCtxFileName();
        if (FileExist(filename))
        {
            DeleteFile(filename);
            VSM_Debug("OnRestoreComplete", "Arquivo de contexto deletado: " + filename);
        }
    }

    string GetCtxFileName()
    {
        return m_VirtualPath + "0ctx_" + m_ContextId + ".bin";
    }
}