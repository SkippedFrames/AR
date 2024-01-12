[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStage119Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStage119 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_42");
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;
		SCR_HintManagerComponent.ShowCustomHint("Continue right after the obstacle and to the road behind the church.", duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};
