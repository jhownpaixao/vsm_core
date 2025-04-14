class CfgPatches
{
	class VSM_Core
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 1.21;
		requiredAddons[] = {"DZ_Data","DZ_Scripts","JM_CF_Scripts"};
	};
};
class CfgMods
{
	class VirtualStorageModule
	{
		dir = "vsm_core";
		picture = "";
		action = "";
		hideName = 0;
		name = "Virtual Storage Module";
		hidePicture = 0;
		credits = "Jhonnata Paixão";
		author = "iTzChronuZ";
		authorID = 76561199441861559;
		version = "1.0.0";
		extra = 0;
		type = "mod";
		dependencies[]=
		{
			"Game",
			"World",
			"Mission"
		};

		class defs
		{
			class engineScriptModule
			{
				value="";
				files[]=
				{
					"vsm_core/Scripts/Common",
					"vsm_core/Scripts/1_Core"
				};
			};

			class gameLibScriptModule
			{
				value="";
				files[]=
				{
					"vsm_core/Scripts/Common",
					"vsm_core/Scripts/2_GameLib"
				};
			};

			class gameScriptModule
			{
				value="";
				files[]=
				{
					"vsm_core/Scripts/Common",
					"vsm_core/Scripts/3_Game"
				};
			};

			class worldScriptModule
			{
				value="";
				files[]=
				{
					"vsm_core/Scripts/Common",
					"vsm_core/Scripts/4_World"
				};
			};

			class missionScriptModule
			{
				value="";
				files[]=
				{
					"vsm_core/Scripts/Common",
					"vsm_core/Scripts/5_Mission"
				};
			};
		};
	};
};