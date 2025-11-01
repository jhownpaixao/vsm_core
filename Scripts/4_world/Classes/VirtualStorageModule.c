
[CF_RegisterModule(VirtualStorageModule)]
class VirtualStorageModule : CF_ModuleWorld
{
	protected static VirtualStorageModule 	sm_Instance;
	protected VSM_Settings 					m_Settings;
	protected VSM_Version 					m_VersionManager;

	static ref map<string, ref VSMBulkProcessingQueue> m_ActiveQueues = new map<string, ref VSMBulkProcessingQueue>();

	bool m_IsNew;
	bool m_IsLoaded;
	bool m_IsMissionFinishing;

	int m_MissionStorageId;

	ref array<ItemBase> m_InitContainers = new array<ItemBase>;

	void VirtualStorageModule()
	{
		sm_Instance = this;
		m_IsLoaded = false;
		m_Settings = VSM_Settings.GetSettings();
		m_VersionManager = VSM_Version.GetManager();
	}

	bool IsNew()
	{
		return m_IsNew;
	}

	bool IsRemoving()
	{
		return m_Settings.m_IsRemovingModule;
	}

	override void OnInit()
	{
		super.OnInit();
        EnableRPC();

#ifdef SERVER
        EnableInvokeConnect();
		EnableMissionStart();
		EnableMissionLoaded();
		EnableMissionFinish();
		VSM_MigrationManager.OnInit();
#endif
	}

#ifdef SERVER
	override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		super.OnMissionStart(sender, args);


		VSM_Info("OnMissionStart", "Iniciando VirtualStorageModule");

		m_MissionStorageId = GetGame().ServerConfigGetInt("instanceId");

		//! Jamais altere isto, pois aqui é definido se é a primeira vez que o modulo inicia
		//! Os armazenamentos falharão no OnLoadStorage se esta verificação falhar
		//! Isso vai gerar Script Corrupted Upon
		//! GetVirtualDirectory() utiliza m_MissionStorageId;
		//TODO: Nas versões futuras, passar isso para o gerenciador de versões
		if (!FileExist(GetVirtualDirectory()))
		{
			m_IsNew = true;
		}

		bool autoClose 				= m_Settings.m_AutoCloseEnable;
		int autoCloseInterval 		= m_Settings.m_AutoCloseInterval;
		float playerDistance 		= m_Settings.m_AutoClosePlayerDistance;
		bool ignoreNearby 			= m_Settings.m_AutoCloseIgnorePlayerNearby;
		int batchSize 				= m_Settings.m_BatchSize;
		int batchInterval 			= m_Settings.m_BatchInterval;
		bool includeDecayItems 		= m_Settings.m_IncludeDecayItems;
		TStringArray ignoreItems 	= m_Settings.m_IgnoredItems;

		VSM_Debug("OnMissionStart", "--------------------Definindo variaveis----------------------------");
		VSM_Debug("OnMissionStart", "autoclose: %1", autoClose.ToString());
		VSM_Debug("OnMissionStart", "autoclose player distance: %1", playerDistance.ToString());
		VSM_Debug("OnMissionStart", "autoclose interval: %1", autoCloseInterval.ToString());
		VSM_Debug("OnMissionStart", "autoclose ignorenearby: %1", ignoreNearby.ToString());
		VSM_Debug("OnMissionStart", "batch size: %1", batchSize.ToString());
		VSM_Debug("OnMissionStart", "batch interval: %1", batchInterval.ToString());
		VSM_Debug("OnMissionStart", "include decay: %1", includeDecayItems.ToString());
		VSM_Debug("OnMissionStart", "ignore itens: %1 items", ignoreItems.Count().ToString());
		VSM_Debug("OnMissionStart", "IS NEW INSTALLATION: %1", m_IsNew.ToString());
		VSM_Debug("OnMissionStart", "REMOVING MODULE: %1", m_Settings.m_IsRemovingModule.ToString());
		VSM_Debug("OnMissionStart", "------------------------------------------------------------------");

