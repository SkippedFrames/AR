[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture3Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture3 : SCR_BaseCampaignTutorialArlandStage
{
	protected bool m_bTaskMenuOpened;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 5;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));

		GetGame().GetInputManager().AddActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bTaskMenuOpened;
	}

	//------------------------------------------------------------------------------------------------
	void RegisterTasksShown()
	{
		m_bTaskMenuOpened = true;
		GetGame().GetInputManager().RemoveActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
	}
};
