// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/layouts/Menus/ContentBrowser/AddonManager/AddonsTools/AddonsExport/AddonsExportDialog.layout

class SCR_AddonsExportDialogWidgets
{
	static const ResourceName s_sLayout = "{06F005FC955C7B13}UI/layouts/Menus/ContentBrowser/AddonManager/AddonsTools/AddonsExport/AddonsExportDialog.layout";
	ResourceName GetLayout() { return s_sLayout; }

	VerticalLayoutWidget m_TabViewRoot;
	SCR_TabViewComponent m_TabViewRootComponent;

	ScrollLayoutWidget m_ScrollLayout;
	SCR_GamepadScrollComponent m_ScrollLayoutComponent;

	TextWidget m_GenContent;

	ButtonWidget m_NavGenCli;
	SCR_InputButtonComponent m_NavGenCliComponent;

	ButtonWidget m_NavGenJson;
	SCR_InputButtonComponent m_NavGenJsonComponent;

	ButtonWidget m_NavCopy;
	SCR_InputButtonComponent m_NavCopyComponent;

	bool Init(Widget root)
	{
		m_TabViewRoot = VerticalLayoutWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Content.m_TabViewRoot"));
		m_TabViewRootComponent = SCR_TabViewComponent.Cast(m_TabViewRoot.FindHandler(SCR_TabViewComponent));

		m_ScrollLayout = ScrollLayoutWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Content.AddonsExportRoot.Overlay.m_ScrollLayout"));
		m_ScrollLayoutComponent = SCR_GamepadScrollComponent.Cast(m_ScrollLayout.FindHandler(SCR_GamepadScrollComponent));

		m_GenContent = TextWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Content.AddonsExportRoot.Overlay.m_ScrollLayout.m_GenContent"));

		m_NavGenCli = ButtonWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Footer.m_NavGenCli"));
		m_NavGenCliComponent = SCR_InputButtonComponent.Cast(m_NavGenCli.FindHandler(SCR_InputButtonComponent));

		m_NavGenJson = ButtonWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Footer.m_NavGenJson"));
		m_NavGenJsonComponent = SCR_InputButtonComponent.Cast(m_NavGenJson.FindHandler(SCR_InputButtonComponent));

		m_NavCopy = ButtonWidget.Cast(root.FindWidget("DialogBase0.VerticalLayout.Footer.m_NavCopy"));
		m_NavCopyComponent = SCR_InputButtonComponent.Cast(m_NavCopy.FindHandler(SCR_InputButtonComponent));

		return true;
	}
}
