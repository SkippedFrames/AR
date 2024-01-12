//------------------------------------------------------------------------------------------------
class SCR_LoginDialogUI : SCR_LoginProcessDialogUI
{
	protected const string USERNAME_WIDGET = "UserName";
	protected const string PASSWORD_WIDGET = "Password";
	
	protected SCR_EditBoxComponent m_UserName;
	protected SCR_EditBoxComponent m_Password;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		// Load credentials
		string name = GetGame().GetBackendApi().GetCredentialsItem(EBackendCredentials.EBCRED_NAME);
		m_UserName = SCR_EditBoxComponent.GetEditBoxComponent(USERNAME_WIDGET, m_wRoot);
		if (m_UserName)
		{
			m_UserName.SetValue(name);
			m_UserName.m_OnChanged.Insert(CheckFilledEditBoxes);

			GetGame().GetWorkspace().SetFocusedWidget(m_UserName.GetRootWidget());
		}

		m_Password = SCR_EditBoxComponent.GetEditBoxComponent(PASSWORD_WIDGET, m_wRoot);
		if (m_Password)
			m_Password.m_OnChanged.Insert(CheckFilledEditBoxes);

		CheckFilledEditBoxes();
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{	
		if (m_bIsLoading)
			return;
		
		// Store credentials
		if (m_UserName)
			GetGame().GetBackendApi().SetCredentialsItem(EBackendCredentials.EBCRED_NAME, m_UserName.GetValue());

		if (m_Password)
			GetGame().GetBackendApi().SetCredentialsItem(EBackendCredentials.EBCRED_PWD, m_Password.GetValue());
		
		GetGame().GetBackendApi().VerifyCredentials(m_Callback, true);
		
		super.OnConfirm();
	}

	//------------------------------------------------------------------------------------------------
	override void OnFail(SCR_BackendCallback callback, int code, int restCode, int apiCode)
	{
		if (restCode == CODE_TWO_FA)
		{
			SCR_LoginProcessDialogUI.Create2FADialog(m_UserName.GetValue(), m_Password.GetValue());
			Close();
		}
		else
			super.OnFail(callback, code, restCode, apiCode);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ShowWarningMessage(bool show)
	{
		super.ShowWarningMessage(show);

		if (!show)
			return;
		
		if (m_UserName)
			m_UserName.OnInvalidInput();
		
		if (m_Password)
			m_Password.OnInvalidInput();
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckFilledEditBoxes()
	{
		m_bForceConfirmButtonDisabled = m_UserName.GetValue().IsEmpty() || m_Password.GetValue().IsEmpty();
		if (m_ConfirmButton)
			m_ConfirmButton.SetEnabled(!m_bForceConfirmButtonDisabled && UpdateButtons());
	}
}

//------------------------------------------------------------------------------------------------
class SCR_LoginDialogConsoleUI : SCR_LoginDialogUI
{
	protected const string OVERLAY_MAIN = "OverlayMain";
	
	protected OverlayWidget m_wOverlayMain; 
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		m_wOverlayMain = OverlayWidget.Cast(m_wRoot.FindAnyWidget(OVERLAY_MAIN));
	}
	
	//------------------------------------------------------------------------------------------------
	override OverlayWidget GetDialogBaseOverlay()
	{
		return m_wOverlayMain;
	}
}
