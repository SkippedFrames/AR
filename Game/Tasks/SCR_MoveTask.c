[EntityEditorProps(category: "GameScripted/Tasks", description: "Move task.", color: "0 0 255 255")]
class SCR_MoveTaskClass: SCR_EditorTaskClass
{
};
class SCR_MoveTask : SCR_EditorTask
{	
	[Attribute("5")]
	protected float m_fMaxDistance;
	
	void PeriodicalCheck()
	{		
		if (!GetTaskManager())
			return;
		
		SCR_BaseTaskSupportEntity supportEntity = SCR_BaseTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity));
		if (!supportEntity)
			return;
		
		array<SCR_BaseTaskExecutor> assignees = new array<SCR_BaseTaskExecutor>();
		GetAssignees(assignees);
		
		for (int x = assignees.Count() - 1; x >= 0; x--)
		{
			int id = SCR_BaseTaskExecutor.GetTaskExecutorID(assignees[x]);
			IEntity controlledEntity = SCR_PossessingManagerComponent.GetPlayerMainEntity(id);
			
			if (!controlledEntity)
				continue;
			
			float distance = vector.Distance(controlledEntity.GetOrigin(), GetOrigin());
			
			if (distance < m_fMaxDistance)
			{
				supportEntity.FinishTask(this);
				return;
			}
		}
	}
	
	override void SetTaskCompletionType(EEditorTaskCompletionType newTaskCompletionType)
	{
		if (m_iTaskCompletionType == newTaskCompletionType)
			return;
		
		super.SetTaskCompletionType(newTaskCompletionType);
		
		if (GetTaskManager().IsProxy())
			return;
		
		if (m_iTaskCompletionType == EEditorTaskCompletionType.AUTOMATIC)
			SCR_BaseTaskManager.s_OnPeriodicalCheck2Second.Insert(PeriodicalCheck);
		else 
			SCR_BaseTaskManager.s_OnPeriodicalCheck2Second.Remove(PeriodicalCheck);
	}
	
	void SCR_MoveTask(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		if (!GetTaskManager().IsProxy() && m_iTaskCompletionType == EEditorTaskCompletionType.AUTOMATIC)
			SCR_BaseTaskManager.s_OnPeriodicalCheck2Second.Insert(PeriodicalCheck);
	}
	void ~SCR_MoveTask()
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;
		
		if (!GetTaskManager().IsProxy() && m_iTaskCompletionType == EEditorTaskCompletionType.AUTOMATIC)
			SCR_BaseTaskManager.s_OnPeriodicalCheck2Second.Remove(PeriodicalCheck);
	}

};
