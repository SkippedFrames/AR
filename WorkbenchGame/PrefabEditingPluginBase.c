class PrefabEditingPluginBase: WorkbenchPlugin
{
	protected WBProgressDialog StartProgressWorldEditor()
	{
		return new WBProgressDialog("Processing Prefabs...", Workbench.GetModule(WorldEditor));
	}
	protected WBProgressDialog StartProgressResourceBrowser()
	{
		return new WBProgressDialog("Processing Prefabs...", Workbench.GetModule(ResourceManager));
	}
	protected WBProgressDialog StartProgress()
	{
		return StartProgressWorldEditor();
	}
	protected void GetSelectedWorldEditor(out array<ResourceName> selection)
	{
		SCR_WorkbenchSearchResourcesCallbackArray context = new SCR_WorkbenchSearchResourcesCallbackArray(selection);
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		worldEditor.GetResourceBrowserSelection(context.Insert, true);
	}
	protected void GetSelectedResourceBrowser(out array<ResourceName> selection)
	{
		SCR_WorkbenchSearchResourcesCallbackArray context = new SCR_WorkbenchSearchResourcesCallbackArray(selection);
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		resourceManager.GetResourceBrowserSelection(context.Insert, true);
	}
	protected void GetSelected(out array<ResourceName> selection)
	{
		GetSelectedWorldEditor(selection);
	}
	protected bool GetPrefabs(out notnull array<ResourceName> compatiblePrefabs, bool includeComponentPrefabs = false)
	{
		//--- Get selected prefabs
		array<ResourceName> selectedPrefabs = new array<ResourceName>;
		GetSelected(selectedPrefabs);
		if (selectedPrefabs.Count() == 0)
		{
			Print("No prefab selected!", LogLevel.ERROR);
			return false;
		}
		
		//--- Evaluate selected prefabs
		string extension;
		Resource prefabResource;
		BaseResourceObject prefabBase;
		BaseContainer prefabContainer;
		typename prefabType;
		foreach (ResourceName prefab: selectedPrefabs)
		{
			FilePath.StripExtension(prefab, extension);
			if (includeComponentPrefabs)
			{
				if (extension != "et" && extension != "ct")
				{
					Print(string.Format("Cannot load prefab '%1', it's not a prefab! Only .et and .ct files can be loaded.", prefab), LogLevel.ERROR);
					continue;
				}
			}
			else
			{
				if (extension != "et")
				{
					Print(string.Format("Cannot load prefab '%1', it's not a prefab! Only .et files can be loaded.", prefab), LogLevel.ERROR);
					continue;
				}
			}
		
			prefabResource = Resource.Load(prefab);
			if (!prefabResource.IsValid())
			{
				Print(string.Format("Cannot load prefab '%1'!", prefab), LogLevel.ERROR);
				continue;
			}
			
			prefabBase = prefabResource.GetResource();
			if (!prefabBase)
			{
				Print(string.Format("Cannot load prefab '%1'!", prefab), LogLevel.ERROR);
				continue;
			}
			
			prefabContainer = prefabBase.ToBaseContainer();
			if (!prefabContainer)
			{
				Print(string.Format("Cannot load prefab '%1'!", prefab), LogLevel.ERROR);
				continue;
			}
			
			prefabType = prefabContainer.GetClassName().ToType();
			if (includeComponentPrefabs)
			{
				if (!prefabType.IsInherited(IEntity) && !prefabType.IsInherited(GenericComponent))
				{
					Print(string.Format("Cannot load prefab '%1', it's neither an entity nor a component!", prefab), LogLevel.ERROR);
					continue;
				}
			}
			else
			{
				if (!prefabType.IsInherited(IEntity))
				{
					Print(string.Format("Cannot load prefab '%1', it's not an entity!", prefab), LogLevel.ERROR);
					continue;
				}
			}
			
			compatiblePrefabs.Insert(prefab);
		}
		return !compatiblePrefabs.IsEmpty();
	}
};
