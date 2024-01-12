//------------------------------------------------------------------------------------------------
class WidgetLibraryMenuUI: ChimeraMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent("Back",GetRootWidget());
		if (back)
			back.m_OnActivated.Insert(OnBack);
	}

	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		GameStateTransitions.RequestGameplayEndTransition();
	}
};
