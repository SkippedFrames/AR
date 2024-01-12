//------------------------------------------------------------------------------------------------
class SCR_SettingsSuperMenu : SCR_SuperMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		SCR_InputButtonComponent comp = SCR_InputButtonComponent.GetInputButtonComponent("Back",GetRootWidget());
		if (comp)
			comp.m_OnActivated.Insert(OnBack);

		bool showBlur = !GetGame().m_bIsMainMenuOpen;

		Widget img = GetRootWidget().FindAnyWidget("MenuBackground");
		if (img)
			img.SetVisible(!showBlur);

		BlurWidget blur = BlurWidget.Cast(GetRootWidget().FindAnyWidget("Blur0"));
		if (blur)
		{
			blur.SetVisible(showBlur);
			if (showBlur)
			{
				float w, h;
				GetGame().GetWorkspace().GetScreenSize(w, h);
				blur.SetSmoothBorder(0, 0, w * 0.8, 0);
			}
		}
		
		Widget w = GetRootWidget().FindAnyWidget("TabViewRoot0");
		if (!w)
			return;
		
		bool isDebug = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, false);
		
		SCR_TabViewComponent tab = SCR_TabViewComponent.Cast(w.FindHandler(SCR_TabViewComponent));
		if (!tab)
			return;
		
		if (!isDebug && System.GetPlatform() == EPlatform.WINDOWS)
		{
			tab.RemoveTabByIdentifier("SettingsVideoConsole");
			tab.ShowTabByIdentifier("SettingsVideoPC");
		}

// Remove video settings and keybinds for consoles, if not in settings debug mode
		
#ifdef PLATFORM_CONSOLE
		if (isDebug)
			return;
		
		tab.RemoveTabByIdentifier("SettingsVideoPC");
		tab.ShowTabByIdentifier("SettingsVideoConsole");
		
#endif
	}

	void OnBack()
	{
		Close();
	}
};
