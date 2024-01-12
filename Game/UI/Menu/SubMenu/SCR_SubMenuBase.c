//------------------------------------------------------------------------------------------------
class SCR_SubMenuBase : SCR_ScriptedWidgetComponent
{
	protected SCR_SuperMenuBase m_ParentMenu;
	//! Alternative super menu component used when menu is not available
	protected SCR_SuperMenuComponent m_AltParentMenuComponent;
	protected ref array<SCR_InputButtonComponent> m_aNavigationButtons  = new ref array<SCR_InputButtonComponent>();
	protected bool m_bShown;
	protected bool m_bFocused;

	//------------------------------------------------------------------------------------------------
	void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta);

	//------------------------------------------------------------------------------------------------
	void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		m_ParentMenu = parentMenu;

		//! Invoker for menu focus
		if (m_ParentMenu)
		{
			m_ParentMenu.GetOnMenuFocusGained().Insert(OnMenuFocusGained);
			m_ParentMenu.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
		}

		// Add navigation buttons through parent menu and insert into a listener
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		ShowNavigationButtons(true);
		m_bShown = true;
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		ShowNavigationButtons(false);
		m_bShown = false;
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		foreach (SCR_InputButtonComponent comp : m_aNavigationButtons)
		{
			if (!comp)
				continue;

			Widget w = comp.GetRootWidget();
			if (!w)
				continue;

			w.RemoveFromHierarchy();
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuFocusGained()
	{
		m_bFocused = true;
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuFocusLost()
	{
		m_bFocused = false;
	}

	//------------------------------------------------------------------------------------------------
	void OnTabChange(SCR_SuperMenuBase parentMenu);

	//------------------------------------------------------------------------------------------------
	//! Returns true when this submenu is shown.
	bool GetShown()
	{
		return m_bShown;
	}

	//------------------------------------------------------------------------------------------------
	//! Set super menu component used as alternative when super menu is not based on chimera menu
	void SetParentMenuComponent(SCR_SuperMenuComponent superMenu)
	{
		m_AltParentMenuComponent = superMenu;
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseParent()
	{
		if (m_ParentMenu)
			m_ParentMenu.CloseMenu();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowNavigationButtons(bool show)
	{
		foreach (SCR_InputButtonComponent comp : m_aNavigationButtons)
		{
			if (!comp)
				continue;

			Widget w = comp.GetRootWidget();
			if (!w)
				continue;

			w.SetVisible(show);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ButtonComponent GetButtonComponent(string name, Widget parent = null)
	{
		if (parent == null)
			parent = m_wRoot;

		Widget w = parent.FindAnyWidget(name);
		if (!w)
			return null;

		SCR_ButtonComponent comp = SCR_ButtonComponent.Cast(w.FindHandler(SCR_ButtonComponent));
		return comp;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_InputButtonComponent CreateNavigationButton(string actionName, string label, bool rightFooter = false)
	{
		SCR_InputButtonComponent comp = null;

		if (m_ParentMenu)
			comp = m_ParentMenu.AddNavigationButton(actionName, label, rightFooter);
		else if (m_AltParentMenuComponent)
			comp = 	m_AltParentMenuComponent.AddNavigationButton(actionName, label, rightFooter);

		if (!comp)
			return null;

		m_aNavigationButtons.Insert(comp);
		return comp;
	}
}
