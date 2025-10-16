/**
 * @class VSMAddonBase
 * @brief Base class for VSM addon management and version control
 * @details This class provides a foundation for managing VSM addons with version control,
 *          allowing safe integration with existing servers and proper addon lifecycle management.
 *          Extends ModStructure to provide persistent configuration and data handling.
 * 
 * Classe base para gerenciamento de addons VSM e controle de versão
 * Esta classe fornece uma fundação para gerenciar addons VSM com controle de versão,
 * permitindo integração segura com servidores existentes e gerenciamento adequado do ciclo de vida do addon.
 * Estende ModStructure para fornecer configuração persistente e manipulação de dados.
 */
class VSMAddonBase extends ModStructure
{
    /** @brief Version addon information container
     *  @details Contains version and identification data for this addon
     *  Container de informações de versão do addon */
    protected VSM_VersionAddon m_Addon;
    
    /** @brief Flag indicating if this is a new addon installation
     *  @details True if addon is being installed for the first time, false otherwise
     *  Flag indicando se esta é uma nova instalação do addon */
    protected bool m_IsNew;
    
    /** @brief Previous version number before update
     *  @details Stores the last version number for migration and compatibility purposes
     *  Número da versão anterior antes da atualização */
    protected int m_LastVersion;

    /** @brief Initialization flag to prevent multiple initializations
     *  @details Ensures the addon is only initialized once during its lifecycle
     *  Flag de inicialização para prevenir múltiplas inicializações */
    protected bool m_VSM_Addon_Initialized;

    //==============================================================================
    //! INITIALIZATION | INICIALIZAÇÃO
    //==============================================================================
    override void LoadData()
    {
        super.LoadData();
        InitAddon();
    }

    //==============================================================================
    //! DATA MANAGEMENT | GERENCIAMENTO DE DADOS
    //==============================================================================
    
    /**
     * @brief Load addon data and register with version manager
     * @details Handles addon registration, version checking, and migration logic
     *          Determines if addon is new or existing and manages version updates
     * @note Called automatically by the ModStructure system during initialization
     * 
     * Carrega dados do addon e registra com o gerenciador de versão
     * Manipula registro do addon, verificação de versão e lógica de migração
     * Determina se o addon é novo ou existente e gerencia atualizações de versão
     * @note Chamado automaticamente pelo sistema ModStructure durante inicialização
     */
    protected void InitAddon()
	{
        string id = GetAddonId();
        if(!id || m_VSM_Addon_Initialized) return;
        m_VSM_Addon_Initialized = true;

        Print("[ITZ][VSM] Inicializando addon: " + id);

		if(VSM_Version.GetManager().HasAddon(id))
		{
            m_Addon = VSM_Version.GetManager().GetAddon(id); 	
		}
        else
        {
            m_IsNew = true;
			VSM_VersionAddon addon = new VSM_VersionAddon();
			addon.m_Id = id;
			addon.m_Version = GetInitVersion();
			VSM_Version.GetManager().RegisterAddon(addon);
            m_Addon = addon;

            Print("[ITZ][VSM] Novo addon detectado: " + id + " com versão inicial: " + addon.m_Version);
        }

        if(m_Addon.m_Version < GetVersion())
        {
            Print("[ITZ][VSM] Atualizando addon: " + id + " da versão: " + m_Addon.m_Version + " para a versão: " + GetVersion());
            m_LastVersion = m_Addon.m_Version;
            //migrar o addon para a versão atual
            //será usado apartir do próximo restart
            m_Addon.m_Version = GetVersion();
            VSM_Version.GetManager().SaveConfig();
        }
        else
        {
            m_LastVersion = GetVersion();
        }

        Print("[ITZ][VSM] Addon carregado: " + id + " com lastVersion: " + m_LastVersion);
	}

    //==============================================================================
    //! VIRTUAL METHODS | MÉTODOS VIRTUAIS
    //==============================================================================
    
    /**
     * @brief Get the unique identifier for this addon
     * @return string The unique addon ID, empty string if not implemented
     * @details Override this method to provide a unique identifier for the addon
     *          Used for registration and version management in the VSM system
     * @note Must be implemented by derived classes to function properly
     * 
     * Obtém o identificador único para este addon
     * @return string O ID único do addon, string vazia se não implementado
     * Sobrescreva este método para fornecer um identificador único para o addon
     * Usado para registro e gerenciamento de versão no sistema VSM
     * @note Deve ser implementado por classes derivadas para funcionar adequadamente
     */
    protected string GetAddonId()
    {   
        return "";
    }

    /**
     * @brief Get the initial version number for this addon
     * @return int The initial version number, defaults to 0
     * @details Used to allow integration with existing running servers
     *          This version is used when the addon is first installed
     * @note Override to set appropriate initial version for server compatibility
     * 
     * Obtém o número da versão inicial para este addon
     * @return int O número da versão inicial, padrão é 0
     * Usado para permitir integração com servidores já em execução
     * Esta versão é usada quando o addon é instalado pela primeira vez
     * @note Sobrescreva para definir versão inicial apropriada para compatibilidade do servidor
     */
    protected int GetInitVersion()
    {
        return 0;
    }
    
    /**
     * @brief Get the current version number for this addon
     * @return int The current version number, defaults to 0
     * @details InitVersion will be used on first startup, after that this version will be used if it's higher
     *          Allows safe usage of OnStoreLoad and OnStoreSave with version control
     * @note Override to provide current addon version for proper migration handling
     * 
     * Obtém o número da versão atual para este addon
     * @return int O número da versão atual, padrão é 0
     * InitVersion será usado no primeiro startup, depois esta versão será usada se for maior
     * Permite uso seguro de OnStoreLoad e OnStoreSave com controle de versão
     * @note Sobrescreva para fornecer versão atual do addon para manipulação adequada de migração
     */
    protected int GetVersion()
    {
        return 0;
    }

    //==============================================================================
    //! PUBLIC METHODS | MÉTODOS PÚBLICOS
    //==============================================================================
    
    /**
     * @brief Get the last version number before current update
     * @return int The previous version number that was running
     * @details Used to indicate the real version that was running, generated automatically
     *          Useful for migration logic and compatibility checks
     * @note This value is automatically managed by the version system
     * 
     * Obtém o número da última versão antes da atualização atual
     * @return int O número da versão anterior que estava rodando
     * Usado para indicar a versão real que estava rodando, gerado automaticamente
     * Útil para lógica de migração e verificações de compatibilidade
     * @note Este valor é gerenciado automaticamente pelo sistema de versão
     */
    int GetLastVersion()
    {
        return m_LastVersion;
    }

    /**
     * @brief Check if this is a new addon installation
     * @return bool True if addon is being installed for the first time, false otherwise
     * @details Indicates if the addon was newly installed during this server session
     * 
     * Verifica se esta é uma nova instalação do addon
     * @return bool Verdadeiro se o addon está sendo instalado pela primeira vez, falso caso contrário
     */
    bool IsNew()
    {
        return m_IsNew;
    }
}