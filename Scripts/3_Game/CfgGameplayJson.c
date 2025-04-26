//!contents of this class will be transfered to client upon connecting, with the variables in either initial state as set through the 'InitServer..()' call, or replaced with contents of the json configuration file if such file is both present and reading is enabled in server.cfg

modded class CfgGameplayJson
{
	ref ITEM_VSMData VSMData 						= new ITEM_VSMData;
};

class ITEM_VSMData : ITEM_DataBase
{
	override void InitServer()
	{
	}
	
	override bool ValidateServer()
	{
		return true;
	}
	
	//-------------------------------------------------------------------------------------------------
	//!!! all member variables must correspond with the cfggameplay.json file contents !!!!
	bool  autoCloseEnable 				= true;
	bool  autoCloseIgnorePlayerNearby 	= false;
	float autoClosePlayerDistance 		= 8.0;
	int   autoCloseInterval				= 120; // seconds

	int  batchSize 		= 50;
	int  batchInterval 	= 1; // seconds
	bool includeDecayItems = false;
	ref TStringArray ignoredItems = {}; 
	bool  useCfgIgnoreList 	= true; //! em teste
	bool logLevel = VSM_LogLevel.ERROR;
};

enum VSM_LogLevel
{
	NONE = 0,
	CRITICAL,
	ERROR,
	WARN,
	INFO,
	DEBUG,
	TRACE
};