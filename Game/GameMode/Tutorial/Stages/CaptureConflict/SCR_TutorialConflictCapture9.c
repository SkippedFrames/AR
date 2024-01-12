[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture9Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture9 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 20;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
				
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (campaign)
			campaign.SetIsTutorial(false);
		
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
				
		if (hudManager)
		{
			SCR_XPInfoDisplay display = SCR_XPInfoDisplay.Cast(hudManager.FindInfoDisplay(SCR_XPInfoDisplay));
			
			if (display)
				display.AllowShowingInfo(true);
		}
	}
};
