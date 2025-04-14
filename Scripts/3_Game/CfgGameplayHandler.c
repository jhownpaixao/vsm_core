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
	static bool GetVSM_EnableAutoClose()
	{
		return m_Data.VSMData.enableAutoClose;
	}
	//----------------------------------------------------------------------------------
	static int GetVSM_TimeToAutoClose()
	{
		return m_Data.VSMData.timeToAutoClose;
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

}