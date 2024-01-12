/** @ingroup Editor_UI Editor_UI_Menus
*/
class EditorMenuUI: EditorMenuBase
{
	protected static void OnShowPlayerList()
	{
		ArmaReforgerScripted.OpenPlayerList();
	}
	protected void OpenPauseMenu()
	{
		if (!GetGame().GetMenuManager().IsAnyDialogOpen() && IsFocused())
			ArmaReforgerScripted.OpenPauseMenu();
	}
	
	override void OnMenuInit()
	{
		super.OnMenuInit();
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("ShowScoreboard", EActionTrigger.DOWN, OnShowPlayerList);
			inputManager.AddActionListener("MenuOpen", EActionTrigger.DOWN, OpenPauseMenu);
			inputManager.AddActionListener("ChatToggle", EActionTrigger.DOWN, ChatToggle);
#ifdef WORKBENCH
			inputManager.AddActionListener("MenuOpenWB", EActionTrigger.DOWN, OpenPauseMenu);
#endif
		}
	}
	
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, OnShowPlayerList);
			inputManager.RemoveActionListener("MenuOpen", EActionTrigger.DOWN, OpenPauseMenu);
			inputManager.RemoveActionListener("ChatToggle", EActionTrigger.DOWN, ChatToggle);
#ifdef WORKBENCH
			inputManager.RemoveActionListener("MenuOpenWB", EActionTrigger.DOWN, OpenPauseMenu);
#endif
		}
	}
	
	void ChatToggle()
	{
		if (!m_ChatPanel)
			return;
		
		if (!m_ChatPanel.IsOpen())
			SCR_ChatPanelManager.GetInstance().OpenChatPanel(m_ChatPanel);
	}
};
