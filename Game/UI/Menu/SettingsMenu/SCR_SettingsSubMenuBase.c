//------------------------------------------------------------------------------------------------
class SCR_SettingsSubMenuBase: SCR_SubMenuBase
{
	protected ref array<ref SCR_SettingsBindingBase> m_aSettingsBindings = {};
	protected ScrollLayoutWidget m_wScroll;
	protected bool m_bLoadingSettings;

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		m_wScroll = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget("ScrollLayout0"));
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);
		GetGame().SaveUserSettings();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		if (m_wScroll)
			m_wScroll.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadSettings(bool forceLoadSettings = false, bool addEntryChangedEventHandler = true)
	{
		if (!m_wScroll)
			return;

		m_bLoadingSettings = true;
		foreach (SCR_SettingsBindingBase bind : m_aSettingsBindings)
		{
			bind.LoadEntry(m_wScroll, forceLoadSettings, addEntryChangedEventHandler);
			if (addEntryChangedEventHandler)
				bind.GetEntryChangedInvoker().Insert(OnMenuItemChanged);
		}
		m_bLoadingSettings = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuItemChanged(SCR_SettingsBindingBase binding)
	{
		GetGame().UserSettingsChanged();
	}
};
