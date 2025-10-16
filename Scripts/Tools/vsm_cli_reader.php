#!/usr/bin/env php
<?php
/**
 * VSM Binary Reader CLI
 * Leitor de arquivos binários VSM otimizado para linha de comando
 * 
 * Uso: php vsm_cli_reader.php [opções] <arquivo>
 * 
 * Opções:
 *   -h, --help        Mostrar ajuda
 *   -v, --verbose     Modo verboso
 *   -x, --hex         Mostrar dump hexadecimal completo
 *   -s, --structure   Apenas análise de estrutura
 *   -p, --pointers    Focar em ponteiros e valores
 *   -c, --compact     Saída compacta
 */

class VSMCLIReader {
    private $verbose = false;
    private $showHex = false;
    private $structureOnly = false;
    private $pointersOnly = false;
    private $compact = false;
    private $filePath = '';
    private $fileData = '';
    private $fileSize = 0;
    
    public function __construct() {
        // Configurar para CLI
        if (php_sapi_name() !== 'cli') {
            die("Este script deve ser executado via linha de comando.\n");
        }
    }
    
    public function run($argc, $argv) {
        if (!$this->parseArguments($argc, $argv)) {
            return 1;
        }
        
        if (!$this->loadFile($this->filePath)) {
            return 1;
        }
        
        $this->analyze();
        return 0;
    }
    
    private function parseArguments($argc, $argv) {
        if ($argc < 2) {
            $this->showHelp();
            return false;
        }
        
        for ($i = 1; $i < $argc; $i++) {
            $arg = $argv[$i];
            
            switch ($arg) {
                case '-h':
                case '--help':
                    $this->showHelp();
                    return false;
                    
                case '-v':
                case '--verbose':
                    $this->verbose = true;
                    break;
                    
                case '-x':
                case '--hex':
                    $this->showHex = true;
                    break;
                    
                case '-s':
                case '--structure':
                    $this->structureOnly = true;
                    break;
                    
                case '-p':
                case '--pointers':
                    $this->pointersOnly = true;
                    break;
                    
                case '-c':
                case '--compact':
                    $this->compact = true;
                    break;
                    
                default:
                    if (empty($this->filePath)) {
                        $this->filePath = $arg;
                    } else {
                        echo "Erro: Arquivo já especificado.\n";
                        return false;
                    }
                    break;
            }
        }
        
        if (empty($this->filePath)) {
            echo "Erro: Nenhum arquivo especificado.\n";
            $this->showHelp();
            return false;
        }
        
        return true;
    }
    
    private function showHelp() {
        echo "VSM Binary Reader CLI v1.0\n";
        echo "Leitor de arquivos binários VSM para linha de comando\n\n";
        echo "Uso: php " . basename(__FILE__) . " [opções] <arquivo>\n\n";
        echo "Opções:\n";
        echo "  -h, --help        Mostrar esta ajuda\n";
        echo "  -v, --verbose     Modo verboso (mais detalhes)\n";
        echo "  -x, --hex         Mostrar dump hexadecimal completo\n";
        echo "  -s, --structure   Apenas análise de estrutura VSM\n";
        echo "  -p, --pointers    Focar em ponteiros e valores numéricos\n";
        echo "  -c, --compact     Saída compacta (menos verbose)\n\n";
        echo "Exemplos:\n";
        echo "  php " . basename(__FILE__) . " main.vsm\n";
        echo "  php " . basename(__FILE__) . " -v -p storage.bin\n";
        echo "  php " . basename(__FILE__) . " --hex --structure file.vsm\n\n";
    }
    
    private function loadFile($filePath) {
        if (!file_exists($filePath)) {
            echo "❌ Erro: Arquivo não encontrado: $filePath\n";
            return false;
        }
        
        $this->fileData = file_get_contents($filePath);
        if ($this->fileData === false) {
            echo "❌ Erro: Não foi possível ler o arquivo: $filePath\n";
            return false;
        }
        
        $this->fileSize = strlen($this->fileData);
        
        if (!$this->compact) {
            echo "📁 Arquivo: " . basename($filePath) . "\n";
            echo "📏 Tamanho: {$this->fileSize} bytes (" . $this->formatBytes($this->fileSize) . ")\n";
            echo "🕒 Modificado: " . date('Y-m-d H:i:s', filemtime($filePath)) . "\n";
            echo str_repeat('─', 60) . "\n";
        }
        
        return true;
    }
    
