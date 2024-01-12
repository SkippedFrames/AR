#ifdef WORKBENCH

class SCR_WorldSetupPluginBasePlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	//! \param 
	//! \param fileName if provided, the desired file name (without extension) e.g "My_Prefab" -> "My_Prefab.et"
	//! \return the created ResourceName or empty if failed
	protected ResourceName CreatePrefabChildInAddon(ResourceName prefab, int addon, string fileName = string.Empty)
	{
		string prefabFilePath = prefab.GetPath();
		string relativePrefabDirPath = FilePath.StripFileName(prefab.GetPath());
		if (!relativePrefabDirPath.EndsWith("/"))
			relativePrefabDirPath += "/";

		if (fileName.IsEmpty())
		{
			string extension;
			FilePath.StripExtension(prefab, extension);
			fileName = SCR_StringHelper.Filter(fileName, "/\\;", true) + extension;
		}
		else
		{
			fileName = FilePath.StripPath(prefabFilePath);
		}

		string destinationAbsolutePath;
		if (!SCR_AddonTool.GetAddonAbsolutePath(addon, relativePrefabDirPath + fileName, destinationAbsolutePath, false))
		{
			Print("Could not create child Prefab; using the provided one", LogLevel.WARNING);
			return string.Empty;
		}

		if (FileIO.FileExists(destinationAbsolutePath))
		{
			Print("Prefab destination already exists " + destinationAbsolutePath, LogLevel.WARNING);
			return string.Empty;
		}

		return SCR_PrefabHelper.CreateChildPrefab(prefab, destinationAbsolutePath);
	}
}

#endif
