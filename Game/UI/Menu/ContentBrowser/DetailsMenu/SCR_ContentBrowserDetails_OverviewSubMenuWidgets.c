// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/layouts/Menus/ContentBrowser/DetailsMenu/ContentBrowserDetails_OverviewSubMenu.layout

class SCR_ContentBrowserDetails_OverviewSubMenuWidgets
{
	static const ResourceName s_sLayout = "{18E59BE07F4DB110}UI/layouts/Menus/ContentBrowser/DetailsMenu/ContentBrowserDetails_OverviewSubMenu.layout";
	ResourceName GetLayout() { return s_sLayout; }

	ScrollLayoutWidget m_MainContentScroll;
	SCR_GamepadScrollComponent m_MainContentScrollComponent0;
	SCR_EventHandlerComponent m_MainContentScrollComponent1;

	VerticalLayoutWidget m_MainContent;

	VerticalLayoutWidget m_Gallery;
	SCR_ContentBrowserDetails_GalleryComponent m_GalleryComponent;

	SizeLayoutWidget m_ImagesHeightSize;

	ButtonWidget ImageWithBackground0;
	SCR_ModularButtonComponent ImageWithBackground0Component0;
	SCR_BackendImageComponent ImageWithBackground0Component1;

	ButtonWidget ImageWithBackground1;
	SCR_ModularButtonComponent ImageWithBackground1Component0;
	SCR_BackendImageComponent ImageWithBackground1Component1;

	ButtonWidget ImageWithBackground2;
	SCR_ModularButtonComponent ImageWithBackground2Component0;
	SCR_BackendImageComponent ImageWithBackground2Component1;

	ButtonWidget m_NextButton;
	SCR_ModularButtonComponent m_NextButtonComponent;

	ButtonWidget m_PrevButton;
	SCR_ModularButtonComponent m_PrevButtonComponent;

	ButtonWidget m_SpinBox;
	SCR_SpinBoxComponent m_SpinBoxComponent0;
	SCR_ModularButtonComponent m_SpinBoxComponent1;

	VerticalLayoutWidget m_MainSection;

	ButtonWidget m_EnableButton;
	SCR_ModularButtonComponent m_EnableButtonComponent;

	ButtonWidget m_DownloadButton;
	SCR_ModularButtonComponent m_DownloadButtonComponent;

	ButtonWidget m_DeleteButton;
	SCR_ModularButtonComponent m_DeleteButtonComponent;

	ButtonWidget m_DownloadingButton;
	SCR_ModularButtonComponent m_DownloadingButtonComponent;

	ButtonWidget m_SolveIssuesButton;
	SCR_ModularButtonComponent m_SolveIssuesButtonComponent;

	ButtonWidget m_VoteUpButton;
	SCR_ModularButtonComponent m_VoteUpButtonComponent;

	ButtonWidget m_VoteDownButton;
	SCR_ModularButtonComponent m_VoteDownButtonComponent;

	ButtonWidget m_ReportButton;
	SCR_ModularButtonComponent m_ReportButtonComponent;

	ButtonWidget m_FavoriteButton;
	SCR_ModularButtonComponent m_FavoriteButtonComponent;

	ButtonWidget m_VersionComboBox;
	SCR_ComboBoxComponent m_VersionComboBoxComponent0;
	SCR_ModularButtonComponent m_VersionComboBoxComponent1;

	TextWidget m_NameText;

	TextWidget m_AuthorNameText;

	HorizontalLayoutWidget m_RatingOverlay;

	ImageWidget m_RatingIcon;

	TextWidget m_RatingText;

	TextWidget m_DescriptionText;

	VerticalLayoutWidget m_ScenarioSection;

	VerticalLayoutWidget m_ScenariosList;

	OverlayWidget m_ScenarioDetailsPanel;
	SCR_ScenarioDetailsPanelComponent m_ScenarioDetailsPanelComponent;

	OverlayWidget m_AddonDetailsPanel;
	SCR_AddonDetailsPanelComponent m_AddonDetailsPanelComponent;

