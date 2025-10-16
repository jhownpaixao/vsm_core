class VSMAutoCloseBehavior extends VSMBase
{
    private ItemBase        m_Container;
	private ref Timer       m_TimerTick;
    private bool            m_IsEnabled;
    private float           m_CloseDistance;
    private float           m_CloseDelay;
    private bool            m_IgnorePlayersNearby;
    private float           m_TimeSinceLastCheck;

    void VSMAutoCloseBehavior(ItemBase container)
    {
        m_Container = container;

        m_IsEnabled = VSM_Settings.GetSettings().m_AutoCloseEnable;
        m_CloseDelay = VSM_Settings.GetSettings().m_AutoCloseInterval;
        m_CloseDistance = VSM_Settings.GetSettings().m_AutoClosePlayerDistance;
        m_IgnorePlayersNearby = VSM_Settings.GetSettings().m_AutoCloseIgnorePlayerNearby;
    }

    void OnTick()
    {
        m_TimeSinceLastCheck ++;

        if(!m_Container.VSM_IsOpen())
        {
            Stop();
            return;
        }

        if (m_TimeSinceLastCheck >= m_CloseDelay)
        {
            m_TimeSinceLastCheck = 0;
            bool shouldClose = true;

            if(!m_IgnorePlayersNearby && VirtualUtils.IsPlayerNearby(m_Container.GetPosition(), m_CloseDistance))
            {
                shouldClose = false;
            }

            if(!m_Container.VSM_CanClose())
            {
                shouldClose = false;
            }

            if(shouldClose)
            {
                m_Container.VSM_Close();
                Stop();
            }
        }
    }

    void Start()
    {
        if (!m_IsEnabled || (m_TimerTick && m_TimerTick.IsRunning()))
            return;

        m_TimerTick = new Timer(CALL_CATEGORY_GAMEPLAY);
        m_TimerTick.Run(1, this, "OnTick", null, true);
        VSM_Trace("Start", "Auto-close behavior started on container: " + m_Container.GetType());
    }

    void Stop()
    {
        if (m_TimerTick)
        {
            m_TimerTick.Stop();
            m_TimerTick = null;
            VSM_Trace("Stop", "Auto-close behavior stopped on container: " + m_Container.GetType());
        }
    }
}