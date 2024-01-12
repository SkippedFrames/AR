[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Component", true)]
class EditablePrefabsComponent_Base
{
	[Attribute(params: "ct")]
	private ResourceName m_Component;
	
	private string m_sClassname;
	
	string GetClassName()
	{
		return m_sClassname;
	}
	
	IEntityComponentSource AddComponent(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource)
	{
		return api.CreateComponent(entitySource, m_Component);
	}
	void EOnCreate(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource, IEntitySource instanceEntitySource, IEntityComponentSource componentSource, IEntityComponentSource componentCurrent);
	void EOnDelete(EditablePrefabsConfig config, WorldEditorAPI api, string prefabPath);
	void EOnMove(EditablePrefabsConfig config, WorldEditorAPI api, string currentPath, string newPath);
	
	void EditablePrefabsComponent_Base()
	{
		if (m_Component.IsEmpty()) return;
		
		//--- Misconfigured
		Resource componentResource = Resource.Load(m_Component);
		if (!componentResource.IsValid()) return;
		
		BaseResourceObject componentBase = componentResource.GetResource();
		if (!componentBase) return;
		
		BaseContainer componentContainer = componentBase.ToBaseContainer();
		if (!componentContainer) return;
		
		m_sClassname = componentContainer.GetClassName();
	}
};
