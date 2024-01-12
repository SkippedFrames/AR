[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced19Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced19 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVINGHEAVY_17");
		m_fWaypointCompletionRadius = 10;
		m_TutorialComponent.SetWaypointMiscImage("RIGHT", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};
