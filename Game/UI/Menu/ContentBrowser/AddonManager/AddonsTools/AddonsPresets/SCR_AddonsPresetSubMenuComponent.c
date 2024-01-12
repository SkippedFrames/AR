/*!
Component for addons presets submenu behavior.
Player can display, save, override and delete mod presets (setups).
This component can be use later on in addons manager tool dialog.
*/

class SCR_AddonsPresetsSubMenuComponent : SCR_SubMenuBase
{
	protected ref SCR_AddonsPresetSubMenuWidgets m_Widgets = new SCR_AddonsPresetSubMenuWidgets();
	
	protected ref array<ref SCR_WorkshopAddonPreset> m_aPresets = {};
	protected ref array<SCR_AddonLinePresetComponent> m_aPresetLines = {};
	
	protected ref SCR_WorkshopAddonPreset m_InteractivePreset;
	
	// New temptorary preset 
	protected SCR_ListBoxElementComponent m_CreatedElement;
	protected SCR_WorkshopAddonPreset m_CreatedPreset;
	
	// Navigation buttons 
	protected SCR_InputButtonComponent m_NavLoad;
	protected SCR_InputButtonComponent m_NavOverride;
	protected SCR_InputButtonComponent m_NavDelete;
	protected SCR_InputButtonComponent m_NavRename;
	
	// Invokers 
	protected ref ScriptInvoker<bool> Event_OnRename;

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnRename(bool renameDone)
	{
		if (Event_OnRename)
			Event_OnRename.Invoke(renameDone);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnRename()
	{
		if (!Event_OnRename)
			Event_OnRename = new ScriptInvoker();

		return Event_OnRename;
	}
	
	//---------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Widgets.Init(w);
		
		// Setup create new button 
		m_Widgets.m_ButonNewPresetComponent.m_OnClicked.Insert(CreateNewPreset);
		m_Widgets.m_ButonNewPresetComponent.m_OnFocus.Insert(OnNewPresetFocus);
		m_Widgets.m_ButonNewPresetComponent.m_OnFocusLost.Insert(OnNewPresetFocusLost);
	}
	
