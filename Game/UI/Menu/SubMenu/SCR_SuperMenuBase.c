//------------------------------------------------------------------------------------------------
class SCR_SuperMenuBase : MenuRootBase //ChimeraMenuBase
{
	protected ResourceName m_sNavigationButtonLayout = "{08CF3B69CB1ACBC4}UI/layouts/WidgetLibrary/WLib_NavigationButton.layout";
	protected const string TAB_VIEW_NAME = "TabViewRoot0";
	protected const string TITLE_NAME = "Title";
	protected int m_iActiveTab = -1;
	protected SCR_TabViewComponent m_TabViewComponent;
	protected SCR_SubMenuBase m_OpenedSubmenu;
	protected ref array<SCR_SubMenuBase> m_aSubMenus = {};

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		if (m_OpenedSubmenu && m_OpenedSubmenu.GetRootWidget().IsVisible())
			m_OpenedSubmenu.OnMenuUpdate(this, tDelta);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		// Find tabViewComponent, listen to OnChange
		Widget w = GetRootWidget();
		Widget tab = w.FindAnyWidget(TAB_VIEW_NAME);
		if (!tab)
			return;

		m_TabViewComponent = SCR_TabViewComponent.Cast(tab.FindHandler(SCR_TabViewComponent));
		if (!m_TabViewComponent)
			return;

		m_TabViewComponent.m_OnContentCreate.Insert(OnTabCreate);
		m_TabViewComponent.m_OnContentShow.Insert(OnTabShow);
		m_TabViewComponent.m_OnContentHide.Insert(OnTabHide);
		m_TabViewComponent.m_OnContentRemove.Insert(OnTabRemove);
		m_TabViewComponent.m_OnTabChange.Insert(OnTabChange);

		m_TabViewComponent.Init();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		if (m_OpenedSubmenu)
			m_OpenedSubmenu.OnMenuShow(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		
		if (m_OpenedSubmenu)
			m_OpenedSubmenu.OnMenuHide(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();

		foreach (SCR_SubMenuBase subMenu : m_aSubMenus)
		{
			if (subMenu)
				subMenu.OnMenuClose(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnTabCreate(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;

		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;

		subMenu.OnMenuOpen(this);
		m_aSubMenus.Insert(subMenu);
	}

	//------------------------------------------------------------------------------------------------
	void OnTabShow(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;

		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;

		subMenu.OnMenuShow(this);
		m_OpenedSubmenu = subMenu;
	}

	//------------------------------------------------------------------------------------------------
	void OnTabChange(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;

		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;

		subMenu.OnTabChange(this);
	}

	//------------------------------------------------------------------------------------------------
	void OnTabHide(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;

		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;

		subMenu.OnMenuHide(this);
	}

	//------------------------------------------------------------------------------------------------
	void OnTabRemove(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;

		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;

		subMenu.OnMenuClose(this);
		m_aSubMenus.RemoveItem(subMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! rightFooter - when true, the button will be added to the right footer.
	SCR_InputButtonComponent AddNavigationButton(string action, string buttonText, bool rightFooter = false)
	{
		if (!m_DynamicFooter || m_sNavigationButtonLayout == string.Empty)
			return null;

		SCR_EDynamicFooterButtonAlignment alignment = SCR_EDynamicFooterButtonAlignment.LEFT;
		if (rightFooter)
			alignment = SCR_EDynamicFooterButtonAlignment.RIGHT;

		return m_DynamicFooter.CreateButton(m_sNavigationButtonLayout, buttonText, buttonText, action, alignment);
	}

	//------------------------------------------------------------------------------------------------
	void CloseMenu(bool animate = true)
	{
		if (!animate)
		{
			Close();
			return;
		}

		AnimateWidget.Opacity(GetRootWidget(), 0, UIConstants.FADE_RATE_FAST);
		float time = 1 / UIConstants.FADE_RATE_FAST;
		GetGame().GetCallqueue().CallLater(Close, time);
	}

	//------------------------------------------------------------------------------------------------
	void SetTitle(string title)
	{
		TextWidget wTitle = TextWidget.Cast(GetRootWidget().FindAnyWidget(TITLE_NAME));
		if (wTitle)
			wTitle.SetText(title);
	}

	//------------------------------------------------------------------------------------------------
	void OpenSubMenu(int index)
	{
		if (m_TabViewComponent)
			m_TabViewComponent.ShowTab(index);
	}

	//------------------------------------------------------------------------------------------------
	SCR_SubMenuBase GetOpenedSubMenu()
	{
		return m_OpenedSubmenu;
	}
}
