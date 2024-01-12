//------------------------------------------------------------------------------------------------
class SCR_PlayerProfileDialogUI : SCR_LoginProcessDialogUI
{
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		GetGame().GetBackendApi().Unlink(m_Callback);
		
		super.OnConfirm();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(SCR_BackendCallback callback)
	{
		Close();
	}
}
