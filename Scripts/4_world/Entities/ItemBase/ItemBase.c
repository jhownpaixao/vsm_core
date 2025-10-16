/**
 * @class ItemBase
 * @brief Extended ItemBase class with Virtual Storage Module (VSM) functionality
 * @details This modded class adds virtual storage capabilities to the base ItemBase class,
 *          allowing items to be virtualized and managed through the VSM system.
 * 
 * Classe ItemBase estendida com funcionalidade do Virtual Storage Module (VSM)
 * Esta classe modificada adiciona capacidades de armazenamento virtual à classe ItemBase base,
 * permitindo que itens sejam virtualizados e gerenciados através do sistema VSM.
 */
modded class ItemBase
{
    /** @brief Whether there are virtualized items in this storage
     *  @details Indicates if this storage container has any items that have been virtualized
     *  Se existem itens virtualizados neste storage */
    protected bool          m_VSM_HasVirtualItems;
    
    /** @brief Whether processing virtualization/restoration (to lock storage)
     *  @details Used to prevent storage manipulation during virtualization/restoration processes
     *  Se está processando virtualização/restauração (para travar o storage) */
    protected bool          m_VSM_ProcessingItems;
    
    /** @brief Whether virtual storage is loaded and ready
     *  @details Indicates if the virtual storage system has been properly initialized for this container
     *  Se o storage virtual está carregado e pronto */
    protected bool          m_VSM_VirtualStorageLoaded;
    
    /** @brief Defined by module to indicate this storage is virtual type
     *  @details Automatically set by the VSM module to mark containers as virtual storage
     *  Definido pelo módulo para indicar que este storage é do tipo virtual */
    protected bool          m_VSM_IsVirtualStorage;
    
    /** @brief Used to ignore restoration queue
     *  @details Allows opening/closing containers without processing virtualization
     *           Useful for synchronization or state recovery scenarios
     *  Utilizado para ignorar a fila de restauração, exemplo: apenas abrir o container sem restaurar */
    protected bool          m_VSM_IgnoreVirtualization;
    
    /** @brief Used to store the player who destroyed the container
     *  @details Stores player reference for GameLabs integration purposes
     *  Utilizado para armazenar o player que destruiu o container (para fins de GameLabs) */
    protected PlayerBase    m_LastHitPlayer;

    /** @brief Force receiving items into this storage
     *  @details When true, forces items to be received even if normally blocked
     *  Força o recebimento de itens neste storage */
    protected bool          m_VSM_ForceReceiveItems;
    
    /** @brief Manages auto-close behavior
     *  @details Controls automatic closing functionality for virtual storage containers
     *  Gerencia o comportamento de auto fechamento */
    protected ref VSMAutoCloseBehavior m_AutoCloseBehavior;
    
    //==============================================================================
    //! BEHAVIOR METHODS | MÉTODOS DE COMPORTAMENTO
    //==============================================================================
    
    /**
     * @brief Initialize item variables for VSM functionality
     * @details Sets up all VSM-related variables and registers network synchronization
     * 
     * Inicializa variáveis do item para funcionalidade VSM
     * Configura todas as variáveis relacionadas ao VSM e registra sincronização de rede
     */
    override void InitItemVariables()
    {
        super.InitItemVariables();

        m_AutoCloseBehavior         = new VSMAutoCloseBehavior(this);
        m_VSM_HasVirtualItems       = false;
        m_VSM_ProcessingItems       = false;
        m_VSM_VirtualStorageLoaded  = false;

        RegisterNetSyncVariableBool("m_VSM_HasVirtualItems");
        RegisterNetSyncVariableBool("m_VSM_ProcessingItems");
        RegisterNetSyncVariableBool("m_VSM_VirtualStorageLoaded");
        RegisterNetSyncVariableBool("m_VSM_IsVirtualStorage");
    }

    /**
     * @brief Handle entity death event
     * @param killer The object that killed this entity
     * @details Reports container destruction to GameLabs integration when applicable
     * 
     * Manipula evento de morte da entidade
     * Reporta destruição do container para integração GameLabs quando aplicável
     */
    override void EEKilled(Object killer)
	{
        super.EEKilled(killer);
        if(!VSM_IsVirtualStorage()) return;

        EntityAI entity = EntityAI.Cast(killer);
        PlayerBase player = PlayerBase.Cast(killer);

        if(entity && !player)
        {
            player = PlayerBase.Cast(entity.GetHierarchyParent());
        }
        
        if(!player && !m_LastHitPlayer) return;

        if(!player) player = m_LastHitPlayer;
        m_LastHitPlayer = null;

        VSMGameLabsIntegration gamelabs = new VSMGameLabsIntegration(player, this);
		gamelabs.ReportContainerInteraction(VSM_ReportAction.DESTROY);
	}

    /**
     * @brief Handle entity hit event
     * @param damageResult The damage result
     * @param damageType The type of damage
     * @param source The source of damage
     * @param component The component hit
     * @param dmgZone The damage zone
     * @param ammo The ammunition type
     * @param modelPos The model position
     * @param speedCoef The speed coefficient
     * @details Tracks the last player who hit this container for GameLabs integration
     * 
     * Manipula evento de impacto na entidade
     * Rastreia o último jogador que atingiu este container para integração GameLabs
     */
    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        super.EEHitBy(damageResult,damageType,source,component,dmgZone,ammo,modelPos,speedCoef);
        if(!VSM_IsVirtualStorage()) return;

        PlayerBase player = PlayerBase.Cast(source.GetHierarchyRootPlayer());
        if(!player || player == m_LastHitPlayer) return;

        m_LastHitPlayer = player;
    }

    /**
     * @brief Check if this storage can currently be manipulated
     * @details Returns false if the storage is processing virtualization/restoration
     * 
     * Verifica se este storage pode ser manipulado atualmente
     * Retorna falso se o storage estiver processando virtualização/restauração
     */
    override bool CanReceiveItemIntoCargo(EntityAI item)
    {
        if(VSM_ForceReceiveItems())
            return true;

        return super.CanReceiveItemIntoCargo(item);
    }

    /**
     * @brief Check if this storage can currently be manipulated
     * @details Returns false if the storage is processing virtualization/restoration
     * 
     * Verifica se este storage pode ser manipulado atualmente
     * Retorna falso se o storage estiver processando virtualização/restauração
     */
    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        if(VSM_ForceReceiveItems())
            return true;

        return super.CanReceiveAttachment(attachment, slotId);
    }

    //==============================================================================
    //! VSM CONTROLLERS | CONTROLADORES VSM
    //==============================================================================
    
    /**
     * @brief Open this storage container
     * @details Virtual method to be overridden by specific storage implementations
     * 
     * Abre este container de armazenamento
     * Método virtual para ser sobrescrito por implementações específicas de storage
     */
    void VSM_Open() 
    {
        VSM_GetAutoCloseBehavior().Start();
    }

    /**
     * @brief Close this storage container
     * @details Virtual method to be overridden by specific storage implementations
     * 
     * Fecha este container de armazenamento
     * Método virtual para ser sobrescrito por implementações específicas de storage
     */
    void VSM_Close() 
    {
        VSM_GetAutoCloseBehavior().Start();
    }

    //==============================================================================
    //! VIRTUALIZATION HOOKS | HOOKS DE VIRTUALIZAÇÃO
    //==============================================================================
    
    /**
     * @brief Called before this storage starts virtualizing
     * @details Override this method to perform pre-virtualization setup
     * 
     * Chamado antes deste storage começar a virtualizar
     * Sobrescreva este método para realizar configuração pré-virtualização
     */
    void VSM_OnBeforeContainerVirtualize() { }

    /**
     * @brief Called after this storage finishes virtualizing
     * @details Stops auto-close behavior when virtualization is complete
     * 
     * Chamado após este storage terminar de virtualizar
     * Para comportamento de auto-fechamento quando virtualização é concluída
     */
    void VSM_OnAfterContainerVirtualize()
    {
        VSM_GetAutoCloseBehavior().Stop();
    }

    /**
     * @brief Called before this item is virtualized by a storage
     * @details Override this method to perform pre-virtualization item setup
     * 
     * Chamado antes deste item ser virtualizado por um storage
     * Sobrescreva este método para realizar configuração pré-virtualização do item
     */
    void VSM_OnBeforeVirtualize() { }

    /**
     * @brief Called after this item is virtualized by a storage
     * @warning This is called moments before the item is deleted - use with caution!
     * @details Override this method to perform cleanup before item deletion
     * 
     * Chamado após este item ser virtualizado por um storage
     * @warning É chamado momentos antes do item ser deletado - use com cuidado!
     * Sobrescreva este método para realizar limpeza antes da exclusão do item
     */
    void VSM_OnAfterVirtualize() { } 

    /**
     * @brief Called before this storage virtualizes its children
     * @details Override this method to perform setup before child virtualization
     * 
     * Chamado antes deste storage virtualizar seus filhos
     * Sobrescreva este método para realizar configuração antes da virtualização dos filhos
     */
    void VSM_OnBeforeVirtualizeChildren() { }

    /**
     * @brief Called after this storage virtualizes its children
     * @details Override this method to perform cleanup after child virtualization
     * 
     * Chamado após este storage virtualizar seus filhos
     * Sobrescreva este método para realizar limpeza após virtualização dos filhos
     */
    void VSM_OnAfterVirtualizeChildren() { }

    /**
     * @brief Save the variable context of this item with VSM
     * @param ctx The write context for parameter serialization
     * @details Equivalent to OnStoreSave() - saves item state for virtualization
     * 
     * Salva o contexto de variáveis deste item com o VSM
     * @param ctx O contexto de escrita para serialização de parâmetros
     * Equivalente a OnStoreSave() - salva estado do item para virtualização
     */
    void VSM_OnVirtualStoreSave(ParamsWriteContext ctx)
    {
        OnStoreSave(ctx);
    }

    //==============================================================================
    //! RESTORATION HOOKS | HOOKS DE RESTAURAÇÃO
    //==============================================================================
    
    /**
     * @brief Called before this storage starts restoring
     * @details Sets processing flag to prevent manipulation during restoration
     * 
     * Chamado antes deste storage começar a restaurar
     * Define flag de processamento para evitar manipulação durante restauração
     */
    void VSM_OnBeforeContainerRestore()
    {
        //! VSM_SetIsProcessing(true);
    }

    /**
     * @brief Called after this storage finishes restoring
     * @details Clears processing flag and starts auto-close behavior
     * 
     * Chamado após este storage terminar de restaurar
     * Limpa flag de processamento e inicia comportamento de auto-fechamento
     */
    void VSM_OnAfterContainerRestore()
    {
        //! VSM_SetIsProcessing(false);
        VSM_GetAutoCloseBehavior().Start();
    }

    /**
     * @brief Called before children are restored
     * @details Override this method to perform setup before child restoration
     * 
     * Chamado antes dos filhos serem restaurados
     * Sobrescreva este método para realizar configuração antes da restauração dos filhos
     */
    void VSM_OnBeforeRestoreChildren() { }

    /**
     * @brief Called after children are restored
     * @details Override this method to perform cleanup after child restoration
     * 
     * Chamado após os filhos serem restaurados
     * Sobrescreva este método para realizar limpeza após restauração dos filhos
     */
    void VSM_OnAfterRestoreChildren() { }

    /**
     * @brief Called before this item is restored by a storage
     * @details Item has been spawned but variables haven't been applied yet
     * @warning Restoration is still ongoing at this point - use with caution!
     * @note Use this to prepare the item to receive its variables
     * 
     * Chamado antes deste item ser restaurado por um storage
     * Item já foi spawnado mas variáveis ainda não foram aplicadas
     * @warning Restauração ainda está em andamento neste ponto - use com cuidado!
     * @note Use para preparar o item para receber suas variáveis
     */
    void VSM_OnBeforeRestore() { }

    /**
     * @brief Called after this item is restored by a storage
     * @details Marks the item as dirty and triggers an update
     * 
     * Chamado após este item ser restaurado por um storage
     * Marca o item como dirty e dispara uma atualização
     */
    void VSM_OnAfterRestore()
    {
        SetSynchDirty();
        Update();
    }

    /**
     * @brief Load the variable context of this item in VSM
     * @param ctx The read context for parameter deserialization
     * @param version The version of the saved data
     * @return bool True if loading was successful, false otherwise
     * @details Equivalent to OnStoreLoad() - loads item state from virtualization
     * 
     * Carrega o contexto de variáveis deste item no VSM
     * @param ctx O contexto de leitura para desserialização de parâmetros
     * @param version A versão dos dados salvos
     * @return bool True se carregamento foi bem-sucedido, false caso contrário
     * Equivalente a OnStoreLoad() - carrega estado do item da virtualização
     */
    bool VSM_OnVirtualStoreLoad(ParamsReadContext ctx, int version)
    {
        return OnStoreLoad(ctx, version);
    }

    /**
     * @brief Called after loading all variable contexts of this item in VSM
     * @details Equivalent to AfterStoreLoad() - performs post-load initialization
     * 
     * Chamado após carregar todos os contextos de variáveis deste item no VSM
     * Equivalente a AfterStoreLoad() - realiza inicialização pós-carregamento
     */
    void VSM_AfterVirtualStoreLoad()
    {
        AfterStoreLoad();
    }

    //==============================================================================
    //! UTILITY METHODS | MÉTODOS UTILITÁRIOS
    //==============================================================================
    
    /**
     * @brief Check if this item is attached to a vehicle
     * @return bool True if attached to a vehicle, false otherwise
     * @details Verifies if the item is attached as an attachment to a car/vehicle
     * 
     * Verifica se este item está anexado a um veículo
     * @return bool True se anexado a um veículo, false caso contrário
     * Verifica se o item está anexado como attachment a um carro/veículo
     */
    bool VSM_IsAttachedOnVehicle()
    {
        EntityAI parent = EntityAI.Cast(GetHierarchyParent());
        if (parent && parent.IsInherited(CarScript))
        {
            InventoryLocation location = new InventoryLocation();
            if (GetInventory().GetCurrentInventoryLocation(location))
            {
                return location.GetType() == InventoryLocationType.ATTACHMENT;
            }
        }

        return false;
    }

    /**
     * @brief Check if this item/storage is with a player
     * @return bool True if the item is in player inventory, false otherwise
     * @details Verifies if the item's parent hierarchy contains a PlayerBase
     * 
     * Verifica se este item/storage está com um jogador
     * @return bool True se o item está no inventário do jogador, false caso contrário
     * Verifica se a hierarquia pai do item contém um PlayerBase
     */
    bool VSM_InPlayer()
    {
        EntityAI parent = EntityAI.Cast(GetHierarchyParent());
        if (parent && parent.IsInherited(PlayerBase))
        {
            return true;
        }

        return false;
    }

    /**
     * @brief Force receiving items into this storage
     * @param requiredOpen Optional parameter to require storage to be open (1) or closed (0)
     * @details When true, allows items to be added even if normally blocked
     * 
     * Força o recebimento de itens neste storage
     * @param requiredOpen Parâmetro opcional para exigir que o storage esteja aberto (1) ou fechado (0)
     * @details Quando verdadeiro, permite que itens sejam adicionados mesmo se normalmente bloqueado
     */
    bool VSM_CanManipule(int requiredOpen = 1)
    {
        if(VSM_CanVirtualize())
        {
            bool can = true;

            if(requiredOpen != -1)
            {
                can = VSM_IsOpen() && requiredOpen == 1 || !VSM_IsOpen() && requiredOpen == 0;
            }

            return (!VSM_IsProcessing() && can) || VSM_ForceReceiveItems();
        }

        return true;
    }
    
    /**
     * @brief Check if container can be opened
     * @return bool True if container can be opened, false otherwise
     * @details Verifies if container is not processing and is currently closed
     * 
     * Verifica se o container pode ser aberto
     * @return bool True se o container pode ser aberto, false caso contrário
     * Verifica se o container não está processando e está atualmente fechado
     */
    bool VSM_CanOpen()
    {
        if(VSM_CanVirtualize())
        {
            return !VSM_IsProcessing() && !VSM_IsOpen();
        }

        return true;
    }

    /**
     * @brief Check if container can be closed
     * @return bool True if container can be closed, false otherwise
     * @details Verifies if container is not processing and is currently open
     * 
     * Verifica se o container pode ser fechado
     * @return bool True se o container pode ser fechado, false caso contrário
     * Verifica se o container não está processando e está atualmente aberto
     */
    bool VSM_CanClose()
    {
        if(VSM_CanVirtualize())
        {
            return !VSM_IsProcessing() && VSM_IsOpen();
        }

        return true;
    }

    /**
     * @brief Check if this item can be virtualized by a storage
     * @return bool True if item can be virtualized, false otherwise
     * @details Checks decay settings and ignore list to determine virtualizability
     * 
     * Verifica se este item pode ser virtualizado por um storage
     * @return bool True se o item pode ser virtualizado, false caso contrário
     * Verifica configurações de decaimento e lista de ignorados para determinar virtualizabilidade
     */
    bool VSM_IsVirtualizable()
    {
        bool includeDecayItems = VSM_Settings.GetSettings().m_IncludeDecayItems;
        if (!includeDecayItems && CanDecay())
            return false;
        
        if(VirtualStorageModule.GetModule().IsIgnoredItem(GetType()))
            return false;
    
        return true;
    }

    /**
     * @brief Check if this storage can virtualize other items
     * @return bool True if storage can virtualize items, false otherwise
     * @details Verifies if this storage type is not in the ignore list
     * 
     * Verifica se este storage pode virtualizar outros itens
     * @return bool True se o storage pode virtualizar itens, false caso contrário
     * Verifica se este tipo de storage não está na lista de ignorados
     */
    bool VSM_CanVirtualize()
    {
        if(VirtualStorageModule.GetModule().IsIgnoredItem(GetType()))
            return false;
    
        return true;
    }

    /**
     * @brief Check if virtual files should be deleted when deleting this item
     * @return bool True if files can be deleted, false otherwise
     * @details Prevents file deletion when virtual storage is with a player
     * 
     * Verifica se arquivos virtuais devem ser deletados ao excluir este item
     * @return bool True se arquivos podem ser deletados, false caso contrário
     * Impede exclusão de arquivos quando storage virtual está com um jogador
     */
    bool VSM_CanDeleteVirtualFile()
    {
        if(VSM_IsVirtualStorage() && VSM_InPlayer())
            return false;
        
        return true;
    }

    /**
     * @brief Check if this item is a virtual storage
     * @return bool True if item is virtual storage, false otherwise
     * @details Returns the automatically defined virtual storage flag
     * 
     * Verifica se este item é um storage virtual
     * @return bool True se o item é storage virtual, false caso contrário
     * Retorna a flag de storage virtual definida automaticamente
     */
    bool VSM_IsVirtualStorage()
    {
        return m_VSM_IsVirtualStorage;
    }

    /**
     * @brief Set if this item is a virtual storage
     * @param isVirtual True to mark as virtual storage, false otherwise
     * @details Automatically defined by the VSM system
     * 
     * Define se este item é um storage virtual
     * @param isVirtual True para marcar como storage virtual, false caso contrário
     * Definido automaticamente pelo sistema VSM
     */
    void VSM_SetIsVirtualStorage(bool isVirtual)
    {
        m_VSM_IsVirtualStorage = isVirtual;
        SetSynchDirty();
    }

    /**
     * @brief Check if virtualization/restoration should be ignored
     * @return bool True if virtualization is ignored, false otherwise
     * @details Used to open/close storage without processing virtualization
     *          Useful in synchronization or state recovery cases
     * 
     * Verifica se virtualização/restauração deve ser ignorada
     * @return bool True se virtualização é ignorada, false caso contrário
     * Usado para abrir/fechar storage sem processar virtualização
     * Útil em casos de sincronização ou recuperação de estado
     */
    bool VSM_IsIgnoreVirtualization()
    {
        return m_VSM_IgnoreVirtualization;
    }

    /**
     * @brief Set if virtualization/restoration should be ignored
     * @param ignore True to ignore virtualization, false otherwise
     * @details Controls whether virtualization processes are bypassed
     * 
     * Define se virtualização/restauração deve ser ignorada
     * @param ignore True para ignorar virtualização, false caso contrário
     * Controla se processos de virtualização são ignorados
     */
    void VSM_SetIgnoreVirtualization(bool ignore)
    {
        m_VSM_IgnoreVirtualization = ignore;
    }

    /**
     * @brief Get the auto-close behavior controller
     * @return VSMAutoCloseBehavior The auto-close behavior instance
     * @details Returns the behavior controller for automatic container closing
     * 
     * Obtém o controlador de comportamento de auto-fechamento
     * @return VSMAutoCloseBehavior A instância do comportamento de auto-fechamento
     * Retorna o controlador de comportamento para fechamento automático de container
     */
    VSMAutoCloseBehavior VSM_GetAutoCloseBehavior()
    {
        return m_AutoCloseBehavior;
    }

    /**
     * @brief Get the virtualization ID of this storage
     * @return string The unique virtualization ID
     * @details Generates ID based on persistent ID and type for virtual storage identification
     * 
     * Obtém o ID de virtualização deste storage
     * @return string O ID único de virtualização
     * Gera ID baseado no ID persistente e tipo para identificação de storage virtual
     */
    string VSM_GetId()
    {
        int persistent1;
        int persistent2;
        int persistent3;
        int persistent4;
        string id;

        GetPersistentID(persistent1, persistent2, persistent3, persistent4);
        id = GetType() + "_" + VSM_NormalizeSingleId(persistent1) + VSM_NormalizeSingleId(persistent2) + VSM_NormalizeSingleId(persistent3) + VSM_NormalizeSingleId(persistent4);
        return id;
    }

    /**
     * @brief Normalize an ID by removing the negative sign
     * @param id The ID to normalize
     * @return string The normalized ID without negative signs
     * @details Removes negative signs from ID components for consistent formatting
     * 
     * Normaliza um ID removendo o sinal de negativo
     * @param id O ID para normalizar
     * @return string O ID normalizado sem sinais negativos
     * Remove sinais negativos dos componentes do ID para formatação consistente
     */
    string VSM_NormalizeSingleId(int id)
    {
        string strId = id.ToString();
        strId.Replace("-", "");
        return strId;
    }

    /**
     * @brief Check if there are virtualized items in this storage
     * @return bool True if storage has virtualized items, false otherwise
     * @details Automatically defined by the VSM system
     * 
     * Verifica se existem itens virtualizados neste storage
     * @return bool True se storage tem itens virtualizados, false caso contrário
     * Definido automaticamente pelo sistema VSM
     */
    bool VSM_HasVirtualItems()
    {
        return m_VSM_HasVirtualItems;
    }

    /**
     * @brief Set if there are virtualized items in this storage
     * @param has True if storage has virtualized items, false otherwise
     * @details Automatically defined by the VSM system
     * 
     * Define se existem itens virtualizados neste storage
     * @param has True se storage tem itens virtualizados, false caso contrário
     * Definido automaticamente pelo sistema VSM
     */
    void VSM_SetHasItems(bool has)
    {
        m_VSM_HasVirtualItems = has;
        SetSynchDirty();
    }

    /**
     * @brief Check if this storage is ready for virtualization
     * @return bool True if storage is loaded and ready, false otherwise
     * @details Defined in OnInitContainer of the VSM Module
     * 
     * Verifica se este storage está preparado para virtualização
     * @return bool True se storage está carregado e pronto, false caso contrário
     * Definido no OnInitContainer do Módulo VSM
     */
    bool VSM_IsLoaded()
    {
        return m_VSM_VirtualStorageLoaded;
    }

    /**
     * @brief Set if this storage is ready for virtualization
     * @param loaded True if storage is ready, false otherwise
     * @details Controls whether storage is prepared for virtual operations
     * 
     * Define se este storage está preparado para virtualização
     * @param loaded True se storage está pronto, false caso contrário
     * Controla se storage está preparado para operações virtuais
     */
    void VSM_SetVirtualLoaded(bool loaded)
    {
        m_VSM_VirtualStorageLoaded = loaded;
        SetSynchDirty();
    }

    /**
     * @brief Check if this storage is processing any virtualization/restoration
     * @return bool True if processing, false otherwise
     * @details Indicates if storage is currently busy with virtual operations
     * 
     * Verifica se este storage está processando alguma virtualização/restauração
     * @return bool True se processando, false caso contrário
     * Indica se storage está atualmente ocupado com operações virtuais
     */
    bool VSM_IsProcessing()
    {
        return m_VSM_ProcessingItems;
    }

    /**
     * @brief Set if this storage is processing any virtualization/restoration
     * @param processing True if processing, false otherwise
     * @details Controls the processing state flag for virtual operations
     * 
     * Define se este storage está processando alguma virtualização/restauração
     * @param processing True se processando, false caso contrário
     * Controla a flag de estado de processamento para operações virtuais
     */
    void VSM_SetIsProcessing(bool processing)
    {
        m_VSM_ProcessingItems = processing;
        SetSynchDirty();
    }

    /**
     * @brief Check if this storage is open
     * @return bool True if storage is open, false otherwise
     * @details Virtual method to be overridden by specific storage implementations
     * 
     * Verifica se este storage está aberto
     * @return bool True se storage está aberto, false caso contrário
     * Método virtual para ser sobrescrito por implementações específicas de storage
     */
    bool VSM_IsOpen()
    {
        return false;
    }

    /**
     * @brief Check if this storage should force receiving items
     * @return bool True if forced to receive items, false otherwise
     * @details When true, forces items to be received even if normally blocked
     * 
     * Verifica se este storage deve forçar o recebimento de itens
     * @return bool True se forçado a receber itens, false caso contrário
     * Quando true, força itens a serem recebidos mesmo que normalmente bloqueados
     */
    bool VSM_ForceReceiveItems()
    {
        return m_VSM_ForceReceiveItems;
    }

    /**
     * @brief Set if this storage should force receiving items
     * @param force True to force receiving items, false otherwise
     * @details When true, forces items to be received even if normally blocked
     * 
     * Define se este storage deve forçar o recebimento de itens
     * @param force True para forçar o recebimento de itens, false caso contrário
     * Quando true, força itens a serem recebidos mesmo que normalmente bloqueados
     */
    void VSM_SetForceReceiveItems(bool force)
    {
        m_VSM_ForceReceiveItems = force;
    }

    //==============================================================================
    //! DEPRECATED METHODS | MÉTODOS DEPRECIADOS
    //==============================================================================
    
    /**
     * @deprecated Use VSM_GetAutoCloseBehavior() instead
     * @brief Start auto-close behavior (deprecated)
     * @details This method is deprecated, use VSM_GetAutoCloseBehavior() for better control
     * 
     * @deprecated Use VSM_GetAutoCloseBehavior() ao invés
     * @brief Inicia comportamento de auto-fechamento (depreciado)
     * Este método está depreciado, use VSM_GetAutoCloseBehavior() para melhor controle
     */
    void VSM_StartAutoClose() { };
    
    /**
     * @deprecated Use VSM_GetAutoCloseBehavior() instead
     * @brief Stop auto-close behavior (deprecated)
     * @details This method is deprecated, use VSM_GetAutoCloseBehavior() for better control
     * 
     * @deprecated Use VSM_GetAutoCloseBehavior() ao invés
     * @brief Para comportamento de auto-fechamento (depreciado)
     * Este método está depreciado, use VSM_GetAutoCloseBehavior() para melhor controle
     */
    void VSM_StopAutoClose() { };
    
    /**
     * @deprecated Use VSM_GetAutoCloseBehavior() instead
     * @brief Check if auto-close is possible (deprecated)
     * @return bool Always returns true
     * @details This method is deprecated, use VSM_GetAutoCloseBehavior() for better control
     * 
     * @deprecated Use VSM_GetAutoCloseBehavior() ao invés
     * @brief Verifica se auto-fechamento é possível (depreciado)
     * @return bool Sempre retorna true
     * Este método está depreciado, use VSM_GetAutoCloseBehavior() para melhor controle
     */
    bool VSM_CanAutoClose() { return true; };
}