    private function analyze() {
        if ($this->showHex && !$this->structureOnly && !$this->pointersOnly) {
            $this->displayHexDump();
        }
        
        if (!$this->showHex || $this->structureOnly || $this->pointersOnly) {
            $this->analyzeStructure();
        }
    }
    
    private function displayHexDump() {
        if (!$this->compact) {
            echo "🔍 DUMP HEXADECIMAL\n";
            echo str_repeat('─', 60) . "\n";
        }
        
        $bytesPerLine = $this->compact ? 32 : 16;
        $totalLines = ceil($this->fileSize / $bytesPerLine);
        
        for ($line = 0; $line < $totalLines; $line++) {
            $offset = $line * $bytesPerLine;
            $lineData = substr($this->fileData, $offset, $bytesPerLine);
            
            $hexPart = '';
            $asciiPart = '';
            
            for ($i = 0; $i < strlen($lineData); $i++) {
                $byte = ord($lineData[$i]);
                $hexPart .= sprintf('%02X ', $byte);
                $asciiPart .= ($byte >= 32 && $byte <= 126) ? chr($byte) : '.';
            }
            
            $hexPart = str_pad($hexPart, $bytesPerLine * 3, ' ');
            
            if ($this->compact) {
                printf("%08X: %s\n", $offset, trim($hexPart));
            } else {
                printf("%08X: %s | %s\n", $offset, $hexPart, $asciiPart);
            }
        }
        
        if (!$this->compact) {
            echo str_repeat('─', 60) . "\n";
        }
    }
    
    private function analyzeStructure() {
        if (!$this->pointersOnly) {
            $this->analyzeVersion();
            $this->findHeaders();
            $this->findClassNames();
        }
        
        if (!$this->structureOnly) {
            $this->analyzePointers();
            $this->analyzeInventoryStructure();
        }
        
        if ($this->verbose) {
            $this->analyzeDataPatterns();
        }
    }
    
    private function analyzeVersion() {
        if (!$this->compact) {
            echo "🔢 ANÁLISE DE VERSÃO\n";
            echo str_repeat('─', 30) . "\n";
        }
        
        if ($this->fileSize >= 4) {
            $version = unpack('V', substr($this->fileData, 0, 4))[1];
            
            if ($this->compact) {
                echo "Versão: $version (0x" . dechex($version) . ")";
            } else {
                echo "📋 Versão detectada: $version (0x" . dechex($version) . ")\n";
            }
            
            switch ($version) {
                case 1409:
                    echo $this->compact ? " [V_1409]" : "   ✅ VSM_StorageVersion.V_1409\n";
                    break;
                case 510:
                    echo $this->compact ? " [V_0510]" : "   ✅ VSM_StorageVersion.V_0510\n";
                    break;
                case 103:
                    echo $this->compact ? " [CUSTOM]" : "   ⚠️  Versão customizada/não padrão\n";
                    break;
                default:
                    echo $this->compact ? " [UNKNOWN]" : "   ❓ Versão desconhecida\n";
                    break;
            }
            
            if ($this->compact) {
                echo "\n";
            }
        }
        
        if (!$this->compact) {
            echo "\n";
        }
    }
    
    private function findHeaders() {
        if (!$this->compact) {
            echo "🏷️  HEADERS VSM\n";
            echo str_repeat('─', 30) . "\n";
        }
        
        $headers = [
            '#S.INIT#' => 'Storage Init',
            '#S.EOF#'  => 'Storage End',
            '#I.INIT#' => 'Item Init',
            '#I.EOF#'  => 'Item End',
            '#C.INIT#' => 'Component Init',
            '#C.EOF#'  => 'Component End'
        ];
        
        $found = 0;
        foreach ($headers as $header => $desc) {
            $pos = strpos($this->fileData, $header);
            if ($pos !== false) {
                $found++;
                if ($this->compact) {
                    echo "$header@0x" . dechex($pos) . " ";
                } else {
                    echo sprintf("📍 %-12s na posição 0x%08X (%d)\n", $header, $pos, $pos);
                    if ($this->verbose) {
                        $this->showContext($pos, strlen($header));
                    }
                }
            }
        }
        
        if ($this->compact && $found > 0) {
            echo "\n";
        } elseif (!$this->compact) {
            if ($found == 0) {
                echo "❌ Nenhum header VSM encontrado\n";
            }
            echo "\n";
        }
    }
    
