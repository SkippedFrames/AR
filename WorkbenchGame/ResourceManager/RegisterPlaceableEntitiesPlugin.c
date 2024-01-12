[WorkbenchPluginAttribute(name: "Register Placeable Entities...", category: "In-game Editor", wbModules: {"ResourceManager"})]
class RegisterPlaceableEntitiesPlugin : WorkbenchPlugin
{
	[Attribute("", UIWidgets.Auto, "File to which the resulting list will be saved.", "conf")]
	protected ResourceName config;

	void Scan()
	{
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		WBProgressDialog progress = new WBProgressDialog("Scanning...", rm);
		
		//--- Open config
		if (config == "")
		{
			Print("Config file not found!", LogLevel.ERROR);
			return;
		}
		
		Resource configContainer = BaseContainerTools.LoadContainer(config);
		if (!configContainer) return;
			
		SCR_PlaceableEntitiesRegistry registry = SCR_PlaceableEntitiesRegistry.Cast(BaseContainerTools.CreateInstanceFromContainer(configContainer.GetResource().ToBaseContainer()));
		if (!registry)
		{
			Print(string.Format("SCR_PlaceableEntitiesRegistry class not found in '%1'!", config), LogLevel.ERROR);
			return;
		}
		string filesystem = SCR_AddonTool.ToFileSystem(registry.GetAddon());
		string directory = registry.GetSourceDirectory().GetPath();
		
		//--- Get all prefabs
		ref array<ResourceName> resources = new array<ResourceName>;
		Workbench.SearchResources(resources.Insert, {"et"},  null, filesystem + directory);
		
		array<ResourceName> prefabs = new array<ResourceName>;
		
		//--- Iterate through all prefabs and their components
		int resourcesCount = resources.Count();
		foreach (int i, ResourceName prefab: resources)
		{
			Resource container = BaseContainerTools.LoadContainer(prefab);
			if (!container) continue;
			
			BaseResourceObject object = container.GetResource();
			IEntitySource entity = object.ToEntitySource();
			
			//--- Find required components
			int componentCount = entity.GetComponentCount();
			IEntityComponentSource editableEntityComponent = null;
			for (int c = 0; c < componentCount; c++)
			{
				IEntityComponentSource component = entity.GetComponent(c);
				
				if (component.GetClassName().ToType().IsInherited(SCR_EditableEntityComponent))
					editableEntityComponent = component;
			}
			
			//--- Register
			if (editableEntityComponent)
			{
				//--- Check if it's marked as PLACEABLE
				if (!SCR_EditableEntityComponentClass.HasFlag(editableEntityComponent, EEditableEntityFlag.PLACEABLE)) continue;
				
				Print(string.Format("Registered '%1'", prefab), LogLevel.DEBUG);
				prefabs.Insert(prefab);
			}
			progress.SetProgress(i / resourcesCount);
		};
		resources.Clear();
		
		//--- Save config to the file
		registry.SetPrefabs(prefabs);
		Resource container = BaseContainerTools.CreateContainerFromInstance(registry);
		if (container)
		{			
			BaseContainerTools.SaveContainer(container.GetResource().ToBaseContainer(), config);
			rm.SetOpenedResource(config);
			rm.Save();
			Print(string.Format("Entities from '%1' saved to '%2'", directory, config.GetPath()), LogLevel.DEBUG);
		}
	}
	override void Run()
	{
		if (Workbench.ScriptDialog("Register Placeable Entities", "Register all entity prefabs that have SCR_EditableEntityComponent\nwith PLACEABLE flag into target config.\nThe game uses it to get the list of all entities available for placing.\n\nMake sure you don't have the target config open!", this))
		{
			Scan();		
		}
	}
	
	override void RunCommandline() 
	{
		Scan();
		Workbench.Exit(0);
	}
	
	[ButtonAttribute("Run")]
	bool OK()
	{
		return true;
	}
	
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}
};
