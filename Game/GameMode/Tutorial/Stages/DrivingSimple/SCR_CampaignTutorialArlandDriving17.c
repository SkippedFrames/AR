[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDriving17Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDriving17 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_44");
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		m_TutorialComponent.SetWaypointMiscImage("RIGHT", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};
