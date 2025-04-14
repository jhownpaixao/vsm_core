
[CF_RegisterModule(VirtualStorageModule)]
class VirtualStorageModule : CF_ModuleWorld
{
	protected static VirtualStorageModule sm_Instance;
	ref array<ref VSM_RestorationQueue> m_ActiveRestorations = new array<ref VSM_RestorationQueue>;
	bool m_Debug = true;
	bool m_IsLoaded;

	ref array<ItemBase> m_InitContainers = new array<ItemBase>;

	void VirtualStorageModule()
	{
		sm_Instance = this;
		m_IsLoaded = false;

		bool autoClose = CfgGameplayHandler.GetVSM_EnableAutoClose();
		m_Debug = CfgGameplayHandler.GetVSM_EnableDebug(); // update
		Log("Constructor", "Starting VirtualStorageModule: m_Debug=" + m_Debug + " autoClose=" + autoClose + " time=" + CfgGameplayHandler.GetVSM_TimeToAutoClose());
	}

#ifdef SERVER
	override void OnInit()
	{
		super.OnInit();
		EnableMissionStart();
		EnableMissionLoaded();

		m_Debug = CfgGameplayHandler.GetVSM_EnableDebug(); // update
	}

	override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		super.OnMissionStart(sender, args);

		Log("OnMissionStart", "init");

