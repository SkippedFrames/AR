/*!
Component which must be attached to tooltip which lists dependencies or dependent mods.
*/
class SCR_ContentBrowser_AddonsTooltipComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{30907DAA2D89E065}UI/layouts/Menus/Tooltips/Tooltip_ListLine_NoBG.layout")]
	protected ResourceName m_sAddonLineLayout;

	protected VerticalLayoutWidget m_wAddonsList;

	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		m_wAddonsList = VerticalLayoutWidget.Cast(w.FindAnyWidget("AddonsList"));

		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	void Init(array<ref SCR_WorkshopItem> addons)
	{
		foreach (SCR_WorkshopItem addon : addons)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sAddonLineLayout, m_wAddonsList);
			TextWidget wText = TextWidget.Cast(w.FindAnyWidget("Text"));
			if (wText)
				wText.SetText(addon.GetName());
		}
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ContentBrowser_AddonsTooltipComponent FindComponent(notnull Widget w)
	{
		return SCR_ContentBrowser_AddonsTooltipComponent.Cast(w.FindHandler(SCR_ContentBrowser_AddonsTooltipComponent));
	}
}
