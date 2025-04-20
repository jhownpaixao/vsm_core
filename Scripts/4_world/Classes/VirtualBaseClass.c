class VSM_Base
{
    //!log
    void VSM_Info(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
    {
        VirtualUtils.Info(this.ToString() + "::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }

    void VSM_Error(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
    {
        VirtualUtils.Error(this.ToString() + "::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }

    void VSM_Trace(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
    {
        VirtualUtils.Trace(this.ToString() + "::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }

    void VSM_Debug(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
    {
        VirtualUtils.Debug(this.ToString() + "::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }

    void VSM_Warn(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
    {
        VirtualUtils.Warn(this.ToString() + "::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }

    void VSM_Critical(string mtd, string msg, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
    {
        VirtualUtils.Critical(this.ToString() + "::" + mtd + " - " + msg, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }
}