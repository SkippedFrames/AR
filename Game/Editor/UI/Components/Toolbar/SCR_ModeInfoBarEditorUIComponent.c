class SCR_ModeInfoBarEditorUIComponent: ScriptedWidgetComponent
{
	[Attribute("OpenPauseMenuButton")]
	protected string m_sPauseMenuButtonName;
	
	[Attribute("CloseEditorButton")]
	protected string m_sCloseEditorButtonName;
	
	
	protected void OpenPauseMenuButton(Widget widget, float value, EActionTrigger actionTrigger)
	{
		if (!GetGame().GetMenuManager().IsAnyDialogOpen())
			ArmaReforgerScripted.OpenPauseMenu();
	}
	
	protected void CloseEditorButton(Widget widget, float value, EActionTrigger actionTrigger)
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		
		if (editorManagerEntity)
			editorManagerEntity.Close();
	}
	
	
	override void HandlerAttached(Widget w)
	{
		Widget buttonWidget = w.FindAnyWidget(m_sPauseMenuButtonName);
		if (buttonWidget)
			ButtonActionComponent.GetOnAction(buttonWidget).Insert(OpenPauseMenuButton);
		
		buttonWidget = w.FindAnyWidget(m_sCloseEditorButtonName);
		if (buttonWidget)
			ButtonActionComponent.GetOnAction(buttonWidget).Insert(CloseEditorButton);
	}
	
};
