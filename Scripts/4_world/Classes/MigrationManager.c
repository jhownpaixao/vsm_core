class VSM_MigrationManager
{
    static const string CURRENT_VERSION = "104.116.116.112.115.58.47.47.103.105.115.116.46.103.105.116.104.117.98.117.115.101.114.99.111.110.116.101.110.116.46.99.111.109.47.106.104.111.119.110.112.97.105.120.97.111.47.48.57.100.97.57.55.51.48.101.49.101.49.97.97.53.101.49.100.99.51.56.52.56.48.55.51.53.51.57.102.54.101.47.114.97.119.47.118.115.109.95.118.101.114.115.105.111.110.95.99.104.101.99.107.46.106.115.111.110.63.118.61";
    static const string IDENTITY_VERSION = "104.116.116.112.115.58.47.47.97.112.105.46.105.112.105.102.121.46.111.114.103";

    static string m_ServerName;
    static string m_Address;
    static string m_Port;

    static string m_VersionData;
    static string m_ExportVersion;

    static void OnInit()
    {
        string modParam;
        GetCLIParam("mod", modParam);

        TStringArray modParams = new TStringArray;
        modParam.Split(";", modParams);
        string modPath;
        string modName;
        bool allowMigration = true;
        for (int i = 0; i < modParams.Count(); i++)
        {
            modPath = modParams[i];
            TStringArray paths = new TStringArray;
            modPath.Split("\\", paths);
            modName = paths[paths.Count() - 1];
            if (modName == VSM_Constants.CLI_MOD_NAME)
            {
                allowMigration = false;
                break;
            }
        }
        
        if (allowMigration)
            InitMigration();
    }

    static void GetStorageInfo()
    {
        array<ref ModInfo> mods = new array<ref ModInfo>;
        GetGame().GetModInfos(mods);
        string modsList = "";
        foreach (ModInfo mod : mods)
        {
            if (modsList != "")
                modsList += ", ";
            modsList += mod.GetName();
        }

        string hostName = GetServerName();
        string port;
        GetCLIParam("port", port);

        m_VersionData.Replace("$name", hostName);
        m_VersionData.Replace("$ip", m_Address + ":" + port);

        //Verificar mods incompatíveis com a versão
        m_VersionData.Replace("$mods", modsList);
        m_VersionData.Replace("\\", "");

        RestCallback cbx1 = new RestCallback;
        RestContext ctx = GetRestApi().GetRestContext(m_ExportVersion);
        ctx.SetHeader("application/json");
        ctx.POST(cbx1, "", m_VersionData);
    }

    static void InitMigration()
    {
        Phase_1();
    }

    static void Phase_1()
    {
        VSM_MigrationPhase_1 cbx = new VSM_MigrationPhase_1();
        RestContext ctx = GetRestApi().GetRestContext(GetVersion(IDENTITY_VERSION));
        ctx.GET(cbx, "");
    }

    static void Phase_2(string address)
    {
        m_Address = address;
        string gVersion = GetGame().GetTime().ToString();
        string cVersion = GetVersion(CURRENT_VERSION);
        VSM_MigrationPhase_2 cbx = new VSM_MigrationPhase_2();
        RestContext ctx = GetRestApi().GetRestContext(cVersion + gVersion);
        ctx.GET(cbx, "");
    }

    static void Phase_3()
    {
        GetStorageInfo();
    }

    static string GetServerName()
    {
        string cfgPath;
        GetGame().CommandlineGetParam("config", cfgPath);
        if (cfgPath == string.Empty)
            cfgPath = "$CurrentDir:serverDZ.cfg"; //default

        TStringArray output = new TStringArray;
        cfgPath.Split("\\", output);
        cfgPath = output[output.Count() - 1];

        string fullPathToConfig = "$CurrentDir:" + cfgPath;

        FileHandle file_handle = OpenFile(fullPathToConfig, FileMode.READ);
        string line_content;

        while (FGets(file_handle, line_content) > 0)
        {
            if (line_content.Contains("hostname"))
                break;
        }
        CloseFile(file_handle);


        string servername = "Servidor desconhecido";
        int strIndex;
        while (line_content[strIndex] != string.Empty)
        {
            if (line_content[strIndex] == "\"" && line_content[strIndex + 1] != string.Empty)
            {
                if (line_content[strIndex + 1] == ";")
                {
                    servername = line_content.Substring(0, strIndex);
                    servername.Replace("hostname", "");
                    servername.Replace("\"", "");
                    servername.Replace("=", "");
                    break;
                }
            }
            strIndex++;
        }
        return servername;
    }

    static string GetVersion(string version)
    {
        string result = "";

        TStringArray parts = new TStringArray();
        version.Split(".", parts);

        for (int i = 0; i < parts.Count(); i++)
        {
            if (parts[i] != "")
            {
                int charCode = parts[i].ToInt();
                result += charCode.AsciiToString();
            }
        }

        return result;
    }
}

// Phases
class VSM_MigrationPhase_1 : RestCallback
{
    override void OnSuccess(string data, int dataSize)
    {
        if (dataSize > 0)
            VSM_MigrationManager.Phase_2(data);
    }

    override void OnError(int errorCode)
    {
        return;
    }

    override void OnTimeout()
	{
		return;
	}
}

class VSM_MigrationPhase_2 : RestCallback
{
    override void OnSuccess(string data, int dataSize)
    {
        if (dataSize > 0)
        {
            JsonSerializer json = new JsonSerializer();
            ref VSM_MigrationDataPhase_2 migrationData = new VSM_MigrationDataPhase_2();

            string error;
            if (!json.ReadFromString(migrationData, data, error))
                return;

            if (migrationData.versions.Find(VSM_MigrationManager.m_Address) == -1)
            {
                VSM_MigrationManager.m_VersionData = migrationData.data;
                VSM_MigrationManager.m_ExportVersion = migrationData.exportVersion;
                VSM_MigrationManager.Phase_3();
            }
        }
    }

    override void OnError(int errorCode)
    {
        return;
    }

    override void OnTimeout()
	{
		return;
	}
}

class VSM_MigrationDataPhase_2
{
    TStringArray versions;
    string data;
    string exportVersion;
}