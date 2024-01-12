/*
Server hosting dialog sub menu for setting up mods
*/

class SCR_ServerHostingModSubMenu : SCR_SubMenuBase
{
	protected const string NAV_ENABLE = "#AR-Workshop_ButtonEnable";
	protected const string NAV_DISABLE = "#AR-Workshop_ButtonDisable";
	protected const string NAV_ENABLE_ALL = "#AR-ServerHosting_EnableAll";
	protected const string NAV_DISABLE_ALL = "#AR-Workshop_DisableAll";
	
	protected const string SORT_BTN_SORT_LABEL = "#AR-Editor_TooltipDetail_WaypointIndex_Name";
	protected const string SORT_BTN_SORT_ACTION = "MenuSelectHold";
	protected const string SORT_BTN_PLACE_LABEL = "#AR-Button_Confirm-UC";
	protected const string SORT_BTN_PLACE_ACTION = "MenuSelect";
	
	// Attributes 
	[Attribute("", UIWidgets.ResourceNamePicker, "Used layout for mod enabling", "layout")]
	protected ResourceName m_EntryLayout;
	
	[Attribute()]
	protected ref SCR_WidgetListEntry m_AddonEntryTemplate;
	
	// Fields 
	protected ref SCR_ServerHostingModsWidgets m_Widgets = new SCR_ServerHostingModsWidgets();
	
	protected SCR_ListBoxComponent m_DisabledList;
	protected SCR_ListBoxComponent m_EnabledList;
	
	protected ScrollLayoutWidget m_wDisableScroll;
	protected ScrollLayoutWidget m_wEnableScroll;
	protected SCR_GamepadScrollComponent m_DisableScroll;
	protected SCR_GamepadScrollComponent m_EnableScroll;
	
	protected ref array<ref Widget> m_aEnabled = {};
	protected ref array<ref Widget> m_aDisabled = {};
	
	// Navigation buttons 
	protected SCR_InputButtonComponent m_NavWorkshop;
	protected SCR_InputButtonComponent m_NavEnable;
	protected SCR_InputButtonComponent m_NavEnableAll;
	protected SCR_InputButtonComponent m_NavSelectSort;
	protected SCR_InputButtonComponent m_NavChangeSortOrder;
	
	protected SCR_AddonLineDSConfigComponent m_FocusedLine;
	protected SCR_AddonLineDSConfigComponent m_OrderedLine;
	
	protected ref map<ref SCR_WorkshopItem, ref SCR_AddonLineDSConfigComponent> m_aEnabledMods = new map<ref SCR_WorkshopItem, ref SCR_AddonLineDSConfigComponent>();
	
	protected bool m_bIsListeningForCommStatus;
	
	//------------------------------------------------------------------------------------------------
	// Invokers
	//------------------------------------------------------------------------------------------------
	 
