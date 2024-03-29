// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/layouts/Menus/ContentBrowser/AddonManager/AddonsTools/AddonsPresets/AddonsPresetDialog.layout

class SCR_AddonsPresetDialogWidgets
{
	static const ResourceName s_sLayout = "{BF75D006F7225F61}UI/layouts/Menus/ContentBrowser/AddonManager/AddonsTools/AddonsPresets/AddonsPresetDialog.layout";
	ResourceName GetLayout() { return s_sLayout; }

	ButtonWidget m_ButonNewPreset;
	SCR_ButtonTextComponent m_ButonNewPresetComponent;

	ScrollLayoutWidget m_ScrollPresets;
	SCR_ListBoxComponent m_ScrollPresetsComponent;

	ButtonWidget m_NavRename;
	SCR_InputButtonComponent m_NavRenameComponent;

	ButtonWidget m_NavDelete;
	SCR_InputButtonComponent m_NavDeleteComponent;

	ButtonWidget m_NavOverride;
	SCR_InputButtonComponent m_NavOverrideComponent;

	ButtonWidget m_NavLoad;
	SCR_InputButtonComponent m_NavLoadComponent;

	bool Init(Widget root)
	{
		m_ButonNewPreset = ButtonWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Content.AddonsPresetRoot.VerticalList.m_ButonNewPreset"));
		m_ButonNewPresetComponent = SCR_ButtonTextComponent.Cast(m_ButonNewPreset.FindHandler(SCR_ButtonTextComponent));

		m_ScrollPresets = ScrollLayoutWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Content.AddonsPresetRoot.VerticalList.m_ScrollPresets"));
		m_ScrollPresetsComponent = SCR_ListBoxComponent.Cast(m_ScrollPresets.FindHandler(SCR_ListBoxComponent));

		m_NavRename = ButtonWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Footer.m_NavRename"));
		m_NavRenameComponent = SCR_InputButtonComponent.Cast(m_NavRename.FindHandler(SCR_InputButtonComponent));

		m_NavDelete = ButtonWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Footer.m_NavDelete"));
		m_NavDeleteComponent = SCR_InputButtonComponent.Cast(m_NavDelete.FindHandler(SCR_InputButtonComponent));

		m_NavOverride = ButtonWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Footer.m_NavOverride"));
		m_NavOverrideComponent = SCR_InputButtonComponent.Cast(m_NavOverride.FindHandler(SCR_InputButtonComponent));

		m_NavLoad = ButtonWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Footer.m_NavLoad"));
		m_NavLoadComponent = SCR_InputButtonComponent.Cast(m_NavLoad.FindHandler(SCR_InputButtonComponent));

		return true;
	}
}
