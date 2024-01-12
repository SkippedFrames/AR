//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class SCR_KeybindRowComponent : SCR_ScriptedWidgetComponent
{
	protected static Widget s_RootWidget;
	protected static SCR_KeybindSetting s_KeybindMenuComponent;
	protected static ref InputBinding s_Binding;
	protected static const string CHANGE_KEYBIND_DIALOG_MESSAGE = "#AR-Settings_Keybind_keybindPressPrompt";
	protected static const string NO_KEYBIND = "<#AR-Settings_Keybind_NoKeybind>";
	protected static const string DIALOGWINDOW_LAYOUT_PATH = "{1123A3569ACDCDEC}/UI/layouts/Menus/Dialogs/DialogBase.layout";
	protected static const string GAMEPAD_PRESET_PREFIX = "gamepad:";
	protected static const string PRIMARY_PRESET_PREFIX = ""; //we do not need to use primary prefixes anymore, so we will get rid of them eventually completely, this and its usage will be removed from code when there is time for that
	protected Widget m_ParentWidget;
	protected SCR_KeybindSetting m_KeybindSettings;
	protected SCR_SettingsManagerKeybindModule m_SettingsKeybindModule;
	
	protected string m_sActionName;
	protected string m_sActionDisplayName;
	protected string m_sActionPreset;
	protected SCR_EActionPrefixType m_eActionPrefixType;

	//------------------------------------------------------------------------------------------------
	void Create(Widget parentWidget, string actionDisplayName, string actionName, SCR_KeybindSetting menuComponent, string preset, Widget rootWidget, InputBinding binding, SCR_EActionPrefixType actionType)
	{
		//set up globals
		s_RootWidget = rootWidget;
		m_sActionPreset = preset;
		m_sActionName = actionName;
		s_KeybindMenuComponent = menuComponent;
		m_sActionDisplayName = actionDisplayName;
		m_ParentWidget = parentWidget;
		s_Binding = binding;
		m_eActionPrefixType = actionType;
		
		m_SettingsKeybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!m_SettingsKeybindModule)
			return;
		
		m_KeybindSettings = SCR_KeybindSetting.Cast(s_RootWidget.FindHandler(SCR_KeybindSetting));
		if (!m_KeybindSettings)
			return;
		
		RichTextWidget textBox = RichTextWidget.Cast(m_ParentWidget.FindAnyWidget("Name"));
		textBox.SetText(actionDisplayName);
		textBox.ElideText(1, 1.0, "...");
	
		SetRichTextAction("PCBind", EInputDeviceType.KEYBOARD, true, actionName, preset);
		SetRichTextAction("ControllerBind", EInputDeviceType.GAMEPAD, false, actionName, preset);
#ifdef PLATFORM_CONSOLE
		OverlayWidget PCKeybindOverlay = OverlayWidget.Cast(parentWidget.FindAnyWidget("PCKeybindOverlay"));
		
		InputManager inputManager = GetGame().GetInputManager();
		
		if (PCKeybindOverlay && inputManager && !inputManager.IsUsingMouseAndKeyboard())
		{
			PCKeybindOverlay.SetEnabled(false);
			PCKeybindOverlay.SetVisible(false);
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonClick()
	{
		string finalPreset;
		
		//in case of gamepad just do nothing, because we do not support keybind changes on gamepad
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD && !GetGame().GetHasKeyboard())
			return;
		
		if (!m_sActionPreset.IsEmpty())
			finalPreset = PRIMARY_PRESET_PREFIX + m_sActionPreset;
		
		s_Binding.StartCapture(m_sActionName, EInputDeviceType.KEYBOARD, finalPreset, false);

		KeybindMenu menu = KeybindMenu.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.KeybindChangeDialog , DialogPriority.CRITICAL, 0, true));
		menu.SetActionRowName(m_sActionDisplayName);
		menu.SetKeybind(s_Binding, s_KeybindMenuComponent);
	}


	//------------------------------------------------------------------------------------------------
	protected void SetRichTextAction(string widgetName, EInputDeviceType device, bool canChangeKeybind, string actionName, string preset)
	{
		string finalPreset = preset;
		string deviceString;
		ButtonWidget keybindButton = ButtonWidget.Cast(m_ParentWidget.FindAnyWidget(widgetName));
		RichTextWidget keybindAction = RichTextWidget.Cast( keybindButton.FindAnyWidget("Text"));
		
		if (device == EInputDeviceType.GAMEPAD)
		{
			deviceString = "gamepad";
			if (!finalPreset.IsEmpty())
				finalPreset = GAMEPAD_PRESET_PREFIX+preset;
		}
		
		if (device == EInputDeviceType.KEYBOARD)
		{
			deviceString = "keyboard";
			if (!finalPreset.IsEmpty())
				finalPreset = PRIMARY_PRESET_PREFIX+preset;
		}
		
		keybindAction.SetText(string.Format("<action name='%1' preset='%2' device='%3' scale='1.25' index='0'/>", actionName, finalPreset, deviceString));
		
		int bindCount = m_SettingsKeybindModule.GetActionBindCount(actionName, finalPreset, device);
		if (bindCount > 1)
		{
			RichTextWidget keybindAdditional = RichTextWidget.Cast(keybindButton.FindAnyWidget("AdditionalBinds"));
			Widget additionalBindLayout = keybindButton.FindAnyWidget("AdditionalBindsLayout");
			if(!additionalBindLayout || !keybindAdditional)
				return;
			
			additionalBindLayout.SetVisible(true);
			// -1 because we are showing how many binds there are other than the one being shown
			keybindAdditional.SetTextFormat(" #AR-ValueUnit_Short_Plus", bindCount - 1);
		}
		
		if (canChangeKeybind)
		{
			SCR_ButtonTextComponent bindComponent = SCR_ButtonTextComponent.Cast(keybindButton.FindHandler(SCR_ButtonTextComponent));
			bindComponent.m_OnClicked.Insert(OnButtonClick);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		m_KeybindSettings.SetSelectedRowComponent(this);
		m_KeybindSettings.SingleResetEnabled(true);
		m_KeybindSettings.UnbindSingleActionEnabled(true);
		m_KeybindSettings.AdvancedBindingEnabled(true);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		m_KeybindSettings.SetSelectedRowComponent(null);
		m_KeybindSettings.SingleResetEnabled(false);
		m_KeybindSettings.UnbindSingleActionEnabled(false);
		m_KeybindSettings.AdvancedBindingEnabled(false);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetAction()
	{
		string finalPreset = m_sActionPreset;
		if (!m_sActionPreset.IsEmpty())
			finalPreset = PRIMARY_PRESET_PREFIX + m_sActionPreset;
		
	//	InputBinding binding = GetGame().GetInputManager().CreateUserBinding();
		s_Binding.ResetDefault(m_sActionName, EInputDeviceType.KEYBOARD, finalPreset);
		s_Binding.ResetDefault(m_sActionName, EInputDeviceType.MOUSE, finalPreset);
		s_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void Unbind(EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		string finalPreset;
		string devicePrefix;
		
		if (device == EInputDeviceType.GAMEPAD)
			devicePrefix = GAMEPAD_PRESET_PREFIX;
		else
			devicePrefix = PRIMARY_PRESET_PREFIX;
		
		if (!m_sActionPreset.IsEmpty())
			finalPreset = devicePrefix + m_sActionPreset;
		
		m_SettingsKeybindModule.UnbindAction(m_sActionName, finalPreset, device);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetActionName()
	{
		return m_sActionName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetActionPreset()
	{
		return m_sActionPreset;
	}
	
	SCR_EActionPrefixType GetActionPrefixType()
	{
		return m_eActionPrefixType;
	}
};
