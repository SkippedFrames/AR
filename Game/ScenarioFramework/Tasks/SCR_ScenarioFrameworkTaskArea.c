//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskAreaClass: SCR_ScenarioFrameworkTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskArea : SCR_ScenarioFrameworkTask
{	
	protected SCR_BaseTriggerEntity 		m_Trigger;
		
	//------------------------------------------------------------------------------------------------
	override void Init()
	{	
		super.Init();
		m_Trigger = SCR_BaseTriggerEntity.Cast(m_Asset);
		if (!m_Trigger)
		{
			if (!m_SupportEntity)
				return;
				
			m_SupportEntity.CancelTask(this.GetTaskID());
			Print("ScenarioFramework: Trigger not found! Make sure the IDs match in both task and Slot component", LogLevel.WARNING);
	 		return;
		}
		m_Trigger.GetOnActivate().Insert(OnTriggerActivated);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTriggerActivated()
	{
		m_SupportEntity.FinishTask(this);	
	}	
}
