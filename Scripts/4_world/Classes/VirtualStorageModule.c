
[CF_RegisterModule(VirtualStorageModule)]
class VirtualStorageModule : CF_ModuleWorld
{
	static const string METADATA_FILENAME = "meta.json";
	static const string VIRTUAL_FILENAME = "main.bin";

	protected static VirtualStorageModule sm_Instance;
	static ref map<string, ref BatchQueue_Base> m_ActiveQueues = new map<string, ref BatchQueue_Base>();

	bool m_Debug;
	bool m_IsLoaded;
	bool m_IsMissionFinishing;

	ref array<ItemBase> m_InitContainers = new array<ItemBase>;

	void VirtualStorageModule()
	{
		sm_Instance = this;
		m_IsLoaded = false;
	}

#ifdef SERVER
	override void OnInit()
	{
		super.OnInit();
		EnableMissionStart();
		EnableMissionLoaded();
		EnableMissionFinish();
		VSM_Info("OnInit", "invoke");
	}

	override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		super.OnMissionStart(sender, args);


		m_Debug = CfgGameplayHandler.GetVSM_EnableDebug();
		bool autoClose = CfgGameplayHandler.GetVSM_AutoCloseEnable();
		int autoCloseInterval = CfgGameplayHandler.GetVSM_AutoCloseInterval();
		float playerDistance = CfgGameplayHandler.GetVSM_AutoClosePlayerDistance();
		bool ignoreNearby = CfgGameplayHandler.GetVSM_AutoCloseIgnorePlayerNearby();
		int batchSize = CfgGameplayHandler.GetVSM_BatchSize();
		int batchInterval = CfgGameplayHandler.GetVSM_BatchInterval();
		bool includeDecayItems = CfgGameplayHandler.GetVSM_IncludeDecayItems();
		TStringArray ignoreItems = CfgGameplayHandler.GetVSM_IgnoredItems();

		VSM_Debug("OnMissionStart", "Definindo variaveis");
		VSM_Debug("OnMissionStart", "debug: %1",m_Debug.ToString());
		VSM_Debug("OnMissionStart",	"autoclose: %1", autoClose.ToString());
		VSM_Debug("OnMissionStart",	"autoclose player distance: %1", playerDistance.ToString());
		VSM_Debug("OnMissionStart",	"autoclose interval: %1", autoCloseInterval.ToString());
		VSM_Debug("OnMissionStart",	"autoclose ignorenearby: %1", ignoreNearby.ToString());
		VSM_Debug("OnMissionStart",	"batch size: %1", batchSize.ToString());
		VSM_Debug("OnMissionStart",	"batch interval: %1", batchInterval.ToString());
		VSM_Debug("OnMissionStart",	"include decay: %1", includeDecayItems.ToString());
		VSM_Debug("OnMissionStart",	"ignore itens: %1 items", ignoreItems.Count().ToString());
		VSM_Debug("OnMissionStart","------------------------------------------------");

		if (!FileExist(GetVirtualDirectory()))
			VirtualUtils.MakeDirectoryRecursive(GetVirtualDirectory());
	}

	override void OnMissionLoaded(Class sender, CF_EventArgs args)
	{
		super.OnMissionLoaded(sender, args);

		VSM_Info("OnMissionLoaded", " missão carregada");

		m_IsLoaded = true;
		ProcessContainersInit();
	}

	override void OnMissionFinish(Class sender, CF_EventArgs args)
	{
		m_IsMissionFinishing = true;
		VSM_Info("OnMissionFinish", " Iniciando auto close");
		super.OnMissionFinish(sender, args);

		Print("OnMissionFinish ITEMS COUNT " + m_InitContainers.Count());
		foreach (ItemBase container : m_InitContainers) {
			if (container.VSM_CanVirtualize())
			{
				VSM_Info("OnMissionFinish", container.GetType() + " analisando o container");
				if (container.VSM_IsOpen())
				{
					VSM_Info("OnMissionFinish", container.GetType() + " container aberto, fechando...");
					container.VSM_Close(); //!Ao fechar segue OnSaveVirtualStore
				}
			}
		}
	}
