[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical2Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical2: SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_Ambulance;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("Ambulance");
		
		m_fWaypointHeightOffset = 5;
		m_bCheckWaypoint = false;
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	
		m_Ambulance = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("Ambulance"));
		m_TutorialComponent.SetWaypointMiscImage("GETIN", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Ambulance && m_Ambulance.GetPilot() == m_Player;
	}
};
