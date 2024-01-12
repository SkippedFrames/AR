/* 
	Parent class of login/profile dialogs
*/
//------------------------------------------------------------------------------------------------
class SCR_LoginProcessDialogUI : SCR_ConfigurableDialogUi
{
	// Widgets
	protected const string TOS_BUTTON = "ProfileTOS";
	protected const string TOS_LINK = "Link_PrivacyPolicy";
	
	protected const string REGISTER_BUTTON = "createAccount";
	protected const string REGISTER_LINK = "Link_RegisterAccount";
	
	protected const string PID_TEXT_WIDGET = "PIDText";
	protected const string PID_BUTTON_WIDGET = "CopyPIDButton";

	// Warning messages
	protected const string WARNING_SUCCESS_IMG = "check";
	protected const string WARNING_FAILED_IMG = "warning";
	protected const string WARNING_TIMEOUT_IMG = "disconnection";
	
	protected const string WARNING_TIMEOUT = "#AR-Account_LoginTimeout";
	
	// Buttons
	protected SCR_InputButtonComponent m_TOSButton;
	protected SCR_InputButtonComponent m_ConfirmButton;
	protected SCR_InputButtonComponent m_CreateAccount;
	
	protected SCR_ButtonImageComponent m_CopyButtonComponent;
	protected RichTextWidget m_wPIDText;
	
	// Other
	protected SCR_LoadingOverlay m_LoadingOverlay;
	protected SCR_SimpleWarningComponent m_Warning;
	
	protected bool m_bForceConfirmButtonDisabled;
	protected bool m_bIsLoading;
	
	protected const int CODE_TWO_FA = 422;
	protected const int CODE_BAD_REQUEST = 400;
	
	protected ref SCR_BackendCallback m_Callback;
	
	protected static const ResourceName DIALOG_CONFIG = "{9381BF296A0E273B}Configs/Dialogs/LoginDialogs.conf";

	// Dialog creation
	//------------------------------------------------------------------------------------------------
	static void CreateLoginDialog()
	{
		if (GetGame().IsPlatformGameConsole())
		{
			SCR_LoginDialogConsoleUI dialog = new SCR_LoginDialogConsoleUI();
			SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "LOGIN_CONSOLE", dialog);
			
			return;
		}
		
