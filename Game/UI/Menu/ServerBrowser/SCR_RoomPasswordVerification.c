//------------------------------------------------------------------------------------------------
/*!
Solve joining to password protected room
*/
//------------------------------------------------------------------------------------------------
class SCR_RoomPasswordVerification
{
	protected string FALLBACK_ERROR = "#AR-Password_FailMessage";
	protected string FALLBACK_TIMEOUT = "#AR-Password_TimeoutMessage";

	protected Room m_Room;
	protected SCR_ConfigurableDialogUi m_Dialog;
	protected SCR_LoadingOverlay m_LoadingOverlay;

	protected ref SCR_BackendCallback m_Callback = new SCR_BackendCallback();
	protected ref RoomJoinData m_PasswordParam = new RoomJoinData();

	protected ref ScriptInvokerRoom m_OnVerified = new ScriptInvokerRoom();
	protected ref ScriptInvokerString m_OnFailVerification = new ScriptInvokerString();

	//------------------------------------------------------------------------------------------------
	ScriptInvokerRoom GetOnVerified()
	{
		return m_OnVerified;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerString GetOnFailVerification()
	{
		return m_OnFailVerification;
	}

	//------------------------------------------------------------------------------------------------
	//! Setup dialog UI for handling password and joining room
	void SetupDialog(notnull SCR_ConfigurableDialogUi dialog, notnull Room room, string message = "")
	{
		m_Dialog = dialog;
		m_Room = room;

		// Callbacks
		dialog.m_OnConfirm.Insert(OnPasswordConfirm);
		//dialog.m_OnCancel.Insert(PasswordClearInvokers);


		// Display dialog message - TODO: Check what is this for?
		SCR_EditboxDialogUi editDialog = SCR_EditboxDialogUi.Cast(dialog);
		if (editDialog)
			editDialog.SetWarningMessage(message);
	}

	//------------------------------------------------------------------------------------------------
	//! Start password verification on confirming password dialog
	protected void OnPasswordConfirm()
	{
		if (!m_Room)
			return;

		SCR_EditboxDialogUi editboxDialog = SCR_EditboxDialogUi.Cast(m_Dialog);

		// Get edit box value
		string value = editboxDialog.GetEditbox().GetValue();

		// Show loading
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(true);
		else
			m_LoadingOverlay = SCR_LoadingOverlay.ShowForWidget(GetGame().GetWorkspace(), string.Empty);

		// Try join with password
		m_PasswordParam.SetPassword(value);

		m_Callback.GetEventOnResponse().Insert(OnPasswordCheckResponse);
		m_Room.VerifyPassword(value, m_Callback);
	}

	//------------------------------------------------------------------------------------------------
	//! Handle password check request response
	protected void OnPasswordCheckResponse(SCR_BackendCallback callback)
	{
		// Hide loading
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(false);

		// Clear
		ClearInvokers();

		// Reaction
		switch (callback.GetResponseType())
		{
			case EBackendCallbackResponse.SUCCESS:
			{
				// Invoke verification with successful password struct
				m_OnVerified.Invoke(m_Room);
				break;
			}

			case EBackendCallbackResponse.ERROR:
			{
				//JoinProcess_PasswordDialogOpen(FALLBACK_ERROR);
				m_OnFailVerification.Invoke(FALLBACK_ERROR);
				break;
			}

			case EBackendCallbackResponse.TIMEOUT:
			{
				//JoinProcess_PasswordDialogOpen(FALLBACK_TIMEOUT);
				m_OnFailVerification.Invoke(FALLBACK_TIMEOUT);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearInvokers()
	{
		m_Callback.GetEventOnResponse().Remove(OnPasswordCheckResponse);
		if(m_Dialog && m_Dialog.m_OnConfirm)
			m_Dialog.m_OnConfirm.Remove(OnPasswordConfirm);
	}
};
