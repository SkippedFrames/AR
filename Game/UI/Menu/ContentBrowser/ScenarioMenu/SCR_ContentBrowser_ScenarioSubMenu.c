enum EScenarioSubMenuMode
{
	MODE_ALL,		// Show all scenarios
	MODE_FAVOURITE,	// Show only favourite scenarios
	MODE_RECENT		// Show only recently played scenarios
}

class SCR_ContentBrowser_ScenarioSubMenu : SCR_ContentBrowser_ScenarioSubMenuBase
{
	// This is quite universal and can work in many modes...
	[Attribute("0", UIWidgets.ComboBox, "Mode in which this submenu must work.", "", ParamEnumArray.FromEnum(EScenarioSubMenuMode))]
	EScenarioSubMenuMode m_eMode;

	// Constants
	protected const int RECENTLY_PLAYED_MAX_ENTRIES = 10; // How many recently played missions to show in the recently played tab

	// Message tags
	// Those which end with '2' should be used when no content is found due to filters.
	protected const string MESSAGE_TAG_NOTHING_FOUND =		"nothing_found";
	protected const string MESSAGE_TAG_NOTHING_FOUND_2 =	"nothing_found2";
	protected const string MESSAGE_TAG_NOTHING_FAVOURITE =	"nothing_favourite";
	protected const string MESSAGE_TAG_NOTHING_FAVOURITE_2 ="nothing_favourite2";
	protected const string MESSAGE_TAG_NOTHING_RECENT =		"nothing_recent";
	protected const string MESSAGE_TAG_NOTHING_RECENT_2 =	"nothing_recent2";
	
	// Other
	protected WorkshopApi m_WorkshopApi;
	protected ref SCR_ContentBrowser_ScenarioSubMenuWidgets m_Widgets = new SCR_ContentBrowser_ScenarioSubMenuWidgets;

	protected Widget m_wBeforeSort;
	
	protected int m_iEntriesTotal;
	protected int m_iEntriesCurrent;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop();

		InitWidgets();

		// Try to restore filters
		m_Widgets.m_FilterPanelComponent.TryLoad();

		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop();
		InitWorkshopApi();

		UpdateScenarioList(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		super.OnMenuUpdate(parentMenu, tDelta);

		//! Set current scenario in the info panel
		m_Widgets.m_ScenarioDetailsPanelComponent.SetScenario(GetSelectedScenario());
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		// Init workshop API
		// We do it on tab show becasue this tab and others persists when all other tabs are closed,
		// But we can switch back to it later, and we must setup the workshop api again
		InitWorkshopApi();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);

		// Save configuration of filters
		m_Widgets.m_FilterPanelComponent.Save();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		//Bring back focus to the last selected line after closing a pop-up dialog, in order to always have an element focused
		Widget target;
		
		if (m_LastSelectedLine)
			target = m_LastSelectedLine.GetRootWidget();
		else if (m_Widgets && m_Widgets.m_ScenarioList)
			target = m_Widgets.m_ScenarioList.GetChildren();
		
		if (target)
			GetGame().GetWorkspace().SetFocusedWidget(target);
		
		super.OnMenuFocusGained();
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateNavigationButtons(bool visible = true)
	{
		visible = GetSelectedLine() && GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;
		super.UpdateNavigationButtons(visible);
	}

	//------------------------------------------------------------------------------------------------
	override void Play(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		super.Play(scenario);
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
	}

	//------------------------------------------------------------------------------------------------
	override void Continue(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		super.Continue(scenario);
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
	}

	//------------------------------------------------------------------------------------------------
	override void OnRestartConfirmed()
	{
		super.OnRestartConfirmed();
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Called from scenario line component when scenario state changes
	override void OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent comp)
	{
		UpdateNavigationButtons();
	}

	// ---- PROTECTED ----
	//------------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		// We provide scenarioSubMenuRoot as root because the widgets of the layours were exported starting from scenarioSubMenuRoot
		m_Widgets.Init(m_wRoot.FindWidget("scenarioSubMenuRoot"));

		m_Widgets.m_FilterPanelComponent.GetEditBoxSearch().m_OnConfirm.Insert(OnSearchConfirm);
		//m_Widgets.m_FilterPanelComponent.GetOnFilterPanelToggled().Insert(OnFilterPanelToggled);

