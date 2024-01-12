//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskExtractClass: SCR_ScenarioFrameworkTaskAreaClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskExtract : SCR_ScenarioFrameworkTaskArea
{	
	//------------------------------------------------------------------------------------------------
	override void OnTriggerActivated()
	{
		//TODO: task related things
		m_SupportEntity.FinishTask(this);	
	}
}
