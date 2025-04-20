
class VirtualUtils
{
	static const float MESSAGE_COOLDOWN = 30.0;
	static ref map<string, int> m_MessageCooldowns = new map<string, int>();

    //! File operations
    static bool MakeDirectoryRecursive(string path)
	{
		TStringArray parts = {};
		path.Split("\\", parts);
		path = "";
		foreach (string part: parts)
		{
			path += part + "\\";
			if (part.IndexOf(":") == part.Length() - 1)
				continue;
			if (!FileExist(path) && !MakeDirectory(path))
			{
				// Print("Não foi possível criar o diretório "+ path);
				return false;
			}
		}
		return true;
	}

    static bool DeleteFiles(string path)
	{
        if (FileExist(path))
        {
            TStringArray files = VirtualUtils.FindFilesInLocation(path);

            foreach (string baseName: files)
            {
                if (!DeleteFile(path + baseName))
                    // Print("[EntityStorage] couldn't delete " + path + baseName);
            }

            if (!DeleteFile(path))
               Print("[EntityStorage] couldn't delete " + path);
            else
                return true;
        }
		
		return false;
	}

    //! @DayZ Expansion
    static array< string > FindFilesInLocation( string folder, string ext = "", bool recursive = false )
	{
		return FindInLocation(folder, ext, 1, recursive);
	}
    
    //! @DayZ Expansion
    static array< string > FindInLocation( string folder, string ext = "", int mode = 1, bool recursive = false )
	{
		return FindInLocationImpl(folder, ext, mode, recursive);
	}

    //! @DayZ Expansion
    static array< string > FindInLocationImpl( string folder, string ext = "", int mode = 1, bool recursive = false )
	{
		array< string > files = new array< string >;
		if (!FileExist(folder))
			return files;

		string fileName;
		FileAttr fileAttr;
		string pattern = folder + "*";

		if (!recursive)
			pattern += ext;
		FindFileHandle findFileHandle = FindFile( pattern, fileName, fileAttr, 0 );

		if ( findFileHandle )
		{
			bool isValid = true;
			bool includeFiles;

			if (mode & 1)
				includeFiles = true;

			bool includeDirs;
			if (mode & 2)
				includeDirs = true;

			while (isValid)
			{
				bool isDir = false;
				if (fileAttr & FileAttr.DIRECTORY)
					isDir = true;

				if (fileName.Length() > 0)
				{
					if ((includeFiles && !isDir) || (includeDirs && isDir))
					{
						
						if (!recursive || fileName.Substring(fileName.Length() - 1, 1) == ext)
						{
						   files.Insert( fileName );
						}	
					}

					if (recursive && isDir)
					{
						TStringArray subFolderFileNames = FindInLocationImpl(folder + fileName + "\\", ext, mode, true);
						foreach (string subFolderFileName: subFolderFileNames)
						{
							files.Insert( fileName + "\\" + subFolderFileName );
						}
					}
				}

				isValid = FindNextFile(findFileHandle, fileName, fileAttr);
			}

			CloseFindFile( findFileHandle );
		}
		return files;
	}

	static bool IsPlayerNearby(vector position, float radius)
	{
		// Print("IsPlayerNearby");
		array<Object> objects = {};
		GetGame().GetObjectsAtPosition(position, radius, objects, NULL);

		foreach (Object obj : objects)
		{
			// Print("IsPlayerNearby verificando "+ obj);
			PlayerBase player = PlayerBase.Cast(obj);
			if (player && player.IsAlive())
			{
				return true;
			}
		}

		return false;
	}

	static void OnLocalPlayerSendMessage(string msg)
	{
		VirtualUtils.Debug("OnLocalPlayerSendMessage: enviando mensagem:: %1", msg);
		if(!msg)
			return;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		VirtualUtils.Debug("OnLocalPlayerSendMessage: player %1", player.ToString());

		if (player && CanSendMessage(player))
		{
			VirtualUtils.Debug("OnLocalPlayerSendMessage: enviado para player %1", player.ToString());
			player.MessageStatus(msg); //Mensagem enviada pelo chat
			
			string playerId = player.GetIdentity().GetId();
			int currentTime = GetGame().GetTime() / 1000;
			m_MessageCooldowns.Set(playerId, currentTime);
		}
		else
		{
			VirtualUtils.Warn("OnLocalPlayerSendMessage: Não foi possível enviar a mensagem");
		}
	}

	static bool CanSendMessage(PlayerBase player)
    {
        string playerId = player.GetIdentity().GetId();
        int currentTime = GetGame().GetTime() / 1000;
		int lastMessageTime = m_MessageCooldowns.Get(playerId);

		VirtualUtils.Debug("CanSendMessage: verificando player %1 id %2 lastMessageTime %3", player.ToString(), playerId);

        if (lastMessageTime)
        {
			VirtualUtils.Debug("CanSendMessage: lastmessage encontrado %1 currentTime", lastMessageTime.ToString(), currentTime.ToString());
            if (currentTime - lastMessageTime < MESSAGE_COOLDOWN)
            {
                return false;
            }
        }

        return true;
    }

	//TODO: porque não usar o CF_LOG ?
	static void Trace(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		PrintFormat("[TRACE] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
	}

	static void Debug(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		PrintFormat("[DEBUG] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
	}

	static void Info(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		PrintFormat("[INFO] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
	}

	static void Warn(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		PrintFormat("[WARNING] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
	}

	static void Error(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		PrintFormat("[ERROR] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
		LogStack();
	}

	static void Critical(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "", string param7 = "", string param8 = "", string param9 = "")
	{
		PrintFormat("[CRITICAL] %1", string.Format(message, param1, param2, param3, param4, param5, param6, param7, param8, param9));
		LogStack();
	}

	//! @DayZ Expansion
	static protected void LogStack()
	{
		string unformated = "";
		DumpStackString(unformated);

		array<string> formated = new array<string>();
		unformated.Split("\n", formated);

		for (int i = 1; i < formated.Count(); i++)
		{
			Print("\t" + formated[i]);
		}
	}
}