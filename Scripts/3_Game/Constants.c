
/**
 * @file Constants.c
 * @brief VSM system constants and version control definitions
 * @details This file contains all constant definitions, version control classes, and enumerations
 *          used throughout the Virtual Storage Module (VSM) system for DayZ/Enfusion.
 * 
 * Constantes do sistema VSM e definições de controle de versão
 * Este arquivo contém todas as definições de constantes, classes de controle de versão e enumerações
 * usadas em todo o sistema Virtual Storage Module (VSM) para DayZ/Enfusion.
 */

//==============================================================================
//! VERSION CONTROL CLASSES | CLASSES DE CONTROLE DE VERSÃO
//==============================================================================

/**
 * @class VSM_StorageVersion
 * @brief Version control for VSM storage files
 * @details Manages compatibility and versioning for virtual storage data files
 *          Ensures proper migration and compatibility between different VSM versions
 * 
 * Controle de versão para arquivos de storage do VSM
 * Gerencia compatibilidade e versionamento para arquivos de dados de storage virtual
 * Garante migração adequada e compatibilidade entre diferentes versões do VSM
 */
class VSM_StorageVersion
{
    /** @brief VSM version 25.04 compatibility
     *  @details Base version for storage file format
     *  Versão base para formato de arquivo de storage */
    static const int V_2504 = 100;
    
    /** @brief VSM version 25.06 compatibility  
     *  @details Updated storage format with enhanced features
     *  Formato de storage atualizado com funcionalidades aprimoradas */
    static const int V_2506 = 101;
    
    /** @brief VSM version 14.09 compatibility
     *  @details Fix for new virtualization system implemented in experimental
     *  Correção para novo sistema de virtualização VPower implementado no experimental */
    static const int V_1409 = 102;

    /** @brief VSM version 05.10 compatibility
     *  @details Latest storage format with all current features
     *  Correções de estabilidade e consistência no gerenciamento do VPower */
    static const int V_0510 = 103;

    /** @brief Compatibility version for legacy support
     *  @details Use this version for backward compatibility with older storage files
     *  @note Use for most recent compatible version
     *  Versão de compatibilidade para suporte legado
     *  Use esta versão para compatibilidade com arquivos de storage mais antigos
     *  @note Utilize para a versão compatível mais recente */
    static const int COMPATIBILITY_VERSION = V_1409;
    
    /** @brief Current active storage version
     *  @details The version currently being used for new storage files
     *  A versão atualmente sendo usada para novos arquivos de storage */
    static const int CURRENT_VERSION = V_0510;
};

/**
 * @class VSM_ConfigVersion
 * @brief Version control for VSM configuration files
 * @details Manages versioning for VSM configuration and settings files
 *          Ensures proper configuration migration between VSM updates
 * 
 * Controle de versão para arquivos de configuração do VSM
 * Gerencia versionamento para arquivos de configuração e definições do VSM
 * Garante migração adequada de configuração entre atualizações do VSM
 */
class VSM_ConfigVersion
{
    /** @brief Configuration version 25.04
     *  @details Initial configuration file format
     *  Formato inicial de arquivo de configuração */
    static const int V_2504 = 100;
    
    /** @brief Configuration version 24.08
     *  @details Updated configuration with new settings
     *  Configuração atualizada com novas definições */
    static const int V_2408 = 101;
    
    /** @brief Configuration version 08.09
     *  @details Enhanced configuration structure
     *  Estrutura de configuração aprimorada */
    static const int V_0809 = 102;
    
    /** @brief Configuration version 14.09
     *  @details Latest configuration format with all features
     *  Formato de configuração mais recente com todas as funcionalidades */
    static const int V_1409 = 103;

    /** @brief Current configuration version in use
     *  @details The version currently being used for configuration files
     *  A versão atualmente sendo usada para arquivos de configuração */
    static const int CURRENT_VERSION = V_1409;
};

/**
 * @class VSM_ModuleVersion
 * @brief Version control for VSM module code
 * @details Manages versioning for the VSM module itself, including code changes
 *          Used for tracking module updates and ensuring compatibility
 * 
 * Controle de versão para código do módulo VSM
 * Gerencia versionamento para o próprio módulo VSM, incluindo mudanças de código
 * Usado para rastrear atualizações do módulo e garantir compatibilidade
 */
class VSM_ModuleVersion
{
    /** @brief Initial module version
     *  @details Base version of the VSM module
     *  Versão base do módulo VSM */
    static const int V_0000 = 100;
    
