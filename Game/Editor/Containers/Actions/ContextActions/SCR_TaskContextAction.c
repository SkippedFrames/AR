[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_TaskContextAction : SCR_SelectedEntitiesContextAction
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Complete", "0", ""), ParamEnum("Fail", "1", "") }, desc: "State to which the task will be set.")]
	protected int m_iTaskState;
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity && selectedEntity.GetEntityType() == EEditableEntityType.TASK && GetTaskManager();
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (!GetTaskManager())
			return;
		
		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
		if (!supportEntity)
			return;
		
		SCR_BaseTask task = SCR_BaseTask.Cast(selectedEntity.GetOwner());
		if (!task)
			return;
		
		switch (m_iTaskState)
		{
			//--- Complete
			case 0:
			{
				supportEntity.FinishTask(task);
				break;
			}
			//--- Fail
			case 1:
			{
				supportEntity.FailTask(task);
				break;
			}
		}
	}
};
