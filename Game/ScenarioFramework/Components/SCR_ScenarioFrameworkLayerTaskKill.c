[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkLayerTaskKillClass : SCR_ScenarioFrameworkLayerTaskClass
{
	// prefab properties here
}

class SCR_ScenarioFrameworkLayerTaskKill : SCR_ScenarioFrameworkLayerTask
{	
	
	protected ref EntitySpawnParams m_SpawnParams = new EntitySpawnParams;
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskKillSupportEntity))
		{
			Print("ScenarioFramework: Task Kill support entity not found in the world, task won't be created!");
			return false;
		}
		m_SupportEntity = SCR_ScenarioFrameworkTaskKillSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskKillSupportEntity));
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLayerTaskKill(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = SCR_ESFTaskType.KILL;
	}	
}
