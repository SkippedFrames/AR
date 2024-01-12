[WorkbenchPluginAttribute(name: "Create/Update Selected Editable Prefabs", category: "In-game Editor", shortcut: "Ctrl+Shift+U", wbModules: {"WorldEditor"}, awesomeFontCode: 0xf1b2)]
class CreateEditablePrefabsPlugin: PrefabEditingPluginBase
{
	[Attribute(defvalue: "{90B4D95CF3610F3D}Configs/Workbench/EditablePrefabs/EditablePrefabsConfig.conf", desc: "", params: "conf")]
	private ResourceName m_Config;
	
	[Attribute(desc: "When enabled, the process will only update existing editable entities, not create new ones.")]
	private bool m_bOnlyUpdate;

	override void Run()
	{
		//TODO: m_bOnlyUpdate = false is Hotfix needs to be fixed and removed
		m_bOnlyUpdate = false;
		
		EditablePrefabsConfig config = EditablePrefabsConfig.GetConfig(m_Config);
		if (!config || !config.IsValid()) return;
		
		config.GetLinks();
		
		Workbench.OpenModule(WorldEditor);
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor) return;
		
		WorldEditorAPI api = worldEditor.GetApi();
		if (!api) return;
		
		//--- Get selected entities
		array<ResourceName> selectedPrefabs = new array<ResourceName>;
		if (!GetPrefabs(selectedPrefabs)) return;
		
		map<string, EEditablePrefabResult> results = new map<string, EEditablePrefabResult>;
		
		if (!config.CreateWorld())
			return;
		
		WBProgressDialog progress = new WBProgressDialog("Processing Prefabs...", worldEditor);
		
		//--- Iterate through all entities
		api.BeginEntityAction();
		for (int i = 0, count = selectedPrefabs.Count(); i < count; i++)
		{
			progress.SetProgress(i / count);
			config.CreateEditablePrefab(api, selectedPrefabs[i], m_bOnlyUpdate, results);
		}
		api.EndEntityAction();
		
		worldEditor.Save(); //--- Save to world so it's not marked as edited
		
		config.LogResults(results);
	}
	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Create/Update Selected Editable Prefabs' plugin", "", this);
	}
	[ButtonAttribute("Close")]
	bool ButtonClose()
	{
	}
};
[WorkbenchPluginAttribute(name: "Create/Update Selected Editable Prefabs", category: "In-game Editor", shortcut: "Ctrl+Shift+U", wbModules: {"ResourceManager"})]
class CreateEditablePrefabsPluginResourceManager: CreateEditablePrefabsPlugin
{
	override void GetSelected(out array<ResourceName> selection)
	{
		GetSelectedResourceBrowser(selection);
	}
};
