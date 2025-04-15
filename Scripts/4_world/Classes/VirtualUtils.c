
class VirtualUtils
{

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
				Print("Não foi possível criar o diretório "+ path);
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
                    Print("[EntityStorage] couldn't delete " + path + baseName);
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
		array<Object> objects = {};
		GetGame().GetObjectsAtPosition(position, radius, objects, NULL);

		foreach (Object obj : objects)
		{
			if (obj.IsKindOf("PlayerBase"))
			{
				PlayerBase player = PlayerBase.Cast(obj);
				if (player && player.IsAlive())
				{
					return true;
				}
			}
		}

		return false;
	}
}