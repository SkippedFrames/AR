[WorkbenchPluginAttribute(name: "Find Linked Resources", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf0c1)]
class FindResourcesPlugin1: ResourceManagerPlugin
{
	[Attribute(desc: "File extensions, divided by comma, e.g.: edds,imageset")]
	protected string m_sExtension;
	
	override void Run()
	{
		if (!Workbench.ScriptDialog("Find Linked Resources", "Scan all selected file and print out resource of given extension(s)\nlinked to from attributes.", this))
			return;
		
		array<string> uiWidgets = {"resourcePickerSimple", "resourcePickerThumbnail", "fileNamePicker"};
		
		m_sExtension.ToLower();
		array<string> extensions = {};
		m_sExtension.Split(",", extensions, true);
		
		array<ResourceName> selection = {};
		SCR_WorkbenchSearchResourcesCallbackArray context = new SCR_WorkbenchSearchResourcesCallbackArray(selection);
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		resourceManager.GetResourceBrowserSelection(context.Insert, true);
		WBProgressDialog progress = new WBProgressDialog("Processing...", resourceManager);
		
		Resource resource;
		array<BaseContainer> containers = {};
		for (int i, count = selection.Count(); i < count; i++)
		{
			resource = Resource.Load(selection[i]);
			if (resource.IsValid())
				containers.Insert(resource.GetResource().ToBaseContainer());
		}
		
		array<string> resources = {};
		BaseContainer container;
		string varName, varExtension, uiWidget;
		ResourceName varValue;
		while (!containers.IsEmpty())
		{
			container = containers[0];
			containers.RemoveOrdered(0);
			
			for (int i, count = container.GetNumVars(); i < count; i++)
			{
				varName = container.GetVarName(i);
				if (!container.IsVariableSetDirectly(varName))
				
				if (container.Get(varName, varValue))
				{
					FilePath.StripExtension(varValue, varExtension);
					varExtension.ToLower();
					if (extensions.Contains(varExtension))
					{
						varValue = varValue.GetPath();
						if (!resources.Contains(varValue))
							resources.Insert(varValue);
					}
				}
				
				BaseContainer object = container.GetObject(varName);
				if (object)
				{
					//--- Object, process its variables
					containers.Insert(object);
				}
				else
				{
					//--- Array of objects, process every element
					BaseContainerList list = container.GetObjectArray(varName);
					if (list)
					{
						int index;
						for (int l = 0, listCount = list.Count(); l < listCount; l++)
						{
							containers.Insert(list.Get(l));
						}
					}
				}
			}
		
			//--- Process children
			if (container)
			{
				for (int e = 0, count = container.GetNumChildren(); e < count; e++)
				{
					containers.Insert(container.GetChild(e));
				}
			}
		}
		
		int count = resources.Count();
		PrintFormat("Linked resources of type(s) %1 (%2):", m_sExtension, count);
		resources.Sort();
		for (int i; i < count; i++)
		{
			PrintFormat("@\"%1\"", resources[i]);
		}
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
