[WorkbenchPluginAttribute(name: "Maintain Editable Entities", category: "In-game Editor", wbModules: {"WorldEditor", "ResourceManager"}, awesomeFontCode: 0xf0ad)]
class EditableEntityMaintenancePlugin: WorkbenchPlugin
{
	[Attribute("ArmaReforger")]
	private string m_sAddon;
	
	[Attribute("PrefabsEditable/Auto", params: "unregFolders")]
	private ResourceName m_sDirectory;
	
	//[Attribute("MeshObject RigidBody RplComponent Hierarchy SCR_DestructionMultiPhaseComponent")]
	//private ref array<string> m_aCheckedComponents;
	
	const ref array<string> UNIQUE_COMPONENTS = { "MeshObject", "RigidBody", "RplComponent", "Hierarchy", "SCR_DestructionMultiPhaseComponent" };
	
	override void Run()
	{
		string uniqueComponentClasses;
		for (int c = 0, count = UNIQUE_COMPONENTS.Count(); c < count; c++)
		{
			if (c > 0)
				uniqueComponentClasses += "\n";
			uniqueComponentClasses += "  - " + UNIQUE_COMPONENTS[c];
		}
		
		if (!Workbench.ScriptDialog("Maintain Editable Entities", string.Format("Go through all entity prefabs in the folder and remove duplicate components of following types:\n%1", uniqueComponentClasses), this))
			return;
		
		ref array<ResourceName> resources = new array<ResourceName>;
		string path = SCR_AddonTool.ToFileSystem(m_sAddon) + m_sDirectory.GetPath();
		Workbench.SearchResources(resources.Insert, {"et"}, null, path);
		
		int repairedCount;
		IEntitySource entitySource, ancestorSource;
		BaseContainerList componentsArray, ancestorComponentsArray;
		array<BaseContainer> ancestorComponents = {};
		array<BaseContainer> componentsToRemove = {};
		array<string> componentClasses = {};
		string componentClass;
		IEntityComponentSource component;
		foreach (ResourceName prefab: resources)
		{
			entitySource = Resource.Load(prefab).GetResource().ToEntitySource();
			
			componentsToRemove.Clear();
			componentClasses.Clear();
			
			//--- Get ancestor components
			ancestorSource = entitySource.GetAncestor();
			if (ancestorSource)
			{
				ancestorComponentsArray = ancestorSource.GetObjectArray("components");
				for (int c = 0, count = ancestorComponentsArray.Count(); c < count; c++)
				{
					ancestorComponents.Insert(ancestorComponentsArray.Get(c));
				}
			}
			
			//--- Get prefab's components
			componentsArray = entitySource.SetObjectArray("components");
			int componentCount = componentsArray.Count();
			for (int c = 0; c < componentCount; c++)
			{
				component = componentsArray.Get(c);
				componentClass = component.GetClassName();
				
				if (componentClasses.Contains(componentClass) && UNIQUE_COMPONENTS.Contains(componentClass) && !ancestorComponents.Contains(component))
				{
					//--- Duplicate component which is on the list of checked components, and is also defined in this prefab and not in ancestor
					componentsToRemove.Insert(component);
				}
				else
				{
					//--- Not duplicate
					componentClasses.Insert(componentClass);
				}
			}
			
			if (!componentsToRemove.IsEmpty())
			{
				foreach (BaseContainer componentToRemove: componentsToRemove)
				{
					Print(string.Format("@\"%1\": Removed duplicate component %2", entitySource.GetResourceName().GetPath(), componentToRemove.GetClassName()), LogLevel.WARNING);
					componentsArray.Remove(componentToRemove);
				}
				BaseContainerTools.SaveContainer(entitySource, prefab);
				repairedCount++;
			}
		}
		PrintFormat("%1 prefabs checked, %2 repaired.", resources.Count(), repairedCount);
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
}
