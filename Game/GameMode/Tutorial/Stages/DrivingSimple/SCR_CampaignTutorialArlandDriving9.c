[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDriving9Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDriving9 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_36");
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		m_TutorialComponent.SetWaypointMiscImage("SERPENTINE", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};
