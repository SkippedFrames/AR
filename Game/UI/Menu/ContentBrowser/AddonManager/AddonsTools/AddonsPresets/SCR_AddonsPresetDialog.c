// We could inherit from MenuBase, but DialogUI implements some basic functionality, like Back button.
class SCR_WorkshopAddonPresetDialog : SCR_TabDialog
{
	protected ref SCR_AddonsPresetDialogWidgets m_Widgets = new SCR_AddonsPresetDialogWidgets();
	
	protected SCR_AddonsPresetsSubMenuComponent m_PresetSubMenu;
	
	//---------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		//m_Widgets.Init(GetRootWidget());
	
		// Handlers  
		/*Widget wPresetSubMenu = GetRootWidget().FindAnyWidget("AddonsPresetRoot");
		if (wPresetSubMenu)
			m_PresetSubMenu = SCR_AddonsPresetsSubMenuComponent.Cast(wPresetSubMenu.FindHandler(SCR_AddonsPresetsSubMenuComponent));
		
		m_PresetSubMenu.GetEventOnRename().Insert(OnRename);
		
		// Init 
		if (m_PresetSubMenu)
			m_PresetSubMenu.UpdatePresetListbox();
		
		// Button actions 
		m_Widgets.m_ButonNewPresetComponent.m_OnClicked.Insert(OnSaveButton);
		m_Widgets.m_ButonNewPresetComponent.m_OnFocus.Insert(OnNewPresetFocus);
		m_Widgets.m_ButonNewPresetComponent.m_OnFocusLost.Insert(OnNewPresetFocusLost);
		
		m_Widgets.m_NavLoadComponent.m_OnActivated.Insert(OnLoadButton);
		m_Widgets.m_NavOverrideComponent.m_OnActivated.Insert(OnOverrideButton);
		m_Widgets.m_NavDeleteComponent.m_OnActivated.Insert(OnDeleteButton);
		m_Widgets.m_NavRenameComponent.m_OnActivated.Insert(OnRenameButton);*/
	}
	
	//----------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();

		GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_ButonNewPreset);
		
		// Hide export tools if no enabled mods 
		//if (SCR_AddonManager.getaddo)
	}
	
	//----------------------------------------------------------------------------------------------
	// BUTTONS
	
	
	//---------------------------------------------------------------------------------------------------
	protected void OnSaveButton()
	{
		if (m_PresetSubMenu)
			m_PresetSubMenu.CreateNewPreset();
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void OnOverrideButton()
	{
		if (!m_PresetSubMenu)
			return;
		
		SCR_AddonLinePresetComponent focusedLine = m_PresetSubMenu.FocusedPresetLine();
		
		if (focusedLine)
			m_PresetSubMenu.OverridePreset(focusedLine);
		else
			OnSaveButton();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnRenameButton()
	{
		if (!m_PresetSubMenu)
			return;
		
		SCR_AddonLinePresetComponent focusedLine = m_PresetSubMenu.FocusedPresetLine();
		
		if (focusedLine)
			focusedLine.StartEditName();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnDeleteButton()
	{
		if (!m_PresetSubMenu)
			return;
		
		SCR_AddonLinePresetComponent focusedLine = m_PresetSubMenu.FocusedPresetLine();
		
		if (focusedLine)
			m_PresetSubMenu.DeletePreset(focusedLine);
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void OnLoadButton()
	{
		if (!m_PresetSubMenu)
			return;
		
		SCR_AddonLinePresetComponent focusedLine = m_PresetSubMenu.FocusedPresetLine();
		
		if (!focusedLine)
			return;
		
		m_PresetSubMenu.LoadPreset(focusedLine);
		//SCR_WorkshopAddonManagerPresetStorage.get
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnNewPresetFocus()
	{
		m_Widgets.m_NavLoadComponent.SetLabel("#AR-Workshop_CreateNewAddonPreset");
		m_Widgets.m_NavOverrideComponent.SetEnabled(false);
		m_Widgets.m_NavDeleteComponent.SetEnabled(false);
		m_Widgets.m_NavRenameComponent.SetEnabled(false);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnNewPresetFocusLost()
	{
		m_Widgets.m_NavLoadComponent.SetLabel("#AR-PauseMenu_Load");	
		m_Widgets.m_NavOverrideComponent.SetEnabled(true);
		m_Widgets.m_NavDeleteComponent.SetEnabled(true);
		m_Widgets.m_NavRenameComponent.SetEnabled(true);
	}
	
	//----------------------------------------------------------------------------------------------
	protected void OnRename(bool done)
	{
		if (done)
		{
			// Show focused widget controsl 
			if (GetGame().GetWorkspace().GetFocusedWidget() == m_Widgets.m_ButonNewPreset)
				OnNewPresetFocus();
			else
				OnNewPresetFocusLost();
		}
		else
		{
			m_Widgets.m_NavLoadComponent.SetLabel("#AR-MainMenu_Rename");	
			m_Widgets.m_NavOverrideComponent.SetEnabled(false);
			m_Widgets.m_NavDeleteComponent.SetEnabled(false);
			m_Widgets.m_NavRenameComponent.SetEnabled(false);
		}
	}
}
