// Autogenerated by the Generate Class from Layout plugin, version 0.2.0
// Layout file: UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_Panel.layout

class SCR_DownloadManager_PanelWidgets
{
	static const ResourceName s_sLayout = "{5742DEC52260A48C}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_Panel.layout";
	ResourceName GetLayout() { return s_sLayout; }

	FrameWidget m_Panel;

	ButtonWidget m_OpenButton;
	SCR_InputButtonComponent m_OpenButtonComponent;

	bool Init(Widget root)
	{
		m_Panel = FrameWidget.Cast(root.FindWidget("m_Panel"));

		m_OpenButton = ButtonWidget.Cast(root.FindWidget("m_Panel.m_OpenButton"));
		m_OpenButtonComponent = SCR_InputButtonComponent.Cast(m_OpenButton.FindHandler(SCR_InputButtonComponent));

		return true;
	}
};
