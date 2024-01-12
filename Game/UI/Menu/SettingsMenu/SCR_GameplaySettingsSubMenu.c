//------------------------------------------------------------------------------------------------
class SCR_GameplaySettingsSubMenu: SCR_SettingsSubMenuBase
{
	protected static const ref array<string> m_aLanguages = {
		"en_us",
		"fr_fr",
		"it_it",
		"de_de",
		"es_es",
		"cs_cz",
		"pl_pl",
		"ru_ru",
		"ja_jp",
		"ko_kr",
		"pt_br",
		"zh_cn"
	};

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
			playerController.SetGameUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		m_aSettingsBindings.Clear();

		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "MouseInvert", "MouseY"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "GamepadInvert", "GamepadY"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HintSettings", "m_bHintsEnabled", "ContextHints"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bControlHints", "ControlHints"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_b2DScopes", "ScopeMode"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bStickyADS", "StickyADS"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bStickyGadgets", "StickyGadgets"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bShowRadioProtocolText", "RadioProtocolSubtitles"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bMouseControlAircraft", "MouseControlAircraft"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_eDrivingAssistance", "DrivingAssistance"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_FieldOfViewSettings", "m_fFirstPersonFOV", "1PP"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_FieldOfViewSettings", "m_fThirdPersonFOV", "3PP"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_FieldOfViewSettings", "m_fVehicleFOV", "VehicleFOV"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_FieldOfViewSettings", "m_fFocusInADS", "FocusIntensityADS"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AimSensitivitySettings", "m_fMouseSensitivity", "AimMouse"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AimSensitivitySettings", "m_fStickSensitivity", "AimGamepad"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AimSensitivitySettings", "m_fAimADS", "AimADS"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("UserInterfaceSettings", "UseSoftwareCursor", "CursorMode"));

		LoadSettings();
		
		BindFOVSettings();
		BindSensitivitySettings();
		BindLanguage();

// Hide unusual console settings (consoles -can- use USB KBM)
#ifdef PLATFORM_CONSOLE
		HideMenuItem("MouseY");
		HideMenuItem("CursorMode");
		HideMenuItem("AimMouse");
#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void HideMenuItem(string widgetName)
	{
		Widget w = GetRootWidget().FindAnyWidget(widgetName);
		if (w)
			w.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void BindLanguage()
	{
		BaseContainer setting = GetGame().GetEngineUserSettings().GetModule("UserInterfaceSettings");
		if (!setting)
			return;

		string currentLang;
		setting.Get("LanguageCode", currentLang);

		int i = m_aLanguages.Find(currentLang);
		if (i == -1)
			return;

		Widget w = m_wRoot.FindAnyWidget("Language");
		if (!w)
			return;

		SCR_SpinBoxComponent comp = SCR_SpinBoxComponent.Cast(w.FindHandler(SCR_SpinBoxComponent));
		if (!comp)
			return;

		comp.SetCurrentItem(i);
		comp.m_OnChanged.Insert(OnLanguageChange);
	}
	//------------------------------------------------------------------------------------------------
	protected void BindFOVSettings()
	{
		SCR_CheckboxComponent chkboxMaximumZoomInADS = SCR_CheckboxComponent.GetCheckboxComponent("MaxZoomADS", m_wRoot);

		if (chkboxMaximumZoomInADS)
			chkboxMaximumZoomInADS.m_OnChanged.Insert(OnGameSettingsCheckboxChange);

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
			playerController.SetGameUserSettings();
	}
	//------------------------------------------------------------------------------------------------
	protected void BindSensitivitySettings()
	{
		SCR_SliderComponent sliderMouse = SCR_SliderComponent.GetSliderComponent("AimMouse", m_wRoot);
		SCR_SliderComponent sliderGamepad = SCR_SliderComponent.GetSliderComponent("AimGamepad", m_wRoot);
		SCR_SliderComponent sliderADS = SCR_SliderComponent.GetSliderComponent("AimADS", m_wRoot);

		if (sliderMouse)
			sliderMouse.GetOnChangedFinal().Insert(OnGameSettingsSliderChange);

		if (sliderGamepad)
			sliderGamepad.GetOnChangedFinal().Insert(OnGameSettingsSliderChange);

		if (sliderADS)
			sliderADS.GetOnChangedFinal().Insert(OnGameSettingsSliderChange);

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
			playerController.SetGameUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLanguageChange(SCR_SpinBoxComponent comp, int i)
	{
		BaseContainer setting = GetGame().GetEngineUserSettings().GetModule("UserInterfaceSettings");
		if (!setting)
			return;

		if (i >= m_aLanguages.Count())
			return;

		string currentLang;
		setting.Set("LanguageCode", m_aLanguages[i]);
		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGameSettingsSliderChange(SCR_SliderComponent comp, float val)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
			playerController.SetGameUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGameSettingsCheckboxChange(SCR_CheckboxComponent comp, bool val)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
			playerController.SetGameUserSettings();
	}
};
