[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical8Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical8: SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_InspectCasualtyWidget m_CasualtyInspectDisplay;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		array<BaseInfoDisplay> infoDisplays = {};
		GetGame().GetPlayerController().GetHUDManagerComponent().GetInfoDisplays(infoDisplays);
		foreach (BaseInfoDisplay baseInfoDisplays : infoDisplays)
		{
			m_CasualtyInspectDisplay = SCR_InspectCasualtyWidget.Cast(baseInfoDisplays);
			if (m_CasualtyInspectDisplay)
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{		
		if (!m_CasualtyInspectDisplay)
			return false;

		return m_CasualtyInspectDisplay.IsActive();
	}
};
