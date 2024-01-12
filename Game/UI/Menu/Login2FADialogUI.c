// TODO: make the number of edit boxes procedural
class SCR_Login2FADialogUI : SCR_LoginProcessDialogUI
{
	protected string m_s2FAText;
	protected string m_sName;
	protected string m_sCode;

	protected ref array<SCR_EditBoxComponent> m_aEditBoxComponents = {};
	
	protected Widget m_wCodeWrapper;
	
	protected const int EDIT_BOX_INITIAL_FOCUS_DELAY = 100;
	protected const int EDIT_BOX_FLICKER_TIME = 400;
	protected const int ON_FAIL_DELAY = 5000;

	//------------------------------------------------------------------------------------------------
	void SCR_Login2FADialogUI(string name, string code)
	{
		m_sName = name;
		m_sCode = code;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		// Find all edit boxes
		m_wCodeWrapper = m_wRoot.FindAnyWidget("CodeWrapper");
		if (m_wCodeWrapper)
		{
			array<ref Widget> editBoxes = {};
			SCR_WidgetHelper.GetAllChildren(m_wCodeWrapper, editBoxes);
			
			foreach (Widget w : editBoxes)
			{
				SCR_EditBoxComponent comp = SCR_EditBoxComponent.Cast(w.FindHandler(SCR_EditBoxComponent));
				if (!comp)
					continue;
				
				m_aEditBoxComponents.Insert(comp);
				BindEditBoxInputEvent(comp);
			}
		}

		//TODO: there is something resetting the focus to null somewhere, so we need to delay the activation. FIX!
		GetGame().GetCallqueue().CallLater(ActivateFirstEditBox, EDIT_BOX_INITIAL_FOCUS_DELAY);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		GetGame().GetCallqueue().Remove(OnFailDelayed);
		GetGame().GetCallqueue().Remove(ActivateFirstEditBox);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		if (m_bIsLoading)
			return;
		
		GetGame().GetBackendApi().SetCredentialsItem(EBackendCredentials.EBCRED_NAME, m_sName);
		GetGame().GetBackendApi().SetCredentialsItem(EBackendCredentials.EBCRED_PWD, m_sCode);
		GetGame().GetBackendApi().SetCredentialsItem(EBackendCredentials.EBCRED_2FA_TOKEN, m_s2FAText);
		GetGame().GetBackendApi().VerifyCredentials(m_Callback, true);
		
		super.OnConfirm();
	}

	//------------------------------------------------------------------------------------------------
	override void OnFail(SCR_BackendCallback callback, int code, int restCode, int apiCode)
	{
		// Add a delay to prevent the backend from being overwhelmed
		GetGame().GetCallqueue().CallLater(OnFailDelayed, ON_FAIL_DELAY, false, callback, code, restCode, apiCode);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTimeout(SCR_BackendCallback callback)
	{
		super.OnTimeout(callback);

		Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFailDelayed(SCR_BackendCallback callback, int code, int restCode, int apiCode)
	{
		super.OnFail(callback, code, restCode, apiCode);

		GetGame().GetCallqueue().Remove(OnFailDelayed);
		Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActivateFirstEditBox()
	{
		if (!m_aEditBoxComponents.IsEmpty())
			m_aEditBoxComponents[0].ActivateWriteMode(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void BindEditBoxInputEvent(SCR_EditBoxComponent comp)
	{
		comp.m_OnTextChange.Insert(OnTextChange);
	}
	
	//------------------------------------------------------------------------------------------------
	//! If number was inserted into current EditBox, add that number to the string and go to next EditBox. If all are filled, proceed to verification
	protected void OnTextChange(string text)
	{
		if (m_bIsLoading || text.IsEmpty())
			return;
		
		m_s2FAText = string.Empty;
		
		foreach (SCR_EditBoxComponent comp : m_aEditBoxComponents)
		{
			string value = comp.GetValue();
			if (value.IsEmpty())
			{
				comp.ActivateWriteMode(true);
				break;
			}

			m_s2FAText += value;
		}

		if (IsCodeComplete())
			OnConfirm();
	}
	
	//------------------------------------------------------------------------------------------------
	//! We have a number of single digit edit boxes
	protected bool IsCodeComplete()
	{
		return m_s2FAText.Length() >= m_aEditBoxComponents.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Clear()
	{
		m_s2FAText = string.Empty;
		
		foreach (SCR_EditBoxComponent comp : m_aEditBoxComponents)
		{
			comp.SetValue(string.Empty);
			
			// Flicker components
			GetGame().GetCallqueue().Remove(comp.ResetOverlayColor);
			
			comp.ChangeOverlayColor(UIColors.WARNING);
			GetGame().GetCallqueue().CallLater(comp.ResetOverlayColor, EDIT_BOX_FLICKER_TIME);
		}
		
		ActivateFirstEditBox();
	}
}

//------------------------------------------------------------------------------------------------
class SCR_Login2FADialogConsoleUI : SCR_Login2FADialogUI
{
	//------------------------------------------------------------------------------------------------
	override void BindEditBoxInputEvent(SCR_EditBoxComponent comp)
	{
		comp.m_OnWriteModeLeave.Insert(OnTextChange);
	}
	
	//------------------------------------------------------------------------------------------------
	// We have a single edit box in which the whole code is inserted
	override bool IsCodeComplete()
	{
		return !m_s2FAText.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	// We don't want to activate the edit box automatically or the dialog will be obscured by the virtual console, so we just focus it
	override void ActivateFirstEditBox()
	{
		if (!m_aEditBoxComponents.IsEmpty())
			GetGame().GetWorkspace().SetFocusedWidget(m_aEditBoxComponents[0].GetRootWidget());
	}
}
