/*
Submenu for local or online addons.

!! If you want filters to work, provide filters through the layout prefab or by some other way - into m_FilterCategories variable.
*/

enum EContentBrowserAddonsSubMenuMode
{
	MODE_OFFLINE,				// Gets items from Workshop offline API
	MODE_ONLINE,				// Gets items from Workshop online API
	MODE_EXTERNAL_ITEM_ARRAY	// Operates on fixed array of items supplied externally
}

class SCR_ContentBrowser_AddonsSubMenu : SCR_SubMenuBase
{
	// This is quite universal and can work in many modes...
	[Attribute("0", UIWidgets.ComboBox, "This menu is quite universal and can work in many modes - what do you want it to be?", "", ParamEnumArray.FromEnum(EContentBrowserAddonsSubMenuMode))]
	EContentBrowserAddonsSubMenuMode m_eMode;

	[Attribute("true", UIWidgets.CheckBox, "Enables the sorting header")]
	bool m_bEnableSorting;

	[Attribute("true", UIWidgets.CheckBox, "Enabled the filter panel functionality")]
	bool m_bEnableFilter;

	[Attribute("true", UIWidgets.CheckBox, "Enables opening details menu of currently selected tile.")]
	bool m_bEnableOpenDetails;

	// ---- Constants ----

	protected const int GRID_N_COLUMNS = 5;
	protected const int GRID_N_ROWS = 4;
	protected const int LOAD_PAGE_DELAY = 500;

	protected const ResourceName LAYOUT_GRID_TILE = "{9A08C4C52FACC45B}UI/layouts/Menus/ContentBrowser/Tile/ContentBrowserTile_5x4.layout";

	// Message tags
	protected const string MESSAGE_TAG_NOTHING_FOUND = "nothing_found";
	protected const string MESSAGE_TAG_NOTHING_DOWNLOADED = "nothing_downloaded";
	protected const string MESSAGE_TAG_NOTHING_DOWNLOADED_2 = "nothing_downloaded2";
	protected const string MESSAGE_TAG_ERROR_CONNECTION = "error_connection";
	protected const string MESSAGE_TAG_LOADING = "loading";

	// ---- Protected / Private ----

	protected ref SCR_ContentBrowser_AddonsSubMenuBaseWidgets m_Widgets = new SCR_ContentBrowser_AddonsSubMenuBaseWidgets;

	protected WorkshopApi m_WorkshopApi;
	
	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	protected ref array<SCR_ContentBrowserTileComponent> m_aTileComponents = {};

	protected bool m_bConnected; // True when we are connected to backend, used to determine when to start sending requests
	protected bool m_bFailToLoad;

	protected bool m_bClearCacheAtNextRequest; // Will force workshop to clear page cache. Should be set wherever sorting or filtering changes.

	protected ref array<ref SCR_WorkshopItem> m_aExternalItems = {};	// Array with externally provided items - used only in MODE_EXTERNAL_ITEM_ARRAY

	protected SCR_ContentBrowserTileComponent m_LastFocusedTile; // Mainly used to restore focus back to something

	protected ref SCR_ContentBrowser_GetAssetListParams m_GetAssetListParams;
	protected ref SCR_ContentBrowser_GetAssetListParams m_GetAssetListParamsDefault;

	protected bool m_bFirstPage = true; // True before first page is received.

	protected bool m_bPanelsModeEmpty; // True when we are showing empty panels

	protected bool m_bMousePageUsed;
	
	protected bool m_bFirstLoad = true;

	// State of paging
	protected int m_iCurrentPage;
	protected int m_iOfflinePageCount;	// Amount of pages, used only in MODE_OFFLINE and MODE_EXTERNAL_ITEM_ARRAY
	protected int m_iOnlinePageCount;
	
	// Navigation buttons
	protected SCR_InputButtonComponent m_NavPrimary;
	protected SCR_InputButtonComponent m_NavEnable;
	protected SCR_InputButtonComponent m_NavFavourite;
	protected SCR_InputButtonComponent m_NavFilter;

	protected ref ScriptInvokerVoid m_OnRequestOpenOfflinePage;

	// Items found message
	protected int m_iItemsTotal;
	protected int m_iOnlineFilteredItems;
	protected ref array<ref SCR_WorkshopApiCallback_RequestPage> m_aSearchCallbacks = {};

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnRequestOpenOfflinePage()
	{
		if (!m_OnRequestOpenOfflinePage)
			m_OnRequestOpenOfflinePage = new ScriptInvokerVoid();

		return m_OnRequestOpenOfflinePage;
	}

