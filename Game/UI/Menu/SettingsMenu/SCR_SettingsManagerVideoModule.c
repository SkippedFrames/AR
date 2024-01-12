//------------------------------------------------------------------------------------------------
class SCR_SettingsManagerVideoModule : SCR_SettingsManagerModuleBase
{
	protected BaseContainer m_DisplayUserSettings;
	protected UserSettings m_VideoUserSettings;
	protected BaseContainer m_VideoSettings;
		
	protected const int COMBO_OPTION_QUALITY = 0;
	protected const int COMBO_OPTION_PERFORMANCE = 1;
	
	//------------------------------------------------------------------------------------------------
	void SetConsolePreset(int presetIndex)
	{
		//todo: marian: split into methods and use switch
		m_DisplayUserSettings = GetGame().GetEngineUserSettings().GetModule("DisplayUserSettings");
		m_VideoUserSettings = GetGame().GetEngineUserSettings().GetModule("VideoUserSettings");
		m_VideoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
		
		if (!m_DisplayUserSettings || !m_VideoUserSettings || !m_VideoSettings)
			return;
		
		if (presetIndex == EVideoQualityPreset.SERIES_S_PRESET_QUALITY)
		{
			m_DisplayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_S_PRESET_QUALITY);
			m_VideoUserSettings.Set("ResolutionScale", 0.75);
			m_VideoUserSettings.Set("Fsaa", 2);
			m_VideoSettings.Set("m_bNearDofEffect", false);
			m_VideoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			m_VideoUserSettings.Set("MaxFps", 30);
			m_VideoUserSettings.Set("Vsynch", true);
			m_VideoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_S_PRESET_QUALITY);
		}		
		else if (presetIndex == EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE)
		{
			m_DisplayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE);
			m_VideoUserSettings.Set("ResolutionScale", 0.60);
			m_VideoUserSettings.Set("Fsaa", 2);
			m_VideoSettings.Set("m_bNearDofEffect", false);
			m_VideoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			m_VideoUserSettings.Set("MaxFps", 60);
			m_VideoUserSettings.Set("Vsynch", true);
			m_VideoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_S_PRESET_PERFORMANCE);
		}
		else if (presetIndex == EVideoQualityPreset.SERIES_X_PRESET_QUALITY)
		{
			m_DisplayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_X_PRESET_QUALITY);
			m_VideoUserSettings.Set("ResolutionScale", 0.75);
			m_VideoUserSettings.Set("Fsaa", 4);
			m_VideoSettings.Set("m_bNearDofEffect", false);
			m_VideoSettings.Set("m_iDofType", DepthOfFieldTypes.BOKEH);
			m_VideoUserSettings.Set("MaxFps", 30);
			m_VideoUserSettings.Set("Vsynch", true);
			m_VideoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_X_PRESET_QUALITY);
		}
		else if (presetIndex == EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE)
		{
			m_DisplayUserSettings.Set("OverallQuality", EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE);
			m_VideoUserSettings.Set("ResolutionScale", 0.50);
			m_VideoUserSettings.Set("Fsaa", 4);
			m_VideoSettings.Set("m_bNearDofEffect", false);
			m_VideoSettings.Set("m_iDofType", DepthOfFieldTypes.SIMPLE);
			m_VideoUserSettings.Set("MaxFps", 60);
			m_VideoUserSettings.Set("Vsynch", true);
			m_VideoSettings.Set("m_iLastUsedPreset", EVideoQualityPreset.SERIES_X_PRESET_PERFORMANCE);
		}
		
		GetGame().ApplySettingsPreset();
		GetGame().UserSettingsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SettingsManagerVideoModule()
	{
		SetModuleType(ESettingManagerModuleType.SETTINGS_MANAGER_VIDEO);
	}
}

//------------------------------------------------------------------------------------------------
enum EVideoQualityPreset
{
	SERIES_X_PRESET_QUALITY = 4,
	SERIES_S_PRESET_QUALITY = 5,
	SERIES_X_PRESET_PERFORMANCE = 6,
	SERIES_S_PRESET_PERFORMANCE = 7,
};