#endif

	void OnProcessContainerInit(ItemBase container)
	{

		if (!container || container.VSM_IsLoaded())
			return;

		if (!container.VSM_CanVirtualize())
		{
			container.VSM_SetVirtualLoaded(true);
			return;
		}

		string virtualFile = GetVirtualFile(container);
		container.VSM_SetVirtualLoaded(true);

		//! storage novo
		if (!FileExist(virtualFile))
		{
			VSM_Warn("OnProcessContainerInit", container.GetType() + " storage sem arquivo virtual, forçando virtualização");
			if (container.VSM_IsOpen())
			{
				VSM_Info("OnProcessContainerInit", container.GetType() + " container aberto, fechando...");
				container.VSM_Close(); //!Ao fechar segue OnSaveVirtualStore
			}
			else
			{
				OnSaveVirtualStore(container); //! testar para servidores novos (onde os storages ainda não foram virutalizados);
			}
		}
		else
		{
			VSM_Debug("OnProcessContainerInit", container.GetType() + " virtualfile detectado");

			VirtualMetadata metadata = new VirtualMetadata(VirtualStorageModule.GetModule().GetVirtualMetadataFile(container));
			metadata.OnInit();

			FileSerializer ctx = new FileSerializer();
			VirtualStorageFile virtualStorage = new VirtualStorageFile();

			if (!ctx.Open(virtualFile, FileMode.READ))
			{
				VSM_Warn("OnProcessContainerInit", "Não foi possível abrir o arquivo virtual" + virtualFile);
				container.VSM_SetIsProcessing(false);
				container.VSM_SetHasItems(false);
				ctx.Close();
				return;
			}

			if (!ctx.Read(virtualStorage)){
				VSM_Warn("OnProcessContainerInit", container.GetType() + " não foi possível ler o arquivo virtual " + virtualFile);
				ctx.Close();
				return;
			}

			VSM_Debug("OnProcessContainerInit", container.GetType() + " iniciando verificação do metadata");

			if (!metadata.IsNew())
			{
				VSM_Debug("OnProcessContainerInit", container.GetType() + " arquivo de metadata detectado");
				if (metadata.IsRestoring() || metadata.IsRestored())
				{
					VSM_Debug("OnProcessContainerInit", container.GetType() + " container em processo de restauração: reiniciando fila...");
					
					container.VSM_OnBeforeContainerRestore();
					VSM_RestorationQueue restoreQueue = new VSM_RestorationQueue(container);
					m_ActiveQueues.Insert(container.VSM_GetId(), restoreQueue);

					restoreQueue.OnInit(virtualStorage.storedItems);

					if (metadata.IsRestoring())
					{
						restoreQueue.OnRestart();
						restoreQueue.Start();
					}
					else if (metadata.IsRestored())
					{
						restoreQueue.OnSync();
					}
				}
				else if (metadata.IsVirtualizing() || metadata.IsVirtualized())
				{
					VSM_Debug("OnProcessContainerInit", container.GetType() + " container em processo de virtualização: reiniciando fila...");
					container.VSM_OnBeforeContainerRestore();
					VSM_VirtualizeQueue virtualizeQueue = new VSM_VirtualizeQueue(container);
					m_ActiveQueues.Insert(container.VSM_GetId(), virtualizeQueue);

					array<EntityAI> items = new array<EntityAI>;
					container.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);
					virtualizeQueue.OnInit(items);

					if (metadata.IsVirtualizing())
					{
						virtualizeQueue.OnRestart();
						virtualizeQueue.Start();
					}
					else if (metadata.IsVirtualized())
					{
						virtualizeQueue.OnSync();
					}

				}
			}
			else
			{
				VSM_Debug("OnProcessContainerInit", container.GetType() + " metadata novo");
			}
			ctx.Close();
		}

		if (container.VSM_IsOpen())
		{
			VSM_Info("OnProcessContainerInit", container.GetType() + " container aberto, fechando...");
			container.VSM_Close(); //!Ao fechar segue OnSaveVirtualStore
		}
		
		
		VSM_Info("OnProcessContainerInit", container.GetType() + " inicialização concluída");
	}

	void OnInitContainer(ItemBase container)
	{
		if (m_IsLoaded)
			OnProcessContainerInit(container);
		else
			m_InitContainers.Insert(container);
	}

	void OnSaveVirtualStore(ItemBase container)
	{
		VSM_Info("OnSaveVirtualStore", container.GetType() + " Init: loaded=" + container.VSM_IsLoaded() + " canvirtualize=" + container.VSM_CanVirtualize() + " processing=" + container.VSM_IsProcessing());
		if (!GetGame().IsServer() || container.IsDamageDestroyed() || !container.VSM_IsLoaded() || !container.VSM_CanVirtualize() || container.VSM_IsProcessing() || HasActiveQueue(container))
			return;

		container.VSM_SetIsProcessing(true);

		string virtualPath = GetVirtualContextDirectory(container);
		if (!FileExist(virtualPath)) MakeDirectory(virtualPath);

		VSM_Info("OnSaveVirtualStore", container.GetType() + " preparing to save");

		container.VSM_OnBeforeContainerVirtualize(); // prepare

		array<EntityAI> items = new array<EntityAI>;
		container.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);

		VSM_VirtualizeQueue queue = new VSM_VirtualizeQueue(container);
		m_ActiveQueues.Insert(container.VSM_GetId(), queue);
		queue.OnInit(items);
		queue.Start();
	}

	void OnLoadVirtualStore(ItemBase container)
	{
		if (!GetGame().IsServer() || container.IsDamageDestroyed())
			return;

		VSM_Info("OnLoadVirtualStore", "Init " + container.GetType() + " loaded=" + container.VSM_IsLoaded() + " hasItems=" + container.VSM_HasVirtualItems() + " processing=" + container.VSM_IsProcessing());

		if (!container.VSM_IsLoaded() || !container.VSM_HasVirtualItems() && !container.VSM_CanVirtualize() || container.VSM_IsProcessing() || HasActiveQueue(container))
			return;

		container.VSM_SetIsProcessing(true);

		string virtualPath = GetVirtualContextDirectory(container);
		if (!FileExist(virtualPath))
		{
			VSM_Error("OnLoadVirtualStore", "A pasta virtual não existe " + virtualPath);
			container.VSM_SetHasItems(false);
			container.VSM_SetIsProcessing(false);

			return;
		}

		string virtualFile = GetVirtualFile(container);
		if (!FileExist(virtualFile))
		{
			VSM_Error("OnLoadVirtualStore", "O arquivo virtual não existe" + virtualFile);
			container.VSM_SetHasItems(false);
			container.VSM_SetIsProcessing(false);

			return;
		}

		FileSerializer serializer = new FileSerializer();
		VirtualStorageFile virtualStorage = new VirtualStorageFile();

		if (!serializer.Open(virtualFile, FileMode.READ))
		{
			VSM_Error("OnLoadVirtualStore", "Não foi possível abrir o arquivo virtual" + virtualFile);
			container.VSM_SetIsProcessing(false);
			return;
		}

		if (serializer.Read(virtualStorage))
		{
			VSM_RestorationQueue queue = new VSM_RestorationQueue(container);
			m_ActiveQueues.Insert(container.VSM_GetId(), queue);
			queue.OnInit(virtualStorage.storedItems);
			queue.Start();
		}
		else
		{
			VSM_Error("OnLoadVirtualStore", container.GetType() + " não foi possível ler o arquivo virtual " + virtualFile);
			container.VSM_SetIsProcessing(false);

		}
		serializer.Close();
		VSM_Info("OnLoadVirtualStore ", container.GetType() + " concluído");
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

		container.VSM_SetIsProcessing(true);

		string virtualPath = GetVirtualContextDirectory(container);
		if (!FileExist(virtualPath))
		{
			VSM_Error("OnDestroyed", "A pasta virtual não existe " + virtualPath);
			return;
		}

		string virtualFile = GetVirtualFile(container);
		if (!FileExist(virtualFile))
		{
			VSM_Error("OnDestroyed", "O arquivo virtual não existe" + virtualFile);
			return;
		}

		FileSerializer serializer = new FileSerializer();
		VirtualStorageFile virtualStorage = new VirtualStorageFile();

		if (!serializer.Open(virtualFile, FileMode.READ))
		{
			VSM_Error("OnDestroyed", "Não foi possível abrir o arquivo virtual" + virtualFile);
			container.VSM_SetIsProcessing(false);
			return;
		}

		if (serializer.Read(virtualStorage))
		{
			container.VSM_OnBeforeContainerRestore(); //prepare
			VSM_DropQueue queue = new VSM_DropQueue(container);
			m_ActiveQueues.Insert(container.VSM_GetId(), queue);
			queue.OnInit(virtualStorage.storedItems);
			queue.Start();
		}
		else
		{
			VSM_Error("OnDestroyed", container.GetType() + " não foi possível ler o arquivo virtual " + virtualFile);
		}
		serializer.Close();
		VSM_Info("OnDestroyed ", container.GetType() + " concluído");
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
		int instanceId = GetGame().ServerConfigGetInt("instanceId");
		string currentStorage = "storage_" + instanceId;
		return "$mission:" + currentStorage + "\\virtual\\";
	}

	string GetVirtualContextDirectory(ItemBase container)
	{
		string vId = container.VSM_GetId();
		return GetVirtualDirectory() + vId + "\\";
	}

	string GetVirtualFile(ItemBase container)
	{
		return GetVirtualContextDirectory(container) + VIRTUAL_FILENAME;
	}

	string GetVirtualMetadataFile(ItemBase container)
	{
		return GetVirtualFile(container) + "." + METADATA_FILENAME;
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
}
