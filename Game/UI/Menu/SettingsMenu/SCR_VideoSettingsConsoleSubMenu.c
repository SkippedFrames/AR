class SCR_VideoSettingsConsoleSubMenu : SCR_SettingsSubMenuBase
{		
	protected const int COMBO_OPTION_QUALITY = 0;
	protected const int COMBO_OPTION_PERFORMANCE = 1;
	
	protected EPlatform m_ePlatform;
	SCR_SettingsManagerVideoModule m_SettingsVideoModule;
		
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		
		SCR_SettingsManager settingsManager = GetGame().GetSettingsManager();

		if (!settingsManager)
			return;
		
		m_SettingsVideoModule = SCR_SettingsManagerVideoModule.Cast(settingsManager.GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_VIDEO));
		if (!m_SettingsVideoModule)
			Print("VideoSettingsConsoleSubMenu: Video settings manager module not found!", LogLevel.WARNING);
		
		m_ePlatform = System.GetPlatform();
		SetupQualityPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnQualityPresetChanged(SCR_ComboBoxComponent combobox, int itemIndex)
	{
		if (m_ePlatform != EPlatform.XBOX_SERIES_X && m_ePlatform != EPlatform.XBOX_SERIES_S)
			return;
		
		ChangePreset(m_ePlatform, itemIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ChangePreset(EPlatform platform, int itemIndex)
	{
		if (!m_SettingsVideoModule)
			return;
		
		if (platform == EPlatform.XBOX_SERIES_S && itemIndex == COMBO_OPTION_QUALITY)
			m_SettingsVideoModule.SetConsolePreset(EVideoQualityPreset.SERIES_S_PRESET_QUALITY);
		
		if (platform == EPlatform.XBOX_SERIES_S && itemIndex == COMBO_OPTION_PERFORMANCE)
			m_SettingsVideoModule.SetConsolePreset(EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE);
		
		if (platform == EPlatform.XBOX_SERIES_X && itemIndex == COMBO_OPTION_QUALITY)
			m_SettingsVideoModule.SetConsolePreset(EVideoQualityPreset.SERIES_X_PRESET_QUALITY);
		
		if (platform == EPlatform.XBOX_SERIES_X && itemIndex == COMBO_OPTION_PERFORMANCE)
			m_SettingsVideoModule.SetConsolePreset(EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE);
		
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupQualityPreset()
	{
		SCR_ComboBoxComponent qualityPreset = SCR_ComboBoxComponent.GetComboBoxComponent("QualityPreset", m_wRoot);
		if (!qualityPreset)
			return;
		
		int lastPreset;
		
		BaseContainer videoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
		if (!videoSettings)
			return;
		
		videoSettings.Get("m_iLastUsedPreset", lastPreset);

		if (lastPreset == EVideoQualityPreset.SERIES_X_PRESET_QUALITY || lastPreset == EVideoQualityPreset.SERIES_S_PRESET_QUALITY)
			qualityPreset.SetCurrentItem(COMBO_OPTION_QUALITY);
		
		if (lastPreset == EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE || lastPreset == EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE)
			qualityPreset.SetCurrentItem(COMBO_OPTION_PERFORMANCE);
		
		qualityPreset.m_OnChanged.Insert(OnQualityPresetChanged);
	}
};
