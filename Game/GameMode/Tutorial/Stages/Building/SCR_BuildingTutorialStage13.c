[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage13Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage13: SCR_BaseCampaignTutorialArlandStage
{
	protected IEntity m_Barricade;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_Barricade = GetGame().GetWorld().FindEntityByName("Barricade");
		RegisterWaypoint(m_Barricade);
		m_bCheckWaypoint = false;
		m_TutorialComponent.SetWaypointMiscImage("CUSTOM", true);
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_Barricade;
	}
};
