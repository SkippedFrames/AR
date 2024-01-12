//------------------------------------------------------------------------------------------------
class ContentBrowserDetailsMenu : SCR_SuperMenuBase
{
	// ----------- Constants ------------
	protected const ResourceName LAYOUT_SCENARIO_ELEMENT = "{5872ACA20319FC8C}UI/layouts/Menus/WorldSelection/ScenarioTile.layout";

	const int TAB_ID_OVERVIEW = 0;	// These values must match the order of tabs in the layout
	const int TAB_ID_DEPENDENCY = 1;
	const int TAB_ID_SCENARIO = 2;

	// ----------- Widgets ------------
	Widget m_wRoot;
	SCR_TabViewComponent m_TabView;
	SCR_ContentBrowserDetails_OverviewSubMenu m_OverviewSubMenu;
	SCR_ContentBrowser_AddonsSubMenu m_DependencySubMenu;

	// ----------- Callbacks --------
	ref ScriptInvoker m_OnItemReported = new ScriptInvoker();

	// ----------- Other ------------
	SCR_WorkshopItem m_WorkshopItem;
	protected SCR_InputButtonComponent m_NavBack;
	ref array<ref SCR_WorkshopItem> m_aDependencies = {};			// These are also accessed from the dependency tab
	ref array<MissionWorkshopItem> m_aMissions = {};
	ref array<string> m_aVersions = {};
	Revision m_LastRevision;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		// DO NOTHING
		// We override it to prevent automatic init.
		// We will call parent's OnMenuOpen inside our custom Init
	}

	//------------------------------------------------------------------------------------------------
	//! Called on each frame
	override void OnMenuUpdate(float tDelta)
	{
		// Will call update of current submenu
		super.OnMenuUpdate(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback passed to tab view, called when a new tab is created
	override void OnTabCreate(SCR_TabViewComponent comp, Widget w)
	{
		// Register the created tabs

		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(w.FindHandler(SCR_SubMenuBase));
		if (subMenu)
		{
			//subMenu.OnMenuOpen(this); // todo fix this, for some reason it doesn't get called automaticall

			SCR_ContentBrowserDetails_OverviewSubMenu overviewSubMenu = SCR_ContentBrowserDetails_OverviewSubMenu.Cast(subMenu);
			SCR_ContentBrowser_AddonsSubMenu dependencySubMenu = SCR_ContentBrowser_AddonsSubMenu.Cast(subMenu);
			if (overviewSubMenu)
				m_OverviewSubMenu = overviewSubMenu;
			if (dependencySubMenu)
			{
				m_DependencySubMenu = dependencySubMenu;
				m_DependencySubMenu.SetWorkshopItems(m_aDependencies);
			}
			// todo other submenus
		}

		super.OnTabCreate(comp, w);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the menu for a given workshop item
	// closeSameMenu - when true, an existing details menu will be closed
	static ContentBrowserDetailsMenu OpenForWorkshopItem(SCR_WorkshopItem item, bool closeExistingMenu = false)
	{
		MenuManager mm = GetGame().GetMenuManager();

		if (closeExistingMenu)
		{
			ContentBrowserDetailsMenu previousMenu = ContentBrowserDetailsMenu.Cast(mm.FindMenuByPreset(ChimeraMenuPreset.ContentBrowserDetailsMenu));
			if (previousMenu)
			{
				if (previousMenu.m_WorkshopItem != item)
					mm.CloseMenu(previousMenu);
				else
					return previousMenu; // Do nothing if asked to open same addon details page again
			}
		}

		ContentBrowserDetailsMenu detailsMenu = ContentBrowserDetailsMenu.Cast(mm.OpenMenu(ChimeraMenuPreset.ContentBrowserDetailsMenu, 0, false));

		if (detailsMenu)
		{
			detailsMenu.Init(item);
		}

		return detailsMenu;
	}

	//------------------------------------------------------------------------------------------------
	//! Here we do initialization of this menu and we can pass some custom data
	protected void Init(SCR_WorkshopItem item)
	{
		// Custom init - pass the workshop item to this menu
		// This way we can use this menu in different contexts
		m_WorkshopItem = item;

		// Init widgets
		m_wRoot = GetRootWidget();

		// Call parent OnMenuOpen
		// First child tab is going to be created now
		super.OnMenuOpen();

		Widget wTabView = m_wRoot.FindAnyWidget("TabViewRoot0");
		m_TabView = SCR_TabViewComponent.Cast(wTabView.FindHandler(SCR_TabViewComponent));

		// Setup the 'back' nav button
		m_NavBack = SCR_InputButtonComponent.GetInputButtonComponent("Back", m_wRoot);
		m_NavBack.m_OnActivated.Insert(OnNavButtonClose);

		// Request data from the backend, if this item is present in the backend and we are connected
		if (m_WorkshopItem.GetOnline() && GetConnectionState())
		{
			m_WorkshopItem.m_OnGetAsset.Insert(Callback_OnDetailsLoaded);
			//m_WorkshopItem.m_OnScenariosLoaded.Insert(OnAskDetailsGetAssetScenarios);
			m_WorkshopItem.m_OnDependenciesLoaded.Insert(OnAskDetailsGetDependencyTree);

			m_WorkshopItem.LoadDetails();
		}
		else
		{
			// If this item is purely local, we call the callbacks ourselves
			string gameEnv = GetGame().GetBackendApi().GetBackendEnv();
			string modEnv = m_WorkshopItem.GetWorkshopItem().GetBackendEnv();
			if (gameEnv == modEnv)
			{
				this.Callback_OnDetailsLoaded();
				this.OnAskDetailsGetDependencyTree();
			}
		}
	}

	// Callbacks of common buttons
	//------------------------------------------------------------------------------------------------
	void OnNavButtonClose()
	{
		// This menu might be not focused, because another details menu is currently open
		if (IsFocused())
			Close();
	}

	// Getters
	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem()
	{
		return m_WorkshopItem;
	}

	// Workshop API requests
	// Events from the callbacks
	//------------------------------------------------------------------------------------------------
	void Callback_OnDetailsLoaded()
	{
		#ifdef WORKSHOP_DEBUG
		m_WorkshopItem.LogState();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	// Called when we have received dependencies
	void OnAskDetailsGetDependencyTree()
	{
		// Whole list of dependencies is available now
		m_aDependencies = m_WorkshopItem.GetLatestDependencies();

		if (m_aDependencies.Count() > 0)
		{
			if (m_DependencySubMenu)
				m_DependencySubMenu.SetWorkshopItems(m_aDependencies);

			// Load details for all dependencies, so we have data about reported state
			foreach (SCR_WorkshopItem dep : m_aDependencies)
			{
				dep.LoadDetails();
			}

			m_TabView.EnableTab(TAB_ID_DEPENDENCY, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called from the report dialog after the report has been sent successfully
	void OnItemReportedSuccessfully(bool authorBlock = false)
	{
		SCR_ConfigurableDialogUi dlg = SCR_WorkshopUiCommon.CreateDialog("report_success");
		dlg.m_OnClose.Insert(OnReportSuccessDialogConfirm);

		// Message
		string msg;
		
		if (!authorBlock)
			msg = dlg.GetMessageStr() + "\n\n" ;
		
		msg += "#AR-Workshop_ReportModReverse";
		dlg.GetMessageWidget().SetTextFormat(msg, "#AR-Workshop_Filter_Reported", "#AR-Workshop_CancelReport");
	}

	//------------------------------------------------------------------------------------------------
	void OnReportSuccessDialogConfirm()
	{
		m_OnItemReported.Invoke();

		this.Close();
	}

	//------------------------------------------------------------------------------------------------
	static bool GetConnectionState()
	{
		BackendApi backend = GetGame().GetBackendApi();
		bool connected = backend.IsActive() && backend.IsAuthenticated();
		return connected;
	}
}