		if (!FileExist(GetVirtualDirectory()))
			VirtualUtils.MakeDirectoryRecursive(GetVirtualDirectory());
	}

	override void OnMissionLoaded(Class sender, CF_EventArgs args)
	{
		super.OnMissionLoaded(sender, args);

		m_IsLoaded = true;
		ProcessContainersInit();
	}

	override void OnMissionFinish(Class sender, CF_EventArgs args)
	{
	
		VSM_Info("OnMissionFinish", " Iniciando processo de fechamento dos containers virtuais");
		m_IsMissionFinishing = true;

		if (FileExist(GetVirtualDirectory()) && m_Settings.m_IsRemovingModule)
		{
			VirtualUtils.DeleteFiles(GetVirtualDirectory());
			return;
		}

		// for (int i = 0; i < m_InitContainers.Count(); i++)
		// {
		// 	ItemBase container = m_InitContainers[i];
		// 	if (container && container.VSM_CanVirtualize() && container.VSM_IsOpen())
		// 	{
		// 		VSM_Debug("OnMissionFinish", container.GetType() + " container aberto, fechando...");
		// 		container.VSM_Close(); //!Ao fechar segue OnSaveVirtualStore
		// 	}
		// }

		super.OnMissionFinish(sender, args);
	}

	override void OnInvokeConnect(Class sender, CF_EventArgs args)
    {
        super.OnInvokeConnect(sender, args);
        auto playerArgs = CF_EventPlayerArgs.Cast(args);

		PlayerIdentity identity = playerArgs.Identity;
		if (!identity) return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(m_Settings);
        rpc.Send(NULL, VSM_RPCTypes.SYNC_SETTINGS, true, identity);
    }
