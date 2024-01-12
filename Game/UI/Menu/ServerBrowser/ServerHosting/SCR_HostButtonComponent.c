/*!
Component for handling server creation action.
Not for viusals.
*/

class SCR_HostButtonComponent : ScriptedWidgetComponent
{
	SCR_ButtonBaseComponent m_Button;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Button = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		
		// Invokers actions 
		m_Button.m_OnClicked.Insert(OnClicked);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnClicked(SCR_ButtonBaseComponent button)
	{
		// Open hosting menu
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServerHostingDialog);
	}
};