	bool Init(Widget root)
	{
		m_MainContentScroll = ScrollLayoutWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll"));
		m_MainContentScrollComponent0 = SCR_GamepadScrollComponent.Cast(m_MainContentScroll.FindHandler(SCR_GamepadScrollComponent));
		m_MainContentScrollComponent1 = SCR_EventHandlerComponent.Cast(m_MainContentScroll.FindHandler(SCR_EventHandlerComponent));

		m_MainContent = VerticalLayoutWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent"));

		m_Gallery = VerticalLayoutWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_Gallery"));
		m_GalleryComponent = SCR_ContentBrowserDetails_GalleryComponent.Cast(m_Gallery.FindHandler(SCR_ContentBrowserDetails_GalleryComponent));

		m_ImagesHeightSize = SizeLayoutWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_Gallery.m_ImagesHeightSize"));

		ImageWithBackground0 = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_Gallery.m_ImagesHeightSize.Overlay.Images.ImageWithBackground0"));
		ImageWithBackground0Component0 = SCR_ModularButtonComponent.Cast(ImageWithBackground0.FindHandler(SCR_ModularButtonComponent));
		ImageWithBackground0Component1 = SCR_BackendImageComponent.Cast(ImageWithBackground0.FindHandler(SCR_BackendImageComponent));

		ImageWithBackground1 = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_Gallery.m_ImagesHeightSize.Overlay.Images.ImageWithBackground1"));
		ImageWithBackground1Component0 = SCR_ModularButtonComponent.Cast(ImageWithBackground1.FindHandler(SCR_ModularButtonComponent));
		ImageWithBackground1Component1 = SCR_BackendImageComponent.Cast(ImageWithBackground1.FindHandler(SCR_BackendImageComponent));

		ImageWithBackground2 = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_Gallery.m_ImagesHeightSize.Overlay.Images.ImageWithBackground2"));
		ImageWithBackground2Component0 = SCR_ModularButtonComponent.Cast(ImageWithBackground2.FindHandler(SCR_ModularButtonComponent));
		ImageWithBackground2Component1 = SCR_BackendImageComponent.Cast(ImageWithBackground2.FindHandler(SCR_BackendImageComponent));

		m_NextButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_Gallery.m_ImagesHeightSize.Overlay.ArrowsOverlay.m_NextButton"));
		m_NextButtonComponent = SCR_ModularButtonComponent.Cast(m_NextButton.FindHandler(SCR_ModularButtonComponent));

		m_PrevButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_Gallery.m_ImagesHeightSize.Overlay.ArrowsOverlay.m_PrevButton"));
		m_PrevButtonComponent = SCR_ModularButtonComponent.Cast(m_PrevButton.FindHandler(SCR_ModularButtonComponent));

		m_SpinBox = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_Gallery.m_SpinBox"));
		m_SpinBoxComponent0 = SCR_SpinBoxComponent.Cast(m_SpinBox.FindHandler(SCR_SpinBoxComponent));
		m_SpinBoxComponent1 = SCR_ModularButtonComponent.Cast(m_SpinBox.FindHandler(SCR_ModularButtonComponent));

		m_MainSection = VerticalLayoutWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection"));

		m_EnableButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.Buttons.m_EnableButton"));
		m_EnableButtonComponent = SCR_ModularButtonComponent.Cast(m_EnableButton.FindHandler(SCR_ModularButtonComponent));

		m_DownloadButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.Buttons.m_DownloadButton"));
		m_DownloadButtonComponent = SCR_ModularButtonComponent.Cast(m_DownloadButton.FindHandler(SCR_ModularButtonComponent));

		m_DeleteButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.Buttons.m_DeleteButton"));
		m_DeleteButtonComponent = SCR_ModularButtonComponent.Cast(m_DeleteButton.FindHandler(SCR_ModularButtonComponent));

		m_DownloadingButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.Buttons.m_DownloadingButton"));
		m_DownloadingButtonComponent = SCR_ModularButtonComponent.Cast(m_DownloadingButton.FindHandler(SCR_ModularButtonComponent));

		m_SolveIssuesButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.Buttons.m_SolveIssuesButton"));
		m_SolveIssuesButtonComponent = SCR_ModularButtonComponent.Cast(m_SolveIssuesButton.FindHandler(SCR_ModularButtonComponent));

		m_VoteUpButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.Buttons.m_VoteUpButton"));
		m_VoteUpButtonComponent = SCR_ModularButtonComponent.Cast(m_VoteUpButton.FindHandler(SCR_ModularButtonComponent));

		m_VoteDownButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.Buttons.m_VoteDownButton"));
		m_VoteDownButtonComponent = SCR_ModularButtonComponent.Cast(m_VoteDownButton.FindHandler(SCR_ModularButtonComponent));

		m_ReportButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.Buttons.m_ReportButton"));
		m_ReportButtonComponent = SCR_ModularButtonComponent.Cast(m_ReportButton.FindHandler(SCR_ModularButtonComponent));

		m_FavoriteButton = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.Buttons.m_FavoriteButton"));
		m_FavoriteButtonComponent = SCR_ModularButtonComponent.Cast(m_FavoriteButton.FindHandler(SCR_ModularButtonComponent));

		m_VersionComboBox = ButtonWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.m_VersionComboBox"));
		m_VersionComboBoxComponent0 = SCR_ComboBoxComponent.Cast(m_VersionComboBox.FindHandler(SCR_ComboBoxComponent));
		m_VersionComboBoxComponent1 = SCR_ModularButtonComponent.Cast(m_VersionComboBox.FindHandler(SCR_ModularButtonComponent));

		m_NameText = TextWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.m_NameText"));

		m_AuthorNameText = TextWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.AuthorNameAndRating.m_AuthorNameText"));

		m_RatingOverlay = HorizontalLayoutWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.AuthorNameAndRating.m_RatingOverlay"));

		m_RatingIcon = ImageWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.AuthorNameAndRating.m_RatingOverlay.Frame1.Frame0.m_RatingIcon"));

		m_RatingText = TextWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.AuthorNameAndRating.m_RatingOverlay.m_RatingText"));

		m_DescriptionText = TextWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_MainSection.MainSectionContent.m_DescriptionText"));

		m_ScenarioSection = VerticalLayoutWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_ScenarioSection"));

		m_ScenariosList = VerticalLayoutWidget.Cast(root.FindWidget("MainPanel.VerticalLayout0.Content.m_MainContentScroll.m_MainContent.m_ScenarioSection.m_ScenariosList"));

		m_ScenarioDetailsPanel = OverlayWidget.Cast(root.FindWidget("RightPanel.VerticalLayout0.Content.m_ScenarioDetailsPanel"));
		m_ScenarioDetailsPanelComponent = SCR_ScenarioDetailsPanelComponent.Cast(m_ScenarioDetailsPanel.FindHandler(SCR_ScenarioDetailsPanelComponent));

		m_AddonDetailsPanel = OverlayWidget.Cast(root.FindWidget("RightPanel.VerticalLayout0.Content.m_AddonDetailsPanel"));
		m_AddonDetailsPanelComponent = SCR_AddonDetailsPanelComponent.Cast(m_AddonDetailsPanel.FindHandler(SCR_AddonDetailsPanelComponent));

		return true;
	}
}
