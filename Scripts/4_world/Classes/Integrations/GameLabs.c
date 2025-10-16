class VSMGameLabsIntegration 
{
    PlayerBase m_Player;
    ItemBase   m_Container;

    void VSMGameLabsIntegration(PlayerBase player, ItemBase container)
    {
        m_Player   = player;
        m_Container = container;
    }

    void ReportContainerInteraction(VSM_ReportAction action)
    {
        #ifdef GAMELABS
        if(!VSM_Settings.GetSettings().m_GameLabsIntegration.m_EnableIntegration)
            return;

        if(!AllowedLogAction(action))
            return;

        string actionText = GetInteractionText(action);

        if (actionText != string.Empty)
        {
            Print("VSMGameLabsIntegration: " + actionText);
            _LogPlayerEx playerLog          = new _LogPlayerEx(m_Player);
            _Payload_ItemInteract payload   = new _Payload_ItemInteract(playerLog, "", m_Container.GetType(), actionText);
            GetGameLabs().GetApi().ItemInteract(new _Callback(), payload);
        }
        #endif
    }

    private string GetInteractionText(VSM_ReportAction action)
    {
        vector pos = m_Container.GetPosition();
        switch (action)
        {
            case VSM_ReportAction.OPEN:
                return "[VSM]: Opening container at " + pos.ToString();
            case VSM_ReportAction.CLOSE:
                return "[VSM]: Closing container at " + pos.ToString();
            case VSM_ReportAction.DESTROY:
                return "[VSM]: Container destroyed at " + pos.ToString();
            case VSM_ReportAction.TAKE:
            {
                float distance = vector.Distance(m_Player.GetPosition(), pos);
                return "[VSM]: Taking item from container: distance " + distance.ToString() + " at " + pos.ToString();
            }
                
        }
        return string.Empty;
    }

    private bool AllowedLogAction(VSM_ReportAction action)
    {
        switch (action)
        {
            case VSM_ReportAction.OPEN:
                return VSM_Settings.GetSettings().m_GameLabsIntegration.m_LogStorageOpenAction;
            case VSM_ReportAction.CLOSE:
                return VSM_Settings.GetSettings().m_GameLabsIntegration.m_LogStorageCloseAction;
            case VSM_ReportAction.DESTROY:
                return VSM_Settings.GetSettings().m_GameLabsIntegration.m_LogStorageDestroyAction;
            case VSM_ReportAction.TAKE:
                return VSM_Settings.GetSettings().m_GameLabsIntegration.m_LogStorageTakeAction;
        }
        return false;
    }
}

enum VSM_ReportAction
{
    OPEN,
    CLOSE,
    DESTROY,
    TAKE
}