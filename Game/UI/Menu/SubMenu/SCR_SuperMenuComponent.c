/*!
Super menu base as component to apply super menu behavior for non menus 
*/

class SCR_SuperMenuComponent : ScriptedWidgetComponent
{
	protected const static string TITLE = "Title";
	protected const static string TAB_VIEW_NAME = "TabView";
	protected const static string LEFT_FOOTER = "Footer";
	protected const static string RIGHT_FOOTER = "FooterRight";
	protected const static ResourceName DEFAULT_NAV_BUTTON = "{08CF3B69CB1ACBC4}UI/layouts/WidgetLibrary/Buttons/WLib_NavigationButton.layout";
	
	// Attributes 
	[Attribute(TITLE, UIWidgets.EditBox, desc: "Name of title widget.")]
	protected string m_sTitle;
	
	[Attribute(TAB_VIEW_NAME, UIWidgets.EditBox, desc: "Name of tabview widget to find tab view component.")]
	protected string m_sTabViewName;
	
	[Attribute(LEFT_FOOTER, UIWidgets.EditBox, desc: "Name of footer root for nav buttons on left.")]
	protected string m_sLeftFooter;
	
	[Attribute(RIGHT_FOOTER, UIWidgets.EditBox, desc: "Name of footer root for nav buttons on right.")]
	protected string m_sRightFooter;
	
	[Attribute(DEFAULT_NAV_BUTTON, UIWidgets.ResourceNamePicker, desc: "Layout of navigation button.")]
	protected ResourceName m_NavButtonLayout;
	
	// Fields 
	protected Widget m_wRoot;
	protected TextWidget m_wTitle;
	protected SCR_TabViewComponent m_TabView;
	protected Widget m_wLeftFooter;
	protected Widget m_wRightFooter;
	
	protected SCR_SubMenuBase m_OpenedSubmenu;
	protected ref array<SCR_SubMenuBase> m_aSubMenus = new ref array<SCR_SubMenuBase>;
	
	//------------------------------------------------------------------------------------------------
	// Override 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		Init(w);
	}
	
	//------------------------------------------------------------------------------------------------
	// Public 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Setup widgets and action
	void Init(Widget w)
	{
		m_wRoot = w;
		
		m_wTitle = TextWidget.Cast(w.FindAnyWidget(m_sTitle));
		
		// Tab view 
		Widget tabView = w.FindAnyWidget(m_sTabViewName); 
		if (tabView)
			m_TabView = SCR_TabViewComponent.Cast(tabView.FindHandler(SCR_TabViewComponent));
	
		if (m_TabView)
		{
			m_TabView.m_OnContentCreate.Insert(OnTabCreate);
			m_TabView.m_OnContentShow.Insert(OnTabShow);
			m_TabView.m_OnContentHide.Insert(OnTabHide);
			m_TabView.m_OnContentRemove.Insert(OnTabRemove);
			
			m_TabView.Init();
			
			// Init opened content 
			int selected = m_TabView.m_iSelectedTab;
			Widget content =  m_TabView.GetEntryContent(selected).m_wTab;
			OnTabShow(m_TabView, content);
		}

		// Footers 
		m_wLeftFooter = w.FindAnyWidget(m_sLeftFooter);
		m_wRightFooter = w.FindAnyWidget(m_sRightFooter); 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update content of opened submenu
	void Update(float tDelta)
	{
		if (m_OpenedSubmenu && m_OpenedSubmenu.GetRootWidget().IsVisible())
			m_OpenedSubmenu.OnMenuUpdate(null, tDelta);
	}
	
	//------------------------------------------------------------------------------------------------
	void Show()
	{
		if (m_OpenedSubmenu)
			m_OpenedSubmenu.OnMenuShow(null);
	}
	
	//------------------------------------------------------------------------------------------------
	void Hide()
	{
		if (m_OpenedSubmenu)
			m_OpenedSubmenu.OnMenuHide(null);
	}
	
	//------------------------------------------------------------------------------------------------
	void Close()
	{
		foreach (SCR_SubMenuBase subMenu : m_aSubMenus)
		{
			if (subMenu)
				subMenu.OnMenuClose(null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTitle(string title)
	{
		if (!m_wTitle)
			return;
		
		if (m_wTitle)
			m_wTitle.SetText(title);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add new navigation button, sub (SCR_SubMenuBase) menu should be owner and take care of display and hide
	//! rightFooter - when true, the button will be added to the right footer.
	SCR_InputButtonComponent AddNavigationButton(string action, string buttonText, bool rightFooter = false)
	{
		// Fallback widget search
		if (!m_wLeftFooter)
			m_wLeftFooter = m_wRoot.FindAnyWidget(m_sLeftFooter);
		
		if (!m_wRightFooter)
			m_wRightFooter = m_wRoot.FindAnyWidget(m_sRightFooter); 
		
		// Check widget 
		if (!m_wLeftFooter || !m_wRightFooter || m_NavButtonLayout == string.Empty)
			return null;
		
		Widget footer = m_wLeftFooter;
		if (rightFooter)
		{
			footer = m_wRightFooter;
		}
			
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_NavButtonLayout, footer);
		if (!w)
			return null;
		
		//Handle padding
		float padding, paddingLeft, paddingRight;
		float a, b, c;
		AlignableSlot.GetPadding(w, a, b, padding, c);
		if(rightFooter)
			paddingLeft = padding;
		else 
			paddingRight = padding;
		
		AlignableSlot.SetPadding(w, paddingLeft, 0.0, paddingRight, 0.0);
		
		
		SCR_InputButtonComponent comp = SCR_InputButtonComponent.Cast(w.FindHandler(SCR_InputButtonComponent));
		if (!comp)
			return null;
		
		comp.SetLabel(buttonText);
		comp.SetAction(action);
		return comp;
	}
	
	//------------------------------------------------------------------------------------------------
	// Tab view callbacks 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void OnTabCreate(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;
		
		subMenu.SetParentMenuComponent(this);
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
		
		subMenu.OnMenuShow(null);
		subMenu.SetParentMenuComponent(this);
		m_OpenedSubmenu = subMenu;
	}

	
	//------------------------------------------------------------------------------------------------
	//TODO: this never gets called
	void OnTabHide(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;
		
		subMenu.OnMenuHide(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO: this gets called when switching tab (if the tab is set to be destroyed), but not when closing the dialog
	void OnTabRemove(SCR_TabViewComponent comp, Widget w)
	{
		if (!w)
			return;
		
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;
		
		subMenu.OnMenuClose(null);
		m_aSubMenus.RemoveItem(subMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TabViewComponent GetTabView()
	{
		return m_TabView;
	}
}
