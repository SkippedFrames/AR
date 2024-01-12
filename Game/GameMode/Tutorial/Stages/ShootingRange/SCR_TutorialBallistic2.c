[EntityEditorProps(insertable: false)]
class SCR_TutorialBallistic2Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

class SCR_TutorialBallistic2: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 10;
   		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		//#AR-Tutorial_ShootingRange_BalisticsProtection
		//BALLISTIC_PROTECTION
	}
};
