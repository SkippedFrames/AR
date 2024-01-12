// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/layouts/Menus/ContentBrowser/GalleryDialog/ContentBrowser_GalleryDialog.layout

class SCR_ContentBrowser_GalleryDialogWidgets
{
	static const ResourceName s_sLayout = "{7D40DE47A476B49C}UI/layouts/Menus/ContentBrowser/GalleryDialog/ContentBrowser_GalleryDialog.layout";
	ResourceName GetLayout() { return s_sLayout; }

	ButtonWidget m_PrevButton;
	SCR_ModularButtonComponent m_PrevButtonComponent;

	OverlayWidget m_BackendImage;
	SCR_BackendImageComponent m_BackendImageComponent;

	ButtonWidget m_NextButton;
	SCR_ModularButtonComponent m_NextButtonComponent;

	ButtonWidget m_SpinBox;
	SCR_SpinBoxComponent m_SpinBoxComponent;

	ButtonWidget m_BackButton;
	SCR_InputButtonComponent m_BackButtonComponent;

	ButtonWidget m_CloseButton;
	SCR_ModularButtonComponent m_CloseButtonComponent;

	bool Init(Widget root)
	{
		m_PrevButton = ButtonWidget.Cast(root.FindWidget("Overlay.VerticalLayout0.ImageAndButtons.PrevButton.Button"));
		m_PrevButtonComponent = SCR_ModularButtonComponent.Cast(m_PrevButton.FindHandler(SCR_ModularButtonComponent));

		m_BackendImage = OverlayWidget.Cast(root.FindWidget("Overlay.VerticalLayout0.ImageAndButtons.ImageSize.m_BackendImage"));
		m_BackendImageComponent = SCR_BackendImageComponent.Cast(m_BackendImage.FindHandler(SCR_BackendImageComponent));

		m_NextButton = ButtonWidget.Cast(root.FindWidget("Overlay.VerticalLayout0.ImageAndButtons.NextButton.Button"));
		m_NextButtonComponent = SCR_ModularButtonComponent.Cast(m_NextButton.FindHandler(SCR_ModularButtonComponent));

		m_SpinBox = ButtonWidget.Cast(root.FindWidget("Overlay.VerticalLayout0.m_SpinBox"));
		m_SpinBoxComponent = SCR_SpinBoxComponent.Cast(m_SpinBox.FindHandler(SCR_SpinBoxComponent));

		m_BackButton = ButtonWidget.Cast(root.FindWidget("Overlay.BottomRowFrame.BottomRowFrame.ButtonsLayout.m_BackButton"));
		m_BackButtonComponent = SCR_InputButtonComponent.Cast(m_BackButton.FindHandler(SCR_InputButtonComponent));

		m_CloseButton = ButtonWidget.Cast(root.FindWidget("Overlay.CloseButtonSize.m_CloseButton"));
		m_CloseButtonComponent = SCR_ModularButtonComponent.Cast(m_CloseButton.FindHandler(SCR_ModularButtonComponent));

		return true;
	}
};
