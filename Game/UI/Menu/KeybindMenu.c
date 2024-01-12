class KeybindMenu : DialogUI
{
	protected static ref InputBinding s_Binding;
	protected SCR_KeybindSetting m_KeybindMenuComponent;
	
	protected RichTextWidget m_wActionRowName;
	
	protected ref ScriptInvoker m_OnKeyCaptured;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_wActionRowName = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("ActionRowName"));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("InteractableDialogContext");
		
		if (!s_Binding)
			return;

		if (s_Binding.GetCaptureState() == EInputBindingCaptureState.IDLE)
		{
			s_Binding.Save();
			if (m_KeybindMenuComponent)
				m_KeybindMenuComponent.ListActionsFromCurrentCategory();
			
			if (m_OnKeyCaptured)
				m_OnKeyCaptured.Invoke();
			CloseAnimated();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnCancel()
	{
		m_OnCancel.Invoke();
		s_Binding.CancelCapture();
		CloseAnimated();
	}

	//------------------------------------------------------------------------------------------------
	void SetKeybind(InputBinding binding, SCR_KeybindSetting keybindMenuComponent = null)
	{
		m_KeybindMenuComponent = keybindMenuComponent;
		s_Binding = binding;
		GetGame().GetInputManager().AddActionListener("MenuBackKeybind", EActionTrigger.DOWN, CancelCapture);
	}

	//------------------------------------------------------------------------------------------------
	void SetActionRowName(string name)
	{
		if (m_wActionRowName)
			m_wActionRowName.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	void CancelCapture()
	{
		s_Binding.CancelCapture();
		m_OnCancel.Invoke();
		CloseAnimated();
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnKeyCaptured()
	{
		if (!m_OnKeyCaptured)
			m_OnKeyCaptured = new ScriptInvoker();
		
		return m_OnKeyCaptured;
	}
};
