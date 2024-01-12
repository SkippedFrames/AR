[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDriving10Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDriving10 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_37");
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