    private function findClassNames() {
        if (!$this->compact) {
            echo "🎯 CLASSES IDENTIFICADAS\n";
            echo str_repeat('─', 30) . "\n";
        }
        
        $classes = [];
        $pos = 0;
        
        // Procurar por strings que parecem nomes de classe
        while ($pos < $this->fileSize - 10) {
            if (ord($this->fileData[$pos]) >= 65 && ord($this->fileData[$pos]) <= 90) {
                $className = '';
                $start = $pos;
                
                while ($pos < $this->fileSize) {
                    $char = ord($this->fileData[$pos]);
                    if (($char >= 65 && $char <= 90) || ($char >= 97 && $char <= 122) || 
                        ($char >= 48 && $char <= 57) || $char == 95) {
                        $className .= chr($char);
                        $pos++;
                    } else {
                        break;
                    }
                }
                
                if (strlen($className) > 5 && $this->isValidClassName($className)) {
                    if (!in_array($className, $classes)) {
                        $classes[] = $className;
                        if ($this->compact) {
                            echo "$className ";
                        } else {
                            echo sprintf("🔸 %-30s @0x%08X\n", $className, $start);
                        }
                    }
                }
            } else {
                $pos++;
            }
        }
        
        if ($this->compact && !empty($classes)) {
            echo "\n";
        } elseif (!$this->compact && empty($classes)) {
            echo "❌ Nenhuma classe identificada\n";
        }
        
        if (!$this->compact) {
            echo "\n";
        }
    }
    
    private function analyzePointers() {
        if (!$this->compact) {
            echo "🎯 PONTEIROS E VALORES\n";
            echo str_repeat('─', 30) . "\n";
        }
        
        $pointers = [];
        
        // Analisar valores int32 em posições alinhadas
        for ($i = 0; $i <= $this->fileSize - 4; $i += 4) {
            $value = unpack('V', substr($this->fileData, $i, 4))[1];
            
            $type = $this->classifyValue($value);
            if ($type) {
                $pointers[] = [
                    'pos' => $i,
                    'value' => $value,
                    'type' => $type
                ];
            }
        }
        
        // Agrupar por tipo se compacto
        if ($this->compact) {
            $grouped = [];
            foreach ($pointers as $p) {
                $grouped[$p['type']][] = $p;
            }
            
            foreach ($grouped as $type => $items) {
                echo "$type: ";
                foreach (array_slice($items, 0, 5) as $item) { // Máximo 5 por tipo
                    echo $item['value'] . "@0x" . dechex($item['pos']) . " ";
                }
                if (count($items) > 5) {
                    echo "... (+" . (count($items) - 5) . " mais)";
                }
                echo "\n";
            }
        } else {
            foreach ($pointers as $p) {
                echo sprintf("📊 0x%08X: %-8d %s\n", $p['pos'], $p['value'], $p['type']);
            }
        }
        
        if (!$this->compact) {
            echo "\n";
        }
    }
    
    private function analyzeInventoryStructure() {
        if (!$this->compact) {
            echo "🎒 ESTRUTURA DE INVENTÁRIO\n";
            echo str_repeat('─', 30) . "\n";
        }
        
        $items = 0;
        $pos = 0;
        
        while ($pos < $this->fileSize - 36) {
            $version = unpack('V', substr($this->fileData, $pos, 4))[1];
            
            if ($version == 1409 || $version == 510 || $version == 103) {
                $className = $this->extractString($pos + 4, 32);
                
                if (!empty($className) && $this->isValidClassName($className)) {
                    $items++;
                    
                    if ($this->compact) {
                        echo "Item$items:$className ";
                    } else {
                        echo "🎁 Item #$items: $className\n";
                        
                        if ($this->verbose) {
                            // Tentar extrair dados de inventário
                            $invPos = $pos + 68; // Pular version + classname + type
                            if ($invPos + 20 < $this->fileSize) {
                                $row = unpack('V', substr($this->fileData, $invPos, 4))[1];
                                $col = unpack('V', substr($this->fileData, $invPos + 4, 4))[1];
                                $type = unpack('V', substr($this->fileData, $invPos + 8, 4))[1];
                                $slotId = unpack('V', substr($this->fileData, $invPos + 16, 4))[1];
                                
                                $typeNames = [0 => 'ATTACHMENT', 1 => 'CARGO', 2 => 'PROXYCARGO'];
                                $typeName = $typeNames[$type] ?? "UNKNOWN($type)";
                                
                                echo "   📍 Posição: Row=$row, Col=$col, Type=$typeName, Slot=$slotId\n";
                            }
                        }
                    }
                    
                    $pos += 100;
                } else {
                    $pos += 4;
                }
            } else {
                $pos += 4;
            }
        }
        
        if ($this->compact && $items > 0) {
            echo "\n";
        } elseif (!$this->compact) {
            if ($items == 0) {
                echo "❌ Nenhum item de inventário encontrado\n";
            } else {
                echo "📈 Total: $items itens encontrados\n";
            }
            echo "\n";
        }
    }
    
