class VSM_CfgIgnoreList : VSM_Base
{
    array<string> GetList()
    {
        array<string> ignoreList = new array<string>();
        string path = "$mission:cfgignorelist.xml";

        if (!FileExist(path))
        {
            VSM_Warn("GetList", "Arquivo de ignore não existe: " + path);
            return ignoreList;
        }

        FileHandle file = OpenFile(path, FileMode.READ);
        if (!file)
        {
            VSM_Warn("GetList", "Erro ao ler o arquivo de ignore: " + path);
            return ignoreList;
        }

        string line;
        while (FGets(file, line) > 0)
        {
            line.Trim();

            // Ignorar linhas comentadas
            if (line.Contains("<!--") || line.Contains("-->"))
                continue;

            if (line.Contains("<type") && line.Contains("name="))
            {
                string name = ExtractAttribute(line, "name");
                if (name != "")
                {
                    ignoreList.Insert(name);
                }
            }
        }

        CloseFile(file);
        return ignoreList;
    }

    string ExtractAttribute(string line, string attribute)
    {
        string pattern = attribute + "=\"";
        int start = line.IndexOf(pattern);
        if (start == -1) return "";

        start += pattern.Length(); // Correto no Enforce Script

        int end = -1;
        int lineLength = line.Length();

        for (int i = start; i < lineLength; i++)
        {
            if (line[i] == "\"") // Comparando corretamente com string
            {
                end = i;
                break;
            }
        }

        if (end == -1) return "";

        return line.Substring(start, end - start);
    }
};