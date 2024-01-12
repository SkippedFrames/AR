[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation18Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation18 : SCR_BaseCampaignTutorialArlandStage
{	
	IEntity m_Waypoint;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_Waypoint = GetGame().GetWorld().FindEntityByName("WP_GREENHOUSE");
		if (!m_Waypoint)
			return;
		
		m_bShowWaypoint = false;
		
		SCR_MapDescriptorComponent descriptor = SCR_MapDescriptorComponent.Cast(m_Waypoint.FindComponent(SCR_MapDescriptorComponent));
		descriptor.Item().SetVisible(true);
		
		RegisterWaypoint(m_Waypoint);
		
		m_fWaypointCompletionRadius = 50;
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
};
