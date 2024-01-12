class SCR_WorkshopListAddonsSubmenu : SCR_SubMenuBase
{
	[Attribute("5", UIWidgets.EditBox, "Max number of addon that will be enabled in single frame")]
	protected int m_iAddonsInFrame;

	[Attribute("{1E9609F84FF1BF73}UI/WEXT_AddonLine.layout")]
	protected ResourceName m_sLineLayout;
	
	protected SCR_InputButtonComponent m_NavEnable;
	protected SCR_InputButtonComponent m_NavEnableAll;
	protected SCR_InputButtonComponent m_NavDelete;
	protected SCR_InputButtonComponent m_NavDeleteAll;
	protected SCR_InputButtonComponent m_NavOpenDetails;
	protected SCR_InputButtonComponent m_NavUpdate;

	protected SCR_InputButtonComponent m_NavOpenTools;
	protected SCR_InputButtonComponent m_NavOpenPresets;

	protected ref SCR_ListAddonsSubMenuWidgets m_Widgets = new SCR_ListAddonsSubMenuWidgets();

	protected SCR_WorkshopAddonLineComponent m_FocusedAddonLine;
	protected SCR_WorkshopItem m_LastEnabledItem;
	protected bool m_bIsFocusedEnabled;

	protected ref array<Widget> m_aEntriesEnabled = {};
	protected ref array<Widget> m_aEntriesDisabled = {};
	protected ref map<SCR_WorkshopItem, SCR_WorkshopAddonLineComponent> m_aEntriesComponents = new map<SCR_WorkshopItem, SCR_WorkshopAddonLineComponent>();
	
	//------------------------------------------------------------------------------------------------
	//! Refreshes all lists
	protected void RefreshAll()
	{
		m_aEntriesComponents.Clear();

		// Get offline items from API
		array<WorkshopItem> rawWorkshopItems = {};
		GetGame().GetBackendApi().GetWorkshop().GetOfflineItems(rawWorkshopItems);
		
		// Register items in Addon Manager
		array<ref SCR_WorkshopItem> itemsRegistered = {};
		foreach (WorkshopItem i : rawWorkshopItems)
		{
			SCR_WorkshopItem itemRegistered = SCR_AddonManager.GetInstance().Register(i);
			itemsRegistered.Insert(itemRegistered);
		}

		// Convert to basic array for sorting...
		array<SCR_WorkshopItem> itemsWeakPtrs = {};
		foreach (SCR_WorkshopItem i : itemsRegistered)
		{
			itemsWeakPtrs.Insert(i);
		}

		// Sort by name...
		SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemName>.HeapSort(itemsWeakPtrs);

		// Convert back to array<ref ...>
		array<ref SCR_WorkshopItem> itemsSorted = {};
		foreach (SCR_WorkshopItem i : itemsWeakPtrs)
		{
			itemsSorted.Insert(i);
		}

		array<ref SCR_WorkshopItem> enabledItems = SCR_AddonManager.SelectItemsBasic(itemsSorted, EWorkshopItemQuery.ENABLED);
		array<ref SCR_WorkshopItem> disabledItems = SCR_AddonManager.SelectItemsBasic(itemsSorted, EWorkshopItemQuery.NOT_ENABLED);

		CreateListLines(m_Widgets.m_EnabledAddonsList, m_Widgets.m_EnabledAddonsScroll, enabledItems, m_aEntriesEnabled);
		CreateListLines(m_Widgets.m_DisabledAddonsList, m_Widgets.m_DisabledAddonsScroll, disabledItems, m_aEntriesDisabled);

		FocusLastWidget();
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusFirstLine()
	{
		Widget focus;

		if (m_Widgets.m_EnabledAddonsPanel && m_Widgets.m_EnabledAddonsPanel.GetChildren())
			focus = m_Widgets.m_EnabledAddonsList.GetChildren();
		else if (m_Widgets.m_DisabledAddonsPanel && m_Widgets.m_DisabledAddonsPanel.GetChildren())
			focus = m_Widgets.m_DisabledAddonsPanel.GetChildren();

		if (!focus || !focus.IsFocusable())
		{
			if (!m_aEntriesDisabled.IsEmpty())
				focus = m_aEntriesDisabled[0];
			else if (!m_aEntriesEnabled.IsEmpty())
				focus = m_aEntriesEnabled[0];
			// Fallback to presets button, which is always present
			else
				focus = m_ParentMenu.GetRootWidget().FindAnyWidget("m_PresetsButton");
		}

		GetGame().GetWorkspace().SetFocusedWidget(focus);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateListLines(VerticalLayoutWidget vLayout, ScrollLayoutWidget scroll, array<ref SCR_WorkshopItem> items, out array<Widget> entries)
	{
		// Record scroll pos
		float scrollx, scrolly;
		scroll.GetSliderPos(scrollx, scrolly);

		if (!entries.IsEmpty())
		{
			for (int i = 0, count = entries.Count(); i < count; i++)
			{
				vLayout.RemoveChild(entries[i]);
			}

			entries.Clear();
		}

		foreach (SCR_WorkshopItem item : items)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sLineLayout, vLayout);
			SCR_WorkshopAddonLineComponent comp = SCR_WorkshopAddonLineComponent.Cast(w.FindHandler(SCR_WorkshopAddonLineComponent));
			comp.Init(item);
			comp.m_OnEnableButton.Insert(OnEnableButton);
			comp.m_OnDisableButton.Insert(OnDisableButton);
			comp.m_OnMouseEnter.Insert(OnLineMouseEnter);
			comp.m_OnMouseLeave.Insert(OnLineMouseLeave);
			comp.m_OnFocus.Insert(OnLineFocus);
			comp.m_OnFocusLost.Insert(OnLineFocusLost);

			entries.Insert(w);

			// Store widgets and component
			m_aEntriesComponents.Insert(item, comp);
		}

		// Restore scroll pos
		scroll.SetSliderPos(scrollx, scrolly);
	}

	//------------------------------------------------------------------------------------------------
	//! Create navigation button for actions
	protected void CreateNavigationButtons()
	{
		m_NavUpdate = CreateNavigationButton("WorkshopPrimary", "#AR-Workshop_ButtonUpdate", true);
		if (m_NavUpdate)
			m_NavUpdate.m_OnActivated.Insert(OnNavUpdateActivate);

		
		m_NavOpenDetails = CreateNavigationButton("MenuSelect", "#AR-Workshop_Details_MenuTitle", true);
		if (m_NavOpenDetails)
			m_NavOpenDetails.m_OnActivated.Insert(OnNavOpenDetailsActivated);

		
		m_NavDelete = CreateNavigationButton("MenuDelete", "#AR-Workshop_ButtonDelete", true);
		if (m_NavDelete)
			m_NavDelete.m_OnActivated.Insert(OnNavDeleteActivated);

		
		m_NavEnable = CreateNavigationButton("MenuEnable", "#AR-Workshop_ButtonEnable", true);
		if (m_NavEnable)
			m_NavEnable.m_OnActivated.Insert(OnNavEnableActivated);

		
		m_NavEnableAll = CreateNavigationButton("MenuEnableAll", "#AR-Workshop_TabName_All", false);
		if (m_NavEnableAll)
		{
			//m_NavDeleteAll.m_OnActivated.Insert(OnNavDeleteAllActivated);
			m_NavEnableAll.m_OnActivated.Insert(OnNavEnableAllActivated);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavEnableActivated()
	{
		if (!m_FocusedAddonLine)
			return;

		SCR_WorkshopAddonLineComponent line = ClosestAddonLine(m_FocusedAddonLine.GetRootWidget(), !m_bIsFocusedEnabled);

		if (line)
			m_LastEnabledItem = line.GetWorkshopItem();
		else
			m_LastEnabledItem = m_FocusedAddonLine.GetWorkshopItem();

		// Actions
		if (m_FocusedAddonLine.IsWorkshopItemEnabled())
			m_FocusedAddonLine.OnDisableButton();
		else
			m_FocusedAddonLine.OnEnableButton();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavEnableAllActivated()
	{
		if (m_FocusedAddonLine)
			m_LastEnabledItem = m_FocusedAddonLine.GetWorkshopItem();

		if (m_bIsFocusedEnabled)
			EnableAll(false);
		else
			EnableAll(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Find last enabled/disabled item by workshop item in other list
	protected void FocusLastWidget()
	{
		if (!m_LastEnabledItem)
			FocusFirstLine();

		array<Widget> list = {};

		// Select list
		if (!m_bIsFocusedEnabled)
		{
			if (!m_aEntriesDisabled.IsEmpty())
				list = m_aEntriesDisabled;
			else
				list = m_aEntriesEnabled;
		}
		else
		{
			if (!m_aEntriesEnabled.IsEmpty())
				list = m_aEntriesEnabled;
			else
				list = m_aEntriesDisabled;
		}

		// Search list
		for (int i = 0, count = list.Count(); i < count; i++)
		{
			SCR_WorkshopAddonLineComponent comp = SCR_WorkshopAddonLineComponent.Cast(list[i].FindHandler(SCR_WorkshopAddonLineComponent));
			if (!comp)
				continue;

			if (comp.GetWorkshopItem() == m_LastEnabledItem)
			{
				ApplyLineFocus(list[i], comp);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyLineFocus(Widget w, SCR_WorkshopAddonLineComponent comp)
	{
		GetGame().GetWorkspace().SetFocusedWidget(w);
		m_FocusedAddonLine = comp;
	}

	//------------------------------------------------------------------------------------------------
	//! Return closest workshop item, first check down, then up if at the end of the list
	protected SCR_WorkshopAddonLineComponent ClosestAddonLine(Widget line, bool enabled)
	{
		array<Widget> list = {};

		if (enabled)
			list = m_aEntriesDisabled;
		else
			list = m_aEntriesEnabled;

		// Single entry?
		if (list.Count() == 1)
			return SCR_WorkshopAddonLineComponent.Cast(line.FindHandler(SCR_WorkshopAddonLineComponent));

		int lineId = list.Find(line);

		// Not found?
		if (lineId == -1)
			return null;

		Widget closestLine;

		// Last?
		if (lineId == list.Count() -1)
		{
			// Get upper
			closestLine = list[lineId - 1];
		}
		else
		{
			// Get lower
			closestLine = list[lineId + 1];
		}

		return SCR_WorkshopAddonLineComponent.Cast(closestLine.FindHandler(SCR_WorkshopAddonLineComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavDeleteActivated()
	{
		if (!m_FocusedAddonLine)
			return;
		
		SCR_WorkshopAddonLineComponent line = ClosestAddonLine(m_FocusedAddonLine.GetRootWidget(), !m_bIsFocusedEnabled);

		if (line)
			m_LastEnabledItem = line.GetWorkshopItem();
		else
			m_LastEnabledItem = m_FocusedAddonLine.GetWorkshopItem();

		if (m_FocusedAddonLine)
		{
			SCR_WorkshopItem item = m_FocusedAddonLine.GetWorkshopItem();

			if (item)
			{
				SCR_DeleteAddonDialog dialog = SCR_DeleteAddonDialog.CreateDeleteAddon(item);
				dialog.m_OnClose.Insert(OnDeleteDialogClose);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavDeleteAllActivated()
	{
		array<ref SCR_WorkshopItem> addons = SCR_AddonManager.GetInstance().GetOfflineAddons();
		for (int i = 0, count = addons.Count(); i < count; i++)
		{
			//Print("Delete: " + addons[i].GetName());
			//addons[i].DeleteLocally();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDeleteDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		GetGame().GetCallqueue().CallLater(FocusLastWidget);
		dialog.m_OnClose.Remove(OnDeleteDialogClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavOpenDetailsActivated()
	{
		if (m_FocusedAddonLine)
			m_FocusedAddonLine.OnOpenDetailsButton();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavUpdateActivate()
	{
		if (!m_FocusedAddonLine)
			return;

		SCR_WorkshopItem item = m_FocusedAddonLine.GetWorkshopItem();
		if (!item)
			return;

		if (item.GetUpdateAvailable())
		{
			// Update
			m_FocusedAddonLine.OnUpdateButton();
		}
		else if (item.GetAnyDependencyMissing() || item.GetAnyDependencyUpdateAvailable())
		{
			// Fix - donwload dependencies
			m_FocusedAddonLine.OnFixButton();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		m_Widgets.Init(GetRootWidget());

		// Tools
		m_Widgets.m_ToolsButtonComponent.m_OnClicked.Insert(OnToolsButton);

		// Move all mods
		m_Widgets.m_ButtonEnableAllComponent.m_OnClicked.Insert(OnButtonEnableAll);
		m_Widgets.m_ButtonDisableAllComponent.m_OnClicked.Insert(OnButtonDisableAll);

		CreateNavigationButtons();
		RefreshAll();
		
		// Subscribe to addon manager events
		SCR_AddonManager.GetInstance().m_OnAddonOfflineStateChanged.Insert(Callback_OnAddonOfflineStateChanged);
		SCR_AddonManager.GetInstance().m_OnAddonsEnabledChanged.Insert(Callback_OnAddonEnabledStateChanged);

		// Change input schceme check
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuClose(parentMenu);

		// Unsubscribe from addon manager events
		SCR_AddonManager.GetInstance().m_OnAddonOfflineStateChanged.Remove(Callback_OnAddonOfflineStateChanged);
		SCR_AddonManager.GetInstance().m_OnAddonsEnabledChanged.Remove(Callback_OnAddonEnabledStateChanged);

		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		GetGame().GetCallqueue().CallLater(FocusFirstLine, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ShowNavigationButtons(bool show)
	{
		super.ShowNavigationButtons(show && !m_aEntriesComponents.IsEmpty());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		FocusLastWidget();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		if (m_bShown)
			FocusLastWidget();

		super.OnMenuFocusGained();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabChange(SCR_SuperMenuBase parentMenu)
	{
		super.OnTabChange(parentMenu);

		if (GetShown())
			FocusLastWidget();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseEnter(SCR_WorkshopAddonLineComponent comp)
	{
		SCR_WorkshopItem item = comp.GetWorkshopItem();
		m_Widgets.m_AddonInfoPanelComponent.SetWorkshopItem(item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseLeave(SCR_WorkshopAddonLineComponent comp)
	{
		if (!m_FocusedAddonLine)
			return;

		SCR_WorkshopItem item = m_FocusedAddonLine.GetWorkshopItem();
		m_Widgets.m_AddonInfoPanelComponent.SetWorkshopItem(item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_WorkshopAddonLineComponent comp)
	{
		if (!comp)
			return;

		m_FocusedAddonLine = comp;

		// Nav buttons
		if (m_NavEnable)
		{
			m_NavEnable.SetEnabled(true);

			m_bIsFocusedEnabled = m_FocusedAddonLine.IsWorkshopItemEnabled();

			if (m_bIsFocusedEnabled)
				m_NavEnable.SetLabel("#AR-Workshop_ButtonDisable");
			else
				m_NavEnable.SetLabel("#AR-Workshop_ButtonEnable");
		}

		if (m_NavDelete)
			m_NavDelete.SetEnabled(true);

		if (m_NavOpenDetails)
			m_NavOpenDetails.SetEnabled(true);

		if (m_NavEnableAll)
		{
			m_NavEnableAll.SetEnabled(true);

			if (m_bIsFocusedEnabled)
				m_NavEnableAll.SetLabel("#AR-Workshop_DisableAll");
			else
				m_NavEnableAll.SetLabel("#AR-ServerHosting_EnableAll");
		}

		if (m_NavUpdate)
		{
			SCR_WorkshopItem item = comp.GetWorkshopItem();
			if (!item)
				m_NavUpdate.SetEnabled(true);

			if (item.GetUpdateAvailable())
			{
				m_NavUpdate.SetEnabled(true);
				m_NavUpdate.SetLabel("#AR-Workshop_ButtonUpdate");
			}
			else if (item.GetAnyDependencyMissing() || item.GetAnyDependencyUpdateAvailable())
			{
				m_NavUpdate.SetEnabled(true);
				m_NavUpdate.SetLabel("#AR-Workshop_ButtonDownloadDependencies");
			}
			else
			{
				m_NavUpdate.SetEnabled(false);
				m_NavUpdate.SetLabel("#AR-Workshop_ButtonUpdate");
			}

		}

		OnLineMouseEnter(comp);
	}


	//------------------------------------------------------------------------------------------------
	protected void OnLineFocusLost(SCR_WorkshopAddonLineComponent comp)
	{
		m_FocusedAddonLine = null;

		// Nav button
		if (m_NavEnable)
			m_NavEnable.SetEnabled(false);

		if (m_NavDelete)
			m_NavDelete.SetEnabled(false);

		if (m_NavOpenDetails)
			m_NavOpenDetails.SetEnabled(false);

		if (m_NavEnableAll)
			m_NavEnableAll.SetEnabled(false);

		if (m_NavUpdate)
			m_NavUpdate.SetEnabled(false);
	}

	
	//------------------------------------------------------------------------------------------------
	// L I N E  B U T T O N S
	//------------------------------------------------------------------------------------------------
	protected void OnEnableButton(SCR_WorkshopAddonLineComponent comp)
	{
		SCR_WorkshopItem item = comp.GetWorkshopItem();
		SCR_WorkshopUiCommon.OnEnableAddonButton(item);

		item.SetEnabled(true);

		// Enable dependencies
		array<ref SCR_WorkshopItem> dependencies = item.GetLatestDependencies();
		foreach (SCR_WorkshopItem dep : dependencies)
		{
			dep.SetEnabled(true);
		}

		// Clear preset name
		SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();
	}


	//------------------------------------------------------------------------------------------------
	protected void OnDisableButton(SCR_WorkshopAddonLineComponent comp)
	{
		SCR_WorkshopItem item = comp.GetWorkshopItem();
		SCR_WorkshopUiCommon.OnEnableAddonButton(item);

		item.SetEnabled(false);

		// Clear preset name
		SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnToolsButton()
	{
		//new SCR_WorkshopToolsDialog();

		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.AddonPresetDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonEnableAll()
	{
		EnableAll(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonDisableAll()
	{
		EnableAll(false);
	}

	protected bool m_bIsEnablingAddons = false;
	protected SCR_LoadingOverlayDialog m_LoadningOverlay;

	//------------------------------------------------------------------------------------------------
	//! By enable arg move all enabled/disabled mods to opposite state
	protected void EnableAll(bool enable)
	{
		array<ref SCR_WorkshopItem> addons = SCR_AddonManager.GetInstance().GetOfflineAddons();

		// Remove mods with issues
		addons = SCR_AddonManager.SelectItemsBasic(addons, EWorkshopItemQuery.DEPENDENCY_NOT_MISSING);

		SCR_AddonManager addonsManager = SCR_AddonManager.GetInstance();

		addonsManager.EnableMultipleAddons(addons, enable);
		addonsManager.GetEventOnAddonEnabled().Insert(OnAddonEnabled);
		addonsManager.GetEventOnAllAddonsEnabled().Insert(OnAllAddonsEnabled);
		m_bIsEnablingAddons = true;

		m_Widgets.m_DisabledAddonsScroll.SetSliderPos(0, 0);
		m_Widgets.m_EnabledAddonsScroll.SetSliderPos(0, 0);

		// Clear preset name
		SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddonEnabled(SCR_WorkshopItem item, int id)
	{
		SCR_WorkshopAddonLineComponent line = m_aEntriesComponents.Get(item);
		if (line)
			line.EnableUpdate(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAllAddonsEnabled()
	{
		m_bIsEnablingAddons = false;
		RefreshAll();

		GetGame().GetCallqueue().CallLater(FocusFirstLine);
	}

	//------------------------------------------------------------------------------------------------
	//! Recursive addon enable calling by frames
	/*protected void EnableAddonsRecursively(array<ref SCR_WorkshopItem> addons, out int remaining, bool enable`)
	{
		/*for (int i = 0; i < 1; i++)
		{
			addons[remaining].SetEnabled(enable);
			remaining--;

			if (remaining == -1)
			{
				m_bIsEnablingAddons = false;
				m_LoadningOverlay.CloseAnimated();
				return;
			}
		}*/

		/*
		SCR_WorkshopAddonLineComponent line = m_aEntriesComponents.Get(addons[remaining]);
		if (line)
			line.EnableUpdate(false);

		addons[remaining].SetEnabled(enable);
		remaining--;

		// Countinue if remaing addons
		if (remaining != -1)
		{
			GetGame().GetCallqueue().CallLater(EnableAddonsRecursively, 0, false, addons, remaining, enable);
		}
		else
		{
			m_bIsEnablingAddons = false;
			m_LoadningOverlay.CloseAnimated();
		}*/
	//}

	//------------------------------------------------------------------------------------------------
	// O T H E R



	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AddonManager when some addon is downloaded or uninstalled
	protected void Callback_OnAddonOfflineStateChanged(SCR_WorkshopItem item, bool newState)
	{
		// Some addon was installed or uninstalled, we must refresh the page
		if (!m_bIsEnablingAddons)
			RefreshAll();
	}

	protected void Callback_OnAddonEnabledStateChanged()
	{
		// Something got enabled or disabled, refresh the page
		if (!m_bIsEnablingAddons)
			RefreshAll();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (!isGamepad)
			return;

		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focused)
			return;

		if (!focused.FindHandler(SCR_WorkshopAddonLineComponent))
			FocusFirstLine();
	}
}
