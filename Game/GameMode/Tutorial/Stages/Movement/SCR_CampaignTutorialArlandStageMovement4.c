[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMovement4Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMovement4 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 0.5;
		m_fConditionCheckPeriod = 0.1;
		RegisterWaypoint("WP_OVERPASS");
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
};
