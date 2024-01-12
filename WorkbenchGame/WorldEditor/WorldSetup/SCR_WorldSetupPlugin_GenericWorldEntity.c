#ifdef WORKBENCH

//! This Plugin is experimental because it directly writes in the layer file, expecting a certain directory structure and file format.
//! An engine solution should be provided for this matter.
[WorkbenchPluginAttribute(
	name: "[Experimental] World Entity Setup (LOAD the world after this, do NOT save!)",
	category: "World Setup",
	description: "Set up GenericWorldEntity's Prefab",
	shortcut: "", // "Ctrl+Shift+N", // for new content?
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF0AC)]
class SCR_WorldSetupPlugin_GenericWorldEntity : SCR_WorldSetupPluginBasePlugin
{
	[Attribute(desc: "The World Entity Prefab to be used for this world", uiwidget: UIWidgets.ResourceNamePicker, params: "et class=GenericWorldEntity", defvalue: "{08A95D735ECC517F}Prefabs/World/DefaultWorld/GenericWorld_Default.et")]
	protected ResourceName m_sWorldEntityPrefab;

	[Attribute(defvalue: "0", desc: "Create Prefab's child from the provided one in the selected addon and use it instead of using the provided Prefab directly")]
	protected bool m_bCopyPrefabToAddon;

	[Attribute(/* defvalue: ParamEnumAddons.FromEnum(titleFormat: 2, hideCoreModules: 2)[0].m_Value, */uiwidget: UIWidgets.ComboBox, desc: "Addon in which the Prefab will be copied (if copied)", enums: ParamEnumAddons.FromEnum(titleFormat: 2, hideCoreModules: 2))]
	protected int m_iAddon;

	protected static const string CHILD_PREFAB = "%1_%2"; // %1 = parent Prefab filename, %2 = world name
	protected static const string LAYERS_DIR = "%1_Layers"; // %1 = world name
	protected static const string DEFAULT_LAYER = "default.layer";
	// protected static const string WORLD_ENTITY_CLASS = ((typename)GenericWorldEntity).ToString();
	protected static const string WORLD_ENTITY_CLASS = "GenericWorldEntity";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("World Setup (WorldEntity)", "This plugin replaces the current WorldEntity (generic entity or Prefab) by the provided Prefab.", this))
			return;

		ResourceName worldEntityPrefab = m_sWorldEntityPrefab;

		if (worldEntityPrefab.IsEmpty())
		{
			Print("No World Entity Prefab provided", LogLevel.WARNING);
			return;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("World Editor API is not available", LogLevel.WARNING);
			return;
		}

		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);
		string worldName = FilePath.StripExtension(FilePath.StripPath(worldPath));

		// duplicate the Prefab
		if (m_bCopyPrefabToAddon)
			worldEntityPrefab = CreatePrefabChildInAddon(worldEntityPrefab, m_iAddon, worldName);

		if (worldEntityPrefab.IsEmpty())
		{
			Print("Child Prefab could not be created", LogLevel.WARNING);
			return;
		}

		string defaultLayerAbsFilePath = GetDefaultLayerAbsoluteFilePath(worldEditorAPI, worldPath, worldName);
		if (!FileIO.FileExists(defaultLayerAbsFilePath))
		{
			Print("The default layer file could not be found - please save the world first", LogLevel.WARNING);
			return;
		}

		Print("Reading " + defaultLayerAbsFilePath, LogLevel.NORMAL);
		array<string> lines = SCR_FileIOHelper.ReadFileContent(defaultLayerAbsFilePath);

		bool found;
		for (int i, count = lines.Count(); i < count; i++)
		{
			if (lines[i].EndsWith(" {") && lines[i].StartsWith(WORLD_ENTITY_CLASS + " "))
			{
				"}";
				lines[i] = WORLD_ENTITY_CLASS + " world : \"" + worldEntityPrefab + "\" {";
				"}";
				found = true;
				break;
			}
		}

		if (!found)
		{
			Print("No " + WORLD_ENTITY_CLASS + " class could be found in " + defaultLayerAbsFilePath, LogLevel.WARNING);
			return;
		}

		if (SCR_FileIOHelper.WriteFileContent(defaultLayerAbsFilePath, lines))
			Print("File written successfully. Please LOAD the world to see the changes applied", LogLevel.NORMAL);
		else
			Print("File could not be written", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetDefaultLayerAbsoluteFilePath(notnull WorldEditorAPI worldEditorAPI, string worldPath, string worldName)
	{
		string defaultLayerAbsFilePath;
		if (!Workbench.GetAbsolutePath(worldPath, defaultLayerAbsFilePath))
		{
			Print("Could not get absolute path for " + worldPath, LogLevel.WARNING);
			return string.Empty;
		}

		string layerAbsDir = FilePath.StripFileName(defaultLayerAbsFilePath);
		if (!layerAbsDir.EndsWith("/"))
			layerAbsDir += "/";

		return layerAbsDir + string.Format(LAYERS_DIR, worldName) + "/" + DEFAULT_LAYER;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}

#endif // WORKBENCH
