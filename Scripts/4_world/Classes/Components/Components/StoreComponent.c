/**
 * @brief Classe para manipulação de contexto/variaveis do objeto
 * @note Esta classe é usada para lidar com o contexto de variaveis de um objeto usados em OnStoreLoad e OnStoreSave
 */
class VSMStoreComponent extends VSMObjectComponent
{   
    override void OnVirtualize(ItemBase virtualize)
    {   
        VSM_Debug("OnVirtualize ", "%1 iniciando virtualização de contexto", virtualize.GetType());
        int version = GetGame().SaveVersion();
        m_StoreCtx.Write(version);
        virtualize.VSM_OnVirtualStoreSave(m_StoreCtx);
        m_StoreCtx.Write("EOF");

        VSM_Debug("OnVirtualize ","%1 salvamento concluído",  virtualize.GetType());
    }

    override bool OnRestore(ItemBase restored) 
    {
        VSM_Debug("OnRestore", "%1 iniciando restauração", restored.GetType());
        
        int version;
        if (!m_StoreCtx.Read(version))
        {
            VSM_Error("OnStoreLoad", "Versão de salvamento inválida");
            return false;
        }

        if(!restored.VSM_OnVirtualStoreLoad(m_StoreCtx, version))
            VSM_Debug("OnRestore","%1 falha na restauração do contexto, vesão: %2", restored.GetType(), version.ToString());

        string blockEnd;
        if (!m_StoreCtx.Read(blockEnd) || blockEnd != "EOF")
        {
            VSM_Error("OnStoreLoad", "Fim de bloco inválido");
            return false;
        }

        restored.VSM_AfterVirtualStoreLoad();
        VSM_Debug("OnRestore", restored.GetType() + " terminada");
        return true;
    }
}