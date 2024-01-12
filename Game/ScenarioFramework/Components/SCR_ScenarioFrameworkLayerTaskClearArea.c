[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkLayerTaskClearAreaClass : SCR_ScenarioFrameworkLayerTaskClass
{
	// prefab properties here
};

class SCR_ScenarioFrameworkLayerTaskClearArea : SCR_ScenarioFrameworkLayerTask
{	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskClearAreaSupportEntity))
		{
			Print("ScenarioFramework: Task Destroy support entity not found in the world, task won't be created!");
			return false;
		}
		m_SupportEntity = SCR_ScenarioFrameworkTaskClearAreaSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskClearAreaSupportEntity));
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLayerTaskClearArea(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = SCR_ESFTaskType.CLEAR_AREA;
	}
};
