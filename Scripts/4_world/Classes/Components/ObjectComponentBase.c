/**
 * @brief Classe base para os manipuladores de componentes
 * @note Esta classe é usada para lidar com caracteristicas/componentes especificos de um objeto ao salvar/restaurar
 *
 */
class VSMObjectComponent : VSMBase
{	
	int 						 m_Version; // versão do armazenamento
    protected ref FileSerializer m_StoreCtx; // ctx.vsm

	void OnInit();

	bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		return true;
	};

	void OnStoreSave(ParamsWriteContext ctx);

	void OnBeforeRestore();

	bool OnRestore(ItemBase restored)
	{
		return true;
	}

	void OnRestoreComplete();

	void OnBeforeVirtualize();
	
	void OnVirtualize(ItemBase virtualize);

	void OnVirtualizeComplete();

	void Virtualize(ParamsWriteContext ctx, ParamsWriteContext storeCtx, ItemBase virtualize)
	{
		m_StoreCtx = FileSerializer.Cast(storeCtx);
		OnVirtualize(virtualize);
		StoreSave(ctx);
	}

	bool Restore(ParamsReadContext ctx, ParamsReadContext storeCtx, int version, ItemBase restored)
	{
		m_StoreCtx = FileSerializer.Cast(storeCtx);
		m_Version = version;
		if (StoreLoad(ctx, version))
			return OnRestore(restored);
		
		return false;
	}

	bool StoreLoad(ParamsReadContext ctx, int version)
	{
		string header;
		VSM_Debug("StoreLoad", "Iniciando carga do componente com versão: " + version.ToString());
		if(version < VSM_StorageVersion.V_0510)
		{
			if (!ctx.Read(header) || header != VSM_HeaderType.COMPONENT_INIT) 
			{
				VSM_Error("StoreLoad", "Cabeçalho inválido");
				return false;
			}
		}

		bool result = OnStoreLoad(ctx, version);

		if(!result)
		{
			if(version < VSM_StorageVersion.V_0510)
			{
				return ReleaseCtx(ctx, version);
			}
			
			return false;
		}
		else if(version < VSM_StorageVersion.V_0510)
		{
			if (!ctx.Read(header) || header != VSM_HeaderType.COMPONENT_EOF) 
			{
				VSM_Error("StoreLoad", "Fim de bloco inválido");
				return false;
			}
		}

		return true;
	}

	void StoreSave(ParamsWriteContext ctx)
	{
		OnStoreSave(ctx);
	}

	bool CanHandler(ItemBase item)
	{
		return item != null;
	}

	private bool ReleaseCtx(ParamsReadContext ctx, int version)
	{
		int attempts = 2000;
        string header;

        while (header != VSM_HeaderType.COMPONENT_EOF && attempts > 0)
		{
            ctx.Read(header);
            attempts--;
        }

        return true;
	}
}