		SCR_LoginDialogUI dialog = new SCR_LoginDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "LOGIN", dialog);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_PlayerProfileDialogUI CreateProfileDialog()
	{
		SCR_PlayerProfileDialogUI dialog = new SCR_PlayerProfileDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "PLAYER_PROFILE", dialog);

		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	static void Create2FADialog(string name, string code)
	{
		if (GetGame().IsPlatformGameConsole())
		{
			SCR_Login2FADialogConsoleUI dialog = new SCR_Login2FADialogConsoleUI(name, code);
			SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "2FA_CONSOLE", dialog);
			
			return;
		}
		
		SCR_Login2FADialogUI dialog = new SCR_Login2FADialogUI(name, code);
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "2FA", dialog);
	}
	
	// We use the SCR_ConfigurableDialogUi because confirming must close the dialog and there's no need for SCR_LoginProcessDialogUI functionalities
	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateLoginSuccessDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "LOGIN_SUCCESS");
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateLoginTimeoutDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "LOGIN_TIMEOUT");
	}
	
	// Overrides
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{	
		super.OnMenuOpen(preset);
		
		m_TOSButton = FindButton(TOS_BUTTON);
		if (m_TOSButton)
			m_TOSButton.m_OnActivated.Insert(OnTOSButton);
		
		m_CreateAccount = FindButton(REGISTER_BUTTON);
		if (m_CreateAccount)
			m_CreateAccount.m_OnActivated.Insert(OnCreateAccount);
		
		m_ConfirmButton = FindButton(BUTTON_CONFIRM);
		
		m_wPIDText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(PID_TEXT_WIDGET));
		
		m_CopyButtonComponent = SCR_ButtonImageComponent.GetButtonImage(PID_BUTTON_WIDGET, m_wRoot);
		if (m_CopyButtonComponent)
			m_CopyButtonComponent.m_OnClicked.Insert(CopyPID);

		m_Warning = SCR_SimpleWarningComponent.FindComponentInHierarchy(m_wRoot);
		ShowWarningMessage(false);
		
		// Status check
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		UpdateButtons();
		
		// Callback events
		m_Callback = new SCR_BackendCallback();
		
		m_Callback.GetEventOnSuccess().Insert(OnSuccess);
		m_Callback.GetEventOnFail().Insert(OnFail);
		m_Callback.GetEventOnTimeOut().Insert(OnTimeout);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		GetGame().GetCallqueue().Remove(OnTimeoutScripted);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		ShowLoadingAnim(true);
		ShowWarningMessage(false);
		
		// fallback timeout should the backend callback not work
		GetGame().GetCallqueue().Remove(OnTimeoutScripted);
		GetGame().GetCallqueue().CallLater(OnTimeoutScripted, SCR_ServicesStatusHelper.AUTOMATIC_REFRESH_RATE);
	}
	
	// Methods
	//------------------------------------------------------------------------------------------------
	protected bool UpdateButtons()
	{	
		string service = SCR_ServicesStatusHelper.SERVICE_ACCOUNT_PROFILE;
		
		SCR_ServicesStatusHelper.ForceConnectionButtonEnabled(m_TOSButton, SCR_ServicesStatusHelper.IsBackendConnectionAvailable(), false);
		SCR_ServicesStatusHelper.ForceConnectionButtonEnabled(m_CreateAccount, SCR_ServicesStatusHelper.IsBackendConnectionAvailable(), false);

		SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_ConfirmButton, service, m_bForceConfirmButtonDisabled || ! SCR_ServicesStatusHelper.IsBackendConnectionAvailable(), false);
	
		string identity;
		BackendApi backendAPI = GetGame().GetBackendApi();
		if (backendAPI)
			identity = backendAPI.GetLocalIdentityId();
		
		if (m_wPIDText)
			m_wPIDText.SetText(identity);
		
		if (m_CopyButtonComponent)
			m_CopyButtonComponent.SetVisible(!GetGame().IsPlatformGameConsole() && !identity.IsEmpty(), false);
		
		if (m_wPIDText)
			m_wPIDText.SetVisible(!identity.IsEmpty());
		
		return SCR_ServicesStatusHelper.IsServiceActive(service);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowLoadingAnim(bool show)
	{
		m_bIsLoading = show;

		if (show)
			m_LoadingOverlay = SCR_LoadingOverlay.ShowForWidget(GetDialogBaseOverlay(), string.Empty, false, true);
		else if (m_LoadingOverlay)
			m_LoadingOverlay.HideAndDelete();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowWarningMessage(bool show)
	{
		if (m_Warning)
			m_Warning.SetWarningVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Copy playerID to clipboard
	protected void CopyPID()
	{
		System.ExportToClipboard(m_wPIDText.GetText());
	}
	
	// Events
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTOSButton()
	{
		GetGame().GetPlatformService().OpenBrowser(GetGame().GetBackendApi().GetLinkItem(TOS_LINK));
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnCreateAccount()
	{
		GetGame().GetPlatformService().OpenBrowser(GetGame().GetBackendApi().GetLinkItem(REGISTER_LINK));
	}
	
	// Ovverridden in children either for the Login or Logout callbacks
	//------------------------------------------------------------------------------------------------
	protected void OnSuccess(SCR_BackendCallback callback)
	{
		CreateLoginSuccessDialog();
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFail(SCR_BackendCallback callback, int code, int restCode, int apiCode)
	{
		if (restCode == CODE_BAD_REQUEST)
		{
			CreateLoginTimeoutDialog();
			return;
		}
		
		//TODO: remove
		PrintFormat("SCR_LoginProcessDialogUI - OnFail | code %1, restCode %2, apiCode %3");
		
		ShowLoadingAnim(false);
		ShowWarningMessage(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTimeout(SCR_BackendCallback callback)
	{
		ShowLoadingAnim(false);
		ShowWarningMessage(false);
		
		CreateLoginTimeoutDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTimeoutScripted()
	{
		OnTimeout(m_Callback);
	}
}
