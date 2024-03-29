// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/layouts/Menus/MainMenu/IntroSplashScreen.layout

class SCR_IntroSplashScreenWidgets
{
	static const ResourceName s_sLayout = "{1C71B463B3B66BAB}UI/layouts/Menus/MainMenu/IntroSplashScreen.layout";
	ResourceName GetLayout() { return s_sLayout; }

	OverlayWidget m_wBackground;

	FrameWidget m_wContent;

	ImageWidget m_wArt;

	ImageWidget m_wArtExperimental;

	ImageWidget m_wDust1;

	ImageWidget m_wDust2;

	ScaleWidget m_wBILogo;

	ScaleWidget m_wEnfusionLogo;

	ScaleWidget m_wReforgerLogo;

	VerticalLayoutWidget m_wWarning;

	OverlayWidget m_wExperimentalBuild;

	RichTextWidget m_wExperimentalTitle;

	RichTextWidget m_wExperimentalText;

	RichTextWidget m_wPressKeyMsg;

	TextWidget m_wDisclaimer;

	FrameWidget m_wSpinner;

	ImageWidget m_wBlackOverlay;

	bool Init(Widget root)
	{
		m_wBackground = OverlayWidget.Cast(root.FindAnyWidget("m_wBackground"));

		m_wContent = FrameWidget.Cast(root.FindAnyWidget("m_wContent"));

		m_wArt = ImageWidget.Cast(root.FindAnyWidget("m_wArt"));

		m_wArtExperimental = ImageWidget.Cast(root.FindAnyWidget("m_wArtExperimental"));

		m_wDust1 = ImageWidget.Cast(root.FindAnyWidget("m_wDust1"));

		m_wDust2 = ImageWidget.Cast(root.FindAnyWidget("m_wDust2"));

		m_wBILogo = ScaleWidget.Cast(root.FindAnyWidget("m_wBILogo"));

		m_wEnfusionLogo = ScaleWidget.Cast(root.FindAnyWidget("m_wEnfusionLogo"));

		m_wReforgerLogo = ScaleWidget.Cast(root.FindAnyWidget("m_wReforgerLogo"));

		m_wWarning = VerticalLayoutWidget.Cast(root.FindAnyWidget("m_wWarning"));

		m_wExperimentalBuild = OverlayWidget.Cast(root.FindAnyWidget("m_wExperimentalBuild"));

		m_wExperimentalTitle = RichTextWidget.Cast(root.FindAnyWidget("m_wExperimentalTitle"));

		m_wExperimentalText = RichTextWidget.Cast(root.FindAnyWidget("m_wExperimentalText"));

		m_wPressKeyMsg = RichTextWidget.Cast(root.FindAnyWidget("m_wPressKeyMsg"));

		m_wDisclaimer = TextWidget.Cast(root.FindAnyWidget("m_wDisclaimer"));

		m_wSpinner = FrameWidget.Cast(root.FindAnyWidget("m_wSpinner"));

		m_wBlackOverlay = ImageWidget.Cast(root.FindAnyWidget("m_wBlackOverlay"));

		return true;
	}
}
