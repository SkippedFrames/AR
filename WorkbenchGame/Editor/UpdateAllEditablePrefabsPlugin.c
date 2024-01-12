[WorkbenchPluginAttribute(name: "Update All Editable Prefabs", category: "In-game Editor", wbModules: {"WorldEditor", "ResourceManager"}, awesomeFontCode: 0xf1b3)]
class UpdateAllEditablePrefabsPlugin: WorkbenchPlugin
{
	const string META_EXTENSION = ".meta";
	
	[Attribute(defvalue: "{90B4D95CF3610F3D}Configs/Workbench/EditablePrefabs/EditablePrefabsConfig.conf", desc: "", params: "conf")]
	private ResourceName m_Config;
	
	[Attribute(desc: "Enable to only check for renamed or delete files, don't update their content.")]
	private bool m_OnlyFileChanges;

	void DeleteEmptyDirectoriesCallback(string path, FileAttribute attributes)
	{
		if ((attributes & FileAttribute.DIRECTORY) && FileIO.DeleteFile(path)) //--- DeleteFile fails when the folder is not empty
		{
			Print(string.Format("Folder DELETED: '%1' (was empty)", path), LogLevel.WARNING);
		}
	}
	
	protected void DeleteEmptyDirectories(string path)
	{
		System.FindFiles(DeleteEmptyDirectoriesCallback, path, string.Empty);
	}
	
	override void Run()
	{
		if (!Workbench.ScriptDialog("Configure 'Update All Editable Prefabs' plugin", "Batch operation applied on all auto-generated editable prefabs\n(check the config to see which folders will be affected).\n\nWARNING: This operation can take several minutes to perform!", this)) return;;
		
		EditablePrefabsConfig config = EditablePrefabsConfig.GetConfig(m_Config);
		if (!config || !config.IsValid()) return;
		
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		if (!resourceManager) return;
		
		Workbench.OpenModule(WorldEditor);
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor) return;
		
		WorldEditorAPI api = worldEditor.GetApi();
		if (!api) return;
		
		//--- Ask first
		//ConfirmationDialogWorkbench dialog = new ConfirmationDialogWorkbench;
		//Workbench.ScriptDialog("", "Do you want to update all editable prefabs?", dialog);
		//if (!dialog.GetResult()) return;
		
		string targetPath = config.GetTargetPathAuto();
		
		WBProgressDialog progress = new WBProgressDialog("Processing Prefabs...", resourceManager);
		Print(string.Format("Updating editable entity prefabs STARTED at '%1'", targetPath), LogLevel.DEBUG);
		
		if (!config.CreateWorld())
			return;
		
		//--- Get all files
		ref array<ResourceName> resources = new array<ResourceName>;
		Workbench.SearchResources(resources.Insert, {"et"});
		int resourcesCount = resources.Count();
		
		string prefabPath;
		array<string> toRebuild = new array<string>;
		
		api.BeginEntityAction();
		foreach (int i, ResourceName prefab: resources)
		{
			progress.SetProgress(i / resourcesCount);
			
			prefabPath = prefab.GetPath();
			if (!prefabPath.StartsWith(targetPath)) continue;
			
			prefabPath = config.VerifyEditablePrefab(api, prefab, m_OnlyFileChanges);
			if (!prefabPath.IsEmpty()) toRebuild.Insert(prefabPath);
		}
		api.EndEntityAction();
		resources.Clear();
		
		resourceManager.RebuildResourceFiles(toRebuild, "PC");
		
		DeleteEmptyDirectories(targetPath);
		
		worldEditor.Save(); //--- Save to world so it's not marked as edited
		
		Print(string.Format("Updating editable entity prefabs FINISHED at '%1'", targetPath), LogLevel.DEBUG);
	}
	[ButtonAttribute("Run")]
	bool ButtonRun()
	{
		return true;
	}
	[ButtonAttribute("Close")]
	bool ButtonClose()
	{
		return false;
	}
};

class ConfirmationDialogWorkbench
{
	private bool m_bResult;
	
	bool GetResult()
	{
		return m_bResult;
	}
	
	[ButtonAttribute("OK")]
	bool ButtonOK()
	{
		m_bResult = true;
		return false;
	}
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
};
