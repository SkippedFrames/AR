[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical6Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical6: SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_Ambulance;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;

		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		RegisterWaypoint("WP_MEDICAL_4");
		
		m_Ambulance = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("Ambulance"));
		m_TutorialComponent.SetWaypointMiscImage("GETOUT", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Ambulance && !m_Ambulance.GetPilot();
	}
};
