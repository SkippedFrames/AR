class ParamEnumAddons : array<ref ParamEnum>
{
	protected static const int CORE_MODULE_COUNT = 2;

	//------------------------------------------------------------------------------------------------
	//! Get ParamEnumArray compatible with Attribute's enums parameter
	//! Example:
	//! @code
	//! [Attribute(desc: "Pick an addon", enums: ParamEnumAddons.FromEnum())]
	//! protected int m_iAddon;
	//! @endcode
	//! \param titleFormat 0 for "AddonID", 1 for "AddonTitle", 2 for "AddonTitle (AddonID)"
	//! \param hideCoreModules 0 to hide nothing, 1 to hide core (vanilla) addons, 2 to hide core addons only when more addons are available
	//! \return [Attribute] combobox-compatible ParamEnumArray value (for int variable)
	static ParamEnumArray FromEnum(int titleFormat = 2, int hideCoreModules = 0)
	{
		ParamEnumArray params = new ParamEnumArray();
		array<string> addonGUIDs = {};
		GameProject.GetLoadedAddons(addonGUIDs);

		for (int i, count = addonGUIDs.Count(); i < count; i++)
		{
			string addonGUID = addonGUIDs[i];

			if (hideCoreModules == 1 && GameProject.IsVanillaAddon(addonGUID))
				continue;

			if (hideCoreModules == 2 && count > CORE_MODULE_COUNT && GameProject.IsVanillaAddon(addonGUID))
				continue;

			string title;
			switch (titleFormat)
			{
				case 0: title = GameProject.GetAddonID(addonGUID); break;
				case 1: title = GameProject.GetAddonTitle(addonGUID); break;
				default:
				case 2: title = string.Format("%1 (%2)", GameProject.GetAddonTitle(addonGUID), GameProject.GetAddonID(addonGUID)); break;
			}
			params.Insert(new ParamEnum(title, i.ToString()));
		}

		return params;
	}
}

// TODO: rename to SCR_AddonTools?
class SCR_AddonTool
{
	protected static const ref array<string> CORE_ADDONS = { "core", "ArmaReforger" };

	//------------------------------------------------------------------------------------------------
	//! Returns an array of addons where given resource is present or modified.
	//! \param prefab Prefab path
	//! \param ignoreCoreAddons if true then ArmaReforger and Core are ignored
	//! \return array of addon ID strings
	static array<string> GetResourceAddons(ResourceName prefab, bool ignoreCoreAddons = false)
	{
		array<string> addonNames = {};

		if (prefab.IsEmpty())
			return addonNames;

		Resource prefabResource = BaseContainerTools.LoadContainer(prefab);
		if (!prefabResource)
			return addonNames;

		BaseResourceObject configContainer;
		configContainer = prefabResource.GetResource();
		if (!configContainer)
			return addonNames;

		BaseContainer configBase = configContainer.ToBaseContainer();
		if (!configBase)
			return addonNames;

		configBase.GetSourceAddons(addonNames);

		if (ignoreCoreAddons && !addonNames.IsEmpty())
		{
			foreach (string item : CORE_ADDONS)
			{
				addonNames.RemoveItem(item);
			}
		}

		return addonNames;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns last addon where given resource is present.
	//! \param prefab Prefab path
	//! \return Class name
	static string GetResourceLastAddon(ResourceName prefab)
	{
		array<string> addonNames = GetResourceAddons(prefab);
		if (addonNames.IsEmpty())
			return string.Empty;

		return addonNames[addonNames.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	//! Get addon name by providing index of the addon.
	//! Can be used in tandem with ParamEnumAddons.
	//! \param index Index number
	//! \return Addon ID
	static string GetAddonIndex(int index)
	{
		array<string> addons = {};
		GameProject.GetLoadedAddons(addons);

		return GameProject.GetAddonID(addons[index]);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the FileSystem prefix-stripped path
	//! e.g from $ArmaReforger:scripts/Game/Global/SCR_AddonTools.c to scripts/Game/Global/SCR_AddonTools.c
	//! $abc:test -> test
	//! $:test -> test
	//! :test -> :test
	//! $abc -> $abc
	//! $abc: -> <empty string>
	//! \return path without addon prefix, or untouched provided path if the format is wrong (proper format = $addonName:somethingElse)
	static string StripFileSystem(string fileSystemPath)
	{
		int length = fileSystemPath.Length();
		if (fileSystemPath.IsEmpty())
			return fileSystemPath;

		if (!fileSystemPath.StartsWith("$"))
			return fileSystemPath;

		int colonIndex /* ha, ha */ = fileSystemPath.IndexOf(":");
		if (colonIndex < 0)
			return fileSystemPath;

		colonIndex++;
		if (colonIndex == length)
			return string.Empty;

		return fileSystemPath.Substring(colonIndex, length - colonIndex);
	}

	//------------------------------------------------------------------------------------------------
	//!  Convert addon name to file system format.
	//! For instance, "ArmaReforger" will get converted to "$ArmaReforger:".
	//! \param addon Addon ID
	//! \return Class name
	static string ToFileSystem(string addon)
	{
		return "$" + addon + ":";
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Return the absolute path (without a trailing '/')
	//! \param relativeDirPath if a directory resourceName is provided, use resourceName.GetPath()
	//! \return true on success, false on failure
	static bool GetAddonAbsolutePath(int addonId, string relativeDirPath, out string result, bool mustExist = true)
	{
		return Workbench.GetAbsolutePath(ToFileSystem(GetAddonIndex(addonId)) + relativeDirPath, result, mustExist);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the absolute path (without a trailing '/')
	//! \param directory a directory resourceName should be provided\
	//! if a -file- ResourceName is provided, the xxx.yyy part WILL be part of the provided directory so use FilePath.StripFileName if not wanted
	//! \return true on success, false on failure
	static bool GetAddonAbsolutePath(int addonId, ResourceName directory, out string result, bool mustExist = true)
	{
		return Workbench.GetAbsolutePath(ToFileSystem(GetAddonIndex(addonId)) + directory.GetPath(), result, mustExist);
	}
#endif
}
