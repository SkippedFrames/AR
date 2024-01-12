[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture10Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture10 : SCR_BaseCampaignTutorialArlandStage
{
	Vehicle m_MHQ;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("MobileHQ");
		
		m_fWaypointHeightOffset = 5;
		m_bCheckWaypoint = false;
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	
		m_MHQ = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("MobileHQ"));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_MHQ)
			return false;
		
		return m_MHQ.GetPilot() == m_Player;
	}
};
