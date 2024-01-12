[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_TaskCompleteDescriptionTooltipDetail: SCR_EntityTooltipDetail
{
	[Attribute()]
	protected LocalizedString m_sManualCompleteOnlyText;
	
	[Attribute(defvalue: "255,255,255,255", desc: "Color of images within the notification message")]
	protected ref Color m_cManualCompleteColor;
	
	protected SCR_EditorTask m_Task;
	protected RichTextWidget m_Text;
	
	protected ref Color m_cDefaultColor;

	
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (m_Task.GetTaskCompletionType() == EEditorTaskCompletionType.MANUAL || m_Task.GetTaskCompletionType() == EEditorTaskCompletionType.ALWAYS_MANUAL)
		{
			m_Text.SetText(m_sManualCompleteOnlyText);
			m_Text.SetColor(m_cManualCompleteColor);
		}
		else 
		{
			m_Text.SetText(m_Task.GetDescription());
			m_Text.SetColor(m_cDefaultColor);
		}
	}
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = RichTextWidget.Cast(widget);
		if (!m_Text)
			return false;
		
		m_Task = SCR_EditorTask.Cast(entity.GetOwner());
		if (!SCR_EditorTask)
			return false;
		
		if (m_Task.GetTaskCompletionType() != EEditorTaskCompletionType.MANUAL && m_Task.GetTaskCompletionType() != EEditorTaskCompletionType.ALWAYS_MANUAL)
		{
			//~ If the same as description do not show
			if (entity.GetInfo().GetDescription() == m_Task.GetDescription())
				return false;
		}

		m_cDefaultColor = m_Text.GetColor();
		
		return true;
	}
};
