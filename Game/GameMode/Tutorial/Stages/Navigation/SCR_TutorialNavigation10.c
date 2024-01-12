[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation10Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation10 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fConditionCheckPeriod = 1;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		float angle = Math.RAD2DEG * m_Player.GetCharacterController().GetInputContext().GetAimingAngles()[0];
		return (Math.AbsFloat(angle) < 3);
	}
};
