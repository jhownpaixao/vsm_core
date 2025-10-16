@echo off
REM Script de conveniência para análise de arquivos VSM no Windows
REM Uso: analyze_vsm.bat <arquivo>

setlocal EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
set "PHP_READER=%SCRIPT_DIR%vsm_cli_reader.php"

if "%~1"=="" (
    echo 🔍 Analisador VSM - Script de Conveniência
    echo.
    echo Uso: %0 [opcoes] ^<arquivo.vsm^>
    echo.
    echo Opções:
    echo   -c    Análise compacta
    echo   -p    Apenas ponteiros  
    echo   -v    Modo verboso
    echo   -s    Apenas estrutura
    echo   -x    Dump hexadecimal
    echo.
    echo Exemplos:
    echo   %0 main.vsm                    # Análise completa
    echo   %0 -c main.vsm                 # Análise compacta
    echo   %0 -p main.vsm                 # Apenas ponteiros
    echo   %0 -v main.vsm                 # Modo verboso
    echo.
    goto :eof
)

REM Encontrar o arquivo (último argumento)
set "FILE_ARG="
for %%a in (%*) do set "FILE_ARG=%%a"

REM Verificar se o arquivo existe
if not exist "%FILE_ARG%" (
    echo ❌ Erro: Arquivo não encontrado: %FILE_ARG%
    exit /b 1
)

REM Verificar se o PHP reader existe
if not exist "%PHP_READER%" (
    echo ❌ Erro: Leitor PHP não encontrado: %PHP_READER%
    exit /b 1
)

echo 🔍 Analisando arquivo VSM: %~nxF%FILE_ARG%
for %%F in ("%FILE_ARG%") do echo 📁 Tamanho: %%~zF bytes
echo.

REM Executar análise
php "%PHP_READER%" %*

echo.
echo ✅ Análise concluída!

pause