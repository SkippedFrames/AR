[WorkbenchPluginAttribute(name: "Wrap Entities", shortcut: "", wbModules: {"WorldEditor"}, awesomeFontCode: 0xf1b2)]
class WrapEntitiesPlugin: WorkbenchPlugin
{
	[Attribute(params: "et")]
	protected ResourceName m_EntityPrefab;
	
	[Attribute(defvalue: "GenericEntity")]
	protected string m_PrefabClass;
	
	override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		
		int selectedCount = api.GetSelectedEntitiesCount();
		if (selectedCount == 0)
			return;
		
		IEntity pivotEntity = api.GetSelectedEntity(selectedCount - 1);
		
		string entityClass;
		if (!m_EntityPrefab.IsEmpty())
			entityClass = m_EntityPrefab;
		else if (!m_PrefabClass.IsEmpty())
			entityClass = m_PrefabClass;
		else
			entityClass = "GenericEntity";
		
		api.BeginEntityAction("WrapEntitiesPlugin");
		IEntity parent = api.CreateEntity(entityClass, "", api.GetCurrentEntityLayerId(), null, pivotEntity.GetOrigin(), pivotEntity.GetAngles());
		for (int i = 0; i < selectedCount; i++)
		{
			api.ParentEntity(parent, api.GetSelectedEntity(i), true);
		}
		api.EndEntityAction();
	}
	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Wrap Entities' plugin", "", this);
	}
	[ButtonAttribute("Close")]
	bool ButtonClose()
	{
	}
};
