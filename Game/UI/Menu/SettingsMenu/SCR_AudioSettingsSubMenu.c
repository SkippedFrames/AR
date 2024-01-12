//------------------------------------------------------------------------------------------------
class SCR_AudioSettingsSubMenu: SCR_SettingsSubMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		
		m_aSettingsBindings.Clear();
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "Volume", "Master"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "VolumeSfx", "SFX"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "VolumeMusic", "Music"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "VolumeVoiceChat", "VOIP"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "VolumeDialog", "Dialogue"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("AudioSettings", "StereoMode", "ProcessingMode"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AudioSettings", "m_fDynamicRange", "DynamicRange"));
		LoadSettings();
	}
};
