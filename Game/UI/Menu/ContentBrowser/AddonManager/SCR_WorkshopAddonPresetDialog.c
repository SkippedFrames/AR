// Small container to attached to each item in the list box
class SCR_WorkshopAddonPresetListItemData
{
	string m_sPresetName; // Ogirinal name of preset
	
	void SCR_WorkshopAddonPresetListItemData(string name)
	{
		m_sPresetName = name;
	}
}

/*
// We could inherit from MenuBase, but DialogUI implements some basic functionality, like Back button.
class SCR_WorkshopAddonPresetDialog : DialogUI
{
	protected ref SCR_WorkshopAddonPresetDialogWidgets widgets = new SCR_WorkshopAddonPresetDialogWidgets();
	
	protected const string STR_ERROR_NAME_INVALID = "Preset name is invalid!";
	protected const string STR_ERROR_NO_ENABLED_ADDONS = "You have not enabled any mods!";
	
	
	//---------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		widgets.Init(GetRootWidget());
		
		widgets.m_SaveButtonComponent.m_OnClicked.Insert(OnSaveButton);
		widgets.m_LoadButtonComponent.m_OnClicked.Insert(OnLoadButton);
		widgets.m_OverrideButtonComponent.m_OnClicked.Insert(OnOverrideButton);
		widgets.m_DeleteButtonComponent.m_OnClicked.Insert(OnDeleteButton);
		widgets.m_DisableAllButtonComponent.m_OnClicked.Insert(OnDisableAllButton);
		
		UpdatePresetListbox();
	}
	
	//----------------------------------------------------------------------------------------------
	// BUTTONS
	
	
	//---------------------------------------------------------------------------------------------------
	protected void OnSaveButton()
	{
		string presetName = widgets.m_PresetNameEditboxComponent.GetValue();
		
		SCR_WorkshopAddonManagerPresetStorage storage = SCR_AddonManager.GetInstance().GetPresetStorage();
		
		// Bail if name empty
		if (presetName.IsEmpty())
		{
			new SCR_WorkshopErrorDialog(STR_ERROR_NAME_INVALID);
			return;
		}
		
		SCR_WorkshopAddonPreset preset = SCR_AddonManager.GetInstance().CreatePresetFromEnabledAddons(presetName);
		
		// Bail if there is nothing enabled
		if (!preset)
		{
			new SCR_WorkshopErrorDialog(STR_ERROR_NO_ENABLED_ADDONS);
			return;
		}
		
		// Check if preset with same name is not unique
		if (storage.PresetExists(presetName))
		{
			SCR_WorkshopPresetConfirmDialog dlg = SCR_WorkshopPresetConfirmDialog.CreateOverridePresetDialog(preset);
			dlg.m_OnConfirm.Insert(Callback_OnConfirmOverride);
			return;
		}
		
		// All fine, save this preset, add item to listbox
		storage.SavePreset(preset);
		UpdatePresetListbox();
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void OnOverrideButton()
	{
		int selectedId = widgets.m_PresetNamesListboxComponent.GetSelectedItem();
		
		if (selectedId == -1)
			return;
		
		string selectedName = LB_GetItemName(selectedId);
		
		SCR_WorkshopAddonPreset preset = SCR_AddonManager.GetInstance().CreatePresetFromEnabledAddons(selectedName);
		
		// Bail if there is nothing enabled
		if (!preset)
		{
			new SCR_WorkshopErrorDialog(STR_ERROR_NO_ENABLED_ADDONS);
			return;
		}
		
		// Create confirmation dialog
		SCR_WorkshopPresetConfirmDialog dlg = SCR_WorkshopPresetConfirmDialog.CreateOverridePresetDialog(preset);
		dlg.m_OnConfirm.Insert(Callback_OnConfirmOverride);
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void OnDeleteButton()
	{
		int selectedId = widgets.m_PresetNamesListboxComponent.GetSelectedItem();
		
		if (selectedId == -1)
			return;
		
		string selectedName = LB_GetItemName(selectedId);
		
		SCR_WorkshopPresetConfirmDialog dlg = SCR_WorkshopPresetConfirmDialog.CreateDeletePresetDialog(selectedName);
		dlg.m_OnConfirm.Insert(Callback_OnConfirmDelete)
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void OnLoadButton()
	{
		int selectedId = widgets.m_PresetNamesListboxComponent.GetSelectedItem();
		
		if (selectedId == -1)
			return;
		
		string selectedName = LB_GetItemName(selectedId);
		
		SCR_WorkshopAddonManagerPresetStorage storage = SCR_AddonManager.GetInstance().GetPresetStorage();
		SCR_WorkshopAddonPreset preset = storage.GetPreset(selectedName);
		
		if (!preset)
			return;
		
		array<ref SCR_WorkshopAddonPresetAddonMeta> addonsNotFound = {};
		SCR_AddonManager.GetInstance().SelectPreset(preset, addonsNotFound);
		
		// Show error dialog if we didn't find all addons
		if (!addonsNotFound.IsEmpty())
		{
			new SCR_WorkshopErrorPresetLoadDialog(addonsNotFound);
		}
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void OnDisableAllButton()
	{
		array<ref SCR_WorkshopItem> offlineAddons = SCR_AddonManager.GetInstance().GetOfflineAddons();
		
		foreach (SCR_WorkshopItem item : offlineAddons)
		{
			item.SetEnabled(false);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	// CALLBACKS
	
	
	//---------------------------------------------------------------------------------------------------
	protected void Callback_OnConfirmOverride(SCR_WorkshopPresetConfirmDialog dlg)
	{
		int id = LB_FindItemByName(dlg.m_Preset.GetName());
		
		if (id == -1)
			return;
		
		SCR_AddonManager.GetInstance().GetPresetStorage().SavePreset(dlg.m_Preset);
		
		// Update LB, the displayed preset names might have changed
		UpdatePresetListbox();
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void Callback_OnConfirmDelete(SCR_WorkshopPresetConfirmDialog dlg)
	{
		SCR_AddonManager.GetInstance().GetPresetStorage().DeletePreset(dlg.m_sPresetName);
		
		// Update LB, the displayed preset names might have changed
		UpdatePresetListbox();
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected void UpdatePresetListbox()
	{
		SCR_ListBoxComponent lb = widgets.m_PresetNamesListboxComponent;
		
		// Remove all items
		while (lb.GetItemCount() > 0)
			lb.RemoveItem(0);
		
		SCR_WorkshopAddonManagerPresetStorage presetStorage = SCR_AddonManager.GetInstance().GetPresetStorage();
		array<ref SCR_WorkshopAddonPreset> presets = presetStorage.GetAllPresets();
		
		foreach (SCR_WorkshopAddonPreset preset : presets)
		{
			SCR_WorkshopAddonPresetListItemData data = new SCR_WorkshopAddonPresetListItemData(preset.GetName());
			lb.AddItem(GetPresetDisplayName(preset), data);
		}
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected string GetPresetDisplayName(notnull SCR_WorkshopAddonPreset preset)
	{
		int count  = preset.GetAddonCount();
		string strMods;
		if (count == 1)
			strMods = "1 mod";
		else
			strMods = string.Format("%1 mods", count);
		
		return string.Format("%1  -  %2", preset.GetName(), strMods);
	}
	
	
	//---------------------------------------------------------------------------------------------------
	// Find item with given name in list box
	protected int LB_FindItemByName(string name)
	{
		SCR_ListBoxComponent lb = widgets.m_PresetNamesListboxComponent;
		int count = lb.GetItemCount();
		
		for (int i = 0; i < count; i++)
		{
			Managed _data = lb.GetItemData(i);
			if (!_data)
				continue;
			SCR_WorkshopAddonPresetListItemData itemData = SCR_WorkshopAddonPresetListItemData.Cast(_data);
			if (!itemData)
				continue;
			
			if (itemData.m_sPresetName == name)
				return i;
		}
		
		return -1;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected string LB_GetItemName(int id)
	{
		Managed _data = widgets.m_PresetNamesListboxComponent.GetItemData(id);
		
		if (!_data)
			return string.Empty;
		
		SCR_WorkshopAddonPresetListItemData itemData = SCR_WorkshopAddonPresetListItemData.Cast(_data);
		
		if (!itemData)
			return string.Empty;
		
		return itemData.m_sPresetName;
	}
}
*/
