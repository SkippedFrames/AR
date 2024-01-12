[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMovement16Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMovement16 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_OBSTACLE_END");
		m_fWaypointCompletionRadius = 2;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		GetGame().GetCallqueue().CallLater(DelayedPopup, 2000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Sprint", 12, "", "", "", "");
	}
};