	//---------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		GetGame().SaveUserSettings();
	}
	
	//---------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		// Create new buttons 
		if (!m_NavLoad)
		{
			m_NavLoad = CreateNavigationButton("MenuSelect", "#AR-PauseMenu_Load", true);
			m_NavLoad.m_OnActivated.Insert(OnLoadButton);
		}

		if (!m_NavRename)
		{
			m_NavRename = CreateNavigationButton("MenuRename", "#AR-MainMenu_Rename", true);
			m_NavRename.m_OnActivated.Insert(OnRenameButton);
		}

		if (!m_NavDelete)
		{
			m_NavDelete = CreateNavigationButton("MenuDelete", "#AR-Workshop_ButtonDelete", true);
			m_NavDelete.m_OnActivated.Insert(OnDeleteButton);
		}

		if (!m_NavOverride)
		{
			m_NavOverride = CreateNavigationButton("MenuSave", "#AR-PauseMenu_Save", true);
			m_NavOverride.m_OnActivated.Insert(OnOverrideButton);
		}

		super.OnMenuShow(parentMenu);
		
		UpdatePresetListbox();
		
		
		if (!GetGame().InPlayMode())
			return;
		
		SCR_AddonManager.GetInstance().GetPresetStorage().GetEventOnUsedPresetChanged().Insert(OnUsedPresetChanged);
		
		// This will update the footer buttons and make sure there's something always focused
		GetGame().GetWorkspace().SetFocusedWidget(null);
		GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_ButonNewPreset);
	}
	
	//---------------------------------------------------------------------------------------------------
	// Display list 
	//---------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------
	//! Display list of current addon presets
	void UpdatePresetListbox()
	{
		if (!m_Widgets || !m_Widgets.m_ScrollPresetsComponent)
			return;
		
		SCR_ListBoxComponent lb = m_Widgets.m_ScrollPresetsComponent;
		if (!lb)
			return;
		
		// Remove all items
		while (lb.GetItemCount() > 0)
			lb.RemoveItem(0);
		
		m_aPresetLines.Clear();
		
		// Read data
		if (!GetGame().InPlayMode())
			return;
		
		SCR_WorkshopAddonManagerPresetStorage presetStorage = SCR_AddonManager.GetInstance().GetPresetStorage();
		m_aPresets = presetStorage.GetAllPresets();
		
		// Add items
		for (int i = m_aPresets.Count() - 1; i >= 0; i--)
		{
			SCR_WorkshopAddonPreset preset = m_aPresets[i];
			
			SCR_WorkshopAddonPresetListItemData data = new SCR_WorkshopAddonPresetListItemData(preset.GetName());
			int id = lb.AddItem(preset.GetName(), data);
			
			// Items actions 
			Widget wLine = lb.GetElementComponent(id).GetRootWidget();
			if (!wLine)
				continue;
			
			SCR_AddonLinePresetComponent line = SCR_AddonLinePresetComponent.Cast(wLine.FindHandler(SCR_AddonLinePresetComponent));
			if (!line)
				continue;	
			
			line.SetPreset(preset);
			line.UpdateWidgets();

			line.GetEventOnNameChanged().Insert(RenamePreset);
			line.GetEventOnNameEditStart().Insert(OnNameEditStart);
			line.GetEventOnLoad().Insert(LoadPreset);
			line.GetEventOnOverride().Insert(OverridePreset);
			line.GetEventOnDelete().Insert(DeletePreset);
			line.GetEventOnFocus().Insert(OnLineFocus);
			line.GetEventOnFocusLost().Insert(OnLineFocusLost);
			line.GetEventOnButtonClick().Insert(OnLineButtonClick);
			
			line.ShowWarning(!HasAllPresetAddons(line));
			
			m_aPresetLines.Insert(line);
		}
		
		GetGame().GetCallqueue().CallLater(OnUsedPresetChanged, 0, false, SCR_AddonManager.GetInstance().GetPresetStorage().GetUsedPreset(), "");
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Return true if all mods are available offline
	protected bool HasAllPresetAddons(SCR_AddonLinePresetComponent line)
	{
		SCR_WorkshopAddonPreset preset = line.GetPreset();
		if (!preset)
			return false;
		
		array<ref SCR_WorkshopAddonPresetAddonMeta> addons = preset.GetAddons();
		foreach (SCR_WorkshopAddonPresetAddonMeta meta : addons)
		{
			string guid = meta.GetGuid();
			SCR_WorkshopItem item = SCR_AddonManager.GetInstance().GetItemsMap().Get(guid);
			
			if (!item || !item.GetOffline())
				return false;
		}
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Show error if preset can't be created 
	protected bool CanCreatePreset(SCR_WorkshopAddonPreset preset)
	{
		if (!preset)
			new SCR_WorkshopErrorDialog("#AR-Workshop_EmptyAddonPresetMsg");
		
		return preset;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Show no mods enabled error is no mods is selected for preset
	protected bool IsPresetAddonsListEmpty(SCR_WorkshopAddonPreset preset)
	{
		if (preset.GetAddons().IsEmpty())
		{
			new SCR_WorkshopErrorDialog("#AR-Workshop_EmptyAddonPresetMsg");
			return false;
		}
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Show no mods enabled error is no mods is selected for preset
	protected bool IsAddonsListEmpty(array<ref SCR_WorkshopItem> items)
	{
		if (items.IsEmpty())
		{
			new SCR_WorkshopErrorDialog("#AR-Workshop_EmptyAddonPresetMsg");
			return false;
		}
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Show invalid name error dialog
	protected bool IsNameValid(string name)
	{
		// Empty 
		if (name.IsEmpty())
		{
			new SCR_WorkshopErrorDialog("#AR-Workshop_EmptyNameAddonPresetMsg");
			return false;
		}
		
		// Name is just spaces 
		bool spaceOnly = true;
		
		for (int i = 0, len = name.Length(); i < len; i++)
		{
			if (name[i] != " ")
			{
				spaceOnly = false;
				break;
			}
		}
		
		if (spaceOnly)
		{
			new SCR_WorkshopErrorDialog("#AR-Workshop_EmptyNameAddonPresetMsg");
			return false;
		}
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnUsedPresetChanged(string name)
	{
		foreach (SCR_AddonLinePresetComponent preset : m_aPresetLines)
		{
			preset.SetSelected(preset.GetName() == name);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Show override dialog if name is reapeted in addon preset list
	protected bool IsNameUnique(SCR_WorkshopAddonPreset preset, string name)
	{
		if (SCR_AddonManager.GetInstance().GetPresetStorage().PresetExists(name))
		{
			SCR_WorkshopPresetConfirmDialog dlg = SCR_WorkshopPresetConfirmDialog.CreateOverridePresetDialog(preset);
			dlg.m_OnConfirm.Insert(Callback_OnConfirmOverride);
			return false;
		}
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	// Preset actions 
	//---------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------
	//! Create new preset with curent mods 
	void CreateNewPreset()
	{
		int addonsCount = SCR_AddonManager.GetInstance().CountOfEnabledAddons();
		
		if(addonsCount == 0)
		{
			SCR_WorkshopAddonPreset empty = null;
			if (!CanCreatePreset(empty))
				return;
		}
		
		SCR_ListBoxComponent lb = m_Widgets.m_ScrollPresetsComponent;
		if (!lb)
			return;
		
		int id = lb.AddItem("");
		m_CreatedElement = lb.GetElementComponent(id);
	
		// Setup line
		Widget wLine = lb.GetElementComponent(id).GetRootWidget();
		if (!wLine)
			return;
		
		wLine.SetZOrder(-1);
		m_InteractivePreset = null;
		
		SCR_AddonLinePresetComponent line = SCR_AddonLinePresetComponent.Cast(wLine.FindHandler(SCR_AddonLinePresetComponent));
		if (!line)
			return;
		
		// Display data 
		line.ShowModCount(addonsCount);
		line.ShowWarning(false);
		
		// Setup edit action
		line.ShowDefaultName();
		line.StartEditName();
		
		SetupLineCreationState(line);
		
		InvokeEventOnRename(false);
		OnRename(false);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void Callback_OnNameNewPreset(SCR_AddonLinePresetComponent line, string name)
	{
		ClearLineCreationState(line);
		
		SCR_WorkshopAddonPreset preset = SCR_AddonManager.GetInstance().CreatePresetFromEnabledAddons(name);
		
		// Check states
		if (!CanCreatePreset(preset))
			return;
		
		if (!IsPresetAddonsListEmpty(preset))
			return;
		
		if (!IsNameValid(name))
			return;
		
		if (!IsNameUnique(preset, name))
			return;
		
		// All fine, save this preset, add item to listbox
		SCR_AddonManager.GetInstance().GetPresetStorage().SavePreset(preset);
		UpdatePresetListbox();
		
		InvokeEventOnRename(true);
		OnRename(true);
		
		m_Widgets.m_ScrollPresetsComponent.SetFocusOnFirstItem();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void Callback_OnNewPresetFocusLost(SCR_AddonLinePresetComponent line)
	{
		UpdatePresetListbox();
		ClearLineCreationState(line);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void SetupLineCreationState(SCR_AddonLinePresetComponent line)
	{
		m_Widgets.m_ButonNewPresetComponent.SetEnabled(false);
		SetExistingLinesEnabled(false);
		
		line.GetEventOnNameChanged().Insert(Callback_OnNameNewPreset);
		line.GetEventOnFocusLost().Insert(Callback_OnNameNewPreset);
		line.GetEventOnNameEditLeave().Insert(Callback_OnNewPresetFocusLost);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void ClearLineCreationState(SCR_AddonLinePresetComponent line)
	{
		m_Widgets.m_ButonNewPresetComponent.SetEnabled(true);
		SetExistingLinesEnabled(true);
		
		line.GetEventOnNameChanged().Remove(Callback_OnNameNewPreset);
		line.GetEventOnFocusLost().Remove(Callback_OnNameNewPreset);
		line.GetEventOnNameEditLeave().Remove(Callback_OnNewPresetFocusLost);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void SetExistingLinesEnabled(bool enabled)
	{
		foreach (SCR_AddonLinePresetComponent line : m_aPresetLines)
		{
			line.SetEnabled(enabled);		
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Activate mods of given preset
	void LoadPreset(SCR_AddonLinePresetComponent line)
	{
		SCR_WorkshopAddonPreset preset = line.GetPreset();
		if (!preset)
			return;
		
		array<ref SCR_WorkshopAddonPresetAddonMeta> addonsNotFound = {};
		SCR_AddonManager.GetInstance().SelectPreset(preset, addonsNotFound);
		
		if (addonsNotFound.IsEmpty())
			SCR_AddonManager.GetInstance().GetPresetStorage().SetUsedPreset(line.GetPreset().GetName());
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Change mod setup of given preset
	void OverridePreset(SCR_AddonLinePresetComponent line)
	{
		SCR_WorkshopAddonPreset preset = line.GetPreset();
		if (!preset)
			return;
		
		SCR_AddonManager addonMgr = SCR_AddonManager.GetInstance();
		array<ref SCR_WorkshopItem> enabledAddons = addonMgr.SelectItemsBasic(addonMgr.GetOfflineAddons(), EWorkshopItemQuery.ENABLED);
		
		if (!IsAddonsListEmpty(enabledAddons))
			return;
		
		// Create confirmation dialog
		SCR_WorkshopPresetConfirmDialog dlg = SCR_WorkshopPresetConfirmDialog.CreateOverridePresetDialog(preset);
		dlg.m_OnConfirm.Insert(Callback_OnConfirmOverride);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void Callback_OnConfirmOverride(SCR_WorkshopPresetConfirmDialog dlg)
	{
		string prevName = "";
		if (m_InteractivePreset)
			prevName = m_InteractivePreset.GetName();
		
		SCR_WorkshopAddonPreset preset = SCR_AddonManager.GetInstance().CreatePresetFromEnabledAddons(dlg.m_Preset.GetName());
		if (!preset)
			return;

		// Change name
		if (!m_sNameOverride.IsEmpty())
		{
			SCR_AddonManager.GetInstance().GetPresetStorage().DeletePreset(m_sNameOverride);
			preset.SetName(m_sNameOverride);
			
			m_sNameOverride = "";
		}
		
 		SCR_AddonManager.GetInstance().GetPresetStorage().SavePreset(preset, prevName);
		
		// Update LB, the displayed preset names might have changed
		UpdatePresetListbox();
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Delete given preset
	void DeletePreset(SCR_AddonLinePresetComponent line)
	{
		SCR_WorkshopAddonPreset preset = line.GetPreset();
		if (!preset)
			return;
		
		SCR_WorkshopPresetConfirmDialog dlg = SCR_WorkshopPresetConfirmDialog.CreateDeletePresetDialog(preset.GetName());
		dlg.m_OnConfirm.Insert(Callback_OnConfirmDelete);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void Callback_OnConfirmDelete(SCR_WorkshopPresetConfirmDialog dlg)
	{
		SCR_AddonManager.GetInstance().GetPresetStorage().DeletePreset(dlg.m_sPresetName);
		
		// Update LB, the displayed preset names might have changed
		UpdatePresetListbox();
	}
	
	protected string m_sNameOverride;
	
	//---------------------------------------------------------------------------------------------------
	//! Change name of given preset
	void RenamePreset(SCR_AddonLinePresetComponent line, string name)
	{
		SCR_WorkshopAddonPreset preset = line.GetPreset();
		if (!preset)
			return;
		
		if (!IsNameValid(name))
			return;
		
		// Check if preset with same name is not unique
		SCR_WorkshopAddonManagerPresetStorage storage = SCR_AddonManager.GetInstance().GetPresetStorage();
		SCR_WorkshopAddonPreset presetCompare = storage.GetPreset(name);
		
		m_sNameOverride = "";
		
		if (presetCompare && presetCompare != preset)
		{
			m_sNameOverride = name;
			SCR_WorkshopPresetConfirmDialog dlg = SCR_WorkshopPresetConfirmDialog.CreateOverridePresetDialog(preset);
			dlg.m_OnConfirm.Insert(Callback_OnConfirmOverride);
			return;
		}
		
		string prevName = preset.GetName();
		preset.SetName(name);
		
		storage.SavePreset(preset, prevName);
		UpdatePresetListbox();
		
		InvokeEventOnRename(true);
		OnRename(true);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnNameEditStart(SCR_AddonLinePresetComponent line)
	{
		if (!line)
			return;
		
		m_InteractivePreset = line.GetPreset();
		
		InvokeEventOnRename(false);
		OnRename(false);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_AddonLinePresetComponent line)
	{
		if (!line)
			return;
		
 		m_InteractivePreset = line.GetPreset();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnLineFocusLost(SCR_AddonLinePresetComponent line)
	{
		InvokeEventOnRename(true);
		OnRename(true);
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Call this when any inner buttons is clicked
	protected void OnLineButtonClick(SCR_AddonLinePresetComponent line)
	{
		m_InteractivePreset = line.GetPreset();
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Return addon preset line component of currently focused widget if has any 
	SCR_AddonLinePresetComponent FocusedPresetLine()
	{
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focused)
			return null;
		
		return SCR_AddonLinePresetComponent.Cast(focused.FindHandler(SCR_AddonLinePresetComponent));
	}
	
	//---------------------------------------------------------------------------------------------------
	// Create new preset button callbacks
	//---------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------
	protected void OnNewPresetFocus()
	{
		if (m_NavLoad)
			m_NavLoad.SetLabel("#AR-Workshop_CreateNewAddonPreset");
		
		if (m_NavOverride)
			m_NavOverride.SetEnabled(false);
		
		if (m_NavDelete)
			m_NavDelete.SetEnabled(false);
		
		if (m_NavRename)
			m_NavRename.SetEnabled(false);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnNewPresetFocusLost()
	{
		if (m_NavLoad)
			m_NavLoad.SetLabel("#AR-PauseMenu_Load");
		
		if (m_NavOverride)
			m_NavOverride.SetEnabled(true);
		
		if (m_NavDelete)
			m_NavDelete.SetEnabled(true);
		
		if (m_NavRename)
			m_NavRename.SetEnabled(true);
	}
	
	//----------------------------------------------------------------------------------------------
	protected void OnRename(bool done)
	{
		if (done)
		{
			// Show focused widget controls 
			if (GetGame().GetWorkspace().GetFocusedWidget() == m_Widgets.m_ButonNewPreset)
				OnNewPresetFocus();
			else
				OnNewPresetFocusLost();
		}
		else
		{
			if (m_NavLoad)
				m_NavLoad.SetLabel("#AR-MainMenu_Rename");
			
			if (m_NavOverride)
				m_NavOverride.SetEnabled(false);
			
			if (m_NavDelete)
				m_NavDelete.SetEnabled(false);
			
			if (m_NavRename)
				m_NavRename.SetEnabled(false);
		}
	}

	//---------------------------------------------------------------------------------------------------
	// Nav buttons callbacks
	//---------------------------------------------------------------------------------------------------

	//---------------------------------------------------------------------------------------------------
	protected void OnLoadButton()
	{
		SCR_AddonLinePresetComponent focusedLine = FocusedPresetLine();
		
		if (!focusedLine)
			return;
		
		LoadPreset(focusedLine);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnOverrideButton()
	{
		SCR_AddonLinePresetComponent focusedLine = FocusedPresetLine();
		
		if (focusedLine)
			OverridePreset(focusedLine);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnRenameButton()
	{
		SCR_AddonLinePresetComponent focusedLine = FocusedPresetLine();
		
		if (focusedLine)
			focusedLine.StartEditName();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnDeleteButton()
	{
		SCR_AddonLinePresetComponent focusedLine = FocusedPresetLine();
		
		if (focusedLine)
			DeletePreset(focusedLine);
	}
}
