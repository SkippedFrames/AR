[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture7Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture7 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("TownBaseBeauregard");
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));

		m_fWaypointCompletionRadius = 20;
		m_fWaypointHeightOffset = 0;
	}
};
