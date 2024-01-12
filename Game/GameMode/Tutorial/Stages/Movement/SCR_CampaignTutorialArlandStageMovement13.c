[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMovement13Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMovement13 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_LADDER_PLATFORM");
		m_fWaypointCompletionRadius = 3;
	}
};
