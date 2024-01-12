/*!
Classes for dialogs which can be used in any context.
*/

class SCR_CommonDialogs
{
	static const ResourceName DIALOGS_CONFIG = "{814FCA3CB7851F6B}Configs/Dialogs/CommonDialogs.conf";
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateDialog(string presetName)
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, presetName);
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateRequestErrorDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "request_error");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateTimeoutOkDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "timeout_ok");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateTimeoutTryAgainCancelDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "timeout_try_again_cancel");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ServicesStatusDialogUI CreateServicesStatusDialog()
	{
		SCR_ServicesStatusDialogUI dialog = new SCR_ServicesStatusDialogUI();
		return SCR_ServicesStatusDialogUI.Cast(SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "services_status", dialog));
	}
};



//---------------------------------------------------------------------------------------------
class SCR_ExitGameDialog : SCR_ConfigurableDialogUi
{
	//---------------------------------------------------------------------------------------------
	void SCR_ExitGameDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "exit_game", this);
	}
	
	//---------------------------------------------------------------------------------------------
	override void OnConfirm()
	{		
		GetGame().RequestClose();
	}
};



//---------------------------------------------------------------------------------------------
class SCR_ExitGameWhileDownloadingDialog : SCR_ConfigurableDialogUi
{
	//---------------------------------------------------------------------------------------------
	void SCR_ExitGameWhileDownloadingDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "exit_game_while_downloading", this);
	}
	
	
	
	//---------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		// Try to terminate all current downloads
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.EndAllDownloads();
		
		// Exit the game
		GetGame().RequestClose();
	}
};



//------------------------------------------------------------------------------------------------
//! Dialog which is shown when user wants to start a scenario while there are still downloads in progress.
//! When confirmed, downloads are paused/canceled, and the scenario is started.
class SCR_StartScenarioWhileDownloadingDialog : SCR_ConfigurableDialogUi
{
	protected ref MissionWorkshopItem m_Scenario;
	protected bool m_bHostScenario;
	
	
	//---------------------------------------------------------------------------------------------
	//! Creates this dialog for a scenario.
	//! host - when true, it will be self-hosted. When false, it will be started in SP mode.
	void SCR_StartScenarioWhileDownloadingDialog(MissionWorkshopItem scenario, bool host)
	{
		m_Scenario = scenario;
		m_bHostScenario = host;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "start_scenario_while_downloading", this)
	}
	
	
	
	//---------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		if (mgr)
			mgr.EndAllDownloads();
		
		// Start scenario
		//TODO
		if (m_bHostScenario)
			m_Scenario.Host(null);
		else
			m_Scenario.Play();
	}
};



//------------------------------------------------------------------------------------------------
//! There is not enough storage on your hard drive. The space required is at least %1.
class SCR_NotEnoughStorageDialog : SCR_ConfigurableDialogUi
{
	protected float m_fSizeBytes;
	protected string m_sPresetMessage;
	
	//------------------------------------------------------------------------------------------------
	static SCR_NotEnoughStorageDialog Create(float sizeBytes)
	{
		SCR_NotEnoughStorageDialog dlg = new SCR_NotEnoughStorageDialog(sizeBytes);
		
		SCR_ConfigurableDialogUi.CreateFromPreset(
			SCR_CommonDialogs.DIALOGS_CONFIG,
			"error_not_enough_storage",
			dlg);
		
		return dlg;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increase size displayed in dialog
	void AddToSize(float sizeBytes)
	{
		m_fSizeBytes += sizeBytes;
		
		string sizeStr = SCR_ByteFormat.GetReadableSize(m_fSizeBytes);
		string messageStr = WidgetManager.Translate(m_sPresetMessage, sizeStr);
		
		SetMessage(messageStr);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		// Set the message with formatted size
		string sizeStr = SCR_ByteFormat.GetReadableSize(m_fSizeBytes);
		
		m_sPresetMessage = preset.m_sMessage;
		string messageStr = WidgetManager.Translate(m_sPresetMessage, sizeStr);
		SetMessage(messageStr);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void SCR_NotEnoughStorageDialog(float sizeBytes)
	{
		m_fSizeBytes = sizeBytes;
	}
};