    /** @brief Module version 15.09
     *  @details Updated module with enhanced functionality
     *  Módulo atualizado com funcionalidade aprimorada */
    static const int V_1509 = 101;

    /** @brief Current module version in use
     *  @details The version currently active for the VSM module
     *  A versão atualmente ativa para o módulo VSM */
    static const int CURRENT_VERSION = V_1509;
};

//==============================================================================
//! SYSTEM CONSTANTS | CONSTANTES DO SISTEMA
//==============================================================================

/**
 * @class VSM_Constants
 * @brief Main constants for VSM system paths and configurations
 * @details Contains all file paths, folder names, and system constants used by VSM
 *          Centralizes path management and system configuration values
 * 
 * Constantes principais para caminhos e configurações do sistema VSM
 * Contém todos os caminhos de arquivo, nomes de pasta e constantes do sistema usadas pelo VSM
 * Centraliza gerenciamento de caminhos e valores de configuração do sistema
 */
class VSM_Constants
{
    /** @brief iTzMods profile folder name
     *  @details Base folder name for iTzMods in the server profile directory
     *  Nome da pasta base para iTzMods no diretório de perfil do servidor */
    static const string ITZ_PROFILE_FOLDER_NAME = "$profile:iTzMods";
    
    /** @brief General VSM path
     *  @details Main directory path for all VSM data and configuration files
     *  Caminho do diretório principal para todos os dados e arquivos de configuração do VSM */
    static const string GENERAL_PATH = ITZ_PROFILE_FOLDER_NAME + "\\VSM";

    /** @brief Settings file path
     *  @details Full path to the VSM settings configuration file
     *  Caminho completo para o arquivo de configuração de definições do VSM */
    static const string SETTING_FILE = GENERAL_PATH + "\\Settings.json";
    
    /** @brief Version file path
     *  @details Full path to the VSM version tracking file
     *  Caminho completo para o arquivo de rastreamento de versão do VSM */
    static const string VERSION_FILE = GENERAL_PATH + "\\Version.json";

    /** @brief Client mod display name
     *  @details Human-readable name for the VSM mod shown to users
     *  Nome legível do mod VSM mostrado aos usuários */
    static const string CLI_MOD_NAME = "Virtual Storage Module";

    /** @brief Metadata filename
     *  @details Standard filename for virtual storage metadata files
     *  Nome padrão do arquivo para arquivos de metadados de storage virtual */
    static const string METADATA_FILENAME = "meta.json";
    
    /** @brief Context filename
     *  @details Standard filename for virtual storage context files
     *  Nome padrão do arquivo para arquivos de contexto de storage virtual */
    static const string CTX_FILENAME = "ctx.vsm";
    
    /** @brief Virtual storage filename
     *  @details Standard filename for main virtual storage data files
     *  Nome padrão do arquivo para arquivos principais de dados de storage virtual */
    static const string VIRTUAL_FILENAME = "main.vsm";
}

//==============================================================================
//! ENUMERATIONS | ENUMERAÇÕES
//==============================================================================

/**
 * @enum VSM_RPCTypes
 * @brief Remote Procedure Call types for VSM network communication
 * @details Defines all RPC message types used for client-server communication in VSM
 *          Each type represents a specific network operation or synchronization event
 * 
 * Tipos de Remote Procedure Call para comunicação de rede do VSM
 * Define todos os tipos de mensagem RPC usados para comunicação cliente-servidor no VSM
 * Cada tipo representa uma operação de rede específica ou evento de sincronização
 */
enum VSM_RPCTypes
{
    /** @brief Invalid/uninitialized RPC type
     *  @details Used as a base value and error indicator
     *  Usado como valor base e indicador de erro */
    INVALID = 8830,
    
    /** @brief Synchronize settings between client and server
     *  @details RPC for sending settings updates from server to client
     *  RPC para envio de atualizações de configurações do servidor para cliente */
    SYNC_SETTINGS,
    
    /** @brief Close container menu context
     *  @details RPC for closing virtual storage container interfaces
     *  RPC para fechar interfaces de container de storage virtual */
    MENUCTX_CLOSE_CONTAINER,
    
    /** @brief Open container menu context
     *  @details RPC for opening virtual storage container interfaces
     *  RPC para abrir interfaces de container de storage virtual */
    MENUCTX_OPEN_CONTAINER,
    
    /** @brief Total count of RPC types
     *  @details Used for validation and array sizing
     *  Usado para validação e dimensionamento de arrays */
    COUNT
};

