class SCR_InputButtonComponent : SCR_ButtonBaseComponent
{
	[Attribute("MenuTabLeft", desc: "Name of action from chimeraInputCommon")]
	protected string m_sActionName;

	[Attribute("Navigation Button")]
	protected string m_sLabel;

	[Attribute("0")]
	protected bool m_bIsMouseInput;

	[Attribute(defvalue: "32", desc: "Define the Height in PX. Wight will be adjusted automatically")]
	int m_iHeightInPixel;

	[Attribute("0.760 0.392 0.080 1", UIWidgets.ColorPicker)]
	ref Color m_ActionDefault;

	[Attribute("1.000000 0.631006 0.246006 1.000000", UIWidgets.ColorPicker)]
	ref Color m_ActionHovered;

	[Attribute("0.760 0.392 0.080 1", UIWidgets.ColorPicker)]
	ref Color m_ActionClicked;

	[Attribute("0.074 0.074 0.074 1", UIWidgets.ColorPicker)]
	ref Color m_ActionDisabled;

	[Attribute("0.597421 0.061189 0.475868 1.000000", UIWidgets.ColorPicker)]
	ref Color m_ActionToggled;

	[Attribute("InputButtonDisplayRoot")]
	protected string m_sButtonWidgetName;

	[Attribute("HorizontalLayout0")]
	protected string m_sHorizontalLayoutWidget;

	[Attribute("TextHint")]
	protected string m_sTextHintWidget;

	[Attribute("{B7C8B633428B153D}UI/layouts/WidgetLibrary/Buttons/WLib_InputButtonDisplay.layout")]
	protected ResourceName m_sInputButtonWidget;

	[Attribute("{983DFCCEB578C1C5}UI/layouts/InputButtons/InputButton_ComboIndicator.layout")]
	protected ResourceName m_sComboIndicatorWidget;

	[Attribute("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset")]
	protected ResourceName m_sComboIndicatorImageSet;

	[Attribute("{00FE3DBDFD15227B}UI/Textures/Icons/icons_wrapperUI-glow.imageset")]
	protected ResourceName m_sComboIndicatorImageSetGlow;

	[Attribute(desc: "If enabled Text size is controlled by set size Inside the RichText widget")]
	protected bool m_bOverrideTextSize;

	[Attribute()]
	protected bool m_bDebugSimulateController;

	protected ref SCR_InputButtonDisplay m_ButtonDisplay;
	protected Widget m_wHorizontalLayout;
	protected RichTextWidget m_wTextHint;

	protected ref array<Widget> m_aComboWidgets = {};
	protected ref array<BaseContainer> filterStack = {};
	protected ref array<int> m_aFilterStackIndexRemover = {};

	InputManager m_InputManager = GetGame().GetInputManager();

	protected bool m_bIsComboInput;
	protected bool m_bIsAlternativeInput;
	protected bool m_bIsHoldAction;
	protected bool m_bIsDoubleTapAction;
	protected bool m_bPressedInput;

	protected bool m_bForceDisabled;
	protected bool m_bShouldBeEnabled;
	protected bool m_bIsInteractionActive;
	
	protected string m_sOldActionName;

	bool m_bIsHoldingButton;
	bool m_bIsHovered;
	bool m_bIsDoubleTapStated;

	float m_fMaxHoldtime;

	int m_iDoubleTapThreshold;

	protected EInputDeviceType m_eCurrentInputDevice;

	protected ref ScriptInvokerVoid m_OnAnimateHover;
	protected ref ScriptInvokerVoid m_OnUpdateEnableColor;
	protected ref ScriptInvokerVoid m_OnHoldAnimComplete;

	ref ScriptInvoker m_OnActivated = new ScriptInvoker();
	protected ref array<DialogUI> m_aDialogs;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wHorizontalLayout = HorizontalLayoutWidget.Cast(w.FindAnyWidget(m_sHorizontalLayoutWidget));
		m_wTextHint = RichTextWidget.Cast(w.FindAnyWidget(m_sTextHintWidget));

		Widget buttonWidget = Widget.Cast(w.FindAnyWidget(m_sButtonWidgetName));
		if (!buttonWidget)
			return;

		m_ButtonDisplay = SCR_InputButtonDisplay.Cast(buttonWidget.FindHandler(SCR_InputButtonDisplay));
		if (!m_ButtonDisplay)
			return;