    private function analyzeDataPatterns() {
        echo "🔍 PADRÕES DE DADOS (VERBOSE)\n";
        echo str_repeat('─', 30) . "\n";
        
        // Analisar repetições
        $this->findRepeatingPatterns();
        
        // Analisar strings
        $this->findAllStrings();
        
        echo "\n";
    }
    
    private function findRepeatingPatterns() {
        echo "🔄 Padrões repetitivos:\n";
        
        $patterns = [];
        for ($len = 4; $len <= 16; $len += 4) {
            for ($i = 0; $i <= $this->fileSize - $len * 2; $i++) {
                $pattern = substr($this->fileData, $i, $len);
                $next = substr($this->fileData, $i + $len, $len);
                
                if ($pattern === $next) {
                    $hex = bin2hex($pattern);
                    if (!isset($patterns[$hex])) {
                        $patterns[$hex] = ['count' => 1, 'pos' => $i, 'len' => $len];
                    }
                    $patterns[$hex]['count']++;
                }
            }
        }
        
        foreach (array_slice($patterns, 0, 5) as $hex => $info) {
            if ($info['count'] > 2) {
                echo "   🔁 $hex (${info['len']} bytes) repetido ${info['count']}x a partir de 0x" . dechex($info['pos']) . "\n";
            }
        }
    }
    
    private function findAllStrings() {
        echo "📝 Strings encontradas:\n";
        
        $strings = [];
        $current = '';
        $start = 0;
        
        for ($i = 0; $i < $this->fileSize; $i++) {
            $char = ord($this->fileData[$i]);
            
            if ($char >= 32 && $char <= 126) {
                if (empty($current)) {
                    $start = $i;
                }
                $current .= chr($char);
            } else {
                if (strlen($current) >= 4) {
                    $strings[] = ['str' => $current, 'pos' => $start];
                }
                $current = '';
            }
        }
        
        foreach (array_slice($strings, 0, 10) as $str) {
            echo "   📄 '" . substr($str['str'], 0, 30) . "' @0x" . dechex($str['pos']) . "\n";
        }
    }
    
    // Métodos auxiliares
    private function classifyValue($value) {
        if ($value == 1409) return "VSM_V1409";
        if ($value == 510) return "VSM_V510";
        if ($value == 103) return "VSM_CUSTOM";
        if ($value >= 0 && $value <= 10) return "COORD/TYPE";
        if ($value >= 20 && $value <= 100) return "QUANTITY";
        if ($value >= 1000 && $value <= 9999) return "ID";
        if ($value > 100000) return "POINTER";
        return null;
    }
    
    private function isValidClassName($name) {
        $prefixes = ['TF_', 'Ammo_', 'Weapon_', 'Container_', 'Item_', 'FreezeBar_', 'Bullet_'];
        foreach ($prefixes as $prefix) {
            if (strpos($name, $prefix) === 0) return true;
        }
        return strlen($name) > 8 && (strpos($name, '_') !== false || preg_match('/\d/', $name));
    }
    
    private function extractString($pos, $maxLen) {
        $str = '';
        for ($i = 0; $i < $maxLen && $pos + $i < $this->fileSize; $i++) {
            $char = ord($this->fileData[$pos + $i]);
            if ($char == 0) break;
            if ($char >= 32 && $char <= 126) {
                $str .= chr($char);
            } else {
                break;
            }
        }
        return $str;
    }
    
    private function showContext($pos, $len) {
        $start = max(0, $pos - 8);
        $end = min($this->fileSize, $pos + $len + 8);
        $context = substr($this->fileData, $start, $end - $start);
        echo "      Contexto: " . bin2hex($context) . "\n";
    }
    
    private function formatBytes($bytes) {
        $units = ['B', 'KB', 'MB'];
        $factor = 0;
        while ($bytes > 1024 && $factor < 2) {
            $bytes /= 1024;
            $factor++;
        }
        return number_format($bytes, 1) . ' ' . $units[$factor];
    }
}

// Executar se chamado diretamente
if (php_sapi_name() === 'cli') {
    $reader = new VSMCLIReader();
    exit($reader->run($argc, $argv));
}
?>