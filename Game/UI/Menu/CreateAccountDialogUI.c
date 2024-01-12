//------------------------------------------------------------------------------------------------
class CreateAccountDialogUI: DialogUI
{
	SCR_EditBoxComponent m_UserName;
	SCR_EditBoxComponent m_Password;
	SCR_EditBoxComponent m_ConfirmPassword;
	Widget m_wOverlay;
	int m_iDelayMs = 500;
	SCR_LoadingOverlay m_LoadingOverlay;
	
	string m_sPasswordNotMatching = "Passwords do not match";
	
	ref ScriptInvoker m_OnDialogClosed = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_UserName = SCR_EditBoxComponent.GetEditBoxComponent("UserName", GetRootWidget());
		if (m_UserName)
			m_UserName.m_OnChanged.Insert(CheckFilledEditboxes);

		m_Password = SCR_EditBoxComponent.GetEditBoxComponent("Password", GetRootWidget());
		if (m_Password)
			m_Password.m_OnChanged.Insert(CheckFilledEditboxes);

		m_ConfirmPassword = SCR_EditBoxComponent.GetEditBoxComponent("ConfirmPassword", GetRootWidget());
		if (m_ConfirmPassword)
			m_ConfirmPassword.m_OnChanged.Insert(CheckFilledEditboxes);

		m_Confirm.SetEnabled(false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		ShowLoadinAnim(false);
		m_OnDialogClosed.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		if (m_Password.GetValue() != m_ConfirmPassword.GetValue())
		{
			ShowResponse(m_sPasswordNotMatching, UIColors.WARNING);
			return;
		}
		
		// TODO: Send request to backend
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCancel()
	{
		super.OnCancel();
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.LoginDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	private void ShowResponse(string name, Color color)
	{
		// TODO
	}
	
	//------------------------------------------------------------------------------------------------
	private void CheckFilledEditboxes()
	{
		bool enabled = m_UserName.GetValue() != string.Empty 
			&& m_Password.GetValue() != string.Empty
			&& m_ConfirmPassword.GetValue() != string.Empty;
		m_Confirm.SetEnabled(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	void CloseDelayed()
	{
		GetGame().GetCallqueue().CallLater(CloseAnimated, m_iDelayMs);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowLoadinAnim(bool show)
	{
		if (show && m_wOverlay)
			 m_LoadingOverlay = SCR_LoadingOverlay.ShowForWidget(m_wOverlay, string.Empty, false, true);
		else if (m_LoadingOverlay)
			m_LoadingOverlay.HideAndDelete();
	}
};
