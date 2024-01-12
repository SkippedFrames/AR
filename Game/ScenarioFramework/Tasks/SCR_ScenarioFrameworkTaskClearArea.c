//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskClearAreaClass: SCR_ScenarioFrameworkTaskAreaClass
{
};


//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskClearArea : SCR_ScenarioFrameworkTaskArea
{	
	//------------------------------------------------------------------------------------------------
	override void OnTriggerActivated()
	{
		m_SupportEntity.FinishTask(this);	
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		m_SupportEntity = SCR_ScenarioFrameworkTaskClearAreaSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskClearAreaSupportEntity));
		
		if (!m_SupportEntity)
		{
			Print("ScenarioFramework: Task Clear area support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		return true;
	}
};