/**
 * @enum VSM_LogLevel
 * @brief Logging levels for VSM debug and error reporting
 * @details Defines severity levels for VSM logging system to control output verbosity
 *          Higher values indicate more detailed logging information
 * 
 * Níveis de logging para debug e relatório de erros do VSM
 * Define níveis de severidade para sistema de logging do VSM para controlar verbosidade de saída
 * Valores maiores indicam informações de logging mais detalhadas
 */
enum VSM_LogLevel
{
    /** @brief No logging output
     *  @details Disables all logging messages
     *  Desabilita todas as mensagens de logging */
    NONE = 0,
    
    /** @brief Critical errors only
     *  @details Only logs critical system failures
     *  Apenas registra falhas críticas do sistema */
    CRITICAL,
    
    /** @brief Error level logging
     *  @details Logs errors and critical issues
     *  Registra erros e problemas críticos */
    ERROR,
    
    /** @brief Warning level logging
     *  @details Logs warnings, errors, and critical issues
     *  Registra avisos, erros e problemas críticos */
    WARN,
    
    /** @brief Information level logging
     *  @details Logs general information and above
     *  Registra informações gerais e acima */
    INFO,
    
    /** @brief Debug level logging
     *  @details Logs debug information and above
     *  Registra informações de debug e acima */
    DEBUG,
    
    /** @brief Trace level logging
     *  @details Most verbose logging, includes all messages
     *  Logging mais verboso, inclui todas as mensagens */
    TRACE
};

/**
 * @enum VirtualStorageState
 * @brief States for virtual storage file operations
 * @details Represents the current state of virtual storage files for a specific container
 *          Used to track the lifecycle of virtualization and restoration processes
 * 
 * Estados para operações de arquivo de storage virtual
 * Representa o estado atual de arquivos de storage virtual para um container específico
 * Usado para rastrear o ciclo de vida de processos de virtualização e restauração
 */
enum VirtualStorageState
{
    /** @brief No virtual storage state
     *  @details Initial state or when virtual storage is not applicable
     *  Estado inicial ou quando storage virtual não é aplicável */
    NONE,
    
    /** @brief Storage has been restored
     *  @details Virtual items have been restored to physical form
     *  Itens virtuais foram restaurados para forma física */
    RESTORED,
    
    /** @brief Storage has been virtualized
     *  @details Physical items have been converted to virtual storage
     *  Itens físicos foram convertidos para storage virtual */
    VIRTUALIZED,
    
    /** @brief Currently restoring storage
     *  @details Storage is in the process of being restored from virtual to physical
     *  Storage está no processo de ser restaurado de virtual para físico */
    RESTORING,
    
    /** @brief Currently virtualizing storage
     *  @details Storage is in the process of being converted from physical to virtual
     *  Storage está no processo de ser convertido de físico para virtual */
    VIRTUALIZING
}

/**
 * @class VSM_HeaderType
 * @brief Header types for virtual storage file format
 * @details Defines string constants used as headers in virtual storage files
 *          These headers help parse and validate virtual storage file structure
 * 
 * Tipos de cabeçalho para formato de arquivo de storage virtual
 * Define constantes de string usadas como cabeçalhos em arquivos de storage virtual
 * Estes cabeçalhos ajudam a analisar e validar estrutura de arquivo de storage virtual
 */
class VSM_HeaderType
{
    /** @brief Storage initialization header
     *  @details Marks the beginning of a storage container section in virtual files
     *  Marca o início de uma seção de container de storage em arquivos virtuais */
    static const string STORAGE_INIT = "#S.INIT#";
    
    /** @brief Storage end-of-file header
     *  @details Marks the end of a storage container section in virtual files
     *  Marca o fim de uma seção de container de storage em arquivos virtuais */
    static const string STORAGE_EOF = "#S.EOF#";
    
    /** @brief Item initialization header
     *  @details Marks the beginning of an individual item section in virtual files
     *  Marca o início de uma seção de item individual em arquivos virtuais */
    static const string ITEM_INIT = "#I.INIT#";
    
    /** @brief Item end-of-file header
     *  @details Marks the end of an individual item section in virtual files
     *  Marca o fim de uma seção de item individual em arquivos virtuais */
    static const string ITEM_EOF = "#I.EOF#";
    
    /** @brief Component initialization header
     *  @details Marks the beginning of a component section in virtual files
     *  Marca o início de uma seção de componente em arquivos virtuais */
    static const string COMPONENT_INIT = "#C.INIT#";
    
    /** @brief Component end-of-file header
     *  @details Marks the end of a component section in virtual files
     *  Marca o fim de uma seção de componente em arquivos virtuais */
    static const string COMPONENT_EOF = "#C.EOF#";
}