// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/layouts/Menus/ContentBrowser/AddonManager/AddonListTab/ListAddonsSubMenu.layout

class SCR_ListAddonsSubMenuWidgets
{
	static const ResourceName s_sLayout = "{0FFE02F22D314775}UI/layouts/Menus/ContentBrowser/AddonManager/AddonListTab/ListAddonsSubMenu.layout";
	ResourceName GetLayout() { return s_sLayout; }

	ButtonWidget m_ButtonEnableAll;
	SCR_ModularButtonComponent m_ButtonEnableAllComponent;
	
	ButtonWidget m_ButtonDisableAll;
	SCR_ModularButtonComponent m_ButtonDisableAllComponent;
	
	ButtonWidget m_ToolsButton;
	SCR_ModularButtonComponent m_ToolsButtonComponent;

	SizeLayoutWidget m_DisabledAddonsPanel;
	
	ScrollLayoutWidget m_DisabledAddonsScroll;

	VerticalLayoutWidget m_DisabledAddonsList;

	SizeLayoutWidget m_EnabledAddonsPanel;
	
	ScrollLayoutWidget m_EnabledAddonsScroll;

	VerticalLayoutWidget m_EnabledAddonsList;

	OverlayWidget m_AddonInfoPanel;
	SCR_AddonDetailsPanelComponent m_AddonInfoPanelComponent;

	bool Init(Widget root)
	{
		m_ButtonEnableAll = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_DisabledAddonsPanel.Overlay0.VerticalLayout0.TopBarHeight.TopBar.Content.HorizontalLayout0.m_ButtonEnableAll"));
		m_ButtonEnableAllComponent = SCR_ModularButtonComponent.Cast(m_ButtonEnableAll.FindHandler(SCR_ModularButtonComponent));
		
		m_ButtonDisableAll = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_EnabledAddonsPanel.Overlay0.VerticalLayout0.TopBarHeight.TopBar.Content.HorizontalLayout0.m_ButtonDisableAll"));
		m_ButtonDisableAllComponent = SCR_ModularButtonComponent.Cast(m_ButtonDisableAll.FindHandler(SCR_ModularButtonComponent));
		
		m_ToolsButton = ButtonWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.TopBar.Content.HorizontalContent.m_ToolsButton"));
		m_ToolsButtonComponent = SCR_ModularButtonComponent.Cast(m_ToolsButton.FindHandler(SCR_ModularButtonComponent));

		m_DisabledAddonsPanel = SizeLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_DisabledAddonsPanel"));

		m_DisabledAddonsScroll = ScrollLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_DisabledAddonsPanel.Overlay0.VerticalLayout0.Content.Scroll"));
		
		m_DisabledAddonsList = VerticalLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_DisabledAddonsPanel.Overlay0.VerticalLayout0.Content.Scroll.m_DisabledAddonsList"));

		m_EnabledAddonsPanel = SizeLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_EnabledAddonsPanel"));

		m_EnabledAddonsScroll = ScrollLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_EnabledAddonsPanel.Overlay0.VerticalLayout0.Content.Scroll"));
		
		m_EnabledAddonsList = VerticalLayoutWidget.Cast(root.FindWidget("Size.PanelsHorizontal.VerticalAddons.HorizontalAddons.m_EnabledAddonsPanel.Overlay0.VerticalLayout0.Content.Scroll.m_EnabledAddonsList"));

		m_AddonInfoPanel = OverlayWidget.Cast(root.FindWidget("Size.PanelsHorizontal.RightPanel.VerticalLayout0.Content.m_AddonInfoPanel"));
		m_AddonInfoPanelComponent = SCR_AddonDetailsPanelComponent.Cast(m_AddonInfoPanel.FindHandler(SCR_AddonDetailsPanelComponent));

		return true;
	}
}