#endif

	override int GetRPCMin()
    {
        return VSM_RPCTypes.INVALID;
    }

    override int GetRPCMax()
    {
        return VSM_RPCTypes.COUNT;
    }

	override void OnRPC(Class sender, CF_EventArgs args)
    {
        super.OnRPC(sender, args);
        auto rpc = CF_EventRPCArgs.Cast(args);

        switch (rpc.ID)
        {
            case VSM_RPCTypes.MENUCTX_OPEN_CONTAINER:
                RPC_OnOpenContainer(rpc.Context, rpc.Sender, rpc.Target);
                break;
            case VSM_RPCTypes.MENUCTX_CLOSE_CONTAINER:
                RPC_OnCloseContainer(rpc.Context, rpc.Sender, rpc.Target);
                break;
			case VSM_RPCTypes.SYNC_SETTINGS:
                RPC_OnSyncSettings(rpc.Context, rpc.Sender, rpc.Target);
                break;
        }
    }

	void RPC_OnOpenContainer(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
    {
        ItemBase container;
		if (!ctx.Read(container)) return;

		if (!container.VSM_CanOpen())
		{
			VirtualUtils.SendMessageToPlayer(PlayerBase.Cast(senderRPC.GetPlayer()), "STR_VSM_NOT_OPEN_CONTAINER");
			return;
		}
		container.VSM_Open();
    }

	void RPC_OnCloseContainer(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
    {
		ItemBase container;
		if (!ctx.Read(container)) return;

		if (!container.VSM_CanClose())
		{
			VirtualUtils.SendMessageToPlayer(PlayerBase.Cast(senderRPC.GetPlayer()), "STR_VSM_NOT_CLOSE_CONTAINER");
			return;
		}
		container.VSM_Close();
    }

	void RPC_OnSyncSettings(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
    {
		if(!ctx.Read(m_Settings))
		{
			VSM_Error("RPC_OnSyncSettings", "Falha ao ler as configurações do VSM");
		};
    }

	void OnProcessContainerInit(ItemBase container)
	{

		if (!container || container.VSM_IsLoaded())
			return;
	
		//!Por algum motivo sem o VSM_InPlayer se o player logar com storage na mão, ele é virtualizado e fechado, então ao abrir nao tem nada pois o id dele vai mudar...
		if (!container.VSM_CanVirtualize() || container.VSM_InPlayer())
		{
			container.VSM_SetVirtualLoaded(true);
			return;
		}

		if(m_Settings.m_IsRemovingModule)
		{
			container.VSM_SetVirtualLoaded(true);
			if (!container.VSM_IsOpen())
				container.VSM_Open();
		}

		string virtualFile = GetVirtualFile(container);
		if (!FileExist(virtualFile))
		{
			container.VSM_SetVirtualLoaded(true);
			if (container.VSM_IsOpen())
			{
				VSM_Debug("OnProcessContainerInit","Container novo detectado, fechando para virtualizar..." + container.GetType());
				container.VSM_Close();
			}
			else
			{
				VSM_Debug("OnProcessContainerInit","Container novo detectado, virtualizando..." + container.GetType());
				OnSaveVirtualStore(container);
			}

			return;
		}
		
		VSMMetadata metadata = GetMetadata(container);
		VSM_Debug("OnProcessContainerInit", container.GetType() + " [Metadata] Versão:" + metadata.GetVersion().ToString() + " - Versão mais recente: " + VSM_StorageVersion.CURRENT_VERSION.ToString());
		
		if (metadata.m_Version < VSM_StorageVersion.CURRENT_VERSION)
		{
			VSM_Warn("OnProcessContainerInit", container.GetType() + " container desatualizado, iniciando migração...");
			OnMigrate(container, metadata);
			return;
		}

		if (!metadata.IsNew() && m_Settings.m_EnableAutoRestoreState)
		{ 
			//!! O VSM_SetVirtualLoaded(true) deve ser chamado do lado da fila
			//!! o container vai ficar bloquado até aqui!
			if (metadata.IsRestoring() || metadata.IsRestored())
			{
				VSMRestorationQueue restoreQueue = new VSMRestorationQueue(container);
				m_ActiveQueues.Insert(container.VSM_GetId(), restoreQueue);

				if (metadata.IsRestoring())
				{
					VSM_Warn("OnProcessContainerInit", container.GetType() + " container em processo de restauração: reiniciando fila...");
					restoreQueue.OnRestart();
				}
				else if (metadata.IsRestored())
				{
					restoreQueue.OnSync();
				}
			}
			else if (metadata.IsVirtualizing() || metadata.IsVirtualized())
			{
				VSMVirtualizeQueue virtualizeQueue = new VSMVirtualizeQueue(container);
				m_ActiveQueues.Insert(container.VSM_GetId(), virtualizeQueue);

				if (metadata.IsVirtualizing())
				{
					VSM_Warn("OnProcessContainerInit", container.GetType() + " container em processo de virtualização: reiniciando fila...");
					virtualizeQueue.OnRestart();
				}
				else if (metadata.IsVirtualized())
				{
					virtualizeQueue.OnSync();
				}
			}
		}

		container.VSM_SetVirtualLoaded(true);

		if (container.VSM_IsOpen())
		{
			VSM_Debug("OnProcessContainerInit", container.GetType() + " Fechando container aberto...");
			container.VSM_GetAutoCloseBehavior().Start(); //! Inicia o auto close caso esteja aberto
			container.VSM_Close();
		}
	}

	void OnInitContainer(ItemBase container)
	{
		m_InitContainers.Insert(container);
		container.VSM_SetIsVirtualStorage(true);

		if (m_IsLoaded)
			OnProcessContainerInit(container);
	}

	void OnSaveVirtualStore(ItemBase container)
	{
		if (!container)
		{
			VSM_Error("OnSaveVirtualStore", "Container is null");
			return;
		}

		VSM_Debug("OnSaveVirtualStore", container.GetType() + " Init: loaded=" + container.VSM_IsLoaded() + " canvirtualize=" + container.VSM_CanVirtualize() + " processing=" + container.VSM_IsProcessing());
		if (!GetGame().IsServer() || container.IsDamageDestroyed() || !container.VSM_IsLoaded() || IsIgnoredItem(container.GetType()) || !container.VSM_CanVirtualize() || container.VSM_IsProcessing() || HasActiveQueue(container) || m_Settings.m_IsRemovingModule || container.VSM_IsIgnoreVirtualization())
		{
			VSM_Debug("OnSaveVirtualStore", "container damaged ? " + container.IsDamageDestroyed());
			VSM_Debug("OnSaveVirtualStore", "container isloaded ? " + container.VSM_IsLoaded());
			VSM_Debug("OnSaveVirtualStore", "container canvirtualize ? " + container.VSM_CanVirtualize());
			VSM_Debug("OnSaveVirtualStore", "container isprocessing ? " + container.VSM_IsProcessing());
			VSM_Debug("OnSaveVirtualStore", "container hasactivequeue ? " + HasActiveQueue(container));
			VSM_Debug("OnSaveVirtualStore", "container isignorevirtualization ? " + container.VSM_IsIgnoreVirtualization());
			VSM_Debug("OnSaveVirtualStore", "m_Settings.m_IsRemovingModule ? " + m_Settings.m_IsRemovingModule);
			
			return;
		}
			
		string virtualPath = GetVirtualContextDirectory(container);
		if (!FileExist(virtualPath)) MakeDirectory(virtualPath);

		VSMVirtualizeQueue queue = new VSMVirtualizeQueue(container);
		m_ActiveQueues.Insert(container.VSM_GetId(), queue);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(queue.Start);
	}

	void OnLoadVirtualStore(ItemBase container)
	{
		if (!container) return;

		if (!GetGame().IsServer() || !container || container.IsDamageDestroyed() || container.VSM_IsIgnoreVirtualization())
			return;

		VSM_Debug("OnLoadVirtualStore", "Init " + container.GetType() + " loaded=" + container.VSM_IsLoaded() + " hasItems=" + container.VSM_HasVirtualItems() + " processing=" + container.VSM_IsProcessing());

		//removido || !container.VSM_HasVirtualItems() sera isto a causa dos itens sumindo?
		if (!container.VSM_IsLoaded()  || container.VSM_IsProcessing() || HasActiveQueue(container))
		{
			VSM_Debug("OnLoadVirtualStore", "container damaged ? " + container.IsDamageDestroyed());
			VSM_Debug("OnLoadVirtualStore", "container isloaded ? " + container.VSM_IsLoaded());
			VSM_Debug("OnLoadVirtualStore", "container hasitems ? " + container.VSM_HasVirtualItems());
			VSM_Debug("OnLoadVirtualStore", "container canvirtualize ? " + container.VSM_CanVirtualize());
			VSM_Debug("OnLoadVirtualStore", "container isprocessing ? " + container.VSM_IsProcessing());
			VSM_Debug("OnLoadVirtualStore", "container hasactivequeue ? " + HasActiveQueue(container));
			return;
		}

		string virtualFile = GetVirtualFile(container);
		if (!FileExist(virtualFile))
		{
			VSM_Warn("OnLoadVirtualStore", container.GetType() + " Não foi possível encontrar o arquivo virtual " + virtualFile);
			container.VSM_SetHasItems(false);
			return;
		}
		
		VSMBulkProcessingQueue queue;
		VSMMetadata metadata = GetMetadata(container);

		if(metadata.m_Version <= VSM_StorageVersion.V_1409)
			queue = new VSMLegacyRestorationQueue(container);
		else
			queue = new VSMRestorationQueue(container);

		m_ActiveQueues.Insert(container.VSM_GetId(), queue);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(queue.Start);
	}

	void OnDeleteContainer(ItemBase container)
	{
		if (!container) return;

		string path = GetVirtualContextDirectory(container);

		if (FileExist(path) && container.VSM_CanDeleteVirtualFile())
		{
			VSM_Warn("OnDeleteContainer", container.GetType() + " Deletando arquivos virtuais");
			VirtualUtils.DeleteFiles(path);
		}
	}

	void OnDestroyed(ItemBase container)
	{
		if (!container) return;

		if (!GetGame().IsServer()) return;

		VSM_Info("OnDestroyed", container.GetType() + " Iniciando processo de drop dos itens");

		if (!container.VSM_IsLoaded())
			return;

		string virtualFile = GetVirtualFile(container);
		if (!FileExist(virtualFile))
		{
			VSM_Debug("OnDestroyed", container.GetType() + " Não foi possível localizar o virtual storage deste item" + virtualFile);
			return;
		}

		VSMBulkProcessingQueue queue;
		VSMMetadata metadata = GetMetadata(container);

		if(metadata.IsOutdated())
			queue = new VSMLegacyDropQueue(container);
		else
			queue = new VSMDropQueue(container);
		
		m_ActiveQueues.Insert(container.VSM_GetId(), queue);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(queue.Start);
	}

	void OnMigrate(ItemBase container, VSMMetadata metadata)
	{
		if (!container) return;

		VSM_Debug("OnMigrate", "Iniciando migração de " + container.GetType() + " para a versão atual do VSM");

		VSMLegacyRestorationQueue restoreQueue = new VSMLegacyRestorationQueue(container);
		m_ActiveQueues.Insert(container.VSM_GetId(), restoreQueue);

		container.VSM_SetIgnoreVirtualization(true);
		container.VSM_Open();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(restoreQueue.Start);
	}

	void ProcessContainersInit()
	{
		VSM_Info("ProcessContainersInit", "Inicializando containers registrados: " + m_InitContainers.Count().ToString());
		for (int i = 0; i < m_InitContainers.Count(); i++)
		{
			ItemBase container = m_InitContainers[i];
			if(container)
				OnProcessContainerInit(container);
		}
	}

	string GetPersistentId(ItemBase container)
	{
		return container.VSM_GetId();
	}

	string GetVirtualDirectory()
	{
		string currentStorage = "storage_" + m_MissionStorageId;
		return "$mission:" + currentStorage + "\\virtual\\";
	}

	string GetVirtualContextDirectory(ItemBase container)
	{
		string vId = container.VSM_GetId();
		return GetVirtualDirectory() + vId + "\\";
	}

	string GetVirtualFile(ItemBase container)
	{
		return GetVirtualContextDirectory(container) + VSM_Constants.VIRTUAL_FILENAME;
	}

	string GetVirtualCtxFile(ItemBase container)
	{
		return GetVirtualContextDirectory(container) + VSM_Constants.CTX_FILENAME;
	}

	string GetVirtualMetadataFile(ItemBase container)
	{
		return GetVirtualContextDirectory(container) + VSM_Constants.METADATA_FILENAME;
	}

	VSMMetadata GetMetadata(ItemBase container)
	{	
		string metaPath = GetVirtualMetadataFile(container);
		
		VSMMetadata metadata = new VSMMetadata(metaPath);
		metadata.OnInit();
		return metadata;
	}

	static VirtualStorageModule GetModule()
	{
		return sm_Instance;
	}

	void RemoveActiveQueue(ItemBase container)
	{
		m_ActiveQueues.Remove(container.VSM_GetId());
	}

	bool HasActiveQueue(ItemBase container)
	{
		return m_ActiveQueues.Contains(container.VSM_GetId());
	}

	bool IsIgnoredItem(string itemType)
	{
		ref TStringArray ignoredItems = m_Settings.m_IgnoredItems;

		int index = ignoredItems.Find(itemType);

		if (ignoredItems.Find(itemType) > -1) return true;

		if (m_Settings.m_UseCfgIgnoreList)
		{
			VSM_CfgIgnoreList cfgIgnoreList = new VSM_CfgIgnoreList();
			ref TStringArray cfgIgnoredItems = cfgIgnoreList.GetList();
			if (ignoredItems.Find(itemType) > -1) return true;
		}

		return false;
	}

	//!log
	void VSM_Info(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		VirtualUtils.Info("VirtualStorageModule::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
	}

	void VSM_Error(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		VirtualUtils.Error("VirtualStorageModule::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
	}

	void VSM_Trace(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		VirtualUtils.Trace("VirtualStorageModule::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
	}

	void VSM_Debug(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		VirtualUtils.Debug("VirtualStorageModule::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
	}

	void VSM_Warn(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		VirtualUtils.Warn("VirtualStorageModule::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
	}

	void VSM_Critical(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		VirtualUtils.Critical("VirtualStorageModule::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
	}

	bool HasRegisteredVirtualContainer(ItemBase container)
	{
		return m_InitContainers.Find(container) > -1;
	}

	void ContextMenu_OnCloseContainer(EntityAI item)
	{
		ItemBase itemBase = ItemBase.Cast(item);
		if(itemBase)
		{
			ScriptRPC rpc = new ScriptRPC();
            rpc.Write(itemBase);
            rpc.Send(NULL, VSM_RPCTypes.MENUCTX_CLOSE_CONTAINER, false, null);
		}
	}

	void ContextMenu_OnOpenContainer(EntityAI item)
	{
		ItemBase itemBase = ItemBase.Cast(item);
		if(itemBase)
		{
			ScriptRPC rpc = new ScriptRPC();
            rpc.Write(itemBase);
            rpc.Send(NULL, VSM_RPCTypes.MENUCTX_OPEN_CONTAINER, false, null);
		}
	}
}
