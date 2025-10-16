# VSM CLI Reader - Guia de Uso Prático

## 🚀 Instalação Rápida

1. Certifique-se que o PHP está instalado e no PATH
2. Coloque os arquivos na pasta Tools do seu projeto VSM
3. Use diretamente via linha de comando

## 📋 Comandos Essenciais

### Análise Básica
```bash
php vsm_cli_reader.php main.vsm
```

### Análise Compacta (Recomendado para arquivos grandes)
```bash
php vsm_cli_reader.php -c main.vsm
```

### Focar Apenas em Ponteiros (Para Debug)
```bash
php vsm_cli_reader.php -p main.vsm
```

### Análise Verbosa (Máximo de detalhes)
```bash
php vsm_cli_reader.php -v main.vsm
```

### Combinações Úteis
```bash
# Ponteiros em modo verboso
php vsm_cli_reader.php -v -p main.vsm

# Estrutura compacta
php vsm_cli_reader.php -c -s main.vsm

# Dump hexadecimal completo
php vsm_cli_reader.php -x main.vsm
```

## 🎯 Para Seu Caso Específico (TF_6B43MagPouch com FreezeBar_Eridium)

### Comando Recomendado:
```bash
php vsm_cli_reader.php -v -p "C:\Program Files (x86)\Steam\steamapps\common\DayZServer\mpmissions\dayzBlackoutOffline.chernarusplus\storage_1\virtual\LargeTentAutumn_166654736473374384032381831913736621\main.vsm"
```

### O que procurar na saída:

1. **Versão**: Deve mostrar `103 (0x67) [CUSTOM]`
2. **Headers**: Deve encontrar `#C.INIT#` e `#C.EOF#` (componentes)
3. **Classes**: 
   - `TF_6B43MagPouch`
   - `Ammo_545x39Tracer`
   - `Bullet_545x39Tracero`
4. **Ponteiros Importantes**:
   - `QUANTITY`: 23@0x4, 25@0x384, 66@0x3b0
   - `COORD/TYPE`: valores 0-3 para posições de inventário
   - `ID`: valores como 4864 para identificadores

## 🔧 Interpretação dos Resultados

### Tipos de Valores:
- **VSM_CUSTOM**: Versão 103 (não padrão)
- **QUANTITY**: Quantidades de munição/itens
- **COORD/TYPE**: Posições no inventário (0=ATTACHMENT, 1=CARGO, 2=PROXYCARGO)
- **ID**: Identificadores únicos
- **POINTER**: Endereços de memória/referências

### Headers Encontrados:
- **#C.INIT#** / **#C.EOF#**: Delimitadores de componentes
- **#I.INIT#** / **#I.EOF#**: Delimitadores de itens (ausentes no seu arquivo)

### Classes Identificadas:
- **TF_6B43MagPouch**: Container principal
- **Ammo_545x39Tracer**: Munição tipo tracer
- **Bullet_545x39Tracero**: Balas individuais

## ⚠️ Problemas Identificados no Seu Arquivo

1. **Versão não padrão (103)**: Pode causar incompatibilidade
2. **Falta de headers de item**: Não há `#I.INIT#`/`#I.EOF#`
3. **Estrutura de componentes apenas**: Apenas `#C.INIT#`/`#C.EOF#`

## 🛠️ Scripts de Conveniência

### Windows:
```batch
analyze_vsm.bat -c main.vsm
```

### Linux/Mac:
```bash
./analyze_vsm.sh -c main.vsm
```

## 📊 Exemplos de Saída

### Saída Compacta:
```
Versão: 103 (0x67) [CUSTOM]
#C.INIT#@0xac #C.EOF#@0xc0
TF_6B43MagPouch Ammo_545x39Tracer Bullet_545x39Tracero
QUANTITY: 23@0x4 25@0x384 66@0x3b0
```

### Saída Verbosa:
```
📊 0x00000000: 103      VSM_CUSTOM
📊 0x00000004: 23       QUANTITY
📊 0x00000094: 3        COORD/TYPE
📊 0x000000A4: 1        COORD/TYPE
```

## 🚨 Troubleshooting

### Erro "arquivo não encontrado":
- Verifique o caminho completo
- Use aspas duplas para caminhos com espaços

### Erro "PHP não encontrado":
- Instale PHP e adicione ao PATH
- Ou use caminho completo: `C:\php\php.exe vsm_cli_reader.php`

### Saída truncada:
- Use `-c` para saída compacta
- Redirecione para arquivo: `> output.txt`

## 💡 Dicas Avançadas

1. **Redirecionar saída para arquivo**:
   ```bash
   php vsm_cli_reader.php -v main.vsm > analysis.txt
   ```

2. **Comparar dois arquivos**:
   ```bash
   php vsm_cli_reader.php -c file1.vsm > out1.txt
   php vsm_cli_reader.php -c file2.vsm > out2.txt
   diff out1.txt out2.txt
   ```

3. **Procurar por padrões específicos**:
   ```bash
   php vsm_cli_reader.php -v main.vsm | grep "QUANTITY"
   ```