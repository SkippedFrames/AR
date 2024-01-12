void SCR_TabViewComponent_TabviewUpdate(SCR_TabViewComponent tabView, Widget widget);
typedef func SCR_TabViewComponent_TabviewUpdate;

void SCR_TabViewComponent_TabviewUpdateAndTabIndex(SCR_TabViewComponent tabView, Widget widget, int index);
typedef func SCR_TabViewComponent_TabviewUpdateAndTabIndex;

void SCR_TabViewComponent_TabviewUpdateAndTabIndex(SCR_TabViewComponent tabView, SCR_TabViewContent tabContent, int index);
typedef func SCR_TabViewComponent_TabviewUpdateAndTabContent;

//------------------------------------------------------------------------------------------------
class SCR_TabViewComponent : ScriptedWidgetComponent
{
	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref SCR_TabViewContent> m_aElements;

	[Attribute("0", UIWidgets.EditBox, "Element selected by default")]
	int m_iSelectedTab;

	[Attribute("", UIWidgets.ColorPicker, "Color of gaps between tabs")]
	ref Color m_ColorBackground;

	[Attribute("0", UIWidgets.CheckBox, "Do not delete tabs, which are hidden")]
	bool m_bKeepHiddenTabs;

	[Attribute("false", UIWidgets.CheckBox, "Create all tabs on initialization, do not wait for first selection")]
	bool m_bCreateAllTabsAtStart;

	[Attribute("false", UIWidgets.CheckBox, "Do not initialize yourself. This is for initialization by supermenu.")]
	bool m_bManualInit;

	[Attribute("0", UIWidgets.CheckBox, "Do not delete tabs, which are hidden")]
	bool m_bCycleMode;

	[Attribute("{668B05FDEF1D3268}UI/layouts/WidgetLibrary/WLib_TabViewElement.layout", UIWidgets.ResourceNamePicker, "Layout element used", "layout")]
	ResourceName m_TabLayout;

	[Attribute("true")]
	protected bool m_ListenToActions;

	[Attribute("MenuTabRight", UIWidgets.EditBox, "Action used for navigating left")]
	string m_sActionLeft;

	[Attribute("MenuTabLeft", UIWidgets.EditBox, "Action used for navigating right")]
	string m_sActionRight;

	[Attribute("300", UIWidgets.EditBox, "Select tab width. Use -1 to use fill mode")]
	float m_fTabWidth;

	[Attribute("32", UIWidgets.EditBox, "Select tab width. Use -1 to use fill mode")]
	float m_fTabWidthTextHidden;

	[Attribute(SCR_SoundEvent.TAB_SWITCH, UIWidgets.EditBox)]
	protected string m_sSwitchSound;

	[Attribute("0", desc: "If true it will hide the text of the tab and only show it when the tab is selected. Tabs should have images otherwise it might look weird")]
	protected bool m_bShowTextOnlyWhenSelectedTab;

	Widget m_wRoot;
	protected Widget m_wButtons;
	protected Widget m_wButtonBar;
	protected Widget m_wContentOverlay;
	protected SCR_PagingButtonComponent m_PagingLeft;
	protected SCR_PagingButtonComponent m_PagingRight;

	// Arguments passed: SCR_TabViewComponent, Widget (contentRoot), int currentIndex
	ref ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdateAndTabIndex> m_OnChanged = new ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdateAndTabIndex>();

