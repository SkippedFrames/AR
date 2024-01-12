/*!
Extended component for button that can contain multiple small buttons with various actions
*/

//------------------------------------------------------------------------------------------------
class SCR_MultiActionButtonComponent : SCR_ScriptedWidgetComponent
{
	[Attribute()]
	protected ref array<string> m_aButtonNames;
	
	protected ref map<string, SCR_ModularButtonComponent> m_aButtons = new map<string, SCR_ModularButtonComponent>();
	
	//----------------------------------------------------------------------------------------------
	// override 
	//----------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		// Find all buttons 
		for (int i = 0, count = m_aButtonNames.Count(); i < count; i++)
		{
			Widget btn = w.FindAnyWidget(m_aButtonNames[i]);
			if (!btn)
				continue;
			
			SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.Cast(btn.FindHandler(SCR_ModularButtonComponent));
			if (comp)
				m_aButtons.Insert(m_aButtonNames[i], comp);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	// API 
	//----------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------
	void ShowAllButtons(bool show)
	{
		for (int i = 0, count = m_aButtons.Count(); i < count; i++)
		{
			SCR_ModularButtonComponent comp = m_aButtons.GetElement(i);
			comp.GetRootWidget().SetVisible(show);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	SCR_ModularButtonComponent FindButton(string name)
	{
		return m_aButtons.Get(name);
	}
}
