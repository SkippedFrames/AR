//------------------------------------------------------------------------------------------------
class SCR_AdvancedKeybindDialogUI: DialogUI
{
	//------------------------------------------------------------------------------------------------
	protected string m_sActionPreset;
	protected string m_sActionName;
	protected SCR_EActionPrefixType m_eActionPrefixType;
	protected int m_iSelectedKeybind;
	
	SCR_SettingsManagerKeybindModule m_SettingsKeybindModule;
	
	protected Widget m_wRootWidget;
	protected Widget m_wPrefixesCombo;
	protected ButtonWidget m_DeleteBindWidget;
	
	protected ref array<ref Widget> m_aBindWidgets = {};
	
	protected const ResourceName KEYBIND_ROW_LAYOUT_PATH = "{25888355688EE402}UI/layouts/Menus/SettingsMenu/BindingMenu/AdvancedKeybindActionRow.layout";
	
	//------------------------------------------------------------------------------------------------
	void InitiateAdvancedKeybindDialog()
	{
		m_wRootWidget = GetRootWidget();
		if (!m_wRootWidget)
			return;
		
		m_SettingsKeybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!m_SettingsKeybindModule)
		{
			Print("SCR_AdvancedKeybindDialogUI::Keybind module for settings manager not found!", LogLevel.WARNING);
			return;
		}
		
		SetupDeleteBindButton();
		SetupCaptureButton();
		SetupUnbindButton();
		SetupPrefixesCombo();
		SetupBindsCombo();
		SetupCombinationCombo();
		