		if (m_wTextHint)
		{
			if (!m_sLabel)
				m_wTextHint.SetVisible(false);

			m_wTextHint.SetText(m_sLabel);
		}

		if (GetGame().InPlayMode())
		{
			m_aDialogs = new array<DialogUI>;

			SCR_MenuHelper.GetOnActiveWidgetInteraction().Insert(OnActiveWidgetInteraction);
			SCR_MenuHelper.GetOnDialogOpen().Insert(OnDialogOpen);
			SCR_MenuHelper.GetOnDialogClose().Insert(OnDialogClose);
			SCR_MenuHelper.GetOnMenuFocusGained().Insert(OnMenuFocusGained);
			SCR_MenuHelper.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
		}

		m_bShouldBeEnabled = IsEnabled();

		m_ButtonDisplay.Init(m_wRoot);

		ChangeInputDevice(m_InputManager.GetLastUsedInputDevice(), false);

		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceUserChanged);

		if (m_bDebugSimulateController)
			OnInputDeviceUserChanged(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		//! Remove old listener
		if (!m_InputManager)
			return;

		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceUserChanged);

		if (GetGame().InPlayMode())
		{
			SCR_MenuHelper.GetOnActiveWidgetInteraction().Remove(OnActiveWidgetInteraction);
			SCR_MenuHelper.GetOnDialogOpen().Remove(OnDialogOpen);
			SCR_MenuHelper.GetOnDialogClose().Remove(OnDialogClose);
			SCR_MenuHelper.GetOnMenuFocusGained().Remove(OnMenuFocusGained);
			SCR_MenuHelper.GetOnMenuFocusLost().Remove(OnMenuFocusLost);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void SetEnabled(bool enabled, bool animate = true)
	{
		m_bShouldBeEnabled = enabled;
		SetEnabled_Internal(enabled, animate);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEnabled_Internal(bool enabled, bool animate = true)
	{
		if (!m_bForceDisabled)
			super.SetEnabled(enabled, animate);
		else
			super.SetEnabled(false, animate);
	}

	//------------------------------------------------------------------------------------------------
	//! React on switching between input methods
	protected void OnInputDeviceUserChanged(bool isGamepad)
	{
		if (isGamepad)
			ChangeInputDevice(EInputDeviceType.GAMEPAD, false);
		else
			ChangeInputDevice(EInputDeviceType.KEYBOARD, false);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check for last used input device and get the correct keybind for it.
	!*/
	protected void ChangeInputDevice(EInputDeviceType inputDevice, bool hasFocus)
	{
		if (inputDevice == m_eCurrentInputDevice && m_sActionName == m_sOldActionName)
			return;
		
		if (inputDevice == EInputDeviceType.MOUSE && hasFocus && m_bIsMouseInput)
		{
			m_eCurrentInputDevice = inputDevice;
		}
		else
		{
			if (inputDevice == EInputDeviceType.GAMEPAD)
				m_eCurrentInputDevice = inputDevice;
			else
				m_eCurrentInputDevice = EInputDeviceType.KEYBOARD;
		}

		DeleteComboWidget();

		if (inputDevice == EInputDeviceType.INVALID)
			return;

		array<string> keyStack = {};
		//! Get keybind for selected input action and input device
		if (m_InputManager.GetActionKeybinding(m_sActionName, keyStack, filterStack, m_eCurrentInputDevice))
		{
			m_aFilterStackIndexRemover.Clear();
			int index = keyStack.Count() - 1;
			string keyCode = ProcessKeybindStack(index, keyStack);

			//Check if InputAction is a Combo or alternative.
			TStringArray m_aKeyCodeString = new TStringArray;
			if (keyCode.Contains(" + "))
			{
				keyCode.Split(" + ", m_aKeyCodeString, true);
				m_bIsComboInput = true;

				keyCode = m_aKeyCodeString[0];
			}
			else if (keyCode.Contains(" | "))
			{
				keyCode.Split(" | ", m_aKeyCodeString, true);

				if (m_aKeyCodeString[0].Contains("keyboard:") && m_aKeyCodeString[1].Contains("mouse:"))
					m_bIsAlternativeInput = false;
				else
					m_bIsAlternativeInput = true;

				keyCode = m_aKeyCodeString[0];
			}

			if (!m_InputManager || !keyCode || !m_InputManager.GetKeyUIMapping(keyCode))
				return;
			
			BaseContainer data = m_InputManager.GetKeyUIMapping(keyCode).GetObject("Data");
			if (!data)
			{
				Print(string.Format("No data found for %1 !! Check 'chimeraMapping.conf' and add data if necessary! Provided Actioname: %2.", keyCode, m_sActionName), LogLevel.ERROR);
				return;
			}

			//Check is there is a valid filter applied
			GetInputFilterSettings(filterStack[0]);

			//Process filter stack for visualisation
			ProcessFilterStack();

			if (m_ButtonDisplay)
				m_ButtonDisplay.SetAction(data, filterStack[0]);

			if (!m_bOverrideTextSize && m_wTextHint && m_ButtonDisplay)
				m_wTextHint.SetDesiredFontSize((int)m_iHeightInPixel / m_ButtonDisplay.GetTextSizeModifier());

			if (m_bIsComboInput || m_bIsAlternativeInput)
				CreateComboWidget(m_aKeyCodeString);

			SetInputAction();
		}
		else
		{
			if (!m_sActionName)
				Print("No action name string found! Check 'Input Action' textbox in SCR_InputbuttonComponent!", LogLevel.ERROR);
			else
				Print(string.Format("No entry in CimeraInputCommon.conf for input action: %1 ! Check 'Input Action' textbox in SCR_InputbuttonComponent!", m_sActionName), LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Check which filter is applied to the action to change the buttons behavior
	protected bool GetInputFilterSettings(BaseContainer filter)
	{
		if (!filter)
			return false;

		switch (filter.GetClassName())
		{
			case "InputFilterHold":
			{
				filter.Get("HoldDuration", m_fMaxHoldtime);
				if (m_fMaxHoldtime == -1 || m_fMaxHoldtime == 0)
					m_fMaxHoldtime = 250;

				m_fMaxHoldtime /= 1000;
				m_bIsHoldAction = true;
				break;
			}
			case "InputFilterHoldOnce":
			{
				filter.Get("HoldDuration", m_fMaxHoldtime);
				if (m_fMaxHoldtime == -1 || m_fMaxHoldtime == 0)
					m_fMaxHoldtime = 250;

				m_fMaxHoldtime /= 1000;

				m_bIsHoldAction = true;
				break;
			}
			case "InputFilterDoubleClick":
			{
				m_bIsDoubleTapAction = true;
				break;
			}
			case "InputFilterPressed":
			{
				m_bPressedInput = true;
				break;
			}
			case "InputFilterToggle":
			{
				m_bCanBeToggled = true;
				filter.Get("Threshold", m_iDoubleTapThreshold);
				if (m_iDoubleTapThreshold == -1)
					m_iDoubleTapThreshold = 500;

				m_iDoubleTapThreshold /= 1000;
				break;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Create a secound InputButton widget to display combos
	\param BaseContainer containing all key information
	!*/
	protected void CreateComboWidget(TStringArray keyCodes)
	{
		if (m_ButtonDisplay.GetIsOverwritten())
			return;
		
		for (int i = 1, count = keyCodes.Count(); i < count; i++)
		{
			if (!keyCodes[i])
				return;

			BaseContainer keyData = m_InputManager.GetKeyUIMapping(keyCodes[i]).GetObject("Data");
			if (!keyData)
			{
				Print(string.Format("No data found for %1 !! Check 'chimeraMapping.conf' and add data if necessary!", keyCodes[i]), LogLevel.ERROR);
				return;
			}

			//! Create Combo indicator
			Widget comboIndicator = GetGame().GetWorkspace().CreateWidgets(m_sComboIndicatorWidget, m_wHorizontalLayout);

			if (!comboIndicator)
			{
				Print(string.Format("Unable to create Widget! %1 or %2 are null!", m_sComboIndicatorWidget, m_wHorizontalLayout), LogLevel.ERROR);
				return;
			}

			ImageWidget comboIndicatorImage = ImageWidget.Cast(comboIndicator.FindAnyWidget("ComboIndicatorImg"));
			ImageWidget comboIndicatorShadow = ImageWidget.Cast(comboIndicator.FindAnyWidget("ComboIndicatorShadow"));

			if (comboIndicatorImage && comboIndicatorShadow)
			{
				if (m_bIsAlternativeInput)
				{
					comboIndicatorImage.LoadImageFromSet(0, m_sComboIndicatorImageSet, "keybind_divider");
					comboIndicatorShadow.LoadImageFromSet(0, m_sComboIndicatorImageSetGlow, "keybind_divider");
				}

				comboIndicatorImage.SetSize(m_iHeightInPixel * 0.5, m_iHeightInPixel * 0.5);
				comboIndicatorShadow.SetSize(m_iHeightInPixel * 0.5, m_iHeightInPixel * 0.5);
			}

			//! Create InputButton widget
			Widget comboWidget = GetGame().GetWorkspace().CreateWidgets(m_sInputButtonWidget, m_wHorizontalLayout);

			if (!comboWidget)
			{
				Print(string.Format("Unable to create Widget! %1 or %2 are null!", m_sInputButtonWidget, m_wHorizontalLayout), LogLevel.ERROR);
				return;
			}

			SCR_InputButtonDisplay component = SCR_InputButtonDisplay.Cast(comboWidget.FindHandler(SCR_InputButtonDisplay));

			if (!component)
			{
				Print(string.Format("%1 has no 'SCR_InputButtonDisplay' component attached!", component), LogLevel.ERROR);
				return;
			}

			component.Init(m_wRoot);
			component.SetAction(keyData, filterStack[i]);

			m_aComboWidgets.Insert(comboIndicator);
			m_aComboWidgets.Insert(comboWidget);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Delete secound InputButton widget if there is one
	!*/
	protected void DeleteComboWidget()
	{
		if (m_aComboWidgets.IsEmpty())
			return;

		foreach (Widget widget : m_aComboWidgets)
		{
			if (!widget)
				continue;

			widget.RemoveFromHierarchy();
		}

		m_aComboWidgets.Clear();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Add action listeners for selected input action
	!*/
	protected void SetInputAction()
	{
		if (m_sActionName == m_sOldActionName)
			return;

		if (!m_sOldActionName.IsEmpty())
		{
			m_InputManager.RemoveActionListener(m_sOldActionName, EActionTrigger.DOWN, OnInput);
			m_InputManager.RemoveActionListener(m_sOldActionName, EActionTrigger.UP, OnInput);
		}

		if (!m_sActionName.IsEmpty())
		{
			m_InputManager.AddActionListener(m_sActionName, EActionTrigger.DOWN, OnInput);
			m_InputManager.AddActionListener(m_sActionName, EActionTrigger.UP, OnInput);
		}

		m_sOldActionName = m_sActionName;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_wRoot.IsVisible() || !m_wRoot.IsEnabled())
			return false;

		if (m_bIsDoubleTapAction)
			return false;

		m_bIsHoldingButton = true;

		OnInput(1, EActionTrigger.DOWN);

		m_OnClicked.Invoke(this);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_wRoot.IsVisible() || !m_wRoot.IsEnabled())
			return false;

		if (!m_bIsHoldAction)
			return false;

		m_bIsHoldingButton = false;

		OnInput(1, EActionTrigger.UP);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (m_bIsDoubleTapAction)
			super.OnDoubleClick(w, x, y, button);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		m_bIsHovered = true;
		if (m_OnAnimateHover)
			m_OnAnimateHover.Invoke(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);

		m_bIsHovered = false;

		if (m_OnAnimateHover)
			m_OnAnimateHover.Invoke(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDisabled(bool animate)
	{
		if (m_OnUpdateEnableColor)
			m_OnUpdateEnableColor.Invoke(this);
		
		if (!m_ButtonDisplay)
			return;
		
		if (animate && m_fAnimationRate != START_ANIMATION_RATE)
			AnimateWidget.Opacity(m_ButtonDisplay.GetBackgroundWidget(), m_fDisabledOpacity, m_fAnimationRate);
		else
			m_ButtonDisplay.GetBackgroundWidget().SetOpacity(m_fDisabledOpacity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEnabled(bool animate)
	{
		super.OnEnabled(animate);

		if (m_OnUpdateEnableColor)
			m_OnUpdateEnableColor.Invoke(this);
		
		if (!m_ButtonDisplay)
			return;
		
		if (animate && m_fAnimationRate != START_ANIMATION_RATE)
			AnimateWidget.Opacity(m_ButtonDisplay.GetBackgroundWidget(), 1, m_fAnimationRate);
		else
			m_ButtonDisplay.GetBackgroundWidget().SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	void PlaySoundClicked()
	{
		PlaySound(m_sSoundClicked);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called when Button is clicked / key is presses
	\param value of pressed key (0 - 1)
	\param EActionTrigger state
	*/
	protected void OnInput(float value, EActionTrigger reason)
	{
		if (!m_wRoot)
			return;

		if (!m_wRoot.IsVisibleInHierarchy() || !m_wRoot.IsEnabledInHierarchy())
			return;

		// Bail if attached to menu but menu is not focused
		if (!IsParentMenuFocused())
			return;

		//--- If a modal widget is active, check if the button is in its hierarchy
		Widget modal = GetGame().GetWorkspace().GetModal();
		if (modal && !SCR_WidgetTools.InHierarchy(m_wRoot, modal))
			return;

		if (reason == EActionTrigger.DOWN)
		{
			if (m_ButtonDisplay)
				m_ButtonDisplay.ActionPressed();

			m_OnActivated.Invoke(this, m_sActionName);

			foreach (Widget inputButton : m_aComboWidgets)
			{
				SCR_InputButtonDisplay component = SCR_InputButtonDisplay.Cast(inputButton.FindHandler(SCR_InputButtonDisplay));
				if (!component)
					continue;

				component.ActionPressed();
			}
		}
		else
		{
			if (!m_bIsHoldAction)
				return;
			
			if (m_ButtonDisplay)
				m_ButtonDisplay.ActionReleased();

			foreach (Widget inputButton : m_aComboWidgets)
			{
				SCR_InputButtonDisplay component = SCR_InputButtonDisplay.Cast(inputButton.FindHandler(SCR_InputButtonDisplay));
				if (!component)
					continue;

				component.ActionReleased();
			}
		}

	}

	//------------------------------------------------------------------------------------------------
	float GetAnimationRate()
	{
		return m_fAnimationRate;
	}

	//------------------------------------------------------------------------------------------------
	float GetAnimationTime()
	{
		return m_fAnimationTime;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsInputCombo()
	{
		return m_bIsComboInput;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnAnimateHover()
	{
		if (!m_OnAnimateHover)
			m_OnAnimateHover = new ScriptInvokerVoid();

		return m_OnAnimateHover;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnUpdateEnableColor()
	{
		if (!m_OnUpdateEnableColor)
			m_OnUpdateEnableColor = new ScriptInvokerVoid();

		return m_OnUpdateEnableColor;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnHoldAnimComplete()
	{
		if (!m_OnHoldAnimComplete)
			m_OnHoldAnimComplete = new ScriptInvokerVoid();

		return m_OnHoldAnimComplete;
	}

	//------------------------------------------------------------------------------------------------
	//! Iterate trough key bind stack
	//! \param[inout] index gets decremented  by one
	string ProcessKeybindStack(inout int index, notnull array<string> keyStack)
	{
		if (!keyStack.IsIndexValid(index))
			return string.Empty;

		// pop back
		string key = keyStack[index];
		index--;

		switch (key)
		{
			case "+": // combo
			{
				m_aFilterStackIndexRemover.Insert(index + 1);
				return ProcessKeybindStack(index, keyStack) + " + " + ProcessKeybindStack(index, keyStack);
			}
			case "|": // sum/alternative
			{
				m_aFilterStackIndexRemover.Insert(index + 1);
				return ProcessKeybindStack(index, keyStack) + " | " + ProcessKeybindStack(index, keyStack);
			}

			default: // key value
				return key;
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Delete not needed entries from FilterStack and bring it into correct order
	!*/
	void ProcessFilterStack()
	{
		if (filterStack.IsEmpty())
			return;

		foreach (int index : m_aFilterStackIndexRemover)
		{
			filterStack.RemoveOrdered(index);
		}

		array<BaseContainer> filteredFilterStack = {};
		int count = filterStack.Count() - 1;
		for (int i = count; i >= 0; i--)
		{
			BaseContainer filterStackIndex = filterStack[i];
			filteredFilterStack.Insert(filterStackIndex);

			if (!filterStackIndex)
				continue;

			if (filterStackIndex.GetClassName() == "InputFilterHold" || filterStackIndex.GetClassName() == "InputFilterHoldOnce")
			{
				filterStackIndex.Get("HoldDuration", m_fMaxHoldtime);
				if (m_fMaxHoldtime == -1 || m_fMaxHoldtime == 0)
					m_fMaxHoldtime = 250;

				m_fMaxHoldtime /= 1000;
				m_bIsHoldAction = true;
			}
		}
		filterStack.Clear();
		filterStack = filteredFilterStack;
	}

	///PUBLIC API\\\
	//------------------------------------------------------------------------------------------------
	void SetLabel(string label)
	{
		if (!m_wTextHint)
			return;

		m_wTextHint.SetText(label);
		m_wTextHint.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	void SetAction(string action)
	{
		m_sActionName = action;
		ChangeInputDevice(m_InputManager.GetLastUsedInputDevice(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override texture in Button 
	//! \param ImagePath
	//! \param Image name if using ImageSet. Default: null
	//! \param Color of the Image
	void SetTexture(string imagePath, string image = string.Empty, Color color = Color.White)
	{
		if (!m_ButtonDisplay)
			return;
		
		DeleteComboWidget();
		m_ButtonDisplay.OverrideTexture(imagePath, image, color);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Restet Button back to it's default texutre
	//! Use after SetTexture() to undo the override
	void ResetTexture()
	{
		if (!m_ButtonDisplay)
			return;
		
		m_ButtonDisplay.SetIsOverwritten(false);
		
		ChangeInputDevice(m_InputManager.GetLastUsedInputDevice(), false);
	}
	

	//------------------------------------------------------------------------------------------------
	void SetColorActionDisabled(Color newColor)
	{
		m_ActionDisabled = newColor;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_InputButtonComponent FindComponent(notnull Widget w)
	{
		return SCR_InputButtonComponent.Cast(w.FindHandler(SCR_InputButtonComponent));
	}

	//------------------------------------------------------------------------------------------------
	bool IsHovering()
	{
		return m_bIsHovered;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetForceDisabled(bool forceDisabled, bool animate = true)
	{
		GetGame().GetCallqueue().Remove(SetForceDisabled); //Stop SetForceDisabledWithDelay

		m_bForceDisabled = forceDisabled;

		if (forceDisabled)
			SetEnabled_Internal(false);
		else
			SetEnabled_Internal(m_bShouldBeEnabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetForceDisabledWithDelay(bool forceDisabled, bool animate = true, int delay = 0)
	{
		m_bForceDisabled = forceDisabled;

		if (delay == 0)
		{
			SetForceDisabled(forceDisabled, animate);
			return;
		}

		GetGame().GetCallqueue().Remove(SetForceDisabled);
		GetGame().GetCallqueue().CallLater(SetForceDisabled, delay, false, forceDisabled, animate);
	}

	//Handling Input through SCR_MenuHelper, ei: edit and combo boxes call this to disable buttons during interaction
	//------------------------------------------------------------------------------------------------
	void OnActiveWidgetInteraction(bool isInteractionActive, int delay)
	{
		m_bIsInteractionActive = isInteractionActive;

		if (IsInTopMenu())
			SetForceDisabledWithDelay(isInteractionActive, true, delay);
	}

	//------------------------------------------------------------------------------------------------
	void OnDialogOpen(DialogUI dialog)
	{
		m_aDialogs.Insert(dialog);

		SetForceDisabled(!IsInTopMenu() || m_bIsInteractionActive);
	}

	//------------------------------------------------------------------------------------------------
	void OnDialogClose(DialogUI dialog)
	{
		m_aDialogs.RemoveItem(dialog);

		SetForceDisabled(!IsInTopMenu() || m_bIsInteractionActive);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusGained(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(m_wRoot))
			SetForceDisabled(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusLost(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(m_wRoot))
			SetForceDisabled(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if there are no dailogs or if this button is in the last dialog opened
	bool IsInTopMenu()
	{
		return m_aDialogs.IsEmpty() || m_aDialogs[m_aDialogs.Count() - 1] == GetGame().GetMenuManager().GetOwnerMenu(GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if there if this button is in the last dialog opened
	bool IsInTopDialog()
	{
		return !m_aDialogs.IsEmpty() && m_aDialogs[m_aDialogs.Count() - 1] == GetGame().GetMenuManager().GetOwnerMenu(GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	bool GetForceDisabled()
	{
		return m_bForceDisabled;
	}

	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_InputButtonComponent GetInputButtonComponent(string name, notnull Widget parent, bool searchAllChildren = true)
	{
		SCR_InputButtonComponent comp = SCR_InputButtonComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_InputButtonComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
}
