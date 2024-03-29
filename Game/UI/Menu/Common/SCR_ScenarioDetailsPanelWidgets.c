// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/layouts/Menus/Common/DetailsPanel/ScenarioDetailsPanel.layout

class SCR_ScenarioDetailsPanelWidgets
{
	static const ResourceName s_sLayout = "{8B4FB3B5604EEE1B}UI/layouts/Menus/Common/DetailsPanel/ScenarioDetailsPanel.layout";
	ResourceName GetLayout() { return s_sLayout; }

	SizeLayoutWidget m_TopSize;

	OverlayWidget m_BackendImage;
	SCR_ScenarioBackendImageComponent m_BackendImageComponent;

	ImageWidget m_Image;

	ButtonWidget m_LoadingOverlay;
	SCR_LoadingOverlay m_LoadingOverlayComponent;

	BlurWidget m_Blur;

	ImageWidget m_BackgroundImage;

	ImageWidget m_AnimationImage;
	SCR_SpinningWidgetComponent m_AnimationImageComponent;

	TextWidget m_Text;

	FrameWidget m_MainArea;

	TextWidget m_NameText;

	HorizontalLayoutWidget m_RatingOverlay;

	ImageWidget m_RatingIcon;

	TextWidget m_RatingText;

	TextWidget m_AuthorNameText;

	HorizontalLayoutWidget m_TypeOverlay;

	TextWidget m_TypeText;

	HorizontalLayoutWidget m_TypeImages;

	HorizontalLayoutWidget m_DownloadedOverlay;

	TextWidget m_DownloadedText;

	HorizontalLayoutWidget m_LastPlayedOverlay;

	TextWidget m_LastPlayedText;

	TextWidget m_DescriptionText;

	HorizontalLayoutWidget m_AddonStateOverlay;

	TextWidget m_VersionText0;

	ImageWidget m_VersionArrow;

	TextWidget m_VersionText1;

	TextWidget m_AddonStateText;

	ImageWidget m_AddonStateIcon;

	HorizontalLayoutWidget m_AddonSizeOverlay;

	TextWidget m_AddonSizeText;
	
	OverlayWidget m_ImageCrossplayOverlay;

	bool Init(Widget root)
	{
		m_TopSize = SizeLayoutWidget.Cast(root.FindWidget("ContentDetailsPanel.m_TopSize"));

		m_BackendImage = OverlayWidget.Cast(root.FindWidget("ContentDetailsPanel.m_TopSize.m_BackendImage"));
		m_BackendImageComponent = SCR_ScenarioBackendImageComponent.Cast(m_BackendImage.FindHandler(SCR_ScenarioBackendImageComponent));

		m_Image = ImageWidget.Cast(root.FindWidget("ContentDetailsPanel.m_TopSize.m_BackendImage.ImageScale.m_Image"));

		m_LoadingOverlay = ButtonWidget.Cast(root.FindWidget("ContentDetailsPanel.m_TopSize.m_BackendImage.m_LoadingOverlay"));
		m_LoadingOverlayComponent = SCR_LoadingOverlay.Cast(m_LoadingOverlay.FindHandler(SCR_LoadingOverlay));

		m_Blur = BlurWidget.Cast(root.FindWidget("ContentDetailsPanel.m_TopSize.m_BackendImage.m_LoadingOverlay.loadingOverlayRoot1.m_Blur"));

		m_BackgroundImage = ImageWidget.Cast(root.FindWidget("ContentDetailsPanel.m_TopSize.m_BackendImage.m_LoadingOverlay.loadingOverlayRoot1.m_BackgroundImage"));

		m_AnimationImage = ImageWidget.Cast(root.FindWidget("ContentDetailsPanel.m_TopSize.m_BackendImage.m_LoadingOverlay.loadingOverlayRoot1.m_AnimationImage"));
		m_AnimationImageComponent = SCR_SpinningWidgetComponent.Cast(m_AnimationImage.FindHandler(SCR_SpinningWidgetComponent));

		m_Text = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.m_TopSize.m_BackendImage.m_LoadingOverlay.loadingOverlayRoot1.m_Text"));

		m_MainArea = FrameWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea"));

		m_NameText = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line1.m_NameText"));

		m_RatingOverlay = HorizontalLayoutWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line1.m_RatingOverlay"));

		m_RatingIcon = ImageWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line1.m_RatingOverlay.Frame1.Frame0.m_RatingIcon"));

		m_RatingText = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line1.m_RatingOverlay.m_RatingText"));

		m_AuthorNameText = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.m_AuthorNameText"));

		m_TypeOverlay = HorizontalLayoutWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line0MinHeight.Line0.m_TypeOverlay"));

		m_TypeText = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line0MinHeight.Line0.m_TypeOverlay.m_TypeText"));

		m_TypeImages = HorizontalLayoutWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line0MinHeight.Line0.m_TypeOverlay.m_TypeImages"));

		m_DownloadedOverlay = HorizontalLayoutWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line2.m_DownloadedOverlay"));

		m_DownloadedText = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line2.m_DownloadedOverlay.m_DownloadedText"));

		m_LastPlayedOverlay = HorizontalLayoutWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line2.m_LastPlayedOverlay"));

		m_LastPlayedText = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.Line2.m_LastPlayedOverlay.m_LastPlayedText"));

		m_DescriptionText = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.m_DescriptionText"));

		m_AddonStateOverlay = HorizontalLayoutWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.StateAndSize.m_AddonStateOverlay"));

		m_VersionText0 = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.StateAndSize.m_AddonStateOverlay.m_VersionText0"));

		m_VersionArrow = ImageWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.StateAndSize.m_AddonStateOverlay.m_VersionArrow"));

		m_VersionText1 = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.StateAndSize.m_AddonStateOverlay.m_VersionText1"));

		m_AddonStateText = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.StateAndSize.m_AddonStateOverlay.m_AddonStateText"));

		m_AddonStateIcon = ImageWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.StateAndSize.m_AddonStateOverlay.m_AddonStateIcon"));

		m_AddonSizeOverlay = HorizontalLayoutWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.StateAndSize.m_AddonSizeOverlay"));

		m_AddonSizeText = TextWidget.Cast(root.FindWidget("ContentDetailsPanel.Bottom.m_MainArea.Offset.StateAndSize.m_AddonSizeOverlay.m_AddonSizeText"));

		m_ImageCrossplayOverlay = OverlayWidget.Cast(root.FindAnyWidget("ImageCrossplayOverlay"));
		
		return true;
	}
};
