
[CF_RegisterModule(VirtualStorageModule)]
class VirtualStorageModule : CF_ModuleWorld
{
	protected static VirtualStorageModule sm_Instance;
	static ref map<string, ref BatchQueue_Base> m_ActiveQueues = new map<string, ref BatchQueue_Base>();

	bool m_IsRemoving;
	bool m_IsNew;
	bool m_IsLoaded;
	bool m_IsMissionFinishing;

	int m_MissionStorageId;

	ref array<ItemBase> m_InitContainers = new array<ItemBase>;

	void VirtualStorageModule()
	{
		sm_Instance = this;
		m_IsLoaded = false;
	}

	bool IsNew()
	{
		return m_IsNew;
	}

	bool IsRemoving()
	{
		return m_IsRemoving;
	}


	override void OnInit()
	{
		super.OnInit();
        EnableRPC();

		#ifdef SERVER
		EnableMissionStart();
		EnableMissionLoaded();
		EnableMissionFinish();
		Metrics.OnInit();
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
		if (!FileExist(GetVirtualDirectory()))
		{
			m_IsNew = true;
		}

		m_IsRemoving = CfgGameplayHandler.GetVSM_IsRemoving();

		bool autoClose = CfgGameplayHandler.GetVSM_AutoCloseEnable();
		int autoCloseInterval = CfgGameplayHandler.GetVSM_AutoCloseInterval();
		float playerDistance = CfgGameplayHandler.GetVSM_AutoClosePlayerDistance();
		bool ignoreNearby = CfgGameplayHandler.GetVSM_AutoCloseIgnorePlayerNearby();
		int batchSize = CfgGameplayHandler.GetVSM_BatchSize();
		int batchInterval = CfgGameplayHandler.GetVSM_BatchInterval();
		bool includeDecayItems = CfgGameplayHandler.GetVSM_IncludeDecayItems();
		TStringArray ignoreItems = CfgGameplayHandler.GetVSM_IgnoredItems();
		

		VSM_Debug("OnMissionStart", "Definindo variaveis");
		VSM_Debug("OnMissionStart", "autoclose: %1", autoClose.ToString());
		VSM_Debug("OnMissionStart", "autoclose player distance: %1", playerDistance.ToString());
		VSM_Debug("OnMissionStart", "autoclose interval: %1", autoCloseInterval.ToString());
		VSM_Debug("OnMissionStart", "autoclose ignorenearby: %1", ignoreNearby.ToString());
		VSM_Debug("OnMissionStart", "batch size: %1", batchSize.ToString());
		VSM_Debug("OnMissionStart", "batch interval: %1", batchInterval.ToString());
		VSM_Debug("OnMissionStart", "include decay: %1", includeDecayItems.ToString());
		VSM_Debug("OnMissionStart", "ignore itens: %1 items", ignoreItems.Count().ToString());
		VSM_Debug("OnMissionStart", "IS NEW INSTALLATION: %1", m_IsNew.ToString());
		VSM_Debug("OnMissionStart", "REMOVING MODULE: %1", m_IsRemoving.ToString());
		VSM_Debug("OnMissionStart", "------------------------------------------------");

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

		if (FileExist(GetVirtualDirectory()) && m_IsRemoving)
		{
			VirtualUtils.DeleteFiles(GetVirtualDirectory());
			return;
		}

		foreach (ItemBase container : m_InitContainers) {

			if (container && container.VSM_CanVirtualize() && container.VSM_IsOpen())
			{
				VSM_Debug("OnMissionFinish", container.GetType() + " container aberto, fechando...");
				container.VSM_Close(); //!Ao fechar segue OnSaveVirtualStore
			}
		}

		super.OnMissionFinish(sender, args);
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

	void OnProcessContainerInit(ItemBase container)
	{

		if (!container || container.VSM_IsLoaded())
			return;

		if (!container.VSM_CanVirtualize())
		{
			container.VSM_SetVirtualLoaded(true);
			return;
		}

		if(m_IsRemoving)
		{
			container.VSM_SetVirtualLoaded(true);
			if (!container.VSM_IsOpen())
				container.VSM_Open();
		}

		string virtualFile = GetVirtualFile(container);
		
		// storage é novo, não tem arquivo virtual
		if (!FileExist(virtualFile))
		{
			container.VSM_SetVirtualLoaded(true);
			if (container.VSM_IsOpen())
			{
				VSM_Debug("OnProcessContainerInit","NOVO - Fechando container inicial " + container.GetType());
				container.VSM_Close(); //!Ao fechar segue OnSaveVirtualStore
			}
			else
			{
				VSM_Debug("OnProcessContainerInit","NOVO - Container fechado, salvando..." + container.GetType());
				OnSaveVirtualStore(container); //! testar para servidores novos (onde os storages ainda não foram virutalizados);
			}
		}
		else
		{
			VSM_Debug("OnProcessContainerInit", container.GetType() + " virtualfile detectado");

			VirtualMetadata metadata = new VirtualMetadata(VirtualStorageModule.GetModule().GetVirtualMetadataFile(container));
			metadata.OnInit();

			if (!metadata.IsNew())
			{

				if (metadata.IsRestoring() || metadata.IsRestored())
				{
					VSM_RestorationQueue restoreQueue = new VSM_RestorationQueue(container);
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
					container.VSM_OnBeforeContainerRestore();
					VSM_VirtualizeQueue virtualizeQueue = new VSM_VirtualizeQueue(container);
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
		VSM_Debug("OnSaveVirtualStore", container.GetType() + " Init: loaded=" + container.VSM_IsLoaded() + " canvirtualize=" + container.VSM_CanVirtualize() + " processing=" + container.VSM_IsProcessing());
		if (!GetGame().IsServer() || container.IsDamageDestroyed() || !container.VSM_IsLoaded() || !container.VSM_CanVirtualize() || container.VSM_IsProcessing() || HasActiveQueue(container) || m_IsRemoving)
			return;

		
		string virtualPath = GetVirtualContextDirectory(container);
		if (!FileExist(virtualPath)) MakeDirectory(virtualPath);

		VSM_Debug("OnSaveVirtualStore", container.GetType() + " preparing to save");

		VSM_VirtualizeQueue queue = new VSM_VirtualizeQueue(container);
		m_ActiveQueues.Insert(container.VSM_GetId(), queue);

		queue.Start();
	}

	void OnLoadVirtualStore(ItemBase container)
	{
		if (!GetGame().IsServer() || container.IsDamageDestroyed())
			return;

		VSM_Info("OnLoadVirtualStore", "Init " + container.GetType() + " loaded=" + container.VSM_IsLoaded() + " hasItems=" + container.VSM_HasVirtualItems() + " processing=" + container.VSM_IsProcessing());

		if (!container.VSM_IsLoaded() || !container.VSM_HasVirtualItems() && !container.VSM_CanVirtualize() || container.VSM_IsProcessing() || HasActiveQueue(container))
			return;



		string virtualFile = GetVirtualFile(container);
		if (!FileExist(virtualFile))
		{
			VSM_Warn("OnLoadVirtualStore", container.GetType() + " Não foi possível encontrar o arquivo virtual " + virtualFile);
			container.VSM_SetHasItems(false);
			return;
		}
	
		VSM_RestorationQueue queue = new VSM_RestorationQueue(container);
		m_ActiveQueues.Insert(container.VSM_GetId(), queue);
		queue.Start();
		
	}

	void OnDeleteContainer(ItemBase container)
	{
		string path = GetVirtualContextDirectory(container);

		if (FileExist(path) && container.VSM_CanDeleteVirtualFile())
		{
			VSM_Warn("OnDeleteContainer", container.GetType() + " Deletando arquivos virtuais");
			VirtualUtils.DeleteFiles(path);
		}
	}

	void OnDestroyed(ItemBase container)
	{
		if (!GetGame().IsServer())
			return;

		VSM_Info("OnDestroyed", container.GetType() + " Iniciando processo de drop dos itens");

		if (!container.VSM_IsLoaded() || !container.VSM_HasVirtualItems() && !container.VSM_CanVirtualize())
			return;

		string virtualFile = GetVirtualFile(container);
		if (!FileExist(virtualFile))
		{
			VSM_Debug("OnDestroyed", container.GetType() + " Não foi possível localizar o virtual storage deste item" + virtualFile);
			return;
		}
		
		VSM_DropQueue queue = new VSM_DropQueue(container);
		m_ActiveQueues.Insert(container.VSM_GetId(), queue);
		queue.Start();	
	}

	void ProcessContainersInit()
	{
		VSM_Info("ProcessContainersInit", "Iniciando containers");
		foreach (ItemBase container : m_InitContainers) {
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

	string GetVirtualMetadataFile(ItemBase container)
	{
		return GetVirtualFile(container) + "." + VSM_Constants.METADATA_FILENAME;
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
		VSM_Debug("HasActiveQueue", "? %1", m_ActiveQueues.Contains(container.VSM_GetId()).ToString());
		return m_ActiveQueues.Contains(container.VSM_GetId());
	}

	bool IsIgnoredItem(string itemType)
	{
		ref TStringArray ignoredItems = CfgGameplayHandler.GetVSM_IgnoredItems();
		if (ignoredItems.Find(itemType) > -1) return true;

		if (CfgGameplayHandler.GetVSM_UseCfgIgnoreList())
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