		m_Widgets.m_SortingHeaderComponent.m_OnChanged.Insert(OnSortingHeaderChange);
	}

	//------------------------------------------------------------------------------------------------
	// Inits workshop API according to current mode
	protected void InitWorkshopApi()
	{
		// Scan offline items if needed
		if (m_WorkshopApi.NeedScan())
			m_WorkshopApi.ScanOfflineItems();
	}

	//------------------------------------------------------------------------------------------------
	//! Requests missions from API and shows them in the list
	protected void UpdateScenarioList(bool setNewFocus)
	{
		// Get missions from Workshop API
		array<MissionWorkshopItem> missionItemsAll = {};
		m_WorkshopApi.GetPageScenarios(missionItemsAll, 0, 4096); // Get all missions at once

		// Remove scenarios from disabled addons
		for (int i = missionItemsAll.Count() - 1; i >= 0; i--)
		{
			MissionWorkshopItem m = missionItemsAll[i];
			WorkshopItem addon = m.GetOwner();

			if (!addon)
				continue;

			SCR_WorkshopItem scriptedItem = SCR_AddonManager.GetInstance().GetItem(addon.Id());

			if (scriptedItem && scriptedItem.GetAnyDependencyMissing())
				missionItemsAll.Remove(i);
		}

		// Store total number of entries
		m_iEntriesTotal = missionItemsAll.Count();
		
		// Filter items according to current tab mode
		array<MissionWorkshopItem> missionItemsTabFiltered = {};
		switch (m_eMode)
		{
			// Select only fav. missions
			case EScenarioSubMenuMode.MODE_FAVOURITE:
			{
				foreach (MissionWorkshopItem m : missionItemsAll)
				{
					if (m.IsFavorite())
						missionItemsTabFiltered.Insert(m);
				}

				// Bail if there are no favourite missions
				if (missionItemsTabFiltered.IsEmpty())
				{
					ScenarioList_ClearMissionEntries();
					
					m_iEntriesCurrent = 0;
					SetPanelsMode(true, MESSAGE_TAG_NOTHING_FAVOURITE);
					return;
				}

				break;
			}

			// Select only recently played missions
			case EScenarioSubMenuMode.MODE_RECENT:
			{
				// Sort missions by time since last play, select top latest N entries
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(missionItemsAll, false);

				int i = 0;
				while (missionItemsTabFiltered.Count() < RECENTLY_PLAYED_MAX_ENTRIES && i < missionItemsAll.Count())
				{
					int dt = missionItemsAll[i].GetTimeSinceLastPlay();
					if (dt != -1)
						missionItemsTabFiltered.Insert(missionItemsAll[i]);
					i++;
				}

				// Bail if no recent missions
				if (missionItemsTabFiltered.IsEmpty())
				{
					ScenarioList_ClearMissionEntries();
					
					m_iEntriesCurrent = 0;
					SetPanelsMode(true, MESSAGE_TAG_NOTHING_RECENT);
					return;
				}

				break;
			}

			// Select all missions
			default:
			{
				if (missionItemsAll.IsEmpty())
				{
					ScenarioList_ClearMissionEntries();
					
					m_iEntriesCurrent = 0;
					SetPanelsMode(true, MESSAGE_TAG_NOTHING_FOUND);
					return;
				}

				missionItemsTabFiltered = missionItemsAll;
				break;
			}
		}

		// Filter scenarios based on search string
		string searchStr = m_Widgets.m_FilterPanelComponent.GetEditBoxSearch().GetValue();
		array<MissionWorkshopItem> missionItemsSearched = SearchScenarios(missionItemsTabFiltered, searchStr);

		// Bail if based on search there is no content
		if (missionItemsSearched.IsEmpty())
		{
			ScenarioList_ClearMissionEntries();

			// The message depends on current tab
			string messageTag;
			switch (m_eMode)
			{
				case EScenarioSubMenuMode.MODE_FAVOURITE:
					messageTag = MESSAGE_TAG_NOTHING_FAVOURITE_2;
					break;

				case EScenarioSubMenuMode.MODE_RECENT:
					messageTag = MESSAGE_TAG_NOTHING_RECENT_2;
					break;

				default:
					messageTag = MESSAGE_TAG_NOTHING_FOUND_2;
					break;
			}
			
			m_iEntriesCurrent = 0;
			SetPanelsMode(false, messageTag);
			return;
		}

		// At this point we are sure that there are some missions found
		// Choose sorting based on selected filter
		array<MissionWorkshopItem> missionItems = missionItemsSearched;
		ESortOrder eSortOrder = m_Widgets.m_SortingHeaderComponent.GetSortOrder();
		string currentSortingItem = m_Widgets.m_SortingHeaderComponent.GetSortElementName();

		bool sortOrder = false;
		if (eSortOrder == ESortOrder.DESCENDING)
			sortOrder = true;

		// Apply sorting of headers if required
		if (currentSortingItem.IsEmpty())
		{
			currentSortingItem = "name";
			sortOrder = false;
		}
		switch (currentSortingItem)
		{
			case "name":
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(missionItems, sortOrder);
				break;
			case "player_count": // sort by player count
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionPlayerCount>.HeapSort(missionItems, sortOrder);
				break;
			case "favourite":
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionFavourite>.HeapSort(missionItems, sortOrder);
				break;
			case "last_played":
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(missionItems, sortOrder);
				break;
			case "source":
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionAddonName>.HeapSort(missionItems, sortOrder);
				break;
		}

		m_iEntriesCurrent = missionItemsSearched.Count();
		SetPanelsMode(false);
		ScenarioList_CreateMissionEntries(missionItems, setNewFocus);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool SearchStringLocalized(string str, string searchStrLower)
	{
		if (str.StartsWith("#"))
		{
			// Check translated lowercased string
			string strTranslated = WidgetManager.Translate(str);
			if (!strTranslated.IsEmpty())
			{
				string strTranslatedLower = strTranslated;
				strTranslatedLower.ToLower();

				if (strTranslatedLower.Contains(searchStrLower))
					return true;
			}
		}
		else
		{
			// Check lowercased string
			string strLower = str;
			strLower.ToLower();

			if (strLower.Contains(searchStrLower))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void ScenarioList_ClearMissionEntries()
	{
		// Delete previous entries in the list
		Widget child = m_Widgets.m_ScenarioList.GetChildren();
		while (child)
		{
			Widget nextChild = child.GetSibling();
			m_Widgets.m_ScenarioList.RemoveChild(child);
			child = nextChild;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates lines for missions in the scroll view
	protected void ScenarioList_CreateMissionEntries(array<MissionWorkshopItem> scenarios, bool setNewFocus)
	{
		ScenarioList_ClearMissionEntries();

		if (!CreateLines(scenarios, m_Widgets.m_ScenarioList))
			return;

		// Set focus on first line
		Widget firstLine = m_Widgets.m_ScenarioList.GetChildren();
		if (firstLine && setNewFocus)
		{
			// If this is called instantly, it doesn't set new focus when menu is first opened.
			// Probably because menu opening is handled in event handler of main menu's tile.
			GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, firstLine, false);
		}

		// Reset scroll position
		m_Widgets.m_ScenarioScroll.SetSliderPos(0, 0, true);
	}
	/*
	//------------------------------------------------------------------------------------------------
	protected void OnFilterButton()
	{
		// Toggle the state of the filter panel
		bool filterShown = m_Widgets.m_FilterPanelComponent.GetFilterListBoxShown();
		bool selectedAnyMission = GetSelectedScenario() != null;
		bool newFilterShown = !filterShown || (filterShown && selectedAnyMission);

		m_Widgets.m_FilterPanelComponent.ShowFilterListBox(newFilterShown);

		// Set focus on the button of the filter panel
		if (newFilterShown)
			GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_FilterPanelComponent.GetWidgets().m_FilterButton);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSortingButton()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget focused = workspace.GetFocusedWidget();
		ScriptedWidgetEventHandler sortComp;
		if (focused)
			sortComp = focused.FindHandler(SCR_SortElementComponent);


		if (sortComp)
		{
			// Focused on a sort element
			// Focus back to previous widget or on first line
			if (m_wBeforeSort)
				GetGame().GetWorkspace().SetFocusedWidget(m_wBeforeSort);
			else
			{
				Widget firstLine = m_Widgets.m_ScenarioList.GetChildren();
				if (firstLine)
					workspace.SetFocusedWidget(firstLine);
			}
		}
		else
		{
			// Not focused on a sort element
			// Focus on a sort button
			if (focused.FindHandler(SCR_ContentBrowser_ScenarioLineComponent))
				m_wBeforeSort = focused;	// Return to this widget only if it's a scenario line
			else
				m_wBeforeSort = null;
			m_Widgets.m_SortingHeaderComponent.SetFocus(1);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCopyIdButton()
	{
		SCR_ContentBrowser_ScenarioLineComponent line = GetSelectedLine();

		if (!line)
			return;

		MissionWorkshopItem scenario = line.GetScenario();

		if (scenario)
			System.ExportToClipboard(scenario.Id());
	}
	*/
	//------------------------------------------------------------------------------------------------
	//! Called when user confirms the value in the seacrh string
	protected void OnSearchConfirm(SCR_EditBoxComponent comp, string newValue)
	{
		UpdateScenarioList(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when sorting header changes
	protected void OnSortingHeaderChange(SCR_SortHeaderComponent sortHeader)
	{
		UpdateScenarioList(false);
	}
	/*
	//------------------------------------------------------------------------------------------------
	//! Called when filter panel is toggled (shown or hidden)
	protected void OnFilterPanelToggled(bool newState)
	{
		// When filter panel is disabled, we restore focus back to last focused line
		if (!newState && GetSelectedLine())
		{
			GetGame().GetWorkspace().SetFocusedWidget(GetSelectedLine().GetRootWidget());
		}
	}
	*/
	// ---- PUBLIC ----
	//------------------------------------------------------------------------------------------------
	void SetPanelsMode(bool showEmptyPanel, string messagePresetTag = string.Empty)
	{
		// Show either main+filter panel or empty panel
		m_Widgets.m_EmptyPanel.SetVisible(showEmptyPanel);
		m_Widgets.m_FilterPanel.SetVisible(!showEmptyPanel);
		m_Widgets.m_LeftPanel.SetVisible(!showEmptyPanel);

		// Show message or hide it
		m_Widgets.m_EmptyPanelMessage.SetVisible(!messagePresetTag.IsEmpty());
		m_Widgets.m_MainPanelMessage.SetVisible(!messagePresetTag.IsEmpty());

		// Set message based on tag
		if (!messagePresetTag.IsEmpty())
		{
			if (showEmptyPanel)
				m_Widgets.m_EmptyPanelMessageComponent.SetContentFromPreset(messagePresetTag);
			else
				m_Widgets.m_MainPanelMessageComponent.SetContentFromPreset(messagePresetTag);
		}
		
		// Items Found Message
		m_Widgets.m_FilterPanelComponent.SetItemsFoundMessage(m_iEntriesCurrent, m_iEntriesTotal, m_iEntriesCurrent != m_iEntriesTotal);
	}

	//------------------------------------------------------------------------------------------------
	array<MissionWorkshopItem> SearchScenarios(array<MissionWorkshopItem> scenarios, string searchStr)
	{
		array<MissionWorkshopItem> scenariosOut = {};

		if (!searchStr.IsEmpty())
		{
			string searchStrLower = searchStr;
			searchStrLower.ToLower();
			foreach (MissionWorkshopItem scenario : scenarios)
			{
				// Name
				if (SearchStringLocalized(scenario.Name(), searchStrLower))
				{
					scenariosOut.Insert(scenario);
					continue;
				}

				// Description
				if (SearchStringLocalized(scenario.Description(), searchStrLower))
				{
					scenariosOut.Insert(scenario);
					continue;
				}
			}
		}
		else
		{
			scenariosOut.Copy(scenarios);
		}

		return scenariosOut;
	}

	//------------------------------------------------------------------------------------------------
	static bool GetHostingAllowed()
	{
		#ifdef HOST_SCENARIO_ENABLED
		return true;
		#else
		return false;
		#endif
	}
}


// Classes to compare mission headers
// Sort by name
class SCR_CompareMissionName : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		string name1 = ResolveName(left.Name());
		string name2 = ResolveName(right.Name());

		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
	}

	static string ResolveName(string name)
	{
		if (name.StartsWith("#"))
			return WidgetManager.Translate(name);
		else
			return name;
	}
}

// Sort by name
class SCR_CompareMissionAddonName : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		WorkshopItem ownerItem = left.GetOwner();


		string name1 = ResolveAddonName(left);
		string name2 = ResolveAddonName(right);

		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
	}

	static string ResolveAddonName(MissionWorkshopItem scenario)
	{

		WorkshopItem sourceAddon = scenario.GetOwner();
		if (!sourceAddon)
			return WidgetManager.Translate("#AR-Editor_Attribute_OverlayLogo_Reforger");
		else
			return sourceAddon.Name();
	}
}

// Sort by player count
class SCR_CompareMissionPlayerCount : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		return left.GetPlayerCount() < right.GetPlayerCount();
	}
}

// Sort by favouriteness
class SCR_CompareMissionFavourite : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		return right.IsFavorite();
	}
}

// Sort by time since last played
class SCR_CompareMissionTimeSinceLastPlay : SCR_SortCompare<MissionWorkshopItem>
{
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		int timeLeft = left.GetTimeSinceLastPlay();
		if (timeLeft < 0)			// Negative value means we have never played this.
			timeLeft =	0x7FFFFFFF;	// We set it to max positive int for sorting purpuse.
		int timeRight = right.GetTimeSinceLastPlay();
		if (timeRight < 0)
			timeRight =	0x7FFFFFFF;

		return timeLeft < timeRight;
	}
}
