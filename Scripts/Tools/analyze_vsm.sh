#!/bin/bash
# Script de conveniência para análise de arquivos VSM
# Uso: ./analyze_vsm.sh <arquivo>

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PHP_READER="$SCRIPT_DIR/vsm_cli_reader.php"

if [ $# -eq 0 ]; then
    echo "🔍 Analisador VSM - Script de Conveniência"
    echo ""
    echo "Uso: $0 <arquivo.vsm>"
    echo ""
    echo "Exemplos:"
    echo "  $0 main.vsm                    # Análise completa"
    echo "  $0 -c main.vsm                 # Análise compacta"
    echo "  $0 -p main.vsm                 # Apenas ponteiros"
    echo "  $0 -v main.vsm                 # Modo verboso"
    echo ""
    exit 1
fi

# Verificar se o arquivo existe
if [ ! -f "$1" ]; then
    echo "❌ Erro: Arquivo não encontrado: $1"
    exit 1
fi

# Verificar se o PHP reader existe
if [ ! -f "$PHP_READER" ]; then
    echo "❌ Erro: Leitor PHP não encontrado: $PHP_READER"
    exit 1
fi

echo "🔍 Analisando arquivo VSM: $(basename "$1")"
echo "📁 Tamanho: $(ls -lh "$1" | awk '{print $5}')"
echo ""

# Executar análise
php "$PHP_READER" "$@"

echo ""
echo "✅ Análise concluída!"