		ShowBindsForAction(m_sActionName, m_sActionPreset);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		ShowBindsForAction(m_sActionName, m_sActionPreset);
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("InteractableDialogContext");
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActionName(string actionName)
	{
		m_sActionName = actionName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetActionName()
	{
		return m_sActionName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActionPreset(string actionPreset)
	{
		m_sActionPreset = actionPreset;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetActionPreset()
	{
		return m_sActionPreset;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelectedKeybindIndex(int index)
	{
		m_iSelectedKeybind = index;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSelectedKeybindIndex()
	{
		return m_iSelectedKeybind;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActionPrefixType(SCR_EActionPrefixType prefixType)
	{
		m_eActionPrefixType = prefixType;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EActionPrefixType GetActionPrefixType()
	{
		return m_eActionPrefixType;
	}
	
	//-----------------------------------------------------------------------------------------------
	void ShowBindsForAction(string actionName, string actionPreset)
	{
		if (!m_wRootWidget)
			return;
		
		Widget contentWidget = m_wRootWidget.FindAnyWidget("ScrollContent");
		if (!contentWidget)
			return;
		
		//clear the content so we prevent duplicates
		Widget children = contentWidget.GetChildren();
		while (children)
		{
			contentWidget.RemoveChild(children);
			children = contentWidget.GetChildren();
		}
		
		int bindCount = m_SettingsKeybindModule.GetActionBindCount(m_sActionName, m_sActionPreset);
	
		m_aBindWidgets.Clear();	
		Widget rowWidget;
		SCR_AdvancedActionRowComponent rowComponent;
		SCR_ModularButtonComponent rowModularButtonComponent;
		
		for (int i = 0; i < bindCount; i++)
		{
			m_aBindWidgets.Insert(GetGame().GetWorkspace().CreateWidgets(KEYBIND_ROW_LAYOUT_PATH, contentWidget));
			
			rowWidget = m_aBindWidgets.Get(i);
			rowComponent = SCR_AdvancedActionRowComponent.Cast(rowWidget.FindHandler(SCR_AdvancedActionRowComponent));
			if (!rowComponent)
				continue;
			
			rowComponent.Init(m_sActionName, m_sActionPreset, i, m_SettingsKeybindModule);
			
			rowModularButtonComponent = SCR_ModularButtonComponent.Cast(rowWidget.FindHandler(SCR_ModularButtonComponent));
			if (!rowModularButtonComponent)
				continue;
			
			rowModularButtonComponent.m_OnClicked.Insert(SetSelectedKeybind);
			rowModularButtonComponent.m_OnFocus.Insert(OnBindFocusGained);
			rowModularButtonComponent.m_OnFocusLost.Insert(OnBindFocusLost);
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	void ShowBindsForCurrentAction()
	{
		ShowBindsForAction(m_sActionName, m_sActionPreset);
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetSelectedKeybind(notnull SCR_ModularButtonComponent modularButtonComp)
	{
		Widget root = modularButtonComp.GetRootWidget();
		if (!root)
			return;
		
		SCR_AdvancedActionRowComponent advancedKeybindComponent = SCR_AdvancedActionRowComponent.Cast(root.FindHandler(SCR_AdvancedActionRowComponent));
		if (!advancedKeybindComponent)
			return;

		m_iSelectedKeybind = advancedKeybindComponent.GetKeybindIndex();
	}
	
	//-----------------------------------------------------------------------------------------------
	void OnBindFocusGained()
	{
		m_wPrefixesCombo.SetEnabled(true);
		SCR_InputButtonComponent.Cast(m_DeleteBindWidget.FindHandler(SCR_InputButtonComponent)).SetEnabled(true);
	}
	
	//-----------------------------------------------------------------------------------------------
	void OnBindFocusLost()
	{
		m_wPrefixesCombo.SetEnabled(false);
		SCR_InputButtonComponent.Cast(m_DeleteBindWidget.FindHandler(SCR_InputButtonComponent)).SetEnabled(false);
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetupDeleteBindButton()
	{
		m_DeleteBindWidget = ButtonWidget.Cast(m_wRootWidget.FindAnyWidget("DeleteBind"));
		if (!m_DeleteBindWidget)
			return;
		
		SCR_InputButtonComponent buttonComp = SCR_InputButtonComponent.Cast(m_DeleteBindWidget.FindHandler(SCR_InputButtonComponent));
		if (!buttonComp)
			return;
		
		buttonComp.m_OnActivated.Insert(DeleteSelectedBind);
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetupCaptureButton()
	{
		ButtonWidget buttonWidget = ButtonWidget.Cast(m_wRootWidget.FindAnyWidget("AddKeybind"));
		if (!buttonWidget)
			return;
		
		SCR_ButtonImageComponent imageButtonComp = SCR_ButtonImageComponent.Cast(buttonWidget.FindHandler(SCR_ButtonImageComponent));
		if (!imageButtonComp)
			return;
		
		imageButtonComp.m_OnClicked.Insert(StartCapturingKeybind);
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetupUnbindButton()
	{
		ButtonWidget buttonWidget = ButtonWidget.Cast(m_wRootWidget.FindAnyWidget("UnbindAll"));
		if (!buttonWidget)
			return;
		
		SCR_InputButtonComponent imageButtonComp = SCR_InputButtonComponent.Cast(buttonWidget.FindHandler(SCR_InputButtonComponent));
		if (!imageButtonComp)
			return;
		
		imageButtonComp.m_OnActivated.Insert(UnbindAction);
	}
	
	//-----------------------------------------------------------------------------------------------
	void DeleteSelectedBind()
	{
		m_SettingsKeybindModule.DeleteActionBindByIndex(m_sActionName, m_iSelectedKeybind, m_sActionPreset);
		ShowBindsForAction(m_sActionName, m_sActionPreset);
	}
	
	//-----------------------------------------------------------------------------------------------
	void StartCapturingKeybind()
	{
		m_SettingsKeybindModule.StartCaptureForAction(m_sActionName, m_sActionPreset, EInputDeviceType.KEYBOARD, true);

		KeybindMenu menu = KeybindMenu.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.KeybindChangeDialog , DialogPriority.CRITICAL, 0, true));
		if (!menu)
			return;
		
		menu.SetActionRowName(m_sActionName);
		menu.SetKeybind(m_SettingsKeybindModule.GetInputBindings());
		menu.GetOnKeyCaptured().Insert(ShowBindsForCurrentAction);
	}
	
	//-----------------------------------------------------------------------------------------------
	void UnbindAction()
	{
		m_SettingsKeybindModule.UnbindAction(m_sActionName, m_sActionPreset);
		ShowBindsForAction(m_sActionName, m_sActionPreset);
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetupPrefixesCombo()
	{
		m_wPrefixesCombo = m_wRootWidget.FindAnyWidget("ActionPrefixCombo");
		if (!m_wPrefixesCombo)
		{
			Print("SCR_AdvancedKeybindDialogUI: Action prefix combo widget was not found", LogLevel.WARNING);
			return;
		}
		
		SCR_ComboBoxComponent comboBoxComponent = SCR_ComboBoxComponent.Cast(m_wPrefixesCombo.FindHandler(SCR_ComboBoxComponent));
		if (!comboBoxComponent)
			return;
		
		array<ref SCR_KeyBindingFilter> filters = m_SettingsKeybindModule.GetFilters(m_eActionPrefixType);		
		
		foreach (SCR_KeyBindingFilter filter : filters)
		{
			comboBoxComponent.AddItem(filter.m_sFilterDisplayName);
		}
		
		comboBoxComponent.m_OnChanged.Insert(OnPrefixesComboChanged);
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetupBindsCombo()
	{
		Widget systemBindCombo = m_wRootWidget.FindAnyWidget("SystemKeyCombo");
		if (!systemBindCombo)
		{
			Print("SCR_AdvancedKeybindDialogUI: System key combo widget was not found", LogLevel.WARNING);
		}
		
		SCR_ComboBoxComponent comboBoxComponent = SCR_ComboBoxComponent.Cast(systemBindCombo.FindHandler(SCR_ComboBoxComponent));
		if (!comboBoxComponent)
			return;
		
		array<ref SCR_KeyBindingBind> binds = m_SettingsKeybindModule.GetCustomBinds();		
		foreach (SCR_KeyBindingBind bind : binds)
		{
			comboBoxComponent.AddItem(bind.m_sBindDisplayName);
		}
		
		comboBoxComponent.m_OnChanged.Insert(OnBindsComboChanged);
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetupCombinationCombo()
	{
		Widget combinationBindCombo = m_wRootWidget.FindAnyWidget("ComboKeyCombo");
		if (!combinationBindCombo)
		{
			Print("SCR_AdvancedKeybindDialogUI: Combination key combo widget was not found", LogLevel.WARNING);
			return;
		}
		
		SCR_ComboBoxComponent comboBoxComponent = SCR_ComboBoxComponent.Cast(combinationBindCombo.FindHandler(SCR_ComboBoxComponent));
		if (!comboBoxComponent)
			return;
		
		array<ref SCR_KeyBindingCombo> combos = m_SettingsKeybindModule.GetCustomComboKeys();
		foreach (SCR_KeyBindingCombo combo : combos)
		{
			comboBoxComponent.AddItem(combo.m_sComboDisplayName);
		}
		
		comboBoxComponent.m_OnChanged.Insert(OnCombinationComboChanged);
	}
	
	//-----------------------------------------------------------------------------------------------
	void OnPrefixesComboChanged(SCR_ComboBoxComponent component, int index)
	{
		m_SettingsKeybindModule.SetFilterForActionByIndex(m_sActionName, m_sActionPreset, index, m_iSelectedKeybind, m_eActionPrefixType);
		ShowBindsForAction(m_sActionName, m_sActionPreset);
	}
	
	//-----------------------------------------------------------------------------------------------
	void OnBindsComboChanged(SCR_ComboBoxComponent component, int index)
	{
		m_SettingsKeybindModule.AddKeybindToActionByIndex(m_sActionName, m_sActionPreset, index);
		ShowBindsForAction(m_sActionName, m_sActionPreset);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCombinationComboChanged(SCR_ComboBoxComponent component, int index)
	{
		m_SettingsKeybindModule.AddComboToActionByIndex(m_sActionName, m_sActionPreset, index, m_iSelectedKeybind);
		ShowBindsForAction(m_sActionName, m_sActionPreset);
	}
}
