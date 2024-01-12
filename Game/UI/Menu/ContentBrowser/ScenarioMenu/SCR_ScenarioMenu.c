/*
Menu with a list view of scenarios.
*/
class SCR_ScenarioMenu : SCR_SuperMenuBase
{
	protected SCR_InputButtonComponent m_NavBack;
	
	//-----------------------------------------------------------------------------------------------------------------
	static SCR_ScenarioMenu Open()
	{
		SCR_ScenarioMenu dlg = SCR_ScenarioMenu.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ScenarioMenu));
		return dlg;
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		// Find nav buttons
		m_NavBack = SCR_InputButtonComponent.GetInputButtonComponent("Back", GetRootWidget());
		m_NavBack.m_OnActivated.Insert(OnNavButtonClose);
	}
	
	
	//-----------------------------------------------------------------------------------------------------------------
	void OnNavButtonClose()
	{
		this.Close();
	}
};
