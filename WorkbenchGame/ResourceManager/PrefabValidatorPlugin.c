[WorkbenchPluginAttribute(name: "Validate Prefabs", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf5bf)]
class PrefabValidatorPlugin: WorkbenchPlugin
{
	const ref array<string> UNIQUE_COMPONENTS = { "SCR_DestructionMultiPhaseComponent", "MeshObject", "RigidBody", "RplComponent", "Hierarchy" };
	const ref array<string> REPLICATED_COMPONENTS = { "RplComponent", "Hierarchy" };
	
	protected void ValidatePrefabs(notnull array<ResourceName> prefabs)
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		WBProgressDialog progress = new WBProgressDialog("Validating Prefabs...", resourceManager);
		
		int count = prefabs.Count();
		int countInvalid;
		string message;
		ResourceName prefab;
		Resource prefabResource;
		IEntitySource prefabEntity;
		for (int i; i < count; i++)
		{
			prefab = prefabs[i];
			
			message = string.Format("@\"%1\"", prefab.GetPath());
			
			prefabResource = Resource.Load(prefab);
			if (prefabResource.IsValid())
			{
				prefabEntity = prefabResource.GetResource().ToEntitySource();
				if (prefabEntity)
				{
					if (!ValidateEntitySource(prefabEntity, message))
						countInvalid++;
				}
			}
			
			progress.SetProgress(i / count);
		}
		
		PrintFormat("%1 prefab(s) validated, %2 invalid ones found.", count, countInvalid);
	}
	protected bool ValidateEntitySource(IEntitySource entitySource, string message, int isReplicated = -1)
	{
		bool valid = true;
		
		//--- Find duplicate components
		array<int> componentCounts = {};
		for (int i, count = UNIQUE_COMPONENTS.Count(); i < count; i++)
		{
			componentCounts.Insert(0);
		}
		
		bool isReplicatedCompatible;
		IEntityComponentSource componentSource;
		string componentSourceClassName;
		for (int i, count = entitySource.GetComponentCount(); i < count; i++)
		{
			componentSource = entitySource.GetComponent(i);
			componentSourceClassName = componentSource.GetClassName();
			
			//--- Check if the prefab is replicated (runs once on root entity)
			if (isReplicated == -1 && componentSourceClassName == "RplComponent")
			{
				bool enabled;
				componentSource.Get("Enabled", enabled);
				if (enabled)
					isReplicated = 1;
			}
			
			//--- Check if child entity in replicated prefab has required components
			if (isReplicated == 1 && REPLICATED_COMPONENTS.Contains(componentSourceClassName))
				isReplicatedCompatible = true;
			
			//--- Check if some unqiue components are not duplicated
			int uniqueIndex = UNIQUE_COMPONENTS.Find(componentSourceClassName);
			if (uniqueIndex >= 0)
			{
				bool enabled;
				componentSource.Get("Enabled", enabled);
				if (enabled)
					componentCounts[uniqueIndex] = componentCounts[uniqueIndex] + 1;
			}
		}
		
		//--- No RplComponent detect in root, mark as not replicated
		if (isReplicated == -1)
			isReplicated = 0;
		
		//--- Show error when duplicates were found
		foreach (int componentIndex, int componentCount: componentCounts)
		{
			if (componentCount > 1)
			{
				Print(string.Format("Duplicate component '%1' in prefab %2!", UNIQUE_COMPONENTS[componentIndex], message), LogLevel.ERROR);
				valid = false;
			}
		}
		
		//--- Show errir when the root is replicated and of of the children is missing required component
		if (isReplicated && !isReplicatedCompatible)
		{
			Print(string.Format("Entity %1 is in replicated prefab, but is missing 'RplComponent' or 'Hierarchy'!", message), LogLevel.ERROR);
			valid = false;
		}
		
		//--- Scan children
		IEntitySource child;
		for (int i, count = entitySource.GetNumChildren(); i < count; i++)
		{
			child = entitySource.GetChild(i);
			valid &= ValidateEntitySource(child, message + string.Format("[%1]", i, child.GetClassName()), isReplicated);
		}
		return valid;
	}
	
	override void Run()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		
		array<ResourceName> selection = {};
		SCR_WorkbenchSearchResourcesCallbackArray context = new SCR_WorkbenchSearchResourcesCallbackArray(selection);
		resourceManager.GetResourceBrowserSelection(context.Insert, true);
		
		ValidatePrefabs(selection);
	}
	// -wbmodule=ResourceManager -plugin=PrefabValidatorPlugin -run -validatePaths="$ArmaReforger:AI,$ArmaReforger:PrefabsEditable/Auto/Props"
	override void RunCommandline()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		string pathsCLI;
		resourceManager.GetCmdLine("-validatePaths", pathsCLI);
		
		array<string> paths = {};
		pathsCLI.Split(",", paths, true);
				
		array<ResourceName> prefabs = {};
		foreach (string path: paths)
		{
			Workbench.SearchResources(prefabs.Insert, {"et"}, null, path);
		}
		
		ValidatePrefabs(prefabs);
		
		Workbench.Exit(0);
	}
};
