//--- General Workbench functions

class SCR_WorkbenchSearchResourcesCallbackArray
{
	protected array <ResourceName> m_Selection;
	
	void SCR_WorkbenchSearchResourcesCallbackArray(array <ResourceName> selection)
	{
		m_Selection = selection;
	}
	
	void Insert(ResourceName resName, string filePath)
	{
		m_Selection.Insert(resName);
	}
};
