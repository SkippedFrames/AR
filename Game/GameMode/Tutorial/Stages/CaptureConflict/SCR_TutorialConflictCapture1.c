[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 10;
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		if (!m_TutorialComponent)
			return;
		
		m_TutorialComponent.StageReset_ResetSeizing();
	}
};
