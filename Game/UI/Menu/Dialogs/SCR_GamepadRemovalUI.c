//------------------------------------------------------------------------------------------------
//! Dialog displayed when the gamepad is removed
class SCR_GamepadRemovalUI : DialogUI
{
	protected const string GAMEPAD_ANY_BUTTON_ACTION = "GamepadAnyButton";
	
	protected static ref ScriptInvokerDialog m_OnGamepadRemovalDialogOpen;
	protected static ref ScriptInvokerDialog m_OnGamepadRemovalDialogClose;
	
	protected Widget m_wBackground;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
	
		if (GetGame().m_bIsMainMenuOpen)
		{
			// Default background opacity is 0.8. In Core Menus, we want it to be fully opaque
			m_wBackground = GetRootWidget().FindAnyWidget("BackgroundRounded");
			if (m_wBackground)
				m_wBackground.SetOpacity(1);
		}
		
		GetGame().GetInputManager().AddActionListener(GAMEPAD_ANY_BUTTON_ACTION, EActionTrigger.PRESSED, Close);
		
		if (m_OnGamepadRemovalDialogOpen)
			m_OnGamepadRemovalDialogOpen.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose() 
	{	
		super.OnMenuClose();
		
		GetGame().GetInputManager().RemoveActionListener(GAMEPAD_ANY_BUTTON_ACTION, EActionTrigger.PRESSED, Close);
		
		if (m_OnGamepadRemovalDialogClose)
			m_OnGamepadRemovalDialogClose.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_GamepadRemovalUI OpenGamepadRemovalDialog()
	{
		MenuBase dialog;
		MenuManager menuManager;
		ArmaReforgerScripted game = GetGame();
		if (game)
			menuManager = game.GetMenuManager();

		if (menuManager && game.IsPlatformGameConsole())
			dialog = menuManager.OpenDialog(ChimeraMenuPreset.GamepadRemovalDialog, DialogPriority.CRITICAL, 0, true);
		
		return SCR_GamepadRemovalUI.Cast(dialog);
		
		//TODO: What happens during interaction with input fields, if the controller is removed while the console keyboard is displayed?
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerDialog GetOnGamepadRemovalDialogOpen()
	{
		if (!m_OnGamepadRemovalDialogOpen)
			m_OnGamepadRemovalDialogOpen = new ScriptInvokerDialog();

		return m_OnGamepadRemovalDialogOpen;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerDialog GetOnGamepadRemovalDialogClose()
	{
		if (!m_OnGamepadRemovalDialogClose)
			m_OnGamepadRemovalDialogClose = new ScriptInvokerDialog();

		return m_OnGamepadRemovalDialogClose;
	}
}
