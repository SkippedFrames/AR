/*!
Dialog that can contain multiple content in tab view.
Dialog can switch between content and display different actions 
*/

class SCR_TabDialog : DialogUI
{
	protected SCR_SuperMenuComponent m_SuperMenu;
	
	//------------------------------------------------------------------------------------------------
	// Override 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_SuperMenu = SCR_SuperMenuComponent.Cast(GetRootWidget().FindHandler(SCR_SuperMenuComponent));
		m_SuperMenu.Init(GetRootWidget());
	}
}
