[WorkbenchPluginAttribute("Create New Script", "", "CTRL+N", "", { "ScriptEditor" }, awesomeFontCode: 0xF1C9)]
class SCR_NewScriptPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "File system where the new script file will be created", enums: ParamEnumAddons.FromEnum(titleFormat: 2, hideCoreModules: 2), category: "General")]
	protected int m_iAddon;

	[Attribute(defvalue: DEFAULT_DIRECTORY, desc: "File destination - the path can be selected in an addon (typically the Arma project) and Addon be defined to something else, the directory tree will then be created in said targeted addon", params: "unregFolders", category: "General")]
	protected ResourceName m_sDestinationDirectory;

	[Attribute(defvalue: DEFAULT_FILENAME, desc: "Created class name", category: "General")]
	protected string m_sClassName;

	[Attribute(desc: "Parent class name - if left empty, the type's default value will be used, if any", category: "General")]
	protected string m_sParentClassName;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "", enums: ParamEnumArray.FromEnum(SCR_EScriptTemplateType), category: "General")]
	protected SCR_EScriptTemplateType m_eType;

	/*
		Category: Advanced
	*/

	[Attribute(defvalue: DEFAULT_CONFIG, params: "conf class=SCR_ScriptTemplateConfig", category: "Advanced")]
	protected ResourceName m_sTemplateConfigFile;

	protected static const string DEFAULT_PREFIX = "MISSINGTAG_";
	protected static const string DEFAULT_FILENAME = "TAG_MyFile";
	protected static const ResourceName DEFAULT_DIRECTORY = "{B92491157EA3E4AD}scripts/Game";
	protected static const ResourceName DEFAULT_CONFIG = "{51BB186E949A991B}Configs/Workbench/ScriptTemplatePlugin/ScriptTemplateConfig.conf";
	protected static const string SCRIPT_EXTENSION = ".c";
	protected static const string ALLOWED_CLASSNAME_CHARS = SCR_StringHelper.LETTERS + SCR_StringHelper.DIGITS + "_";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.OpenModule(ScriptEditor) || !Workbench.ScriptDialog("Script Creation", "Define the new file's settings. Class name will be used as file name.", this))
			return;

		string name = SCR_StringHelper.Filter(m_sClassName, ALLOWED_CLASSNAME_CHARS);
		if (name.IsEmpty())
			name = DEFAULT_FILENAME;

		if (!name.Contains("_"))
			name = DEFAULT_PREFIX + name;

		SCR_ScriptTemplateConfigEntry config = GetConfig();
		if (config && !name.EndsWith(config.m_sSuffix))
			name += config.m_sSuffix;

		string path = GetPath(name);
		if (FileIO.FileExists(path))
		{
			Workbench.Dialog("Script Creation error", "\"" + path + "\" already exists!");
			return;
		}

		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);
		if (!file)
		{
			Workbench.Dialog("Script Creation error", "\"" + path + "\" could not be created.");
			return;
		}

		if (config)
			TryFillFile(file, name, config);

		file.Close();

		Workbench.GetModule(ScriptEditor).SetOpenedResource(path);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ScriptTemplateConfigEntry GetConfig()
	{
		ResourceName configPath = m_sTemplateConfigFile;
		if (configPath.IsEmpty())
			configPath = DEFAULT_CONFIG;

		Resource configResource = Resource.Load(configPath);
		if (!configResource || !configResource.IsValid())
		{
			Print("Cannot load config '" + configPath + "'", LogLevel.WARNING);
			return null;
		}

		SCR_ScriptTemplateConfig config = SCR_ScriptTemplateConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configResource.GetResource().ToBaseContainer()));
		if (!config)
		{
			Print("Config is of the wrong type (not SCR_ScriptTemplateConfig)", LogLevel.ERROR);
			return null;
		}

		if (!config.m_aEntries)
		{
			Print("Config entries are null", LogLevel.ERROR);
			return null;
		}

		foreach (SCR_ScriptTemplateConfigEntry entry : config.m_aEntries)
		{
			if (entry.m_eType == m_eType)
				return entry;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetPath(string filename)
	{
		string addonIDPrefix = SCR_AddonTool.ToFileSystem(SCR_AddonTool.GetAddonIndex(m_iAddon));
		string path = m_sDestinationDirectory.GetPath();
		if (path.IsEmpty())
			path = DEFAULT_DIRECTORY.GetPath();

		array<string> folders = {};
		path.Split("/", folders, true);

		string currentFolderPath = addonIDPrefix;
		for (int i = 0, foldersCount = folders.Count(); i < foldersCount; i++)
		{
			if (i > 0)
				currentFolderPath += "/";
			currentFolderPath += folders[i];

			if (!FileIO.FileExists(currentFolderPath))
				FileIO.MakeDirectory(currentFolderPath);
		}

		if (!path.EndsWith("/"))
			path += "/";

		if (!filename.EndsWith(SCRIPT_EXTENSION))
			filename += SCRIPT_EXTENSION;

		return addonIDPrefix + path + filename;
	}

	//------------------------------------------------------------------------------------------------
	protected void TryFillFile(notnull FileHandle file, string name, notnull SCR_ScriptTemplateConfigEntry config)
	{
		if (config.m_sTemplate.IsEmpty())
			return;

		string parentName = SCR_StringHelper.Filter(m_sParentClassName, ALLOWED_CLASSNAME_CHARS);
		if (parentName.IsEmpty())
			parentName = config.m_sDefaultParentName;

		if (!parentName.IsEmpty())
			parentName = " : " + parentName;

		file.WriteLine(string.Format(config.m_sTemplate, name, parentName));
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	protected bool OkButton()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool CancelButton()
	{
		return false;
	}
}
