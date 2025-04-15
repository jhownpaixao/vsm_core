modded class CfgGameplayHandler
{

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!! =====================================================================================
	//!!! the naming convention for static functions in this file is Get+MemberVariableName !!
	//!! =====================================================================================
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//========================
	// !!! ONLY GETTERS BELLOW
	//========================

	//----------------------------------------------------------------------------------
	static bool GetVSM_EnableDebug()
	{
		return m_Data.VSMData.enableDebug;
	}
	//----------------------------------------------------------------------------------
	static bool GetVSM_AutoCloseEnable()
	{
		return m_Data.VSMData.autoCloseEnable;
	}
	//----------------------------------------------------------------------------------
	static int GetVSM_AutoCloseInterval()
	{
		return m_Data.VSMData.autoCloseInterval;
	} 
	//----------------------------------------------------------------------------------
	static int GetVSM_AutoClosePlayerDistance()
	{
		return m_Data.VSMData.autoClosePlayerDistance;
	} 
	//----------------------------------------------------------------------------------
	static bool GetVSM_AutoCloseIgnorePlayerNearby()
	{
		return m_Data.VSMData.autoCloseIgnorePlayerNearby;
	} 
	//----------------------------------------------------------------------------------
	static int GetVSM_BatchSize()
	{
		return m_Data.VSMData.batchSize;
	} 
	//----------------------------------------------------------------------------------
	static int GetVSM_BatchInterval()
	{
		return m_Data.VSMData.batchInterval;
	} 
	//----------------------------------------------------------------------------------
	static int GetVSM_IncludeDecayItems()
	{
		return m_Data.VSMData.includeDecayItems;
	} 
	//----------------------------------------------------------------------------------
	static TStringArray GetVSM_IgnoredItems()
	{
		return m_Data.VSMData.ignoredItems;
	} 
	//----------------------------------------------------------------------------------

}