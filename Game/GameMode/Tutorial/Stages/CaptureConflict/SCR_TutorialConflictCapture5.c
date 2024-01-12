[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture5Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture5 : SCR_BaseCampaignTutorialArlandStage
{
	SCR_CampaignTask m_Task;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		if (!GetTaskManager())
			return;
	
		SCR_CampaignMilitaryBaseComponent baseBeauregard = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseBeauregard").FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!baseBeauregard)
			return;
	
		SCR_CampaignTaskSupportEntity supportClass = SCR_CampaignTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignTaskSupportEntity));
		if (!supportClass)
			return;
		
		m_Task = supportClass.GetTask(baseBeauregard, SCR_GameModeCampaign.GetInstance().GetFactionByEnum(SCR_ECampaignFaction.BLUFOR), SCR_CampaignTaskType.CAPTURE);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_Task)
			return (m_Task.GetAssignee() == SCR_BaseTaskExecutor.GetLocalExecutor());
		else
			return true;
	}
};
