class SCR_KeybindSetting : SCR_SettingsSubMenuBase
{
	protected ref SCR_KeyBindingMenuConfig m_KeybindConfig;

	// Resources
	protected static const string ACTIONROW_LAYOUT_PATH = "{75B1F7B766CA8C91}UI/layouts/Menus/SettingsMenu/BindingMenu/KeybindRow.layout";
	protected static const string SEPARATOR_LAYOUT_PATH = "{01D9FD7791700ADA}UI/layouts/Menus/SettingsMenu/BindingMenu/KeybindSeparator.layout";
	protected static const string KEY_BINDING_CONFIG = "{4EE7794C9A3F11EF}Configs/System/keyBindingMenu.conf";

	// Widgets
	protected VerticalLayoutWidget m_wActionsLayout;
	protected ScrollLayoutWidget m_wActionsScrollLayout;

	// Bindings
	protected ref InputBinding m_Binding;
	protected static const string PRIMARY_PRESET_PREFIX = "";

	// Strings (should be localised)
	protected static const string RESET_ALL_DIALOG_TITLE = "#AR-Settings_Keybind_WarningResetAll";
	protected static const string RESET_ALL_DIALOG_MESSAGE = "#AR-Settings_Keybind_MessageResetAll";
	
	protected SCR_KeybindRowComponent m_SelectedRowComponent;
	protected SCR_InputButtonComponent m_ResetSingleButtonComponent;
	protected SCR_InputButtonComponent m_UnbindSingleActionButtonComponent;
	protected SCR_InputButtonComponent m_AdvancedBindingButtonComponent;
	protected SCR_SpinBoxComponent m_CategoriesSpinBox;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		m_wActionsLayout = VerticalLayoutWidget.Cast(GetRootWidget().FindAnyWidget("ActionRowsContent"));
		if (!m_wActionsLayout)
			return;
		
		m_wActionsScrollLayout = ScrollLayoutWidget.Cast(GetRootWidget().FindAnyWidget("ScrollLayout0"));
		if (!m_wActionsScrollLayout)
			return;

		SCR_SettingsManagerKeybindModule settingsKeybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!settingsKeybindModule)
			return;
		
		m_Binding = settingsKeybindModule.GetInputBindings();

		SCR_InputButtonComponent reset = CreateNavigationButton("MenuResetAllKeybind", "#AR-Settings_Keybind_ResetEveryKeybind", true);
		reset.m_OnActivated.Insert(ResetKeybindsToDefault);
		
#ifdef PLATFORM_CONSOLE
		if (!GetGame().GetHasKeyboard())
		{
			reset.SetVisible(false);
			reset.SetEnabled(false);
		}
