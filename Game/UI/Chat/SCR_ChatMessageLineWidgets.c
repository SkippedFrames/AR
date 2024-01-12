// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/layouts/HUD/Chat/ChatMessageLine.layout

class SCR_ChatMessageLineWidgets
{
	static const ResourceName s_sLayout = "{973C90F6B6135A50}UI/layouts/HUD/Chat/ChatMessageLine.layout";
	ResourceName GetLayout() { return s_sLayout; }

	ImageWidget m_BackgroundImage;

	ImageWidget m_TypeImage;

	TextWidget m_MessageText;

	SizeLayoutWidget m_Badge;

	bool Init(Widget root)
	{
		m_BackgroundImage = ImageWidget.Cast(root.FindWidget("SizeLayout0.Overlay0.m_BackgroundImage"));

		m_TypeImage = ImageWidget.Cast(root.FindWidget("SizeLayout0.Overlay0.MessageLayout.m_TypeImage"));

		m_MessageText = TextWidget.Cast(root.FindWidget("SizeLayout0.Overlay0.MessageLayout.Width.m_MessageText"));

		m_Badge = SizeLayoutWidget.Cast(root.FindWidget("SizeLayout0.Overlay0.m_Badge"));

		return true;
	}
}