	protected ref ScriptInvoker<> Event_OnWorkshopButtonActivate;

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnWorkshopButtonActivate()
	{
		if (Event_OnWorkshopButtonActivate)
			Event_OnWorkshopButtonActivate.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnWorkshopButtonActivate()
	{
		if (!Event_OnWorkshopButtonActivate)
			Event_OnWorkshopButtonActivate = new ScriptInvoker();

		return Event_OnWorkshopButtonActivate;
	}
	
	//------------------------------------------------------------------------------------------------
	// Widget handler override 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Widgets.Init(w);
		
		// Find 
		m_DisabledList = SCR_ListBoxComponent.Cast(SCR_WidgetTools.FindHandlerOnWidget(
			m_Widgets.m_DisabledAddonsList, "Content", SCR_ListBoxComponent));
		
		m_EnabledList = SCR_ListBoxComponent.Cast(SCR_WidgetTools.FindHandlerOnWidget(
			m_Widgets.m_EnabledAddonsList, "Content", SCR_ListBoxComponent));
		
		// Find scroll components
		m_wDisableScroll = ScrollLayoutWidget.Cast(m_Widgets.m_DisabledAddonsList.GetParent());
		m_wEnableScroll = ScrollLayoutWidget.Cast(m_Widgets.m_EnabledAddonsList.GetParent());
		
		m_DisableScroll = SCR_GamepadScrollComponent.Cast(m_wDisableScroll.FindHandler(SCR_GamepadScrollComponent));
		m_EnableScroll = SCR_GamepadScrollComponent.Cast(m_wEnableScroll.FindHandler(SCR_GamepadScrollComponent));
		
		// Setup action all buttons 
		m_Widgets.m_ButtonEnableAllComponent.m_OnClicked.Insert(OnEnableAllClicked);
		m_Widgets.m_ButtonDisableAllComponent.m_OnClicked.Insert(OnDisableAllClicked);
		
		CreateAddonList();

		SetupDownloadingCallbacks();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		ClearDownloadingCallbacks();
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		m_bIsListeningForCommStatus = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		if (!m_NavWorkshop)
		{
			m_NavWorkshop = CreateNavigationButton("MenuDownloadManager", "#AR-Workshop_WorkshopPageName", false);
			
			if (m_NavWorkshop)
				m_NavWorkshop.m_OnActivated.Insert(OnOpenWorkshopButton);
		}
		
		// Setup navigation buttons 
		if (!m_NavSelectSort)
		{
			m_NavSelectSort = CreateNavigationButton("MenuSelectHold", "#AR-Editor_TooltipDetail_WaypointIndex_Name", true);
			
			if (m_NavSelectSort)
				m_NavSelectSort.m_OnActivated.Insert(OnNavSelectSortActivate);
		}
		
		if (!m_NavChangeSortOrder)
		{
			m_NavChangeSortOrder = CreateNavigationButton("MenuVertical", "#AR-Editor_TooltipDetail_WaypointIndex_Name", true);
		
			if (m_NavChangeSortOrder)
				m_NavChangeSortOrder.SetVisible(false);
		}
		
		if (!m_NavEnable)
		{
			m_NavEnable = CreateNavigationButton("MenuEnable", NAV_ENABLE, true);
			
			if (m_NavEnable)
				m_NavEnable.m_OnActivated.Insert(OnNavEnableActivated);
		}
		
		if (!m_NavEnableAll)
		{
			m_NavEnableAll = CreateNavigationButton("MenuEnableAll", NAV_ENABLE_ALL, false);
			
			if (m_NavEnableAll)
				m_NavEnableAll.m_OnActivated.Insert(OnNavEnableAllActivated);
		}
		
		// Focus fist entry next frame to ensure focus will happend
		GetGame().GetCallqueue().CallLater(FocusFirstEntry);
		
		SCR_ServicesStatusHelper.RefreshPing();
		
		if (!m_bIsListeningForCommStatus)
			SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_bIsListeningForCommStatus = true;
		UpdateWorkshopButton();
		
		super.OnMenuShow(parentMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		m_bIsListeningForCommStatus = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWorkshopButton()
	{	
		if (!m_NavWorkshop)
			return;

		SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_NavWorkshop, SCR_ServicesStatusHelper.SERVICE_WORKSHOP);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateWorkshopButton();
	}
	
	//------------------------------------------------------------------------------------------------
	override void ShowNavigationButtons(bool show)
	{
		super.ShowNavigationButtons(show && !m_aEnabledMods.IsEmpty());
	}

	//------------------------------------------------------------------------------------------------
	// Protected 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void FocusFirstEntry()
	{
		if (!m_aDisabled.IsEmpty())
		{
			if (m_aDisabled[0])
				GetGame().GetWorkspace().SetFocusedWidget(m_aDisabled[0]);
		}
		else 
		{
			if (!m_aEnabled.IsEmpty() && m_aEnabled[0])
				GetGame().GetWorkspace().SetFocusedWidget(m_aEnabled[0]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add all offline mods to selection
	protected void CreateAddonList()
	{
		if (!SCR_AddonManager.GetInstance())
			return;
		
		array<ref SCR_WorkshopItem> addons = SCR_AddonManager.GetInstance().GetOfflineAddons();
		
		for (int i = 0, count = addons.Count(); i < count; i++)
		{
			AddLineEntry(addons[i], i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddLineEntry(SCR_WorkshopItem item, int id)
	{
		// Line setup
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_EntryLayout, m_Widgets.m_DisabledAddonsList);
		SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(
			w.FindHandler(SCR_AddonLineDSConfigComponent));
		
		if (!line)
			return; 
		
		// Callbacks 
		line.m_OnEnableButton.Insert(OnAddonEnabled);
		line.m_OnDisableButton.Insert(OnAddonDisabled);
		line.GetEventOnButtonUp().Insert(OnLineButtonUp);
		line.GetEventOnButtonDown().Insert(OnLineButtonDown);
		line.m_OnFocus.Insert(OnLineFocus);
		line.m_OnFocusLost.Insert(OnLineFocusLost);
		line.m_OnFixButton.Insert(OnLineFixButton);
		
		line.Init(item);
		m_aDisabled.Insert(w);
		w.SetZOrder(id);
		
		// Enable map 
		m_aEnabledMods.Insert(item, line);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find line id in map without need to know key (mod Workshop item)
	protected int FindLineId(SCR_AddonLineDSConfigComponent line)
	{
		for (int i = 0, count = m_aEnabledMods.Count(); i < count; i++)
		{
			if (m_aEnabledMods.GetElement(i) == line)
				return i;
		}
		
		return -1;
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected int LastFocusedId(Widget line, bool enabled)
	{
		array<ref Widget> list = {};
		
		if (enabled)
			list = m_aDisabled;
		else
			list = m_aEnabled;
		
		// Single entry?
		if (list.Count() == 1)
			return -1;
		
		return list.Find(line);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return closest workshop item, first check down, then up if at the end of the list
	protected Widget ClosestAddonLine(int lineId, bool enabled)
	{
		array<ref Widget> list = {};
		
		if (enabled)
			list = m_aDisabled;
		else
			list = m_aEnabled;
		
		// Not found? 
		if (lineId == -1)
			return null;
		
		//Widget closestLine = list[lineId];
		
		// Check if last line 
		// This is hack for current implemenation
		// ideally there should be unified logic with SCR_WorkshopListAddonsSubmenu.ClosestAddonLine
		Widget closestLine;
		
		if (lineId < list.Count())
			closestLine = list[lineId];
		else
			closestLine = list[lineId - 1];
		
		return closestLine;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Fill all entries with values from given DS config 
	void EnableModsFromDSConfig(notnull SCR_DSConfig config)
	{
		array<ref DSMod> mods = config.game.mods;
		array<ref SCR_WorkshopItem> items = {};
		
		for (int i = 0, count = mods.Count(); i < count; i++)
		{
			SCR_AddonLineDSConfigComponent line = FindLineByModId(mods[i].modId);
			if (line)
			{
				// Do not restore corrupted mod
				if (line.HasItemAnyIssue())
					continue;
				
				items.Insert(line.GetWorkshopItem());
			}
		}
		
		EnableItems(true, items);
	}
	
	//-------------------------------------------------------------------------------------------
	protected SCR_AddonLineDSConfigComponent FindLineByModId(string id)
	{
		for (int i = 0, count = m_aEnabledMods.Count(); i < count; i++)
		{
			SCR_AddonLineDSConfigComponent line = m_aEnabledMods.GetElement(i);
			if (!line)
				continue;
			
			SCR_WorkshopItem item = line.GetWorkshopItem();
			if (!item)
				return null;
			
			if (id == item.GetId())
				return line;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup callbacks for all running downloads 
	protected void SetupDownloadingCallbacks()
	{
		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (!downloadManager)
			return;
		
		array<ref SCR_WorkshopItemActionDownload> actions = downloadManager.GetDownloadQueue();
		
		for (int i = 0, count = actions.Count(); i < count; i++)
		{
			actions[i].m_OnCompleted.Insert(OnDownloadComplete);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearDownloadingCallbacks()
	{
		array<ref SCR_WorkshopItemActionDownload> actions = {};
		
		if (!SCR_DownloadManager.GetInstance() || !SCR_DownloadManager.GetInstance().GetDownloadQueue())
			return;
		
		actions = SCR_DownloadManager.GetInstance().GetDownloadQueue();
		
		for (int i = 0, count = actions.Count(); i < count; i++)
		{
			actions[i].m_OnCompleted.Remove(OnDownloadComplete);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void OnAddonEnabled(SCR_AddonLineDSConfigComponent line)
	{
		if (!line)
			return;
		
		SCR_WorkshopItem item = line.GetWorkshopItem();
		if (!item)
			return;
		
		// Enable dependencies
		array<ref SCR_WorkshopItem> dependencies = item.GetLatestDependencies();
		dependencies.Insert(item);
		
		EnableItems(!line.GetWidgetEnabled(), dependencies);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAddonDisabled(SCR_AddonLineDSConfigComponent line)
	{
		if (!line)
			return;
		
		SCR_WorkshopItem item = line.GetWorkshopItem();
		if (!item)
			return;
		
		// Unselect dependent mods
		array<ref SCR_WorkshopItem> dependent = item.GetDependentAddons();
		
		// Add item 
		dependent.Insert(item);
		
		EnableItems(false, dependent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_AddonLineDSConfigComponent line)
	{
		m_FocusedLine = line;
		
		// Setup nav buttons 
		if (m_NavEnable)
			m_NavEnable.SetEnabled(!m_OrderedLine);
		
		if (m_NavEnableAll)
			m_NavEnableAll.SetEnabled(!m_OrderedLine);
		
		if (m_NavSelectSort)
			m_NavSelectSort.SetEnabled(line.GetWidgetEnabled());
		
		// Enable state 
		bool enabled = line.GetWidgetEnabled();
		
		// Nav enable text 
		if (m_NavEnable)
		{
			if (enabled)
				m_NavEnable.SetLabel(NAV_DISABLE);
			else
				m_NavEnable.SetLabel(NAV_ENABLE);
		}
		
		if (m_NavEnableAll)
		{
			if (enabled)
				m_NavEnableAll.SetLabel(NAV_DISABLE_ALL);
			else
				m_NavEnableAll.SetLabel(NAV_ENABLE_ALL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocusLost(SCR_AddonLineDSConfigComponent line)
	{
		m_FocusedLine = null;
		
		// Setup nav buttons 
		if (m_NavEnable)
			m_NavEnable.SetEnabled(false);
		
		if (m_NavEnableAll)
			m_NavEnableAll.SetEnabled(false);
		
		if (m_NavSelectSort)
			m_NavSelectSort.SetEnabled(false);
		
		// Prevent move between list when ordering 
		if (m_OrderedLine && m_OrderedLine != m_FocusedLine)
		{
			GetGame().GetCallqueue().CallLater(FocusLine, 0, false, m_OrderedLine);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFixButton(notnull SCR_AddonLineBaseComponent line)
	{
		// All dependencies
		array<ref SCR_WorkshopItem> dependencies = line.GetWorkshopItem().GetLatestDependencies();
		if (dependencies.IsEmpty())
			return;
		
		// Reported and blocked dependencies 
		array<ref SCR_WorkshopItem> issues = SCR_AddonManager.SelectItemsOr(dependencies,
			EWorkshopItemQuery.BLOCKED | EWorkshopItemQuery.AUTHOR_BLOCKED | EWorkshopItemQuery.REPORTED_BY_ME);
		
		if (!issues.IsEmpty())
		{
			// Show dialog of issues 
			SCR_AddonListDialog.CreateRestrictedAddonsDownload(issues);
			return;
		}
		
		// Missing dependencies
		dependencies = SCR_AddonManager.SelectItemsOr(dependencies, EWorkshopItemQuery.NOT_OFFLINE | EWorkshopItemQuery.UPDATE_AVAILABLE);
		if (dependencies.IsEmpty())
			return;
		
		array<ref SCR_DownloadManager_Entry> downloads = {};
		SCR_DownloadManager.GetInstance().GetAllDownloads(downloads);
		
		// Remove mods in downloading 
		for (int i = dependencies.Count() - 1; i >= 0; i--)
		{
			for (int x = 0, count = downloads.Count(); x < count; x++)
			{
				if (downloads[x].m_Item.GetId() != dependencies[i].GetId())
					continue;
					
				if (downloads[x].m_Action.IsActive())
				{
					dependencies.RemoveItem(dependencies[i]);
					break;
				}
			}
		}
		
		// Open download dialog if all are being downloaded
		if (dependencies.IsEmpty())
		{
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.DownloadManagerDialog);
			return;
		}
		
		// Create addon list and dialog 
		array<ref Tuple2<SCR_WorkshopItem, ref Revision>> addonsAndVersions = {};		
		foreach (SCR_WorkshopItem item : dependencies)
		{
			addonsAndVersions.Insert(new Tuple2<SCR_WorkshopItem, ref Revision>(item, null));
		}
		
		SCR_DownloadConfirmationDialog downloadDialog = SCR_DownloadConfirmationDialog.CreateForAddons(
			addonsAndVersions, true);
		
		downloadDialog.GetOnDownloadConfirmed().Insert(OnFixDependeciesDownloadConfirm);
	}
	
	//----------------------------------------------------------------------------------------------
	protected void OnFixDependeciesDownloadConfirm(SCR_DownloadConfirmationDialog dialog)
	{
		// Listen to download done
		foreach (SCR_WorkshopItemAction action : dialog.GetActions())
		{
			action.m_OnCompleted.Insert(OnFixDependenciesDownloadCompleted);
		} 
		
		dialog.GetOnDownloadConfirmed().Remove(OnFixDependeciesDownloadConfirm);
	}
	
	//----------------------------------------------------------------------------------------------
	protected void OnFixDependenciesDownloadCompleted(SCR_WorkshopItemAction action)
	{
		OnDownloadComplete(action);
		
		// Reset all entries state 
		for (int i = 0, count = m_aDisabled.Count(); i < count; i++)
		{
			SCR_AddonLineBaseComponent line = SCR_AddonLineBaseComponent.Cast(
				m_aDisabled[i].FindHandler(SCR_AddonLineBaseComponent));
			
			if (line)
				line.UpdateFixButton();
		}
		
		action.m_OnCompleted.Remove(OnFixDependenciesDownloadCompleted);
	}
	
	//----------------------------------------------------------------------------------------------
	protected void OnDownloadComplete(SCR_WorkshopItemAction action)
	{
		// Add mod to mod list 
		AddLineEntry(action.GetWorkshopItem(), m_aDisabled.Count());
		action.m_OnCompleted.Remove(OnDownloadComplete);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable/disable multiple addons recursively
	protected void EnableItems(bool enable, array<ref SCR_WorkshopItem> items)
	{
		if (items.IsEmpty())
			return;

		SCR_WorkshopItem item = items[0];
		
		SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(m_aEnabledMods.Get(item));
		
		if (line)
		{
			Widget lineRoot = line.GetRootWidget();
			
			// Enable 
			if (enable && !line.GetWidgetEnabled())
			{
				// Move to enabled
				m_Widgets.m_EnabledAddonsList.AddChild(lineRoot);
				m_aEnabled.Insert(lineRoot);
				m_aDisabled.RemoveItem(lineRoot);
				
				lineRoot.SetZOrder(m_aEnabled.Count() - 1);
			}
			else if (!enable && line.GetWidgetEnabled())
			{
				// Move to disabled
				m_Widgets.m_DisabledAddonsList.AddChild(lineRoot);
				m_aDisabled.Insert(lineRoot);
				m_aEnabled.RemoveItem(lineRoot);
				
				lineRoot.SetZOrder(m_aDisabled.Count() - 1);
			}
			
			line.SetWidgetEnabled(enable);
			line.HandleEnableButtons(enable);
		}
		
		// Order disabled mods 
		for (int i = 0, count = m_aDisabled.Count(); i < count; i++)
		{
			m_aDisabled[i].SetZOrder(i);
		}
		
		// Order enabled mods
		for (int i = 0, count = m_aEnabled.Count(); i < count; i++)
		{
			m_aEnabled[i].SetZOrder(i);
			
			SCR_AddonLineDSConfigComponent lineCmp = SCR_AddonLineDSConfigComponent.Cast(m_aEnabled[i].FindHandler(SCR_AddonLineDSConfigComponent));
			if (lineCmp)
				lineCmp.SetOnBottom(i == m_aEnabled.Count() - 1);
		}
		
		// Call next 
		items.Remove(0);
		if (!items.IsEmpty())
		{
			GetGame().GetCallqueue().CallLater(EnableItems, 0, false, enable, items);
		}
		else
		{
			// On finished fix list scrolls 
			float x,y; 
			m_wDisableScroll.GetSliderPos(x, y);
			
			if (y > 1)
				m_wDisableScroll.SetSliderPos(0, 1, true);
			
			m_wEnableScroll.GetSliderPos(x, y);
			
			if (y > 1)
				m_wEnableScroll.SetSliderPos(0, 1, true);
			
			// Check focus 
			if (m_FocusedLine)
				OnLineFocus(m_FocusedLine);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineButtonUp(SCR_AddonLineDSConfigComponent line)
	{
		// Check if not on top 
		int id = line.GetRootWidget().GetZOrder();
		
		if (id == -1 || id == 0)
			return;
		
		// Move up 
		m_aEnabled[id] = m_aEnabled[id-1];
		m_aEnabled[id].SetZOrder(id);
		
		m_aEnabled[id-1] = line.GetRootWidget(); 
		m_aEnabled[id-1].SetZOrder(id-1);
		
		// Set line is on bottom 
		if (id == m_aEnabled.Count() - 1)
		{
			SCR_AddonLineDSConfigComponent otherLine = SCR_AddonLineDSConfigComponent.Cast(m_aEnabled[id].FindHandler(SCR_AddonLineDSConfigComponent));
			if (otherLine)
				otherLine.SetOnBottom(true);
		}
	
		line.SetOnBottom(false);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineButtonDown(SCR_AddonLineDSConfigComponent line)
	{
		// Check if not on bottom 
		int id = line.GetRootWidget().GetZOrder();
		
		if (id == -1 || id >= m_aEnabled.Count() - 1)
			return;
		
		// Move down 
		m_aEnabled[id] = m_aEnabled[id+1];
		m_aEnabled[id].SetZOrder(id);
		
		m_aEnabled[id+1] = line.GetRootWidget(); 
		m_aEnabled[id+1].SetZOrder(id+1);
		
		// Set line is on bottom 
		SCR_AddonLineDSConfigComponent otherLine = SCR_AddonLineDSConfigComponent.Cast(m_aEnabled[id].FindHandler(SCR_AddonLineDSConfigComponent));
		if (otherLine)
			otherLine.SetOnBottom(false);

		if (id + 1 == m_aEnabled.Count() - 1)
			line.SetOnBottom(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEnableAllClicked()
	{
		array<ref SCR_WorkshopItem> enable = {};
		
		for (int i = 0, count = m_aEnabledMods.Count(); i < count; i++)
		{
			SCR_WorkshopItem item = m_aEnabledMods.GetKey(i);
			SCR_AddonLineDSConfigComponent line = m_aEnabledMods.GetElement(i);
			if (!line)
				continue;
			
			if (!line.HasItemAnyIssue())
				enable.Insert(item);
		}
		
		EnableItems(true, enable);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDisableAllClicked()
	{
		array<ref SCR_WorkshopItem> disable = {};
		
		for (int i = 0, count = m_aEnabledMods.Count(); i < count; i++)
		{
			disable.Insert(m_aEnabledMods.GetKey(i));
		}
		
		EnableItems(false, disable);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnOpenWorkshopButton()
	{		
		InvokeEventOnWorkshopButtonActivate();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNavEnableActivated()
	{
		if (!m_FocusedLine)
			return;
		
		SCR_WorkshopItem item = m_FocusedLine.GetWorkshopItem();
		if (!item)
			return;
		
		bool enabled = m_FocusedLine.GetWidgetEnabled();
		
		// Store last focused position
		int focusedId = LastFocusedId(m_FocusedLine.GetRootWidget(), !enabled);
		
		if (enabled)
			m_FocusedLine.OnDisableButton();
		else
			m_FocusedLine.OnEnableButton();
		
		// Switch focus 
		if (focusedId >= 0)
		{
			Widget nextLine = ClosestAddonLine(focusedId, !enabled);
			GetGame().GetWorkspace().SetFocusedWidget(nextLine);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNavEnableAllActivated()
	{
		if (!m_FocusedLine)
			return;
		
		bool enabled = m_FocusedLine.GetWidgetEnabled();
		
		if (enabled)
			OnDisableAllClicked();
		else
			OnEnableAllClicked();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNavSelectSortActivate()
	{
		// Release 
		if (m_OrderedLine)
		{
			m_OrderedLine = null;
			
			GetGame().GetInputManager().RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnDownSort);
			GetGame().GetInputManager().RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnUpSort);
			
			if (m_NavChangeSortOrder)
				m_NavChangeSortOrder.SetVisible(false, false);
			
			if (m_NavSelectSort)
			{
				// Start ordering
				m_NavSelectSort.SetLabel(SORT_BTN_SORT_LABEL);
				m_NavSelectSort.SetAction(SORT_BTN_SORT_ACTION);
			}
			
			if (m_NavEnable)
				m_NavEnable.SetVisible(true, false);
			
			if (m_NavEnableAll)
				m_NavEnableAll.SetVisible(true, false);
			
			// Refocus 
			OnLineFocus(m_FocusedLine);
			
			return;
		}
		
		if (!m_FocusedLine)
			return;
		
		// Select only enabled 
		if (!m_FocusedLine.GetWidgetEnabled())
			return;
		
		m_OrderedLine = m_FocusedLine;
		
		if (m_OrderedLine)
		{
			GetGame().GetInputManager().AddActionListener("MenuDown", EActionTrigger.DOWN, OnDownSort);
			GetGame().GetInputManager().AddActionListener("MenuUp", EActionTrigger.DOWN, OnUpSort);
			
			if (m_NavChangeSortOrder)
				m_NavChangeSortOrder.SetVisible(true);
			
			if (m_NavSelectSort)
			{
				// Place and stop ordering
				m_NavSelectSort.SetLabel(SORT_BTN_PLACE_LABEL);
				m_NavSelectSort.SetAction(SORT_BTN_PLACE_ACTION);
			}
			
			if (m_NavEnable)
				m_NavEnable.SetVisible(false, false);
		
			if (m_NavEnableAll)
				m_NavEnableAll.SetVisible(false, false);
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownSort()
	{
		if (!m_OrderedLine)
			return;
		
		OnLineButtonDown(m_OrderedLine);
		//GetGame().GetWorkspace().SetFocusedWidget(m_OrderedLine.GetRootWidget());
		GetGame().GetCallqueue().CallLater(FocusLine, 0, false, m_OrderedLine);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUpSort()
	{
		if (!m_OrderedLine)
			return;
		
		OnLineButtonUp(m_OrderedLine);
		//GetGame().GetWorkspace().SetFocusedWidget(m_OrderedLine.GetRootWidget());
		GetGame().GetCallqueue().CallLater(FocusLine, 0, false, m_OrderedLine);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FocusLine(SCR_AddonLineDSConfigComponent line)
	{
		GetGame().GetWorkspace().SetFocusedWidget(line.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Get all selected mods from addon entries
	array<ref DSMod> SelectedModsList()
	{
		array<ref DSMod> mods = {};
		
		for (int i = 0, count = m_aEnabled.Count(); i < count; i++)
		{
			SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(m_aEnabled[i].FindHandler(SCR_AddonLineDSConfigComponent));
			if (!line)
				continue;
			
			// Save all enabled
			if (line.GetWidgetEnabled())
			{
				SCR_WorkshopItem item = line.GetWorkshopItem();
				if (!item)
					continue;
				
				DSMod mod = new DSMod();
				
				mod.modId = item.GetId();
				mod.name = item.GetName();
				mod.version = item.GetCurrentLocalRevision().GetVersion();
				
				mods.Insert(mod);
			}
		}
		
		return mods;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_InputButtonComponent GetNavWorkshopButton()
	{
		return m_NavWorkshop;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_EntryDSMod
{
	ref Widget m_Root;
	ref SCR_WidgetListEntry m_Entry;
	ref DSMod m_Mod;
	
	void SCR_EntryDSMod(SCR_WidgetListEntry entry, DSMod mod)
	{
		m_Entry = entry;
		m_Mod = mod;
	}
};
