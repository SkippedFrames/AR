[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical3Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical3: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 10;
		m_fWaypointHeightOffset = 0;

		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
				
		RegisterWaypoint("WP_MEDICAL_1");
	}
};
