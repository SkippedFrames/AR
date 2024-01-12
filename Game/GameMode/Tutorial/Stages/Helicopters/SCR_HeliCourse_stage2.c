[EntityEditorProps(insertable: false)]
class SCR_HeliCourse_stage2Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HeliCourse_stage2 : SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_Helicopter;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("UH1COURSE");
		
		m_fWaypointHeightOffset = 5;
		m_bCheckWaypoint = false;
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	
		m_Helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		m_TutorialComponent.SetWaypointMiscImage("GETIN", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Helicopter)
			return false;
		
		return m_Helicopter.GetPilot() == m_Player;
	}
};