	// Arguments passed for all invokers below: SCR_TabViewComponent, Widget (contentRoot)
	ref ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate> m_OnContentCreate = new ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate>();
	ref ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdateAndTabContent> m_OnContentSelect = new ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdateAndTabContent>(); //--- Called when a tab is selected, no matter if any content widget is tied to it
	ref ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate> m_OnContentShow = new ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate>();
	ref ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate> m_OnContentHide = new ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate>();
	ref ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate> m_OnContentRemove = new ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate>();
	ref ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate> m_OnTabChange = new ScriptInvokerBase<SCR_TabViewComponent_TabviewUpdate>();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		super.HandlerAttached(w);
		if (!m_bManualInit)
			Init();
	}

	//------------------------------------------------------------------------------------------------
	void Init()
	{
		// Prevent multiple initialization
		if (!m_wRoot || m_wButtonBar)
			return;

		m_wButtonBar = m_wRoot.FindAnyWidget("HorizontalLayout0");
		m_wButtons = m_wRoot.FindAnyWidget("Tabs");
		m_wContentOverlay = m_wRoot.FindAnyWidget("ContentOverlay");

		if (!m_wContentOverlay || !m_wButtonBar || !m_wButtons)
			return;

		m_PagingLeft = SCR_PagingButtonComponent.GetPagingButtonComponent("PagingLeft", m_wRoot);
		m_PagingRight = SCR_PagingButtonComponent.GetPagingButtonComponent("PagingRight", m_wRoot);

		if (!m_aElements)
			return;

		foreach (int i, SCR_TabViewContent content : m_aElements)
		{
			CreateTab(content);
			if (m_bCreateAllTabsAtStart)
				CreateTabContent(content);
		}

		int realSelected = m_iSelectedTab;
		m_iSelectedTab = -1;
		ShowTab(realSelected, true, false);

		if (m_PagingLeft)
			m_PagingLeft.SetAction(m_sActionLeft);

		if (m_PagingRight)
			m_PagingRight.SetAction(m_sActionRight);

		AddActionListeners();
		UpdatePagingButtons();

		if (m_bShowTextOnlyWhenSelectedTab)
			ShowAllTabsText(false, m_fTabWidthTextHidden);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		RemoveActionListeners();
	}

	//------------------------------------------------------------------------------------------------
	void CreateTab(SCR_TabViewContent content)
	{
		// Create tab button
		Widget button = GetGame().GetWorkspace().CreateWidgets(m_TabLayout, m_wButtons);
		if (!button)
			return;

		HorizontalLayoutSlot.SetPadding(button, 0, 0, 4, 0); // Add padding

		SCR_ButtonTextComponent comp = SCR_ButtonTextComponent.Cast(button.FindHandler(SCR_ButtonTextComponent));
		if (!comp)
			return;

		if (!content.m_bEnabled)
			comp.SetEnabled(false, false);

		if (m_fTabWidth < 0)
		{
			LayoutSlot.SetSizeMode(button, LayoutSizeMode.Fill);
			content.m_wLayout = SizeLayoutWidget.Cast(button.GetChildren());
		}
		else
		{
			SizeLayoutWidget layout = SizeLayoutWidget.Cast(button.GetChildren());
			content.m_wLayout = layout;
			if (layout)
			{
				layout.EnableWidthOverride(true);
				layout.SetWidthOverride(m_fTabWidth);
			}
		}

		comp.m_OnClicked.Insert(OnSelection);
		comp.SetTextWithParam(content.m_sTabButtonContent, content.m_sTabButtonContentParam1, content.m_sTabButtonContentParam2);
		content.m_ButtonComponent = comp;

		ImageWidget image = ImageWidget.Cast(button.FindAnyWidget("TabImage"));
		if (image)
		{
			image.SetVisible(content.m_bShowImage);
			if (content.m_bShowImage)
				SCR_WLibComponentBase.SetTexture(image, content.m_TabImage, content.m_sTabImageName);
		}
		
		Widget imageSize = button.FindAnyWidget("SizeTabImage");
		if (imageSize)
			imageSize.SetVisible(content.m_bShowImage);

		if (content.m_bShowIcon)
			CreateIcon(content);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateIcon(SCR_TabViewContent content)
	{
		Widget overlay = content.m_ButtonComponent.GetRootWidget().FindAnyWidget("Overlay");
		if (!overlay)
			return;

		SizeLayoutWidget size = SizeLayoutWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.SizeLayoutWidgetTypeID, WidgetFlags.VISIBLE, Color.White, 0, overlay));
		if (!size)
			return;

		float width = content.m_fIconWidth;
		float height = content.m_fIconHeight;

		size.EnableWidthOverride(true);
		size.EnableHeightOverride(true);
		size.SetWidthOverride(width);
		size.SetHeightOverride(height);

		OverlaySlot.SetVerticalAlign(size, LayoutVerticalAlign.Top);
		OverlaySlot.SetHorizontalAlign(size, LayoutHorizontalAlign.Right);
		OverlaySlot.SetPadding(size, 0, -height * 0.5, 4, 0); // Set some reasonable offset for the icon

		content.m_wIcon = GetGame().GetWorkspace().CreateWidgets(content.m_IconLayout, size);
	}

	//------------------------------------------------------------------------------------------------
	void CreateTabContent(SCR_TabViewContent content)
	{
		ResourceName path = content.m_ElementLayout;
		if (path == string.Empty)
			return;

		Widget w = GetGame().GetWorkspace().CreateWidgets(path, m_wContentOverlay);
		if (!w)
			return;

		OverlaySlot.SetVerticalAlign(w, LayoutVerticalAlign.Stretch);
		OverlaySlot.SetHorizontalAlign(w, LayoutHorizontalAlign.Stretch);
		m_OnContentCreate.Invoke(this, w);
		content.m_wTab = w;

		int i = m_aElements.Find(content);
		if (i != m_iSelectedTab)
		{
			w.SetVisible(false);
		}
		else
		{
			FocusFirstWidget(w);
			m_OnContentShow.Invoke(this, w);
		}

		SCR_MenuHelper.OnTabChange(ChimeraMenuBase.GetOwnerMenu(m_wRoot));
		m_OnTabChange.Invoke(this, w);
	}

	//------------------------------------------------------------------------------------------------
	void FocusFirstWidget(Widget w)
	{
		Widget child = w;
		while (child)
		{
			ButtonWidget button = ButtonWidget.Cast(child);
			if (button && button.IsEnabled())
			{
				GetGame().GetWorkspace().SetFocusedWidget(button);
				return;
			}

			child = child.GetChildren();
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowIcon(int entry, bool show)
	{
		if (entry < 0 || entry >= m_aElements.Count())
			return;

		Widget w = m_aElements[entry].m_wIcon;
		if (w)
			w.SetVisible(show);
		else if (show)
			CreateIcon(m_aElements[entry]);
	}

	//------------------------------------------------------------------------------------------------
	bool IsIconShown(int entry)
	{
		if (entry < 0 || entry >= m_aElements.Count())
			return false;

		Widget w = m_aElements[entry].m_wIcon;
		if (w && w.IsVisible())
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetEntryIcon(int entry)
	{
		if (entry < 0 || entry > m_aElements.Count())
			return null;

		return m_aElements[entry].m_wIcon;
	}

	SCR_TabViewContent GetEntryContent(int index)
	{
		if (index < 0 || index >= m_aElements.Count())
			return null;

		return m_aElements[index];
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectIndex(bool select, int i)
	{
		if (!m_aElements || i < 0 || i >= m_aElements.Count())
			return;

		SCR_TabViewContent content = m_aElements[i];
		if (!content)
			return;

		if (!content.m_ButtonComponent)
			return;

		content.m_ButtonComponent.SetToggled(select, false);

		Widget tab = content.m_wTab;
		if (select)
		{
			if (m_bKeepHiddenTabs && tab)
			{
				m_OnContentShow.Invoke(this, tab);
				tab.SetVisible(true);
				FocusFirstWidget(tab);
			}
			else
			{
				CreateTabContent(content);
			}
			m_OnContentSelect.Invoke(this, content, i);
		}
		else
		{
			if (!tab)
				return;

			if (m_bKeepHiddenTabs)
			{
				m_OnContentHide.Invoke(this, tab);
				tab.SetVisible(false);
			}
			else
			{
				m_OnContentRemove.Invoke(this, tab);
				tab.RemoveFromHierarchy();
			}
		}

		SCR_MenuHelper.OnTabChange(ChimeraMenuBase.GetOwnerMenu(m_wRoot));
		m_OnTabChange.Invoke(this, tab);
	}

	//------------------------------------------------------------------------------------------------
	void ShowTab(int i, bool callAction = true, bool playSound = true)
	{
		if (i == m_iSelectedTab)
			callAction = false;

		if (m_bShowTextOnlyWhenSelectedTab && i != m_iSelectedTab)
		{
			ShowTabText(i, true, m_fTabWidth);
			ShowTabText(m_iSelectedTab, false, m_fTabWidthTextHidden);
		}

		// Do not switch into invalid or disabled element
		if (i < 0 || i >= m_aElements.Count() || !m_aElements[i].m_bEnabled)
			return;

		// Deselect old tab, select new tab
		SelectIndex(false, m_iSelectedTab);
		m_iSelectedTab = i;
		SelectIndex(true, i);

		if (playSound)
			SCR_UISoundEntity.SoundEvent(m_sSwitchSound);

		if (callAction)
			m_OnChanged.Invoke(this, m_wRoot, m_iSelectedTab);

		UpdatePagingButtons();
	}

	//------------------------------------------------------------------------------------------------
	void ShowTabByIdentifier(string identifier, bool callAction = true, bool playSound = true)
	{
		for (int i = 0; i < m_aElements.Count(); i++)
		{
			if (m_aElements[i].m_sTabIdentifier == identifier)
			{
				ShowTab(i, callAction, playSound);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetShownTab()
	{
		return m_iSelectedTab;
	}

	//------------------------------------------------------------------------------------------------
	int GetTabCount()
	{
		return m_aElements.Count();
	}

	//------------------------------------------------------------------------------------------------
	void AddTab(ResourceName layout, string content, bool enabled = true, ResourceName tabImage = string.Empty, ResourceName iconLayout = string.Empty, float width = 32, float height = 32, string contentParam1 = string.Empty, string contentParam2 = string.Empty, string identifier = string.Empty)
	{
		SCR_TabViewContent tabContent = new SCR_TabViewContent;
		tabContent.m_ElementLayout = layout;
		tabContent.m_bEnabled = enabled;
		tabContent.m_sTabButtonContent = content;
		tabContent.m_sTabButtonContentParam1 = contentParam1;
		tabContent.m_sTabButtonContentParam2 = contentParam2;
		tabContent.m_sTabIdentifier = identifier;

		if (iconLayout != string.Empty)
		{
			tabContent.m_bShowIcon = true;
			tabContent.m_IconLayout = iconLayout;
			tabContent.m_fIconWidth = width;
			tabContent.m_fIconHeight = height;
		}

		if (!tabImage.IsEmpty())
		{
			tabContent.m_TabImage = tabImage;
			tabContent.m_bShowImage = true;
		}

		m_aElements.Insert(tabContent);

		// If tabView wasn't initialized do not create anything
		if (!m_wButtonBar)
			return;

		// Create button
		CreateTab(tabContent);

		if (m_bCreateAllTabsAtStart)
			CreateTabContent(tabContent);

		UpdatePagingButtons();
	}

	//------------------------------------------------------------------------------------------------
	void RemoveTab(int index)
	{
		SCR_TabViewContent content = m_aElements[index];
		if (!content)
			return;

		// Always remove ordered
		if (content.m_ButtonComponent)
		{
			Widget button = content.m_ButtonComponent.GetRootWidget();
			if (button)
				button.RemoveFromHierarchy();
		}

		Widget tab = content.m_wTab;
		if (tab)
		{
			m_OnContentHide.Invoke(this, tab);
			m_OnContentRemove.Invoke(this, tab);
			content.m_wTab.RemoveFromHierarchy();
		}

		SCR_TabViewContent currentContent = m_aElements[m_iSelectedTab];

		m_aElements.RemoveOrdered(index);

		int newIndex = m_aElements.Find(currentContent);

		if (index == m_iSelectedTab)
			ShowTab(0, true);
		else
			ShowTab(newIndex, true);

		UpdatePagingButtons();
	}

	//------------------------------------------------------------------------------------------------
	void RemoveTabByIdentifier(string identifier)
	{
		for (int i = 0; i < m_aElements.Count(); i++)
		{
			if (m_aElements[i].m_sTabIdentifier == identifier)
			{
				RemoveTab(i);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddContentElement(SCR_TabViewContent content)
	{
		m_aElements.Insert(content);
	}

	//------------------------------------------------------------------------------------------------
	void ShowTabText(int index, bool show, int buttonWidth = -1)
	{
		if (index < 0 || index >= m_aElements.Count())
			return;

		m_aElements[index].m_ButtonComponent.GetTextWidget().SetVisible(show);

		if (buttonWidth > 0 && m_aElements[index].m_wLayout)
		{
			if (m_aElements[index].m_wLayout)
			{
				m_aElements[index].m_wLayout.EnableWidthOverride(true);
				m_aElements[index].m_wLayout.SetWidthOverride(buttonWidth);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowAllTabsText(bool show, int buttonWidth = -1, bool ignoreSelected = true)
	{
		if (ignoreSelected)
		{
			for (int i = 0; i < m_aElements.Count(); i++)
			{
				if (i != m_iSelectedTab)
					ShowTabText(i, show, buttonWidth);
			}
		}
		else
		{
			for (int i = 0; i < m_aElements.Count(); i++)
			{
				ShowTabText(i, show, buttonWidth);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetTabText(int index, string text, string textParam1 = string.Empty, string textParam2 = string.Empty)
	{
		if (index < 0 || index >= m_aElements.Count())
			return;

		m_aElements[index].m_sTabButtonContent = text;
		m_aElements[index].m_sTabButtonContentParam1 = textParam1;
		m_aElements[index].m_sTabButtonContentParam2 = textParam2;

		m_aElements[index].m_ButtonComponent.GetTextWidget().SetTextFormat(text, textParam1, textParam2);
	}

	//------------------------------------------------------------------------------------------------
	void SetTabImage(int index, ResourceName tabImage)
	{
		if (index < 0 || index >= m_aElements.Count())
			return;

		m_aElements[index].m_TabImage = tabImage;
		m_aElements[index].m_bShowImage = true;
		
		Widget tabRoot = m_aElements[index].m_ButtonComponent.GetRootWidget();

		ImageWidget image = ImageWidget.Cast(tabRoot.FindAnyWidget("TabImage"));
		if (image)
		{
			image.LoadImageTexture(0, tabImage);
			image.SetVisible(true);
		}
		
		Widget imageSize = tabRoot.FindAnyWidget("SizeTabImage");
		if (imageSize)
			imageSize.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	void ShowImage(int index, bool show)
	{
		if (index < 0 || index >= m_aElements.Count())
			return;
		
		Widget tabRoot = m_aElements[index].m_ButtonComponent.GetRootWidget();

		ImageWidget image = ImageWidget.Cast(tabRoot.FindAnyWidget("TabImage"));
		if (image)
			image.SetVisible(show);
		
		Widget imageSize = tabRoot.FindAnyWidget("SizeTabImage");
		if (imageSize)
			imageSize.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelection(SCR_ButtonBaseComponent button)
	{
		for (int i = 0, len = m_aElements.Count(); i < len; i++)
		{
			// Deselect the old button, select the new button
			if (m_aElements[i].m_ButtonComponent != button)
				continue;

			// Prevent change when clicking same button 
			if (m_iSelectedTab == i)
			{
				m_aElements[i].m_ButtonComponent.SetToggled(true);
				return;
			}
			
			ShowTab(i);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabLeft()
	{
		if (!m_ListenToActions)
			return;

		int i = GetNextValidItem(true);
		if (i >= 0)
			ShowTab(i);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabRight()
	{
		if (!m_ListenToActions)
			return;

		int i = GetNextValidItem(false);
		if (i >= 0)
			ShowTab(i);
	}

	//------------------------------------------------------------------------------------------------
	//! Enable or disable paging buttons, if it's possible to page to the right or left
	protected void UpdatePagingButtons(bool animate = true)
	{
		if (m_PagingLeft)
			m_PagingLeft.SetEnabled(GetNextValidItem(true) > -1 && m_ListenToActions, animate);
		if (m_PagingRight)
			m_PagingRight.SetEnabled(GetNextValidItem(false) > -1 && m_ListenToActions, animate);
	}

	//------------------------------------------------------------------------------------------------
	int GetNextValidItem(bool toLeft)
	{
		if (m_aElements.IsEmpty())
			return -1;

		SCR_ButtonTextComponent selectedElement;
		int lastIndex = m_aElements.Count() - 1;

		int i = m_iSelectedTab;
		int foundItem = -1;
		int nextItem = 1;
		if (toLeft)
			nextItem = -1;

		while (foundItem < 0)
		{
			i += nextItem;
			if (i < 0)
			{
				if (m_bCycleMode)
					i = lastIndex;
				else
					return -1;

			}
			else if (i > lastIndex)
			{
				if (m_bCycleMode)
					i = 0;
				else
					return -1;
			}

			if (m_iSelectedTab >= 0 && m_iSelectedTab >= lastIndex && m_aElements[m_iSelectedTab] == m_aElements[i])
				return -1; // Went through all elements, found nothing

			if (m_aElements[i].m_bEnabled)
				foundItem = i;
		}
		return foundItem;
	}

	//------------------------------------------------------------------------------------------------
	void EnableTab(int tabIndex, bool enable, bool animate = true)
	{
		if (tabIndex < 0 || tabIndex >= m_aElements.Count())
			return;

		SCR_TabViewContent tab = m_aElements[tabIndex];
		if (!tab)
			return;

		if (!tab.m_ButtonComponent)
			return;

		tab.m_ButtonComponent.SetEnabled(enable, animate);
		tab.m_bEnabled = tab.m_ButtonComponent.IsVisible() && tab.m_ButtonComponent.IsEnabled();

		UpdatePagingButtons();
	}

	//------------------------------------------------------------------------------------------------
	void EnableAllTabs(bool enable, bool ignoreCurrentActive = true, bool animate = true)
	{
		int count = m_aElements.Count();

		for (int i = 0; i < count; i++)
		{
			if (ignoreCurrentActive && m_iSelectedTab == i)
				continue;

			EnableTab(i, enable, animate);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetTabVisible(int tabIndex, bool visible, bool animate = true)
	{
		if (tabIndex < 0 || tabIndex >= m_aElements.Count())
			return;

		SCR_TabViewContent tab = m_aElements[tabIndex];
		if (!tab)
			return;

		if (!tab.m_ButtonComponent)
			return;

		tab.m_ButtonComponent.SetVisible(visible, animate);
		tab.m_bEnabled = tab.m_ButtonComponent.IsVisible() && tab.m_ButtonComponent.IsEnabled();

		UpdatePagingButtons();

		//~ If active tab is set invisible try to set new active tab. If fails it will keep the current tab active
		if (!visible && tabIndex == GetShownTab())
		{
			for (int i = 0; i < m_aElements.Count(); i++)
			{
				if (!m_aElements[i].m_ButtonComponent)
					continue;

				if (m_aElements[i].m_ButtonComponent.IsVisible() && m_aElements[i].m_ButtonComponent.IsEnabled())
				{
					ShowTab(i, true, animate);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void Enable(bool enable)
	{
		Widget w = m_wRoot.FindAnyWidget("ContentOverlay");
		w.SetEnabled(enable);
	}

	//------------------------------------------------------------------------------------------------
	bool IsTabEnabled(int tabIndex)
	{
		if (tabIndex == -1 || tabIndex >= m_aElements.Count())
			return false;

		return m_aElements[tabIndex].m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	void SetCanNavigate(bool isAllowed)
	{
		m_ListenToActions = isAllowed;
	}

	//------------------------------------------------------------------------------------------------
	bool GetCanNavigate()
	{
		return m_ListenToActions;
	}

	//------------------------------------------------------------------------------------------------
	void SetListenToActions(bool isAllowed)
	{
		m_ListenToActions = isAllowed;
	}

	//------------------------------------------------------------------------------------------------
	bool GetListenToActions()
	{
		return m_ListenToActions;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddActionListeners()
	{
		if (m_PagingLeft)
			m_PagingLeft.m_OnActivated.Insert(OnTabLeft);

		if (m_PagingRight)
			m_PagingRight.m_OnActivated.Insert(OnTabRight);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		if (m_PagingLeft)
			m_PagingLeft.m_OnActivated.Remove(OnTabLeft);
		if (m_PagingRight)
			m_PagingRight.m_OnActivated.Remove(OnTabRight);
	}

	//------------------------------------------------------------------------------------------------
	SCR_TabViewContent GetShownTabComponent()
	{
		return m_aElements[m_iSelectedTab];
	}

	//------------------------------------------------------------------------------------------------
	void SetEntryIconSize(int index, float width = -1, float height = -1)
	{
		if (index < 0 || index >= m_aElements.Count())
			return;

		if (width > 0)
			m_aElements[index].m_fIconWidth = width;

		if (height > 0)
			m_aElements[index].m_fIconHeight = height;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sTabButtonContent")]
class SCR_TabViewContent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout element used", "layout")]
	ResourceName m_ElementLayout;

	[Attribute("Button", UIWidgets.EditBox, "Content of the button: It can contain either name or image resource")]
	string m_sTabButtonContent;

	[Attribute("", UIWidgets.EditBox, "Unique identifier of tab used in script to look for this one - optional for now")]
	string m_sTabIdentifier;

	[Attribute(desc: "Param if m_sTabButtonContent is name rather then image resource")]
	LocalizedString m_sTabButtonContentParam1;

	[Attribute(desc: "Param if m_sTabButtonContent is name rather then image resource")]
	LocalizedString m_sTabButtonContentParam2;

	[Attribute("true", UIWidgets.CheckBox, "Is tab enabled?")]
	bool m_bEnabled;

	SCR_ButtonTextComponent m_ButtonComponent;
	Widget m_wTab;
	SizeLayoutWidget m_wLayout;

	[Attribute(desc: "Image shown in front of tab text", params: "edds imageset")]
	ResourceName m_TabImage;
	
	[Attribute(desc: "Image name used in front of tab text when using image set")]
	string m_sTabImageName;

	[Attribute("0")]
	bool m_bShowImage;

	[Attribute()]
	bool m_bShowIcon;

	[Attribute("{6B997B683BD0FC21}UI/layouts/WidgetLibrary/TabView/WLib_TabViewInfoIcon.layout", UIWidgets.ResourceNamePicker, "", "Path to layout")]
	ResourceName m_IconLayout;

	[Attribute("32")]
	float m_fIconWidth;

	[Attribute("32")]
	float m_fIconHeight;

	Widget m_wIcon;
}