#endif
		
		CreateSingleKeybindResetButton();
		CreateUnbindSingleButton();
		CreateAdvancedBindingButton();
		
		//read the categories and actions from KEY_BINDING_CONFIG
		Resource holder = BaseContainerTools.LoadContainer(KEY_BINDING_CONFIG);
		BaseContainer container = holder.GetResource().ToBaseContainer();
		m_KeybindConfig = SCR_KeyBindingMenuConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		InsertCategoriesToComboBox();
		ListActionsFromCurrentCategory();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		super.OnMenuUpdate(parentMenu, tDelta);
		
		//TODO: change this to use events insetad of tick
		SetAdvancedKeybindButtonVisible(!GetGame().IsPlatformGameConsole() && GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.GAMEPAD);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetAdvancedKeybindButtonVisible(bool visible)
	{
		if (m_AdvancedBindingButtonComponent)
			m_AdvancedBindingButtonComponent.SetVisible(visible, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void ListActionsFromCurrentCategory()
	{
		Widget spinBox = GetRootWidget().FindAnyWidget("CategoriesBox");
		bool firstSeparator = true;
		if (!spinBox)
			return;

		m_CategoriesSpinBox = SCR_SpinBoxComponent.Cast(spinBox.GetHandler(0));
		if (!m_CategoriesSpinBox)
			return;

		// Allow category switching with gamepad from anywere in this menu. 
		// Otherwise the player will need to scroll all the way back to the spin box
		m_CategoriesSpinBox.SetKeepActionListeners(true);
		m_CategoriesSpinBox.AddActionListeners();
		
		SCR_KeyBindingCategory category = m_KeybindConfig.m_KeyBindingCategories.Get(m_CategoriesSpinBox.GetCurrentIndex());
		if (!category)
			return;

		Widget rowToDelete = m_wActionsLayout.GetChildren();
		while (rowToDelete)
		{
			m_wActionsLayout.RemoveChild(rowToDelete);
			rowToDelete = m_wActionsLayout.GetChildren();
		}

		string displayName;
		Widget separator;
		Widget keybindTitle;
		Widget gamepadTitle;
		TextWidget separatorText;
		Widget actionRowWidget;
		SCR_KeybindRowComponent component;
		foreach (SCR_KeyBindingEntry entry : category.m_KeyBindingEntries)
		{
			displayName = entry.m_sDisplayName;
			if (displayName.Length() == 0)
				displayName = "<#AR-Settings_Keybind_MissingName>" + entry.m_sActionName;

			if (entry.m_sActionName == "separator")
			{
				separator = GetGame().GetWorkspace().CreateWidgets(SEPARATOR_LAYOUT_PATH ,m_wActionsLayout);
				if (!separator)
					continue;
				separatorText = TextWidget.Cast(separator.FindAnyWidget("ActionCategoryName"));
				if (!separatorText)
					continue;
				separatorText.SetVisible(!entry.m_sDisplayName.IsEmpty());
				separatorText.SetText(entry.m_sDisplayName);
				if (firstSeparator)
				{
					keybindTitle = separator.FindAnyWidget("Keybind");
					gamepadTitle = separator.FindAnyWidget("Gamepad");
					if (keybindTitle)
						keybindTitle.SetVisible(true);
					if (gamepadTitle)
						gamepadTitle.SetVisible(true);
					firstSeparator = false;
#ifdef PLATFORM_CONSOLE
					if (!GetGame().GetHasKeyboard())
						keybindTitle.SetOpacity(0);
#endif
				}
			}
			else
			{
				actionRowWidget = GetGame().GetWorkspace().CreateWidgets(ACTIONROW_LAYOUT_PATH ,m_wActionsLayout);
				component = SCR_KeybindRowComponent.Cast(actionRowWidget.FindHandler(SCR_KeybindRowComponent));
				if (component)
					component.Create(actionRowWidget, displayName, entry.m_sActionName, this, entry.m_sPreset,  GetRootWidget(), m_Binding, entry.m_ePrefixType);
			}
		}
		
		if (m_wActionsScrollLayout)
			m_wActionsScrollLayout.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void InsertCategoriesToComboBox()
	{
		Widget spinBox = GetRootWidget().FindAnyWidget("CategoriesBox");
		if (!spinBox)
			return;

		SCR_SpinBoxComponent spinBoxComponent = SCR_SpinBoxComponent.Cast(spinBox.GetHandler(0));
		if (!spinBoxComponent)
			return;

		spinBoxComponent.ClearAll();
		spinBoxComponent.m_OnChanged.Insert(ListActionsFromCurrentCategory);

		foreach (SCR_KeyBindingCategory category : m_KeybindConfig.m_KeyBindingCategories)
			spinBoxComponent.AddItem(category.m_sDisplayName);

		spinBoxComponent.SetCurrentItem(0);

	}

	//------------------------------------------------------------------------------------------------
	protected void ResetKeybindsToDefault()
	{
		SCR_ConfigurableDialogUi menu = SCR_CommonDialogs.CreateDialog("reset_keybinds");
		menu.m_OnConfirm.Insert(ResetKeybindsToDefaultConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateSingleKeybindResetButton()
	{
#ifdef PLATFORM_CONSOLE
			if (!GetGame().GetHasKeyboard())
				return;
#endif
		
		m_ResetSingleButtonComponent = CreateNavigationButton("MenuResetKeybind", "#AR-Settings_Keybind_ResetAllKeybinds", true);
		m_ResetSingleButtonComponent.m_OnActivated.Insert(ResetSingleKeybindToDefault);
		m_ResetSingleButtonComponent.SetEnabled(false);
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetSingleKeybindToDefault()
	{
		if (!m_SelectedRowComponent)
			return;
		
		m_SelectedRowComponent.ResetAction();
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetKeybindsToDefaultConfirm()
	{
		array<string> contexts = {};
		m_Binding.GetContexts(contexts);
		
		foreach (SCR_KeyBindingCategory category: m_KeybindConfig.m_KeyBindingCategories)
		{
			foreach (SCR_KeyBindingEntry entry: category.m_KeyBindingEntries)
			{
				string finalPreset = entry.m_sPreset;
				if (!entry.m_sPreset.IsEmpty())
					finalPreset = PRIMARY_PRESET_PREFIX + entry.m_sPreset;
				
				m_Binding.ResetDefault(entry.m_sActionName, EInputDeviceType.KEYBOARD, finalPreset);
				m_Binding.ResetDefault(entry.m_sActionName, EInputDeviceType.MOUSE, finalPreset);
			}
		}
		ListActionsFromCurrentCategory();
		m_Binding.Save();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
		ListActionsFromCurrentCategory();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);
		
		if (m_CategoriesSpinBox)
		{
			m_CategoriesSpinBox.SetKeepActionListeners(false);
			m_CategoriesSpinBox.RemoveActionListeners();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		
		if (m_CategoriesSpinBox)
		{
			m_CategoriesSpinBox.SetKeepActionListeners(true);
			m_CategoriesSpinBox.AddActionListeners();
		}

		// Restore focus to last selected entry on dialog close
		if (!GetShown())
			return;
		
		if (m_SelectedRowComponent)
			GetGame().GetWorkspace().SetFocusedWidget(m_SelectedRowComponent.GetRootWidget());
		else if (m_CategoriesSpinBox)
			GetGame().GetWorkspace().SetFocusedWidget(m_CategoriesSpinBox.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		
		if (m_CategoriesSpinBox)
		{
			m_CategoriesSpinBox.SetKeepActionListeners(false);
			m_CategoriesSpinBox.RemoveActionListeners();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_KeybindRowComponent GetSelectedRowComponent()
	{
		return m_SelectedRowComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelectedRowComponent(SCR_KeybindRowComponent rowComponent)
	{
		m_SelectedRowComponent = rowComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	void SingleResetEnabled(bool isEnabled)
	{
		if (m_ResetSingleButtonComponent)
			m_ResetSingleButtonComponent.SetEnabled(isEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateUnbindSingleButton()
	{
#ifdef PLATFORM_CONSOLE
			if (!GetGame().GetHasKeyboard())
				return;
#endif
		m_UnbindSingleActionButtonComponent = CreateNavigationButton("MenuUnbindKeybind", "#AR-Settings_Keybind_UnbindOne", true);
		m_UnbindSingleActionButtonComponent.m_OnActivated.Insert(UnbindSingleAction);
		m_UnbindSingleActionButtonComponent.SetEnabled(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateAdvancedBindingButton()
	{		
		m_AdvancedBindingButtonComponent = CreateNavigationButton("MenuAdvancedKeybind", "#AR_Settings_KeybindAdvanced_Title", true);
		m_AdvancedBindingButtonComponent.m_OnActivated.Insert(AdvancedKeybindButtonClick);
		m_AdvancedBindingButtonComponent.SetEnabled(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnbindSingleActionEnabled(bool isEnabled)
	{
		if (m_UnbindSingleActionButtonComponent)
			m_UnbindSingleActionButtonComponent.SetEnabled(isEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	void AdvancedBindingEnabled(bool isEnabled)
	{
		if (m_AdvancedBindingButtonComponent)
			m_AdvancedBindingButtonComponent.SetEnabled(isEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnbindSingleAction()
	{		
		if (!m_SelectedRowComponent)
			return;
		
		m_SelectedRowComponent.Unbind();
	}
	
	//------------------------------------------------------------------------------------------------
	void AdvancedKeybindButtonClick()
	{
		//save the selected row component because it will loose focus as soon as dialog opens
		SCR_KeybindRowComponent rowComp = m_SelectedRowComponent;
		
		SCR_AdvancedKeybindDialogUI keybindDialog = SCR_AdvancedKeybindDialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.AdvancedKeybindDialog));
		if (!keybindDialog || !rowComp)
			return;
		
		keybindDialog.SetActionName(rowComp.GetActionName());
		keybindDialog.SetActionPreset(PRIMARY_PRESET_PREFIX + rowComp.GetActionPreset());
		keybindDialog.SetActionPrefixType(rowComp.GetActionPrefixType());
		keybindDialog.InitiateAdvancedKeybindDialog();
		keybindDialog.m_OnCancel.Insert(ListActionsFromCurrentCategory);
	}
};
