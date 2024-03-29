// Autogenerated by the Generate Class from Layout plugin, version 0.2.0
// Layout file: UI/layouts/Menus/ContentBrowser/DetailsMenu/Gallery/ContentBrowser_Gallery.layout

class SCR_ContentBrowser_GalleryWidgets
{
	static const ResourceName s_sLayout = "{8244E8D2AD01E149}UI/layouts/Menus/ContentBrowser/DetailsMenu/Gallery/ContentBrowser_Gallery.layout";
	ResourceName GetLayout() { return s_sLayout; }

	SizeLayoutWidget m_ImagesHeightSize;

	ButtonWidget m_NextButton;
	SCR_ModularButtonComponent m_NextButtonComponent;

	ButtonWidget m_PrevButton;
	SCR_ModularButtonComponent m_PrevButtonComponent;

	ButtonWidget m_SpinBox;
	SCR_SpinBoxComponent m_SpinBoxComponent;

	bool Init(Widget root)
	{
		m_ImagesHeightSize = SizeLayoutWidget.Cast(root.FindWidget("m_ImagesHeightSize"));

		m_NextButton = ButtonWidget.Cast(root.FindWidget("m_ImagesHeightSize.Overlay.ArrowsOverlay.m_NextButton"));
		m_NextButtonComponent = SCR_ModularButtonComponent.Cast(m_NextButton.FindHandler(SCR_ModularButtonComponent));

		m_PrevButton = ButtonWidget.Cast(root.FindWidget("m_ImagesHeightSize.Overlay.ArrowsOverlay.m_PrevButton"));
		m_PrevButtonComponent = SCR_ModularButtonComponent.Cast(m_PrevButton.FindHandler(SCR_ModularButtonComponent));

		m_SpinBox = ButtonWidget.Cast(root.FindWidget("m_SpinBox"));
		m_SpinBoxComponent = SCR_SpinBoxComponent.Cast(m_SpinBox.FindHandler(SCR_SpinBoxComponent));

		return true;
	}
};
