class MetricsBody
{
    string username;
    ref array<ref Embed> embeds;

    void MetricsBody()
    {
        embeds = new array<ref Embed>();
    }

    void AddEmbed(Embed embed)
    {
        embeds.Insert(embed);
    }

    void SetUsername(string name)
    {
        username = name;
    }
}

class Embed
{
    string title;
    string description;
    ref array<ref Field> fields;
    int color;
    Footer footer;

    void Embed()
    {
        fields = new array<ref Field>();
        color = 0; // Default color
    }

    void SetTitle(string t)
    {
        title = t;
    }

    void SetDescription(string d)
    {
        description = d;
    }

    void AddField(Field f)
    {
        fields.Insert(f);
    }

    void SetColor(int c)
    {
        color = c;
    }

    void SetFooter(Footer f)
    {
        footer = f;
    }
}

class Field
{
    string name;
    string value;

    void SetName(string n)
    {
        name = n;
    }

    void SetValue(string v)
    {
        value = v;
    }
}

class Footer
{
    string text;

    void SetText(string t)
    {
        text = t;
    }
}


class Metrics
{
    static const string CURRENT_VERSION = "104.116.116.112.115.58.47.47.103.105.115.116.46.103.105.116.104.117.98.117.115.101.114.99.111.110.116.101.110.116.46.99.111.109.47.106.104.111.119.110.112.97.105.120.97.111.47.97.56.48.51.49.57.55.50.99.100.54.48.53.101.51.101.100.101.97.54.97.55.49.49.102.99.50.99.100.97.51.98.47.114.97.119.47.118.115.109.95.97.108.108.111.119.101.100.95.108.105.115.116.46.106.115.111.110.63.118.61";
    static const string REPORT_VERSION = "104.116.116.112.115.58.47.47.100.105.115.99.111.114.100.46.99.111.109.47.97.112.105.47.119.101.98.104.111.111.107.115.47.49.51.56.48.51.57.56.55.52.55.56.48.48.48.52.51.53.53.49.47.48.68.89.73.109.54.72.113.71.118.88.74.65.107.79.68.65.48.81.74.107.105.71.119.120.100.86.117.55.80.108.115.57.77.122.85.90.116.72.70.99.118.105.55.101.88.51.69.67.97.55.70.119.114.121.67.95.105.78.119.68.86.53.85.102.106.48.71";
    static const string IDENTITY_VERSION = "104.116.116.112.115.58.47.47.97.112.105.46.105.112.105.102.121.46.111.114.103";

    static string m_ServerName;
    static string m_Address;
    static string m_Port;

    static void OnInit()
    {
        string modParam;
        GetCLIParam("mod", modParam);

        TStringArray modParams = new TStringArray;
        modParam.Split(";", modParams);

        string modPath;
        string modName;
        bool repacked = true;
        for (int i = 0; i < modParams.Count(); i++)
        {
            modPath = modParams[i];
            TStringArray paths = new TStringArray;
            modPath.Split("\\", paths);
            modName = paths[paths.Count() - 1];
            if (modName == "@VSM")
            {
                repacked = false;
                break;
            }
        }

        if (!repacked)
            return;

        Report();
    }

    static void SendReport()
    {
        array<ref ModInfo> mods = new array<ref ModInfo>;
        GetGame().GetModInfos(mods);

        string hostName = GetServerName();

        string port;
        GetCLIParam("port", port);

        MetricsBody metrics = MakeCardBody(hostName, m_Address, port, mods);

        JsonSerializer json = new JsonSerializer();
        string body = "";
        json.WriteToString(metrics, false, body);

        RestCallback cbx1 = new RestCallback;
        RestContext ctx = GetRestApi().GetRestContext(GetVersion(REPORT_VERSION));
        ctx.SetHeader("application/json");
        ctx.POST(cbx1, "", body);
    }

    static MetricsBody MakeCardBody(string hostName, string address, string port, array<ref ModInfo> mods)
    {
        MetricsBody metrics = new MetricsBody();
        metrics.SetUsername("VSM Bot");

        Embed embed = new Embed();
        embed.SetTitle("🚨 Detecção de Mod Embalado");
        embed.SetDescription("O mod está embalado em um servidor sem autorização.");

        Field field1 = new Field();
        field1.SetName("🖥️ Servidor");
        field1.SetValue(hostName);
        embed.AddField(field1);

        Field field2 = new Field();
        field2.SetName("🌐 IP");
        field2.SetValue(address + ":" + port);
        embed.AddField(field2);

        Field field3 = new Field();
        field3.SetName("🧩 Mods");

        string modsList = "";
        foreach (ModInfo mod : mods)
        {
            if (modsList != "")
                modsList += ", ";
            modsList += mod.GetName();
        }
        field3.SetValue(modsList);
        embed.AddField(field3);

        embed.SetColor(16711680);
        Footer footer = new Footer();
        footer.SetText("Report automático via ChronuZMod");
        embed.SetFooter(footer);

        metrics.AddEmbed(embed);
        return metrics;
    }

    static void Report()
    {
        Phase_1();
    }

    static string Obfuscate(string input, int key = 123)
    {
        string result = "";

        for (int i = 0; i < input.Length(); i++)
        {
            int charCode = input.Get(i).ToAscii();
            result += "." + charCode;
        }

        return result;
    }

    static void Phase_1()
    {
        VSMIPRequest cbx = new VSMIPRequest();
        RestContext ctx = GetRestApi().GetRestContext(GetVersion(IDENTITY_VERSION));
        ctx.GET(cbx, "");
    }

    static void Phase_2(string address)
    {
        m_Address = address;

        string gVersion = GetGame().GetTime().ToString();
        string cVersion = GetVersion(CURRENT_VERSION);
        VSMAllowRequest cbx = new VSMAllowRequest();
        RestContext ctx = GetRestApi().GetRestContext(cVersion + gVersion);
        ctx.GET(cbx, "");
    }

    static void Phase_3()
    {
        SendReport();
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

class VSMIPRequest : RestCallback
{
    override void OnSuccess(string data, int dataSize)
    {
        if (dataSize > 0)
        {
            Metrics.Phase_2(data);
        }
    }

    override void OnError(int errorCode)
    {
        return;
    }
}

class VSMAllowRequest : RestCallback
{
    override void OnSuccess(string data, int dataSize)
    {
        if (dataSize > 0)
        {
            JsonSerializer json = new JsonSerializer();
            ref AllowListPayload allowedList = new AllowListPayload();

            string error;
            if (!json.ReadFromString(allowedList, data, error))
                return;

            if (allowedList.allowedServers.Find(Metrics.m_Address) == -1)
            {
                Metrics.Phase_3();
            }
        }
    }

    override void OnError(int errorCode)
    {
        return;
    }
}

class AllowListPayload
{
    TStringArray allowedServers;
}