		if (!FileExist(GetVirtualDirectory()))
			VirtualUtils.MakeDirectoryRecursive(GetVirtualDirectory());
	}

	override void OnMissionLoaded(Class sender, CF_EventArgs args)
	{
		super.OnMissionLoaded(sender, args);

		Log("OnMissionLoaded", " init");

		m_IsLoaded = true;
		ProcessContainersInit();
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

		Log("OnInitContainer", " iniciando virtual para " + container.GetType() + " aberto? " + container.VSM_IsOpen());

		if (container.VSM_IsOpen())
		{
			Log("OnInitContainer", "container aberto, fechando..." + container.GetType());
			container.VSM_SetVirtualLoaded(true);
			container.VSM_Close(); //!Ao fechar segue OnSaveVirtualStore
			return;
		}

		container.VSM_SetVirtualLoaded(true);
		if (!FileExist(virtualFile))
		{
			Error("OnInitContainer", "o arquivo de storage não existe " + container.GetType() + " chamando save");
			OnSaveVirtualStore(container);
		}
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
		if (!GetGame().IsServer() || !container.VSM_IsLoaded())
			return;

		Log("OnSaveVirtualStore", container.GetType());

		string virtualPath = GetVirtualContextDirectory(container);
		if (!FileExist(virtualPath))
			MakeDirectory(virtualPath);

		string virtualFile = GetVirtualFile(container);

		FileSerializer serializer = new FileSerializer();
		VirtualStorageFile virtualStorage = new VirtualStorageFile();
		virtualStorage.storedItems = new array<ref VirtualObject>;

		Log("OnSaveVirtualStore", container.GetType() + " preparing to save: " + virtualFile);

		container.VSM_OnBeforeContainerVirtualize(); // prepare

		array<EntityAI> items = new array<EntityAI>;
		container.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);

		int total = items.Count();
		for (int i = 0; i < total; i++)
		{
			ItemBase item = ItemBase.Cast(items.Get(i));
			if (!item)
			{
				Error("OnSaveVirtualStore", container.GetType() + " item not valid");
				continue;
			}

			Log("OnSaveVirtualStore", container.GetType() + " saving: " + item.GetType() + " virtualize? " + item.VSM_CanVirtualize());

			if (item && item.VSM_CanVirtualize())
			{
				item.VSM_OnBeforeVirtualize();
				VirtualObject virtualObj = new VirtualObject();

				virtualObj.OnVirtualize(virtualPath, item, container);

				virtualStorage.storedItems.Insert(virtualObj);

				Log("OnSaveVirtualStore", "Adicionando novo item " + virtualObj);
			}
		}

		container.VSM_SetHasItems(total != 1);

		if (!serializer.Open(virtualFile, FileMode.WRITE))
		{
			Error("OnSaveVirtualStore", "Não foi possível abrir o arquivo virtual para escrever:" + virtualFile);
			return;
		}

		if (serializer.Write(virtualStorage))
		{
			OnAfterSaveVirtualStorage(container);
			container.VSM_OnAfterContainerVirtualize();
			Log("OnSaveVirtualStore", " concluído" + container.GetType());
		}
		else
		{
			Error("OnSaveVirtualStore", "Não foi possível escrever o arquivo de virtualização" + container.GetType() + " file=" + virtualFile);
		}

		serializer.Close();
		Log("OnSaveVirtualStore", "terminado" + container.GetType());
	}

	void OnAfterSaveVirtualStorage(ItemBase container)
	{
		array<EntityAI> items = new array<EntityAI>;
		container.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);

		int total = items.Count();

		for (int j = 0; j < total; j++)
		{
			ItemBase item = ItemBase.Cast(items.Get(j));
			if (item && item.VSM_CanVirtualize())
			{
				item.VSM_OnAfterVirtualize();
				item.Delete();
			}
		}
	}

	void OnLoadVirtualStore(ItemBase container)
	{
		if (!GetGame().IsServer())
			return;

		Log("OnLoadVirtualStore", " init" + container.GetType() + " LOADED=" + container.VSM_IsLoaded() + " HASITEMS=" + container.VSM_HasVirtualItems() + " RESTORING" + container.VSM_IsRestoring());

		if (!container.VSM_IsLoaded() || !container.VSM_HasVirtualItems() && !container.VSM_CanVirtualize() || container.VSM_IsRestoring())
			return;

		Log("OnLoadVirtualStore", container.GetType());

		string virtualPath = GetVirtualContextDirectory(container);
		if (!FileExist(virtualPath))
		{
			Error("OnLoadVirtualStore", "A pasta virtual não existe " + virtualPath);
			return;
		}

		string virtualFile = GetVirtualFile(container);
		if (!FileExist(virtualFile))
		{
			Error("OnLoadVirtualStore", "O arquivo virtual não existe" + virtualFile);
			return;
		}

		FileSerializer serializer = new FileSerializer();
		VirtualStorageFile virtualStorage = new VirtualStorageFile();

		if (!serializer.Open(virtualFile, FileMode.READ))
		{
			Error("OnLoadVirtualStore", "Não foi possível abrir o arquivo virtual" + virtualFile);
			return;
		}

		if (serializer.Read(virtualStorage))
		{
			container.VSM_OnBeforeContainerRestore(); //prepare
			VSM_RestorationQueue queue = new VSM_RestorationQueue(container, virtualStorage.storedItems);
			m_ActiveRestorations.Insert(queue);
			queue.OnInit();
			queue.Start();
		}
		else
		{
			Error("OnLoadVirtualStore", container.GetType() + " não foi possível ler o arquivo virtual " + virtualFile);
		}
		serializer.Close();
		Log("OnLoadVirtualStore ", container.GetType() + " terminated ");
	}

	void OnDeleteContainer(ItemBase container)
	{
		string path = GetVirtualContextDirectory(container);

		if (FileExist(path) && container.VSM_CanDeleteVirtualFile())
		{
			Log("OnDeleteContainer", container.GetType());
			VirtualUtils.DeleteFiles(path);
		}
	}

	void ProcessContainersInit()
	{
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
		return GetVirtualContextDirectory(container) + "main.bin";
	}

	static VirtualStorageModule GetModule()
	{
		return sm_Instance;
	}

	void RemoveActiveRestoration(VSM_RestorationQueue queue)
	{
		m_ActiveRestorations.RemoveItem(queue);
	}


	void Log(string method, string msg)
	{
		if (!m_Debug)
			return;

		Print("[VirtualStorageModule]" + "[" + method + "]" + msg);
	}

	void Error(string method, string msg)
	{
		if (!m_Debug)
			return;

		Print("[VirtualStorageModule]" + "[" + method + "] " + msg);
	}
}
