// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManagerDialog.layout

class SCR_DownloadManagerDialogWidgets
{
	static const ResourceName s_sLayout = "{EE039503B1CCD8C6}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManagerDialog.layout";
	ResourceName GetLayout() { return s_sLayout; }

	ButtonWidget m_AutoenableAddonsButton;
	SCR_ModularButtonComponent m_AutoenableAddonsButtonComponent;

	OverlayWidget m_Overlay;

	VerticalLayoutWidget m_ProgressContent;

	ImageWidget m_ProgressIcon;

	TextWidget m_ProgressText;

	TextWidget m_StateText;

	TextWidget m_DownloadSpeed;

	SizeLayoutWidget m_ProgressBar;
	SCR_WLibProgressBarComponent m_ProgressBarComponent;

	RichTextWidget m_DownloadSummaryText;

	bool Init(Widget root)
	{
		m_AutoenableAddonsButton = ButtonWidget.Cast(root.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.DownloadManagerContent.m_AutoenableAddonsButton"));
		m_AutoenableAddonsButtonComponent = SCR_ModularButtonComponent.Cast(m_AutoenableAddonsButton.FindHandler(SCR_ModularButtonComponent));

		m_Overlay = OverlayWidget.Cast(root.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.DownloadManagerContent.Progress.m_Overlay"));

		m_ProgressContent = VerticalLayoutWidget.Cast(root.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.DownloadManagerContent.Progress.m_Overlay.m_ProgressContent"));

		m_ProgressIcon = ImageWidget.Cast(root.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.DownloadManagerContent.Progress.m_Overlay.m_ProgressContent.HorizontaTexts.Progress.m_ProgressIcon"));

		m_ProgressText = TextWidget.Cast(root.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.DownloadManagerContent.Progress.m_Overlay.m_ProgressContent.HorizontaTexts.Progress.m_ProgressText"));

		m_StateText = TextWidget.Cast(root.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.DownloadManagerContent.Progress.m_Overlay.m_ProgressContent.HorizontaTexts.m_StateText"));

		m_DownloadSpeed = TextWidget.Cast(root.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.DownloadManagerContent.Progress.m_Overlay.m_ProgressContent.HorizontaTexts.OverlaySpeed.m_DownloadSpeed"));

		m_ProgressBar = SizeLayoutWidget.Cast(root.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.DownloadManagerContent.Progress.m_Overlay.m_ProgressContent.m_ProgressBar"));
		m_ProgressBarComponent = SCR_WLibProgressBarComponent.Cast(m_ProgressBar.FindHandler(SCR_WLibProgressBarComponent));

		m_DownloadSummaryText = RichTextWidget.Cast(root.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.DownloadManagerContent.SizeSummary.m_DownloadSummaryText"));

		return true;
	}
}
