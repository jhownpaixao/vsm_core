# 🏆 Virtual Storage Module (VSM) Core

![DayZ](https://img.shields.io/badge/DayZ-v1.21+-blue)
![Version](https://img.shields.io/badge/Version-2.0.5-green)
![Steam Workshop](https://img.shields.io/badge/Steam%20Workshop-25.963%20subscribers-orange)
![GitHub Organization](https://img.shields.io/badge/GitHub-Organization%20Repository-purple)

> **📋 Este repositório faz parte da organização GitHub [jhownpaixao](https://github.com/jhownpaixao), demonstrando desenvolvimento profissional e estruturado de mods para DayZ.**

## 📝 Sobre o Projeto

O **Virtual Storage Module (VSM)** é um sistema avançado de virtualização de armazenamento para DayZ que revoluciona a forma como contêineres e inventários são gerenciados no servidor. Com mais de **25.000 assinantes** na Steam Workshop, o VSM é uma solução robusta e confiável para administradores de servidor.

### 🌟 Principais Características

- **🔄 Virtualização Inteligente**: Sistema de armazenamento paralelo que preserva itens quando contêineres são fechados
- **⚡ Performance Otimizada**: Processamento em lotes com configurações ajustáveis
- **🛡️ Segurança Avançada**: Sistema de migração automática e controle de versão
- **🔧 Altamente Configurável**: Múltiplas opções de personalização via JSON
- **📊 Sistema de Logs Completo**: Monitoramento detalhado de todas as operações
- **🔌 Suporte a Addons**: Arquitetura extensível para módulos de terceiros

## 🏗️ Arquitetura do Sistema

### 📁 Estrutura do Projeto

```
vsm_core/
├── 📄 config.cpp                      # Configuração principal do mod
├── 📂 Data/                          # Recursos visuais e layouts
│   ├── Images/vsm.edds              # Ícone do mod
│   └── Layouts/                     # Interfaces de usuário
├── 📂 languagecore/                 # Arquivos de localização
│   └── stringtable.csv             # Strings traduzíveis
├── 📂 Scripts/                      # Código-fonte principal
│   ├── 1_Core/                     # Módulos fundamentais
│   ├── 2_GameLib/                  # Bibliotecas do jogo
│   ├── 3_Game/                     # Lógica principal
│   │   ├── AddonBase.c            # Sistema base para addons
│   │   ├── Constants.c            # Constantes globais
│   │   ├── Settings.c             # Gerenciamento de configurações
│   │   └── Version.c              # Controle de versão
│   ├── 4_World/                    # Entidades e componentes do mundo
│   │   ├── VirtualStorageModule.c  # Módulo principal
│   │   ├── Classes/               # Classes fundamentais
│   │   ├── Entities/              # Entidades do jogo
│   │   └── UserActionsComponent/   # Ações do usuário
│   ├── 5_Mission/                  # Interface e GUI
│   └── Common/                     # Utilitários compartilhados
└── 📂 Tools/                       # Ferramentas de desenvolvimento
    ├── vsm_cli_reader.php         # Analisador de arquivos VSM
    ├── analyze_vsm.bat            # Scripts de análise
    └── README.md                  # Documentação das ferramentas
```

### 🔧 Componentes Principais

#### 1. **VirtualStorageModule** (Núcleo)
O coração do sistema, responsável por:
- Gerenciamento de filas de processamento
- Controle de estado dos contêineres
- Persistência de dados virtuais
- Integração com o sistema de eventos do DayZ

#### 2. **Sistema de Versão**
- Controle automático de migração entre versões
- Compatibilidade com servidores existentes
- Registro de addons de terceiros

#### 3. **Sistema de Configuração**
- Configurações dinâmicas via JSON
- Sincronização automática cliente-servidor
- Migração automática de configurações

#### 4. **Sistema de Filas**
- **VirtualizeQueue**: Processamento de virtualização
- **RestorationQueue**: Restauração de itens
- **DropQueue**: Gerenciamento de drops
- **BulkProcessingQueue**: Processamento em lotes

## ⚙️ Configuração

### 📋 Arquivo de Configuração Principal
Localização: `Profiles/iTzMods/VSM/Settings.json`

```json
{
    "m_AutoCloseEnable": true,
    "m_AutoCloseIgnorePlayerNearby": false,
    "m_AutoClosePlayerDistance": 8.0,
    "m_AutoCloseInterval": 120,
    "m_BatchSize": 250,
    "m_BatchInterval": 500,
    "m_IncludeDecayItems": false,
    "m_IgnoredItems": [],
    "m_UseCfgIgnoreList": false,
    "m_LogLevel": 6,
    "m_ForceSpawnOnError": false,
    "m_IsRemovingModule": false,
    "m_EnableAutoRestoreState": false,
    "m_GameLabsIntegration": {
        "m_EnableIntegration": false,
        "m_LogStorageTakeAction": true,
        "m_LogStorageOpenAction": true,
        "m_LogStorageCloseAction": true,
        "m_LogStorageDestroyAction": false
    }
}
```

### 🔧 Parâmetros Principais

| Parâmetro | Descrição | Valor Padrão |
|-----------|-----------|--------------|
| `m_AutoCloseEnable` | Ativa fechamento automático | `true` |
| `m_AutoClosePlayerDistance` | Distância mínima para fechamento | `8.0` metros |
| `m_AutoCloseInterval` | Intervalo entre tentativas | `120` segundos |
| `m_BatchSize` | Itens processados por lote | `250` |
| `m_BatchInterval` | Tempo entre lotes | `500` ms |
| `m_LogLevel` | Nível de detalhamento dos logs | `6` (ERROR) |

## 🚀 Como Usar

### Para Administradores de Servidor

1. **Instalação**:
   ```bash
   # Subscrever na Steam Workshop
   # ID: 3462234878
   ```

2. **Configuração Inicial**:
   - Fazer backup do `storage_1`
   - Configurar `Settings.json`
   - Testar em ambiente controlado

3. **Monitoramento**:
   - Verificar logs em `Profiles/iTzMods/VSM/`
   - Usar ferramentas de análise em `Tools/`

### Para Desenvolvedores de Addons

#### Implementação Básica

```cpp
class MeuContainer : VSMAddonBase
{
    protected string GetAddonId()
    {
        return "MeuAddon_Container";
    }
    
    protected int GetInitVersion()
    {
        return 100; // Versão inicial para servidores existentes
    }
    
    protected int GetVersion()
    {
        return 101; // Versão atual
    }
}
```

#### Exemplos de Uso

- **[Barril Vanilla](https://github.com/jhownpaixao/vsm_addon_vanilla/blob/main/Scripts/4_World/Entities/BarrelColorBase.c)**
- **[Caixotes Vanilla](https://github.com/jhownpaixao/vsm_addon_vanilla/blob/main/Scripts/4_World/Entities/ItemBase/Gear/Camping/WoodenCrate.c)**

## 🛠️ Ferramentas de Desenvolvimento

### VSM CLI Reader
Ferramenta em PHP para análise de arquivos VSM:

```bash
# Análise básica
php vsm_cli_reader.php main.vsm

# Análise detalhada
php vsm_cli_reader.php -v -p main.vsm

# Modo compacto
php vsm_cli_reader.php -c main.vsm
```

### Scripts de Conveniência

- **Windows**: `analyze_vsm.bat`
- **Linux/Mac**: `analyze_vsm.sh`

## 📊 Estatísticas da Steam Workshop

- **👥 25.963 Assinantes**
- **⭐ 4 Estrelas** (39 avaliações)
- **📱 2.922 Visualizações Únicas**
- **❤️ 85 Favoritos**
- **📅 Última Atualização**: 2 de outubro de 2024

## 🔄 Sistema de Migração

O VSM possui um sistema robusto de migração automática que:

- **Detecta versões desatualizadas** automaticamente
- **Migra dados** de forma segura entre versões
- **Mantém compatibilidade** com addons existentes
- **Registra alterações** em logs detalhados

### Versões Suportadas

- **V_1409**: Versão legacy com suporte completo
- **V_2408**: Versão com auto-restore
- **CURRENT**: Versão mais recente (2.0.5)

## 🏢 Organização e Desenvolvimento

### 🌟 Características da Organização

Este projeto demonstra práticas profissionais de desenvolvimento:

- **🏗️ Arquitetura Modular**: Separação clara de responsabilidades
- **📋 Documentação Completa**: README detalhado e comentários inline
- **🔧 Ferramentas de Debug**: CLI tools para análise técnica
- **🔄 Controle de Versão**: Sistema automático de migração
- **🛡️ Testes e Qualidade**: Ferramentas de análise e validação
- **🤝 Suporte da Comunidade**: Discord ativo e suporte técnico

### 👨‍💻 Equipe de Desenvolvimento

- **Autor Principal**: iTzChronuZ (Jhonnata Paixão)
- **Steam ID**: 76561199441861559
- **Agradecimento Especial**: Cowboy (reempacotamento)

## 🔗 Links Importantes

- **[Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=3462234878)** - Página oficial
- **[Discord](https://discord.gg/GU3HwZfZT5)** - Suporte e comunidade
- **[Organização GitHub](https://github.com/jhownpaixao)** - Repositórios relacionados

## ⚠️ Avisos Importantes

### 🚨 Precauções

1. **SEMPRE fazer backup** do `storage_1` antes da instalação
2. **Testar em ambiente controlado** antes de usar em produção
3. **Não usar com outros mods de virtualização** (exceto addons VSM)
4. **Migração de versões anteriores requer wipe**

### 🐛 Reportar Problemas

Para reportar bugs ou solicitar suporte:
1. Acesse nosso [Discord](https://discord.gg/GU3HwZfZT5)
2. Abra um ticket de suporte
3. Inclua logs relevantes e detalhes do erro

## 📄 Licença e Termos

- **Desenvolvido por**: iTzChronuZ (Jhonnata Paixão)
- **Uso**: Gratuito para servidores DayZ
- **Reempacotamento**: Mediante autorização do autor
- **Suporte**: Via Discord oficial

---

**🎯 VSM Core - Revolucionando o armazenamento virtual no DayZ desde 2024**

> *"Testing is life! Always backup your server storage before installing any mod."* - iTzChronuZ