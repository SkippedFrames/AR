[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkLayerTaskDestroyClass : SCR_ScenarioFrameworkLayerTaskClass
{
	// prefab properties here
};

class SCR_ScenarioFrameworkLayerTaskDestroy : SCR_ScenarioFrameworkLayerTask
{	
	
	protected ref EntitySpawnParams m_SpawnParams = new EntitySpawnParams;
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDestroySupportEntity))
		{
			Print("ScenarioFramework: Task Destroy support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}
		m_SupportEntity = SCR_ScenarioFrameworkTaskDestroySupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDestroySupportEntity));
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLayerTaskDestroy(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = SCR_ESFTaskType.DESTROY;
	}	
};
