//------------------------------------------------------------------------------------------------
class BetaWarningDialogUI: ChimeraMenuBase
{
	[MenuBindAttribute()]
	ButtonWidget ConfirmButton;
	
	//------------------------------------------------------------------------------------------------
	[MenuBindAttribute()]
	void ConfirmButton()
	{
		CloseDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	void CloseDialog()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.MainMenu);
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.MainMenu);
		GetGame().GetInputManager().AddActionListener("MenuSelect", EActionTrigger.PRESSED, CloseDialog);
	}
};


