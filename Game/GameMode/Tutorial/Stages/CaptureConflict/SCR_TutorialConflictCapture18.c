[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture18Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture18 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 15;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
};
