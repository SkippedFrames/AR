[EntityEditorProps(insertable: false)]
class SCR_HeliCourse_stage1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HeliCourse_stage1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_TutorialComponent.StageReset_Helicopter();
		
		m_fDuration = 10;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
};