	//------------------------------------------------------------------------------------------------
	protected void RequestOpenOfflinePage()
	{
		if (!m_OnRequestOpenOfflinePage)
			m_OnRequestOpenOfflinePage = new ScriptInvokerVoid();

		m_OnRequestOpenOfflinePage.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the item array on which this tab operates. Only possible in external item array mode.
	void SetWorkshopItems(array<ref SCR_WorkshopItem> items)
	{
		if (m_eMode != EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY)
			return;

		m_aExternalItems.Clear();

		foreach (SCR_WorkshopItem i : items)
		{
			m_aExternalItems.Insert(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets filters to mode which lists only 'enabled' addons. Only works for offline tab.
	void SetFilterConfiguration_EnabledAddons()
	{
		if (m_eMode != EContentBrowserAddonsSubMenuMode.MODE_OFFLINE)
			return;

		SCR_FilterPanelComponent fp = m_Widgets.m_FilterPanelComponent;
		fp.ResetToDefaultValues();
		SCR_FilterSet filterSet = fp.GetFilter();
		SCR_FilterEntry filterEntry = filterSet.FindFilterCategory("state").FindFilter("enabled");
		filterEntry.SetSelected(true);
		fp.SelectFilter(filterEntry, true, invokeOnChanged: false);
		RequestPage(0); // Update the page again
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		InitWidgets();

		// Init main parts of the layout
		GridScreen_Init();

		// Try to load filters
		m_Widgets.m_FilterPanelComponent.TryLoad();

		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop();
		InitWorkshopApi();

		// Init event handlers
		InitWidgetEventHandlers();

		// In online mode request page only when menu is opened first time
		if (m_eMode == EContentBrowserAddonsSubMenuMode.MODE_ONLINE)
		{
			m_bConnected = SCR_WorkshopUiCommon.GetConnectionState();
			if (m_bConnected)
				OnConnected();
			else
				SetPanelsMode(true, false, MESSAGE_TAG_ERROR_CONNECTION, false);
		}
		
		else
		{
			// Offline modes - request page instantly
			RequestPage(0);
		}

		// Subscribe to addon manager events
		SCR_AddonManager.GetInstance().m_OnAddonOfflineStateChanged.Insert(Callback_OnAddonOfflineStateChanged);

		// Subscribe to page refresh request events
		SCR_AddonManager.GetInstance().m_OnAddonReportedStateChanged.Insert(Callback_OnAddonReportedStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		m_Widgets.Init(GetRootWidget());

		// Hide elements which are disabled through attributes
		m_Widgets.m_SortingHeader.SetVisible(m_bEnableSorting);

		// Create nav buttons
		if (m_bEnableFilter)
			m_NavFilter = CreateNavigationButton("MenuFilter", "#AR-Editor_ContentBrowser_ButtonOpenFilter", false);
		m_NavPrimary = CreateNavigationButton("WorkshopPrimary", "#AR-Workshop_ButtonDownload", true);
		m_NavEnable = CreateNavigationButton("MenuEnable", "#AR-Workshop_ButtonEnable", true);
		m_NavFavourite = CreateNavigationButton("MenuFavourite", UIConstants.FAVORITE_LABEL_ADD, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitWidgetEventHandlers()
	{
		m_Widgets.m_PrevPageButton_NoScrollComponent.m_OnActivated.Insert(OnPrevPageButton);
		m_Widgets.m_NextPageButton_NoScrollComponent.m_OnActivated.Insert(OnNextPageButton);
		GetGame().GetInputManager().AddActionListener("MouseWheel", EActionTrigger.VALUE, OnScrollPage);

		m_Widgets.m_FilterPanelComponent.GetWidgets().m_FilterSearchComponent.m_OnConfirm.Insert(OnFilterSearchConfirm);
		m_Widgets.m_SortingHeaderComponent.m_OnChanged.Insert(OnSortingHeaderChange);
		m_Widgets.m_FilterPanelComponent.GetOnFilterChanged().Insert(OnFilterChange);

		// Event handlers of nav buttons
		m_NavEnable.m_OnActivated.Insert(OnEnableButton);
		if (m_NavFilter)
			m_NavFilter.m_OnActivated.Insert(OnFilterButton);
		m_NavPrimary.m_OnActivated.Insert(OnPrimaryButton);
		m_NavFavourite.m_OnActivated.Insert(OnFavouriteButton);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		// Check and show backend down
		super.OnMenuShow(parentMenu);

		// Init workshop API
		// We do it on tab show becasue this tab and others persists when all other tabs are closed,
		// But we can switch back to it later, and we must setup the workshop api again
		InitWorkshopApi();
		
		if (m_bFailToLoad)
			RequestPage(m_iCurrentPage);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);

		// Save configuration of filters
		m_Widgets.m_FilterPanelComponent.Save();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuClose(parentMenu);

		// Unsubscribe from addon manager events
		SCR_AddonManager.GetInstance().m_OnAddonOfflineStateChanged.Remove(Callback_OnAddonOfflineStateChanged);

		// Unsubscribe from page refresh request events
		SCR_AddonManager.GetInstance().m_OnAddonReportedStateChanged.Remove(Callback_OnAddonReportedStateChanged);
		
		GetGame().GetCallqueue().Remove(RequestOnlinePageUnfiltered);
	}

	//------------------------------------------------------------------------------------------------
	//! Called on each frame
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		// Update connection state, check if we have reconnected
		bool connectedOld = m_bConnected;
		bool connectedNew = SCR_WorkshopUiCommon.GetConnectionState();
		m_bConnected = connectedNew;
		if (connectedNew != connectedOld)
		{
			if (connectedNew)
				ContentBrowserUI._print("Connected");
			else
				ContentBrowserUI._print("Disconnected");

			if (connectedNew && m_eMode == EContentBrowserAddonsSubMenuMode.MODE_ONLINE) // Only call OnConnected if it's an online tab
				OnConnected();
		}

		UpdatePagingWidgets();
		UpdateButtons();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		if (m_bShown)
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST);

		super.OnMenuFocusGained();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabChange(SCR_SuperMenuBase parentMenu)
	{
		super.OnTabChange(parentMenu);

		if (GetShown())
		{
			if (!m_bConnected && m_eMode == EContentBrowserAddonsSubMenuMode.MODE_ONLINE)
				SwitchFocus(SCR_EListMenuWidgetFocus.NULL);
			else
				SwitchFocus(SCR_EListMenuWidgetFocus.LIST);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Inits workshop API according to current mode
	protected void InitWorkshopApi()
	{
		// Set grid thumbnail size
		float imageWidth = SCR_WorkshopUiCommon.GetPreferedTileImageWidth();
		WorkshopItem.SetThumbnailGridScale(imageWidth);

		switch (m_eMode)
		{
			case EContentBrowserAddonsSubMenuMode.MODE_ONLINE:
				m_bClearCacheAtNextRequest = true;
				break;

			case EContentBrowserAddonsSubMenuMode.MODE_OFFLINE:
				m_WorkshopApi.ScanOfflineItems();
				break;

			case EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY:
				// Don't do anything, we are not using the Workshop API to get items
				// In this case
				break;
		}

		// Register tags for filters
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.m_FilterSet;
		if (filterSet)
		{
			foreach (SCR_FilterCategory category : filterSet.GetFilterCategories())
			{
				foreach (SCR_FilterEntry filterEntry : category.GetFilters())
				{
					SCR_ContentBrowserFilterTag filterTag = SCR_ContentBrowserFilterTag.Cast(filterEntry);
					if (filterTag)
						filterTag.RegisterInWorkshopApi(m_WorkshopApi);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Requests page from backend or displays items according to current mode
	protected void RequestPage(int pageId)
	{
		// When we request page, we also must clean the grid and side panel
		GridScreen_Clear();
		m_Widgets.m_AddonDetailsPanelComponent.SetWorkshopItem(null);
		SetPanelsMode(showEmptyPanel: m_bFirstPage, messagePresetTag: MESSAGE_TAG_LOADING);

		m_iCurrentPage = pageId;
		m_bFailToLoad = false;

		switch (m_eMode)
		{
			case EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY:
				DisplayExternalItems(pageId);
				break;
			case EContentBrowserAddonsSubMenuMode.MODE_ONLINE:
				GetGame().GetCallqueue().Remove(RequestOnlinePage);
				GetGame().GetCallqueue().CallLater(RequestOnlinePage, LOAD_PAGE_DELAY, false, pageId);
				break;
			case EContentBrowserAddonsSubMenuMode.MODE_OFFLINE:
				DisplayOfflineItems(pageId);
				break;
		}
	}

	// ---- Backend page requests ----
	//------------------------------------------------------------------------------------------------
	//! Total number of items
	//! Requests an unfiltered page from backend, does not do anything else
	protected void RequestOnlinePageUnfiltered(int pageId)
	{
		GetGame().GetCallqueue().Remove(RequestOnlinePageUnfiltered);
		
		m_WorkshopApi.SetPageItems(0);
		
		ref SCR_WorkshopApiCallback_RequestPage callback = new SCR_WorkshopApiCallback_RequestPage(pageId);
		callback.m_OnGetAssets.Insert(Callback_OnRequestPageGetAllAssets);
		callback.m_OnError.Insert(Callback_OnRequestPageGetAllAssets);
		callback.m_OnTimeout.Insert(Callback_OnRequestPageGetAllAssets);

		m_aSearchCallbacks.Insert(callback);
		
		if (!m_GetAssetListParamsDefault)
			m_GetAssetListParamsDefault = new SCR_ContentBrowser_GetAssetListParams(this, true);

		m_GetAssetListParamsDefault.limit = GRID_N_ROWS * GRID_N_COLUMNS;
		m_GetAssetListParamsDefault.offset = pageId * GRID_N_ROWS * GRID_N_COLUMNS;

		m_WorkshopApi.RequestPage(callback, m_GetAssetListParamsDefault, m_bClearCacheAtNextRequest);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Items with filters
	//! Requests a filtered page from backend, does not do anything else
	protected void RequestOnlinePage(int pageId)
	{	
		m_WorkshopApi.SetPageItems(GRID_N_COLUMNS * GRID_N_ROWS);
		
		ref SCR_WorkshopApiCallback_RequestPage callback = new SCR_WorkshopApiCallback_RequestPage(pageId);
		callback.m_OnGetAssets.Insert(Callback_OnRequestPageGetAssets);
		callback.m_OnError.Insert(Callback_OnRequestPageTimeout);
		callback.m_OnTimeout.Insert(Callback_OnRequestPageTimeout);
		
		m_aSearchCallbacks.Insert(callback);
		
		if (!m_GetAssetListParams)
			m_GetAssetListParams = new SCR_ContentBrowser_GetAssetListParams(this);

		m_GetAssetListParams.limit = GRID_N_ROWS * GRID_N_COLUMNS;
		m_GetAssetListParams.offset = pageId * GRID_N_ROWS * GRID_N_COLUMNS;

		//m_GetAssetListParams.PackToFile("$profile:GetAssetList.json");
		#ifdef WORKSHOP_DEBUG
		ContentBrowserUI._print(string.Format("WorkshopApi.RequestPage() limit: %1, offset: %2", m_GetAssetListParams.limit, m_GetAssetListParams.offset));
		#endif

		m_WorkshopApi.RequestPage(callback, m_GetAssetListParams, m_bClearCacheAtNextRequest);
		
		m_bClearCacheAtNextRequest = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void PageRequestCallbackCleanup(SCR_WorkshopApiCallback_RequestPage callback)
	{
		callback.m_OnSuccess.Clear();
		callback.m_OnError.Clear();
		callback.m_OnTimeout.Clear();
		
		m_aSearchCallbacks.RemoveItem(callback);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called from OnPack of a Json API Struct.
	//! Here we pack the request data according to current filtering and sorging parameters.
	void PackGetAssetListParams(SCR_ContentBrowser_GetAssetListParams p, bool defaultValues = false)
	{
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.GetFilter();
		
		SCR_FilterCategory otherCategory = filterSet.FindFilterCategory("other");
		
		// Pack sorting
		string orderBy = "name";
		string currentSortingItem = m_Widgets.m_SortingHeaderComponent.GetSortElementName();
		bool sortAscending = m_Widgets.m_SortingHeaderComponent.GetSortOrderAscending();

		const string orderAsc = "asc";
		const string orderDesc = "desc";

		switch (currentSortingItem)
		{
			case "popularity":
				orderBy = "popularity";
				break;
			case "rating":
				orderBy = "averageRating";
				break;
			case "recently_added":
				orderBy = "createdAt";
				sortAscending = !sortAscending; // Invert sorting direction, when ascen
				break;
			case "name":
				orderBy = "name";
				break;
			case "subscribers":
				orderBy = "subscriberCount";
				break;
		}

		string orderDirection = orderAsc;
		if (!sortAscending)
			orderDirection = orderDesc;

		p.StoreString("orderBy", orderBy);					// <- "orderBy": "..."
		p.StoreString("orderDirection", orderDirection);	// <- "orderDirection": "..."

		// Pack search string
		string searchStr = m_Widgets.m_FilterPanelComponent.GetWidgets().m_FilterSearchComponent.GetValue();
		if (!searchStr.IsEmpty() && !defaultValues)
		{
			p.StoreString("search", searchStr);				// <- "search": "..."
		}

		// Pack 'quick filters'
		SCR_FilterCategory quickCategory = filterSet.FindFilterCategory("quick_filters");
		SCR_FilterEntry filterAll = quickCategory.FindFilter("all");
		SCR_FilterEntry filterFav = quickCategory.FindFilter("favourite");
		//SCR_FilterEntry filterSub = quickCategory.FindFilter("subscribed");
		SCR_FilterEntry filterRated = quickCategory.FindFilter("rated");

		if (filterFav.GetSelected(defaultValues))
			p.StoreBoolean("favorited", true);				// <- "favorited": true
		//else if (filterSub.GetSelected())
		//	p.StoreBoolean("subscribed", true);				// <- "subscribed": true
		else if (filterRated.GetSelected(defaultValues))
		{
			p.StartObject("averageRating");					// <- "AverageRating" object
			p.StoreFloat("gt", 0.75);						// <- "gt": 1.23
			p.StoreFloat("lt", 1);							// <- "lt": 1.23
			p.EndObject();									// <- End AverageRating
		}

		// -------- Pack tags --------

		// Make arrays with tags to include or exclude
		array<ref array<string>> tagArraysInclude = {};
		array<string> tagsExclude = {};

		// Tags from type filters
		SCR_FilterCategory typeCategory = filterSet.FindFilterCategory("type");
		if (typeCategory.GetAnySelected() && !defaultValues)
		{
			array<string> typeTags = {};
			foreach (SCR_FilterEntry filter : typeCategory.GetFilters())
			{
				if (filter.GetSelected(defaultValues))
					typeTags.Insert(filter.m_sInternalName);
			}
			tagArraysInclude.Insert(typeTags);
		}

		// Tags from test mods
		SCR_FilterEntry testFilter = otherCategory.FindFilter("TESTDEV");
		if (!testFilter.GetSelected(defaultValues))
			tagsExclude.Insert(testFilter.m_sInternalName);

		// Pack the tags
		if (!tagArraysInclude.IsEmpty() || !tagsExclude.IsEmpty())
		{
			p.StartObject("tags");							// <- Start "tags" object

			if (!tagArraysInclude.IsEmpty())
			{
				p.StartArray("include");					// <- Start "include"  array

				foreach (array<string> tagArray : tagArraysInclude)
				{
					p.ItemArray();							// <- Start tags array
					foreach (string tag : tagArray)
					{
						p.ItemString(tag);					// <- Array element
					}
					p.EndArray();							// <- End tags array
				}

				p.EndArray();								// <- End "include" array
			}

			if (!tagsExclude.IsEmpty())
			{
				p.StartArray("exclude");					// <- Start "exclude" array
				foreach (string tag : tagsExclude)
				{
					p.ItemString(tag);						// <- Array element
				}
				p.EndArray();								// <- End "exclude" array
			}

			p.EndObject();									// <- End "tags" object
		}

		// --------------------------------

		// Pack reported
		SCR_FilterEntry reportedFilter = otherCategory.FindFilter("reported");
		p.StoreBoolean("reported", reportedFilter.GetSelected(defaultValues));		// <- "reported": true
		p.StoreBoolean("hidden", reportedFilter.GetSelected(defaultValues));

		// Pack test mods
	}

	//------------------------------------------------------------------------------------------------
	//! Selects items from the externally provided array and shows them on grid, only makes sense in external array mode
	protected void DisplayExternalItems(int pageId)
	{
		m_WorkshopApi.SetPageItems(GRID_N_COLUMNS * GRID_N_ROWS);
		
		array<ref SCR_WorkshopItem> itemsAtPage = SelectItemsAtPage(m_aExternalItems, pageId);

		// Filtering is disabled in this mode

		// Update page count
		m_iOfflinePageCount = Math.Ceil((float)itemsAtPage.Count() / (float)(GRID_N_ROWS * GRID_N_COLUMNS));

		SetPanelsMode(false, m_Widgets.m_FilterPanelComponent.AnyFilterButtonsVisible(), string.Empty, m_bFirstPage);

		GridScreen_DisplayItems(itemsAtPage);

		m_bFirstPage = false;
		
		UpdateItemsFoundMessage(m_aExternalItems.Count(), m_aExternalItems.Count());
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayOfflineItems(int pageId)
	{
		m_WorkshopApi.SetPageItems(GRID_N_COLUMNS * GRID_N_ROWS);
		
		// Get offline items from API
		array<WorkshopItem> rawWorkshopItems = {};
		m_WorkshopApi.GetOfflineItems(rawWorkshopItems);

		// Bail if there is no downloaded content
		if (rawWorkshopItems.IsEmpty())
		{
			SetPanelsMode(showEmptyPanel: true, messagePresetTag: MESSAGE_TAG_NOTHING_DOWNLOADED);
			UpdateItemsFoundMessage(0, 0);
			return;
		}

		// Register items in Addon Manager
		array<ref SCR_WorkshopItem> itemsUnfiltered = {};
		foreach (WorkshopItem i : rawWorkshopItems)
		{
			SCR_WorkshopItem itemRegistered = SCR_AddonManager.GetInstance().Register(i);
			itemsUnfiltered.Insert(itemRegistered);
		}

		// Filter items
		array<ref SCR_WorkshopItem> itemsFiltered = Filter_OfflineFilterAndSearch(itemsUnfiltered);

		// Bail if no items after filtering
		if (itemsFiltered.IsEmpty())
		{
			SetPanelsMode(showEmptyPanel: false, forceFiltersList: true, messagePresetTag: MESSAGE_TAG_NOTHING_DOWNLOADED_2);
			UpdateItemsFoundMessage(0, rawWorkshopItems.Count());
			return;
		}

		// Sort items
		array<SCR_WorkshopItem> itemsSortedWeakPtrs = {};
		foreach (SCR_WorkshopItem i : itemsFiltered)
		{
			itemsSortedWeakPtrs.Insert(i);
		}

		bool sortOrder = !m_Widgets.m_SortingHeaderComponent.GetSortOrderAscending();
		string currentSortingItem = m_Widgets.m_SortingHeaderComponent.GetSortElementName();
		if (currentSortingItem.IsEmpty())
		{
			currentSortingItem = "name";
			sortOrder = false;
		}
		
		switch (currentSortingItem)
		{
			case "name":
				SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemName>.HeapSort(itemsSortedWeakPtrs, sortOrder);
				break;
			case "time_since_played":
				SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemTimeSinceLastPlay>.HeapSort(itemsSortedWeakPtrs, sortOrder);
				break;
			case "time_since_downloaded":
				SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemTimeSinceFirstDownload>.HeapSort(itemsSortedWeakPtrs, sortOrder);
				break;
		}

		array<ref SCR_WorkshopItem> itemsSorted = {};
		foreach (SCR_WorkshopItem i : itemsSortedWeakPtrs)
		{
			itemsSorted.Insert(i);
		}

		// Update page count
		m_iOfflinePageCount = Math.Ceil((float)itemsSorted.Count() / (float)(GRID_N_ROWS * GRID_N_COLUMNS));

		// Display items
		SetPanelsMode(false, m_Widgets.m_FilterPanelComponent.AnyFilterButtonsVisible(), string.Empty, m_bFirstPage);
		array<ref SCR_WorkshopItem> itemsAtPage = SelectItemsAtPage(itemsSorted, pageId);
		GridScreen_DisplayItems(itemsAtPage);

		m_bFirstPage = false;
		
		UpdateItemsFoundMessage(itemsSorted.Count(), rawWorkshopItems.Count());
	}


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// W I D G E T S
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//------------------------------------------------------------------------------------------------
	//! When pages are scrolled, tiles are not deleted, we show different data in the tiles instead
	//! to avoid unneeded widget allocations.
	//! Here we create tile widgets in advance.
	protected void GridScreen_Init()
	{
		for (int i = 0; i < GRID_N_COLUMNS * GRID_N_ROWS; i++)
		{
			int colId = i % GRID_N_COLUMNS;
			int rowId = i / GRID_N_COLUMNS;

			// Create tile widgets
			Widget w = GetGame().GetWorkspace().CreateWidgets(LAYOUT_GRID_TILE, m_Widgets.m_Grid);

			// Set grid positioning
			GridSlot.SetRow(w, rowId);
			GridSlot.SetColumn(w, colId);

			// Init the tile component
			SCR_ContentBrowserTileComponent comp = SCR_ContentBrowserTileComponent.Cast(w.FindHandler(SCR_ContentBrowserTileComponent));
			comp.SetWorkshopItem(null); // Widget will be hidden because of that
			comp.SetRatingVisible(m_eMode == EContentBrowserAddonsSubMenuMode.MODE_ONLINE); // Rating is only visible in online mode

			// Tile event handlers
			comp.m_OnClick.Insert(OnTileClick);
			comp.m_OnFocus.Insert(OnTileFocus);

			m_aTileComponents.Insert(comp);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Tiles are cleared up here. They are not deleted, but hidden.
	protected void GridScreen_Clear()
	{
		foreach (SCR_ContentBrowserTileComponent comp : m_aTileComponents)
		{
			comp.SetWorkshopItem(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GridScreen_DisplayItems(array<ref SCR_WorkshopItem> items)
	{
		// Clear old items
		GridScreen_Clear();

		int nItems = Math.ClampInt(items.Count(), 0, GRID_N_COLUMNS * GRID_N_ROWS);

		bool showReportedAddons = GetShowReportedAddons();

		for (int i = 0; i < nItems; i++)
		{
			SCR_ContentBrowserTileComponent comp = m_aTileComponents[i];
			comp.SetWorkshopItem(items[i], showReportedAddons);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetPanelsMode(bool showEmptyPanel, bool forceFiltersList = false, string messagePresetTag = string.Empty, bool animateFiltersList = false)
	{
		m_bPanelsModeEmpty = showEmptyPanel;

		// Show either main+filter panel or empty panel
		m_Widgets.m_EmptyPanel.SetVisible(showEmptyPanel);
		m_Widgets.m_FilterPanel.SetVisible(!showEmptyPanel);
		m_Widgets.m_LeftPanelArea.SetVisible(!showEmptyPanel);

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

		// Force-show filter list
		if (forceFiltersList)
		{
			m_Widgets.m_FilterPanelComponent.ShowFilterListBox(true, animateFiltersList);
			if (!m_Widgets.m_FilterPanelComponent.GetFocused())
				SwitchFocus(SCR_EListMenuWidgetFocus.FILTERING);
		}
	}

	//------------------------------------------------------------------------------------------------
	//					F I L T E R    A N D   S O R T I N G
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> SearchItems(array<ref SCR_WorkshopItem> items, string searchStr)
	{
		array<ref SCR_WorkshopItem> itemsOut = {};

		if (searchStr.IsEmpty())
		{
			foreach (SCR_WorkshopItem i : items)
			{
				itemsOut.Insert(i);
			}

			return itemsOut;
		}

		string searchStrLower = searchStr;
		searchStrLower.ToLower();

		foreach (SCR_WorkshopItem i : items)
		{
			// Name
			string nameLower = i.GetName();
			nameLower.ToLower();
			if (nameLower.Contains(searchStrLower))
			{
				itemsOut.Insert(i);
				continue;
			}

			// Author name
			string authorNameLower = i.GetName();
			authorNameLower.ToLower();
			if (authorNameLower.Contains(searchStrLower))
			{
				itemsOut.Insert(i);
				continue;
			}

			// Description
			string descriptionLower = i.GetDescription();
			descriptionLower.ToLower();
			if (descriptionLower.Contains(searchStrLower))
			{
				itemsOut.Insert(i);
				continue;
			}

			// Tags
			array<WorkshopTag> tags = {};
			i.GetWorkshopItem().GetTags(tags);
			bool foundTag = false;
			foreach (WorkshopTag tag : tags)
			{
				string tagNameLower = tag.Name();
				tagNameLower.ToLower();
				if (tagNameLower == searchStrLower)
				{
					foundTag = true;
					break;
				}
			}

			if (foundTag)
			{
				itemsOut.Insert(i);
				continue;
			}
		}

		return itemsOut;
	}

	//------------------------------------------------------------------------------------------------
	//! Filters the array according to currently selected filters.
	//! !! Works only for offline configuration of filters!
	protected array<ref SCR_WorkshopItem> Filter_OfflineFilterAndSearch(array<ref SCR_WorkshopItem> itemsIn)
	{
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.GetFilter();

		string searchStr = m_Widgets.m_FilterPanelComponent.GetEditBoxSearch().GetValue();

		array<ref SCR_WorkshopItem> items = SearchItems(itemsIn, searchStr);

		// !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !
		// !  !  Filter categories and filter names are bound by strings, be careful while changing them   !  !
		// !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !
		// Arrays produced by each category. In the end we find intersection of those arrays.
		array<ref array<ref SCR_WorkshopItem>> arraysFromCategories = {};

		// Quick filters - only one is selected at a time
		SCR_FilterCategory catQuick = filterSet.FindFilterCategory("quick_filters");
		SCR_FilterEntry filterAll = catQuick.FindFilter("all");
		SCR_FilterEntry filterFav = catQuick.FindFilter("favourite");
		if (filterAll.GetSelected())
			arraysFromCategories.Insert(items);
		else if (filterFav.GetSelected())
			arraysFromCategories.Insert(SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.FAVOURITE));

		// Type tags
		array<WorkshopTag> selectedWorkshopTags = {};
		SCR_FilterCategory catType = filterSet.FindFilterCategory("type");
		foreach (SCR_FilterEntry f : catType.GetFilters())
		{
			SCR_ContentBrowserFilterTag filterEntryTag = SCR_ContentBrowserFilterTag.Cast(f);
			if (filterEntryTag && filterEntryTag.GetSelected())
				selectedWorkshopTags.Insert(filterEntryTag.GetWorkshopTag());
		}
		// If any tag filter is selected, but not all of them (because it would be same as selecting none)
		if (!selectedWorkshopTags.IsEmpty() && selectedWorkshopTags.Count() != catType.GetFilters().Count())
		{
			array<ref SCR_WorkshopItem> itemsFilteredByTags = {};
			foreach (SCR_WorkshopItem item : items)
			{
				if (item.GetWorkshopItem().HasAnyTag(selectedWorkshopTags))
					itemsFilteredByTags.Insert(item);
			}
			arraysFromCategories.Insert(itemsFilteredByTags);
		}

		// State
		SCR_FilterCategory catState = filterSet.FindFilterCategory("state");
		if (!catState)
			return {};

		if (catState.GetAllSelected() || catState.GetAllDeselected())
			arraysFromCategories.Insert(items);
		else
		{
			EWorkshopItemQuery query = 0;
			if (catState.FindFilter("enabled").GetSelected())
				query = query | EWorkshopItemQuery.ENABLED;
			if (catState.FindFilter("disabled").GetSelected())
				query = query | EWorkshopItemQuery.NOT_ENABLED;

			auto queryResult = SCR_AddonManager.SelectItemsOr(items, query);
			arraysFromCategories.Insert(queryResult);
		}

		// Condition
		SCR_FilterCategory catCondition = filterSet.FindFilterCategory("condition");
		if (catCondition.GetAllSelected() || catCondition.GetAllDeselected())
			arraysFromCategories.Insert(items);
		else
		{
			EWorkshopItemQuery query = 0;
			if (catCondition.FindFilter("ready").GetSelected())
				query = query | EWorkshopItemQuery.NO_PROBLEMS;
			if (catCondition.FindFilter("update_available").GetSelected())
				query = query | EWorkshopItemQuery.UPDATE_AVAILABLE | EWorkshopItemQuery.DEPENDENCY_UPDATE_AVAILABLE;
			if (catCondition.FindFilter("repair_required").GetSelected())
				query = query | EWorkshopItemQuery.DEPENDENCY_MISSING | EWorkshopItemQuery.ENABLED_AND_DEPENDENCY_DISABLED;

			auto queryResult = SCR_AddonManager.SelectItemsOr(items, query);
			arraysFromCategories.Insert(queryResult);
		}

		// Find intersection of all arrays
		array<ref SCR_WorkshopItem> intersection = Filter_IntersectArrays(arraysFromCategories);

		return intersection;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns intersection of all arrays (array of elements which are contained in all arrays)
	protected array<ref SCR_WorkshopItem> Filter_IntersectArrays(array<ref array<ref SCR_WorkshopItem>> arrays)
	{
		array<ref SCR_WorkshopItem> result = {};

		// Trivial cace: no arrays
		if (arrays.IsEmpty())
			return result;

		// Trivial case: one array
		if (arrays.Count() == 1)
		{
			int count = arrays[0].Count();
			result.Resize(count);
			for (int i = 0; i < count; i++)
			{
				result[i] = arrays[0][i];
			}

			return result;
		}

		// Non-trivial case: more than one array
		auto array0 = arrays[0];
		foreach (auto item : array0)
		{
			// This item must be found in all other arrays too
			bool foundInAll = true;
			for (int arrayId = 1; arrayId < arrays.Count(); arrayId++)
			{

				if (!arrays[arrayId].Contains(item))
				{
					foundInAll = false;
					break;
				}
			}

			if (foundInAll)
				result.Insert(item);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateItemsFoundMessage(int current, int total)
	{
		if (!m_Widgets || !m_Widgets.m_FilterPanelComponent)
			return;
		
		m_Widgets.m_FilterPanelComponent.SetItemsFoundMessage(current, total, current != total);

	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns page count, might depend on current mode
	protected int GetPageCount()
	{
		switch (m_eMode)
		{
			case EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY:
				return m_iOfflinePageCount;
			case EContentBrowserAddonsSubMenuMode.MODE_ONLINE:
				return m_iOnlinePageCount;
			case EContentBrowserAddonsSubMenuMode.MODE_OFFLINE:
				return m_iOfflinePageCount;
		}
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns currently selected workshop item
	protected SCR_WorkshopItem GetSelectedItem()
	{
		Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();

		if (!focusedWidget)
			return null;

		SCR_ContentBrowserTileComponent comp = SCR_ContentBrowserTileComponent.Cast(focusedWidget.FindHandler(SCR_ContentBrowserTileComponent));

		if (!comp)
			return null;

		return comp.GetWorkshopItem();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the widgets related to paging: buttons, text
	protected void UpdatePagingWidgets()
	{
		int pageCount = GetPageCount();

		m_Widgets.m_PagingButtons.SetVisible(pageCount != 0);

		if (pageCount == 0)
			return;

		m_Widgets.m_PageIndicatorText.SetTextFormat("#AR-Editor_ContentBrowser_PageIndex_Text", m_iCurrentPage + 1, pageCount);

		m_Widgets.m_PrevPageButton_NoScrollComponent.SetEnabled(m_iCurrentPage > 0);
		m_Widgets.m_NextPageButton_NoScrollComponent.SetEnabled(m_iCurrentPage < pageCount - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateButtons()
	{
		// Filters button
		if (m_NavFilter)
			m_NavFilter.SetEnabled(!m_bPanelsModeEmpty);
		
		// Item buttons
		SCR_WorkshopItem item = GetSelectedItem();
		
		if (!item)
		{
			// Nothing is selected, disable all buttons
			m_NavFavourite.SetEnabled(false);
			m_NavEnable.SetEnabled(false);
			m_NavPrimary.SetEnabled(false);

			return;
		}

		if (item.GetRestricted())
		{
			m_NavFavourite.SetEnabled(false);
			m_NavEnable.SetEnabled(false);
			m_NavPrimary.SetEnabled(false);
			return;
		}

		auto actionThisItem = item.GetDownloadAction();
		auto actionDependencies = item.GetDependencyCompositeAction();
		bool downloading = actionThisItem || actionDependencies;

		// Favourite button - always enabled
		m_NavFavourite.SetEnabled(true);
		string favLabel;
		if (!item.GetFavourite())
			favLabel = UIConstants.FAVORITE_LABEL_ADD;
		else
			favLabel = UIConstants.FAVORITE_LABEL_REMOVE;
		m_NavFavourite.SetLabel(favLabel);

		// Enable button - enabled only for downloaded addons, but not when downloading
		m_NavEnable.SetEnabled(item.GetOffline() && !downloading);
		if (item.GetOffline())
		{
			string enableLabel;
			if (item.GetEnabled())
				enableLabel = "#AR-Workshop_ButtonDisable";
			else
				enableLabel = "#AR-Workshop_ButtonEnable";
			m_NavEnable.SetLabel(enableLabel);
		}

		// Primary button
		string primaryLabel = SCR_WorkshopUiCommon.GetPrimaryActionName(item);
		if (!primaryLabel.IsEmpty())
			m_NavPrimary.SetLabel(primaryLabel);
		m_NavPrimary.SetEnabled(!primaryLabel.IsEmpty());
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when we have selected a filter to show reported mods
	protected bool GetShowReportedAddons()
	{
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.GetFilter();

		if (!filterSet)
			return false;

		SCR_FilterCategory otherCategory = filterSet.FindFilterCategory("other");

		if (!otherCategory)
			return false;

		SCR_FilterEntry reportedFilter = otherCategory.FindFilter("reported");

		if (!reportedFilter)
			return false;

		return reportedFilter.GetSelected();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if cursor over a given widget
	bool IsWidgetUnderCursor(Widget w)
	{
		Widget wCurrent = WidgetManager.GetWidgetUnderCursor();
		while (wCurrent)
		{
			if (wCurrent == w)
				return true;
			wCurrent = wCurrent.GetParent();
		}
		return false;
	}

	// ------ Paging -------
	//------------------------------------------------------------------------------------------------
	protected void OnPrevPageButton()
	{
		EInputDeviceType inputType = -1;
		if (m_bMousePageUsed)
			inputType = EInputDeviceType.MOUSE;

		m_bMousePageUsed = false;

		if (!IsWidgetUnderCursor(m_Widgets.m_LeftPanelArea) && inputType == EInputDeviceType.MOUSE)
			return;

		m_iCurrentPage--;
		m_iCurrentPage = Math.ClampInt(m_iCurrentPage, 0, GetPageCount() - 1);

		RequestPage(m_iCurrentPage);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNextPageButton()
	{
		EInputDeviceType inputType = -1;
		if (m_bMousePageUsed)
			inputType = EInputDeviceType.MOUSE;

		m_bMousePageUsed = false;

		if (!IsWidgetUnderCursor(m_Widgets.m_LeftPanelArea) && inputType == EInputDeviceType.MOUSE)
			return;

		m_iCurrentPage++;
		m_iCurrentPage = Math.ClampInt(m_iCurrentPage, 0, GetPageCount() - 1);

		RequestPage(m_iCurrentPage);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScrollPage()
	{
		float value = GetGame().GetInputManager().GetActionValue("MouseWheel");

		if (value != 0)
			m_bMousePageUsed = true;

		if (value < 0 && m_iCurrentPage < GetPageCount() - 1)
			OnNextPageButton();
		else if (value > 0 && m_iCurrentPage > 0)
			OnPrevPageButton();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns array of items within specified page, page ID starting from 0.
	protected array<ref SCR_WorkshopItem> SelectItemsAtPage(array<ref SCR_WorkshopItem> items, int pageId)
	{
		array<ref SCR_WorkshopItem> itemsOut = {};

		float nItemsPerPage = GRID_N_ROWS * GRID_N_COLUMNS;
		int nPages = Math.Ceil(items.Count() / nItemsPerPage);

		if (pageId < 0 || pageId >= nPages)
			return itemsOut;

		int idStart = nItemsPerPage * pageId;			// Select from this
		int idEnd = nItemsPerPage * (pageId + 1) - 1;	// Up to this (including this)

		if (idEnd >= items.Count())
			idEnd = items.Count() - 1;

		for (int i = idStart; i <= idEnd; i++)
		{
			itemsOut.Insert(items[i]);
		}

		return itemsOut;
	}

	//------------------------------------------------------------------------------------------------
	//! Switch focus
	protected void SwitchFocus(SCR_EListMenuWidgetFocus focus)
	{
		Widget focusTarget;

		switch (focus)
		{
			case SCR_EListMenuWidgetFocus.LIST:
			{
				if (m_LastFocusedTile && m_LastFocusedTile.IsVisible())
					focusTarget = m_LastFocusedTile.GetRootWidget();
				else if (m_Widgets.m_Grid)
					focusTarget = m_Widgets.m_Grid.GetChildren();

				break;
			}

			case SCR_EListMenuWidgetFocus.FILTERING:
			{
				focusTarget = m_Widgets.m_FilterPanelComponent.GetWidgets().m_FilterButton;
				break;
			}

			case SCR_EListMenuWidgetFocus.SORTING:
			{
				focusTarget = m_Widgets.m_SortingHeader.GetChildren();
				break;
			}
			
			case SCR_EListMenuWidgetFocus.NULL:
			{
				GetGame().GetWorkspace().SetFocusedWidget(null);
				return;
			}
		}

		if (!focusTarget || !focusTarget.IsVisible())
		{
			// Fallback
			focusTarget = m_Widgets.m_SortingHeader.GetChildren();
		}

		GetGame().GetWorkspace().SetFocusedWidget(focusTarget);

		// If we've focused a tile, we need to manually update the details panel (by switching page we focus the same widget, but the tile data is different)
		SCR_ContentBrowserTileComponent comp = SCR_ContentBrowserTileComponent.FindComponent(focusTarget);
		if (comp)
			OnTileFocus(comp, GetSelectedItem())
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// C A L L B A C K S  A N D  E V E N T S
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//------------------------------------------------------------------------------------------------
	//! Called when we have lost connection and connected again,
	//! or when the tab is opened first time and connection is already active
	//! !!! Not called for offline tab. For offline tab connection is not needed. !!!
	protected void OnConnected()
	{
		// Reinit the workshop API
		InitWorkshopApi();

		// Ask the same page again
		RequestPage(m_iCurrentPage);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileClick(SCR_ScriptedWidgetComponent baseComp)
	{
		SCR_ContentBrowserTileComponent comp = SCR_ContentBrowserTileComponent.Cast(baseComp);

		if (!comp || !m_bEnableOpenDetails)
			return;

		SCR_WorkshopItem selectedItem = comp.GetWorkshopItem();
		if (selectedItem)
		{
			if (! (selectedItem.GetRestricted()) || GetShowReportedAddons())
			{
				ContentBrowserDetailsMenu.OpenForWorkshopItem(selectedItem);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocus(SCR_ContentBrowserTileComponent tile, SCR_WorkshopItem item)
	{
		if (!item)
			return;
		
		if (m_Widgets.m_AddonDetailsPanelComponent.GetItem() == item)
			return;

		bool hide = item.GetRestricted();

		if (hide)
			item = null;

		m_Widgets.m_AddonDetailsPanelComponent.SetWorkshopItem(item);

		m_LastFocusedTile = tile;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFilterButton()
	{
		// Toggle the state of the filter panel
		bool show = !m_Widgets.m_FilterPanelComponent.GetFilterListBoxShown();
		m_Widgets.m_FilterPanelComponent.ShowFilterListBox(show);

		if (show)
			SwitchFocus(SCR_EListMenuWidgetFocus.FILTERING);
		else
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrimaryButton()
	{
		SCR_WorkshopItem item = GetSelectedItem();

		if (!item)
			return;

		SCR_WorkshopUiCommon.ExecutePrimaryAction(item, m_DownloadRequest);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEnableButton()
	{
		SCR_WorkshopUiCommon.OnEnableAddonButton(GetSelectedItem());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFavouriteButton()
	{
		SCR_WorkshopItem item = GetSelectedItem();

		if (!item)
			return;

		if (item.GetRestricted())
			return;

		bool fav = item.GetFavourite();
		item.SetFavourite(!fav);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageGetAllAssets(SCR_WorkshopApiCallback_RequestPage callback)
	{
		PageRequestCallbackCleanup(callback);
		
		m_iItemsTotal = m_WorkshopApi.GetTotalItemCount();
		UpdateItemsFoundMessage(m_iOnlineFilteredItems, m_iItemsTotal);
		m_bFirstLoad = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageGetAssets(SCR_WorkshopApiCallback_RequestPage callback)
	{		
		PageRequestCallbackCleanup(callback);
		
		// Search Box Message
		m_iOnlineFilteredItems = m_WorkshopApi.GetTotalItemCount();
		
		array<WorkshopItem> items = {};
		m_WorkshopApi.GetPageItems(items);
		
		if (m_iOnlineFilteredItems < items.Count())
			m_iOnlineFilteredItems = items.Count();
		
		if (m_iItemsTotal < m_iOnlineFilteredItems)
			m_iItemsTotal = m_iOnlineFilteredItems;

		if (!m_bFirstLoad)
			UpdateItemsFoundMessage(m_iOnlineFilteredItems, m_iItemsTotal);
		
		// Update Displayed items
		if (callback.m_iPageId != m_iCurrentPage)
			return; // ?!

		array<ref SCR_WorkshopItem> itemsRegistered = {};

		foreach (auto i : items)
		{
			SCR_WorkshopItem iRegistered = SCR_AddonManager.GetInstance().Register(i);
			if (iRegistered)
				itemsRegistered.Insert(iRegistered);
		}

		if (!itemsRegistered.IsEmpty())
		{
			// There are some items received
			SetPanelsMode(false, m_bFirstPage && m_Widgets.m_FilterPanelComponent.AnyFilterButtonsVisible(), string.Empty, m_bFirstPage);
		}
		else
		{
			// No items received
			SetPanelsMode(false, messagePresetTag: MESSAGE_TAG_NOTHING_FOUND, forceFiltersList: true);
		}

		m_iOnlinePageCount = m_WorkshopApi.GetPageCount();
		GridScreen_DisplayItems(itemsRegistered);

		m_bFirstPage = false;

		if (!m_Widgets.m_FilterPanelComponent.GetFilterListBoxShown() || !m_Widgets.m_FilterPanelComponent.GetFocused())
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST);

		if (m_bFirstLoad)
			GetGame().GetCallqueue().CallLater(RequestOnlinePageUnfiltered, LOAD_PAGE_DELAY, false, callback.m_iPageId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageTimeout(SCR_WorkshopApiCallback_RequestPage callback)
	{
		PageRequestCallbackCleanup(callback);

		if (GetShown())
		{
			// This submenu is shown, show the timeout dialog
			SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateTimeoutTryAgainCancelDialog();
			dlg.m_OnCancel.Insert(Callback_OnRequestPageTimeout_OnCloseDialog);
			dlg.m_OnConfirm.Insert(Callback_OnRequestPageTimeout_OnDialogTryAgain);
		}
		else
		{
			// This submenu is not shown (probably we switched to another tab)
			// Try to perform the request again
			m_bClearCacheAtNextRequest = true;
			RequestPage(m_iCurrentPage);
		}

		m_bFailToLoad = true;
		
		m_iOnlinePageCount = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageTimeout_OnCloseDialog()
	{
		// Set message
		SetPanelsMode(m_bPanelsModeEmpty, messagePresetTag: MESSAGE_TAG_ERROR_CONNECTION);
		RequestOpenOfflinePage();
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageTimeout_OnDialogTryAgain()
	{
		m_bClearCacheAtNextRequest = true;
		RequestPage(m_iCurrentPage);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CheckBackendDown()
	{
		if (!ContentBrowserUI.IsBackenRunning() && m_eMode == EContentBrowserAddonsSubMenuMode.MODE_ONLINE)
		{
			// Set message
			SetPanelsMode(m_bPanelsModeEmpty, messagePresetTag: MESSAGE_TAG_ERROR_CONNECTION);
			Callback_OnRequestPageTimeout(null);
		}

		return !ContentBrowserUI.IsBackenRunning();
	}

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AddonManager when some addon is downloaded or uninstalled
	protected void Callback_OnAddonOfflineStateChanged(SCR_WorkshopItem item, bool newState)
	{
		if (m_eMode == EContentBrowserAddonsSubMenuMode.MODE_OFFLINE)
		{
			// Some addon was installed or uninstalled, we must refresh the page
			RequestPage(m_iCurrentPage);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AddonManager when some addon is reported or unreported
	protected void Callback_OnAddonReportedStateChanged(SCR_WorkshopItem item, bool newReported)
	{
		// Refresh current page if it has this addon
		bool found = false;
		foreach (SCR_ContentBrowserTileComponent comp : m_aTileComponents)
		{
			if (comp.GetWorkshopItem() == item)
			{
				m_bClearCacheAtNextRequest = true;
				RequestPage(m_iCurrentPage);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called when filter listbox selection changes
	protected void OnFilterChange(SCR_FilterEntry filter)
	{
		m_bClearCacheAtNextRequest = true;
		RequestPage(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when user confirms the value in the seacrh string
	protected void OnFilterSearchConfirm(SCR_EditBoxComponent comp, string newValue)
	{
		m_bClearCacheAtNextRequest = true;
		RequestPage(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when sorting header changes
	protected void OnSortingHeaderChange(SCR_SortHeaderComponent sortHeader)
	{
		m_bClearCacheAtNextRequest = true;
		RequestPage(0);
	}

	//------------------------------------------------------------------------------------------------
	SCR_ContentBrowser_AddonsSubMenuBaseWidgets GetWidgets()
	{
		return m_Widgets;
	}

	//------------------------------------------------------------------------------------------------
	array<SCR_ContentBrowserTileComponent> GetTileComponents()
	{
		array<SCR_ContentBrowserTileComponent> tiles = {};
		m_aTileComponents.Copy(tiles);

		return tiles;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ContentBrowserTileComponent GetTileComponentById(int id)
	{
		if (!m_aTileComponents.IsIndexValid(id))
			return null;

		return m_aTileComponents[id];
	}

	//------------------------------------------------------------------------------------------------
	int GetTileComponentsCount()
	{
		return m_aTileComponents.Count();
	}
}


class SCR_ContentBrowser_GetAssetListParams : PageParams
{
	// We will call back to submenu to perform packing of the JSON struct
	SCR_ContentBrowser_AddonsSubMenu m_AddonsSubMenu;
	protected bool m_bUseDefaultValues;

	//------------------------------------------------------------------------------------------------
	void SCR_ContentBrowser_GetAssetListParams(SCR_ContentBrowser_AddonsSubMenu subMenu, bool defaultValues = false)
	{
		m_bUseDefaultValues = defaultValues;
		m_AddonsSubMenu = subMenu;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{
		m_AddonsSubMenu.PackGetAssetListParams(this, m_bUseDefaultValues);
	}
}


class SCR_CompareWorkshopItemName : SCR_SortCompare<SCR_WorkshopItem>
{
	override static int Compare(SCR_WorkshopItem left, SCR_WorkshopItem right)
	{
		string name1 = left.GetName();
		string name2 = right.GetName();

		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
	}
}

class SCR_CompareWorkshopItemTimeSinceLastPlay : SCR_SortCompare<SCR_WorkshopItem>
{
	override static int Compare(SCR_WorkshopItem left, SCR_WorkshopItem right)
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


class SCR_CompareWorkshopItemTimeSinceFirstDownload : SCR_SortCompare<SCR_WorkshopItem>
{
	override static int Compare(SCR_WorkshopItem left, SCR_WorkshopItem right)
	{
		int timeLeft = left.GetTimeSinceFirstDownload();
		if (timeLeft < 0)			// Negative value means we never downloaded this
			timeLeft =	0x7FFFFFFF;	// We set it to max positive int for sorting purpuse.
		int timeRight = right.GetTimeSinceFirstDownload();
		if (timeRight < 0)
			timeRight =	0x7FFFFFFF;

		return timeLeft < timeRight;
	}
}


//------------------------------------------------------------------------------------------------
// Sort by time since last played
class SCR_CompareWorkshopItemTargetSize : SCR_SortCompare<SCR_WorkshopItem>
{
	override static int Compare(SCR_WorkshopItem left, SCR_WorkshopItem right)
	{
		float leftSize = left.GetTargetRevisionPatchSize();
		float rightSize = right.GetTargetRevisionPatchSize();

		return leftSize < rightSize;
	}
};
