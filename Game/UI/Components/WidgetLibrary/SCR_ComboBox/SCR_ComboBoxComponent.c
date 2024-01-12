class SCR_ComboBoxComponent : SCR_SelectionWidgetComponent
{
	[Attribute("-1", UIWidgets.EditBox, "Maximum size of the list without using scrollbar. -1 for no limit")]
	float m_fMaxListHeight;

	[Attribute("true", UIWidgets.CheckBox, "Create the list below or above or below the root widget")]
	bool m_bCreateListBelow;

	[Attribute("{B8C4345E3A833B05}UI/layouts/WidgetLibrary/ComboBox/WLib_OpenedComboRoot.layout", UIWidgets.ResourceNamePicker, "Combo box element", "layout")]
	ResourceName m_sListRootLayout;

	[Attribute("{323CF37F81DF9B8A}UI/layouts/Common/Settings/ComboBox/ARComboBoxElement.layout", UIWidgets.ResourceNamePicker, "Combo box element", "layout")]
	ResourceName m_sElementLayout;

	[Attribute("0", UIWidgets.EditBox, "Gap between combo root and combo box items")]
	float m_fItemsGap;

	[Attribute("0", UIWidgets.EditBox, "Extra pixels to the left from the center of the widget")]
	float m_fExtraPaddingLeft;

	[Attribute("0", UIWidgets.EditBox, "Extra pixels to the right from the center of the widget")]
	float m_fExtraPaddingRight;
	
	[Attribute("0", UIWidgets.EditBox, "Starting angle of arrow icon to coordinate texture rotation")]
	protected float m_fArrowDefaultAngle;
	
	[Attribute("0", UIWidgets.CheckBox, "Allow manual drop down list width enforcing")]
	protected bool m_bForceListWidth;
	
	[Attribute("200", UIWidgets.EditBox, "How wide should drop down list be")]
	protected float m_fListForcedWidth;
	
	[Attribute("0", UIWidgets.EditBox, "How wide should drop down list be")]
	protected float m_fListXOffset;
	
	[Attribute(SCR_SoundEvent.FOCUS, UIWidgets.EditBox, "")]
	protected string m_sSoundClosed;
	
	[Attribute("OverlayArrow")]
	protected string m_sButton;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.EDIT_WIDGET_BACKGROUND), UIWidgets.ColorPicker)]
	ref Color m_BackgroundDefault;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.IDLE_DISABLED), UIWidgets.ColorPicker)]
	ref Color m_BackgroundInteracting;

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY), UIWidgets.ColorPicker)]
	ref Color m_ArrowDefault;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.CONTRAST_COLOR), UIWidgets.ColorPicker)]
	ref Color m_ArrowInteracting;
	
	protected InputManager m_InputManager;
	protected ref array<Widget> m_aElementWidgets = new ref array<Widget>();
	protected Widget m_wButton;
	protected ImageWidget m_wArrowImage;
	protected TextWidget m_wText;
	protected VerticalLayoutWidget m_wContent;
	protected Widget m_wElementsRoot;
	protected Widget m_wContentRoot;
	protected WorkspaceWidget m_Workspace;
	protected ref SCR_ComboModalHandler m_ModalHandler;
	protected bool m_bOpened;

	// Script invokers sends
	ref ScriptInvoker m_OnOpened = new ScriptInvoker();
	ref ScriptInvoker m_OnClosed = new ScriptInvoker();
	
	protected float posX, posY;
	
	protected Widget m_wTextBackground;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_InputManager = GetGame().GetInputManager();
		m_Workspace = GetGame().GetWorkspace();
		m_wText = TextWidget.Cast(w.FindAnyWidget("Content"));
		m_wContentRoot = w.FindAnyWidget("ComboButton");
		if (m_wContentRoot)
		{
			SCR_EventHandlerComponent comp = SCR_EventHandlerComponent.Cast(m_wContentRoot.FindHandler(SCR_EventHandlerComponent));
			if (comp)
			{
				comp.GetOnClick().Insert(OpenList);
				comp.GetOnFocus().Insert(OnHandlerFocus);
				comp.GetOnFocusLost().Insert(OnHandlerFocusLost);
			}
		}
		
		// Find arrow button
		m_wButton = w.FindAnyWidget(m_sButton);

		m_wArrowImage = ImageWidget.Cast(w.FindAnyWidget("ImageArrow"));
		if (m_wArrowImage)
		{
			m_wArrowImage.SetRotation(m_fArrowDefaultAngle);
			m_wArrowImage.SetColor(m_ArrowDefault);
		}

		UpdateName();
		
		Widget overlayText = w.FindAnyWidget("OverlayText");
		if(overlayText)
			m_wTextBackground = overlayText.FindAnyWidget("Background");
		
		if(m_wTextBackground)
			m_wTextBackground.SetColor(m_BackgroundDefault);
		
		if (GetGame().InPlayMode())
			SCR_MenuHelper.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		CloseList();
		
		SCR_MenuHelper.GetOnMenuFocusLost().Remove(OnMenuFocusLost);
	}

	//------------------------------------------------------------------------------------------------
	bool OnHandlerClicked()
	{
		OpenList();
		if (GetGame().GetWorkspace().GetFocusedWidget() != m_wRoot)
			GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		// Prevent regular focus, handled by OnHandlerFocus
		if (m_wContentRoot)
			GetGame().GetWorkspace().SetFocusedWidget(m_wContentRoot);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		// Prevent regular focus, handled by OnHandlerFocusLost
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void OnHandlerFocus()
	{
		// Call focus event on parent class
		super.OnFocus(m_wRoot, 0, 0);

		// Make the widget unfocusable
		m_wRoot.SetFlags(WidgetFlags.NOFOCUS);
	}

	//------------------------------------------------------------------------------------------------
	void OnHandlerFocusLost()
	{
		// Call focusLost event on parent class
		super.OnFocusLost(m_wRoot, 0, 0);

		// Make focusable again
		m_wRoot.ClearFlags(WidgetFlags.NOFOCUS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusLost(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(m_wRoot) && m_bOpened)
			CloseList();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnEnabled(bool animate)
	{
		super.OnEnabled(animate);
		
		if (m_wButton)
			m_wButton.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDisabled(bool animate)
	{
		super.OnDisabled(animate);
		
		if (m_wButton)
			m_wButton.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateName()
	{
		if (!m_wText)
			return;

		if (m_aElementNames && m_iSelectedItem > -1 && m_iSelectedItem < m_aElementNames.Count())
			m_wText.SetText(m_aElementNames[m_iSelectedItem]);
		else
			m_wText.SetText(string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	override bool SetCurrentItem(int i, bool playSound = false, bool animate = false)
	{
		bool result = super.SetCurrentItem(i, playSound, animate);
		UpdateName();
		if (m_bOpened)
			CreateEntries();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	void CreateEntries()
	{
		if (!m_wContent)
			return;

		foreach (Widget w : m_aElementWidgets)
		{
			w.RemoveFromHierarchy();
		}
		m_aElementWidgets.Clear();

		foreach (int i, string name : m_aElementNames)
		{		
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sElementLayout, m_wContent);
			if (!w)
				continue;
			
			if (m_wElementsRoot && !m_wElementsRoot.IsVisible())
				m_wElementsRoot.SetVisible(true);

			m_aElementWidgets.Insert(w);
			OnCreateElement(w, i);

			SCR_ButtonTextComponent comp = SCR_ButtonTextComponent.Cast(w.FindHandler(SCR_ButtonTextComponent));
			if (comp)
			{
				if (i == m_iSelectedItem)
				{
					GetGame().GetWorkspace().SetFocusedWidget(comp.GetRootWidget());
					comp.SetToggled(true, false, false);
				}

				comp.SetText(m_aElementNames[i]);
				comp.m_OnClicked.Insert(OnElementSelected);
			}
		}

		// Set focus on the first one or current index
		// TODO: fix "Given widget is already modal, not adding again" message
		if (m_iSelectedItem > -1 && m_iSelectedItem < m_aElementWidgets.Count())
			GetGame().GetWorkspace().AddModal(m_wElementsRoot, m_aElementWidgets[m_iSelectedItem]);
		else if (m_aElementWidgets.Count() > 0 && m_aElementWidgets[0])
			GetGame().GetWorkspace().AddModal(m_wElementsRoot, m_aElementWidgets[0]);
	}

	//------------------------------------------------------------------------------------------------
	void OpenList()
	{
		if (m_bOpened || !m_aElementNames || /*m_aElementNames.IsEmpty() ||*/ !m_wContentRoot)
			return;

		m_bOpened = true;

	 	SCR_MenuHelper.SetActiveWidgetInteractionState(true);
		
		if(m_wTextBackground)
			AnimateWidget.Color(m_wTextBackground, m_BackgroundInteracting, m_fAnimationRate);
		
		// Add escape handling
		m_InputManager.ResetAction("MenuSelect");
		m_InputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
#ifdef WORKBENCH
		m_InputManager.AddActionListener("MenuBackWB", EActionTrigger.DOWN, OnMenuBack);
#endif
		
		float x, y, w, h;
		m_wContentRoot.GetScreenPos(x, y);
		m_wContentRoot.GetScreenSize(w, h);

		// Unscale all layout-based positions
		x = m_Workspace.DPIUnscale(x);
		y = m_Workspace.DPIUnscale(y);
		w = m_Workspace.DPIUnscale(w);
		h = m_Workspace.DPIUnscale(h);

		// Modify width with the extra paddings on sides
		x += m_fExtraPaddingLeft;
		w -= m_fExtraPaddingLeft + m_fExtraPaddingRight;

		m_wElementsRoot = GetGame().GetWorkspace().CreateWidgets(m_sListRootLayout, GetGame().GetWorkspace());
		if (m_aElementData.IsEmpty())
			m_wElementsRoot.SetVisible(false);
		
		if (!m_wElementsRoot)
			return;

		m_wContent = VerticalLayoutWidget.Cast(m_wElementsRoot.FindAnyWidget("Content"));
		if (!m_wContent)
			return;

		if (m_bCreateListBelow)
		{
			y += h + m_fItemsGap;
		}
		else
		{
			y -= m_fItemsGap;
			m_wContent.SetFillOrigin(VerticalFillOrigin.BOTTOM);
			Widget separator = m_wContent.GetChildren();
			if (separator)
				separator.SetZOrder(-1);
		}

		FrameSlot.SetPos(m_wElementsRoot, x + m_fListXOffset, y);
		SizeLayoutWidget size = SizeLayoutWidget.Cast(m_wElementsRoot.FindAnyWidget("SizeLayout"));
		if (size)
		{
			size.EnableWidthOverride(true);
			
			if (m_bForceListWidth)
				size.SetWidthOverride(m_fListForcedWidth);
			else
				size.SetWidthOverride(w);
			
			if (m_fMaxListHeight > 0)
			{
				size.SetMaxDesiredHeight(m_fMaxListHeight);
				size.EnableMaxDesiredHeight(true);
			}
		}

		CreateEntries();
  
		m_ModalHandler = new SCR_ComboModalHandler();
		m_wElementsRoot.AddHandler(m_ModalHandler);
		m_ModalHandler.m_OnModalClickOut.Insert(CloseList);
		
		// Modal owners 
		ChimeraMenuBase menu = ChimeraMenuBase.GetOwnerMenu(m_wRoot);
		m_ModalHandler.SetupOwners(this, menu);

		// Set arrow image angle
		if (m_wArrowImage)
		{
			//m_wArrowImage.SetRotation(m_fArrowDefaultAngle + 180);
			AnimateWidget.Color(m_wArrowImage, m_ArrowInteracting, m_fAnimationRate);
		}

		m_OnOpened.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCreateElement(Widget elementWidget, int index)
	{
	}

	//------------------------------------------------------------------------------------------------
	void CloseList()
	{
		if (!m_bOpened || !m_wElementsRoot || !m_aElementNames)
			return;
		
		SCR_MenuHelper.SetActiveWidgetInteractionState(false);
		if(m_wTextBackground)
			AnimateWidget.Color(m_wTextBackground, m_BackgroundDefault, m_fAnimationRate);
		
		// Remove escape handling
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnMenuBack);
#ifdef WORKBENCH
		GetGame().GetInputManager().RemoveActionListener("MenuBackWB", EActionTrigger.DOWN, OnMenuBack);
#endif

		m_bOpened = false;
		m_wElementsRoot.RemoveFromHierarchy();

		foreach (Widget w : m_aElementWidgets)
		{
			if (!w)
				continue;

			SCR_ButtonTextComponent comp = SCR_ButtonTextComponent.Cast(w.GetHandler(0));
			if (!comp)
				continue;

			comp.m_OnToggled.Remove(OnElementSelected);
		}
		m_aElementWidgets.Clear();

		// Reset nagivation rules
		m_wRoot.SetNavigation(WidgetNavigationDirection.UP, WidgetNavigationRuleType.ESCAPE);
		m_wRoot.SetNavigation(WidgetNavigationDirection.DOWN, WidgetNavigationRuleType.ESCAPE);

		GetGame().GetWorkspace().SetFocusedWidget(m_wContentRoot);

		// Set arrow image angle
		if (m_wArrowImage)
		{
			//m_wArrowImage.SetRotation(m_fArrowDefaultAngle);
			AnimateWidget.Color(m_wArrowImage, m_ArrowDefault, m_fAnimationRate);
		}

		PlaySound(m_sSoundClosed);
		m_OnClosed.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuBack()
	{
		if (m_bOpened)
			CloseList();
	}

	//------------------------------------------------------------------------------------------------
	override int AddItem(string item, Managed data = null)
	{
		int i = super.AddItem(item, data);
		UpdateName();
		if (m_bOpened)
			CreateEntries();

		return i;
	}

	//------------------------------------------------------------------------------------------------
	override void ClearAll()
	{
		super.ClearAll();
		UpdateName();
		if (m_bOpened)
			CreateEntries();
	}

	//------------------------------------------------------------------------------------------------
	override void RemoveItem(int item)
	{
		super.RemoveItem(item);
		UpdateName();
		if (m_bOpened)
			CreateEntries();
	}

	//------------------------------------------------------------------------------------------------
	private void OnElementSelected(SCR_ButtonTextComponent comp)
	{
		int i = m_aElementWidgets.Find(comp.m_wRoot);
		if (i > -1)
		{
			SetCurrentItem(i, true, true);
			if (m_wText)
				m_wText.SetText(m_aElementNames[i]);
		}

		CloseList();
		m_OnChanged.Invoke(this, i);
	}

	/*!
	Get elementWidgets array
	\param[out] elementWidgets array of Widgets taken from m_aElementWidgets
	\return int count of elements in m_aElementWidgets
	*/
	int GetElementWidgets(notnull array<Widget> elementWidgets)
	{
		foreach (Widget w: m_aElementWidgets)
			elementWidgets.Insert(w);

		return elementWidgets.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpened()
	{
		return m_bOpened;
	}

	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_ComboBoxComponent GetComboBoxComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_ComboBoxComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_ComboBoxComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_ComboModalHandler : ScriptedWidgetEventHandler
{
	protected SCR_ComboBoxComponent m_Owner;
	protected ChimeraMenuBase m_OwnerMenu;
	
	ref ScriptInvoker m_OnModalClickOut = new ScriptInvoker();
	
    override bool OnModalClickOut(Widget modalRoot, int x, int y, int button)
    {
		m_OnModalClickOut.Invoke();
        return true;
    }

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_OwnerMenu)
			m_OwnerMenu.m_OnUpdate.Remove(OnMenuUpdate);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupOwners(SCR_ComboBoxComponent owner, ChimeraMenuBase menu)
	{
		m_Owner = owner;
		m_OwnerMenu = menu;
		
		if (m_OwnerMenu)
			m_OwnerMenu.m_OnUpdate.Insert(OnMenuUpdate);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float tDelta)
	{
		// Scroll input 
		if (GetGame().GetInputManager().GetActionValue("MenuScrollVertical") != 0)
		{
			m_Owner.CloseList();
			return;
		}
		
		// Visible in hierarchy 
		if (!m_Owner.GetRootWidget().IsVisibleInHierarchy())
		{
			m_Owner.CloseList();
			return;
		}
	}
};
