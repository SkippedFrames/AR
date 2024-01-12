class SCR_FieldManualUI : MenuRootBase
{
	protected TextWidget m_wMenuTitle;
	protected VerticalLayoutWidget m_wMenuCategoryList;
	protected SCR_EditBoxSearchComponent m_MenuSearchbar;
	protected SCR_InputButtonComponent m_MenuBtnBack;

	protected Widget m_wReadingWidget;
	protected SCR_BreadCrumbsComponent m_BreadCrumbsComponent;
	protected TextWidget m_wPageEntryTitle;
	protected Widget m_wPageFrame;
	protected ScrollLayoutWidget m_wGridScrollLayoutWidget;
	protected GridLayoutWidget m_wMenuGridLayout;

	protected string m_sLastSearch;

	protected ref SCR_FieldManualConfigRoot m_ConfigRoot;
	protected ref array<ref SCR_FieldManualConfigEntry> m_aAllEntries;
	protected ref map<Widget, ref SCR_FieldManualConfigEntry> m_mWidgetEntryMap;
	protected ref map<Widget, ref SCR_FieldManualConfigCategory> m_mWidgetSubCategoryMap;
	protected ref SCR_FieldManualSubCategoryScriptedWidgetEventHandler m_SubCategoryButtonEventHandler;
	protected ref SCR_FieldManualEntryScriptedWidgetEventHandler m_EntryButtonEventHandler;
	protected Widget m_wLastClickedSubCategory;

	protected SCR_FieldManualConfigEntry m_CurrentEntry;

	protected ButtonWidget m_wFirstSubCategoryButton;

	protected bool m_bIsInEntryViewMode;
	protected bool m_bIsInSearchMode;
	protected bool m_bOpenedFromOutside;

	protected ResourceName m_ConfigPath;

	protected static const int S_TILES_GRID_WIDTH = 5;
	protected static const ResourceName S_ENTRY_TITLE_LAYOUT = "{B3656334DE3D701F}UI/layouts/WidgetLibrary/TextWidgets/Text_Heading3.layout";
	protected static const string S_SEARCH_RESULT_KEY = "#AR-FieldManual_SearchResult";
	protected static const string S_SEARCH_NO_RESULT_KEY = "#AR-FieldManual_SearchResult_None";

	protected bool m_bArmaWarning;

	//------------------------------------------------------------------------------------------------
	void SCR_FieldManualUI()
	{
		m_aAllEntries = {};
		m_mWidgetEntryMap = new map<Widget, ref SCR_FieldManualConfigEntry>();
		m_mWidgetSubCategoryMap = new map<Widget, ref SCR_FieldManualConfigCategory>();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		Widget rootWidget = GetRootWidget();
		SCR_ConfigUIComponent component = SCR_ConfigUIComponent.Cast(rootWidget.FindHandler(SCR_ConfigUIComponent));
		if (!component)
		{
			Print("No config component found | " + __FILE__ + ": " + __LINE__, LogLevel.WARNING);
			Close();
			return;
		}

		SCR_FieldManualConfigRoot configRoot = SCR_FieldManualConfigLoader.LoadConfigRoot(component.m_ConfigPath);
		if (!configRoot)
		{
			Print("Could not load provided config | " + __FILE__ + ": " + __LINE__, LogLevel.WARNING);
			Close();
			return;
		}

		m_ConfigRoot = configRoot;
		SetAllEntriesAndParents();

		// which background do you want, gents?
		bool isMainMenu = GetGame().m_bIsMainMenuOpen;
		Widget blur = GetRootWidget().FindAnyWidget("Blur");
		Widget backgroundImage = GetRootWidget().FindAnyWidget("MenuBackground");
		if (blur && isMainMenu)
			blur.RemoveFromHierarchy();
		if (backgroundImage && !isMainMenu)
		    backgroundImage.RemoveFromHierarchy();

		Widget menuFrame = rootWidget.FindAnyWidget("MenuFrame");
		TextWidget menuTitle = TextWidget.Cast(menuFrame.FindAnyWidget("Title"));
		VerticalLayoutWidget menuCategoryList = VerticalLayoutWidget.Cast(rootWidget.FindAnyWidget("CategoryList"));
		GridLayoutWidget menuGridLayout = GridLayoutWidget.Cast(rootWidget.FindAnyWidget("GridLayout"));
		if (!menuFrame || !menuTitle || !menuCategoryList || !menuGridLayout)
		{
			Print("Important elements (menu frame & title, category list, grid layout) are missing | " + __FILE__ + ": " + __LINE__, LogLevel.WARNING);
			Close();
			return;
		}

		Widget readingWidget = rootWidget.FindAnyWidget("ReadingWidget");

		TextWidget entryTitle = TextWidget.Cast(readingWidget.FindAnyWidget("EntryTitle"));
		Widget entryFrame = readingWidget.FindAnyWidget("EntryFrame");
		if (!readingWidget || !entryTitle || !entryFrame)
		{
			Print(string.Format("missing one: reading widget (%1), entryTitle (%2), entryFrame (%3) | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, readingWidget != null, entryTitle != null, entryFrame != null), LogLevel.WARNING);
			Close();
			return;
		}

		SCR_InputButtonComponent backButtonMenuFrame = SCR_InputButtonComponent.GetInputButtonComponent("Back", menuFrame);
		if (!backButtonMenuFrame || !backButtonMenuFrame.m_OnClicked)
		{
			Print(string.Format("missing menu frame's back button menu (%1) or m_OnClicked (%2) | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, backButtonMenuFrame != null, backButtonMenuFrame.m_OnClicked != null), LogLevel.WARNING);
			Close();
			return;
		}

		m_MenuBtnBack = backButtonMenuFrame;

		m_wMenuTitle = menuTitle;
		m_wMenuCategoryList = menuCategoryList;
		m_wMenuGridLayout = menuGridLayout;

		m_wReadingWidget = readingWidget;
		m_wPageEntryTitle = entryTitle;
		m_wPageFrame = entryFrame;

		// breadcrumbs is not mandatory
		m_BreadCrumbsComponent = SCR_BreadCrumbsComponent.Cast(SCR_BreadCrumbsComponent.GetComponent(SCR_BreadCrumbsComponent, "BreadCrumbs", rootWidget));

		// scrolling reset is not mandatory
		m_wGridScrollLayoutWidget = ScrollLayoutWidget.Cast(rootWidget.FindAnyWidget("GridScrollLayout"));

		// searchbar functionality is not mandatory
		m_MenuSearchbar = SCR_EditBoxSearchComponent.Cast(SCR_EditBoxSearchComponent.GetEditBoxComponent("Searchbar", rootWidget));
		if (m_MenuSearchbar)
		{
			if (m_MenuSearchbar.m_OnConfirm)
				m_MenuSearchbar.m_OnConfirm.Insert(ProcessSearch);
		}

		m_MenuBtnBack.m_OnActivated.Insert(CloseMenuOrReadingPanel);

#ifdef WORKBENCH
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("MenuOpenWB", EActionTrigger.DOWN, CloseMenuOrReadingPanel);
			inputManager.AddActionListener("MenuBackWB", EActionTrigger.DOWN, CloseMenuOrReadingPanel);
		}
#endif

		m_SubCategoryButtonEventHandler = new SCR_FieldManualSubCategoryScriptedWidgetEventHandler(this);
		m_EntryButtonEventHandler = new SCR_FieldManualEntryScriptedWidgetEventHandler(this);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuShow()
	{
		super.OnMenuShow();
		
		CreateCategoryMenuWidgets();
		CloseReadingPanel();
		OpenFirstSubCategory();
	}

	//------------------------------------------------------------------------------------------------
	void OnSubCategoryClicked(Widget w)
	{
		bool resetTiles = m_bIsInSearchMode || m_bIsInEntryViewMode || w != m_wLastClickedSubCategory;
		m_bIsInSearchMode = false;
		m_wLastClickedSubCategory = w;
		CloseReadingPanel();
		ResetLastSearch();
		if (resetTiles)
			SetTilesByWidget(w);
	}

	//------------------------------------------------------------------------------------------------
	void OnTileClicked(Widget w)
	{
		m_bIsInSearchMode = false;
		SetCurrentEntryByWidget(w);
		
		if (m_wLastClickedSubCategory)
			GetGame().GetWorkspace().SetFocusedWidget(m_wLastClickedSubCategory);
	}

	//------------------------------------------------------------------------------------------------
	//! creates a tile and sets its image, background & text
	protected Widget CreateTileWidget(notnull SCR_FieldManualConfigEntry entry, notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(m_ConfigRoot.m_MenuEntryTileLayout, parent);
		if (!createdWidget)
		{
			Print("could not create tile widget for entry | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return null;
		}

		Widget assetImage = createdWidget.FindAnyWidget("AssetImage");
		if (assetImage)
		{
			ImageWidget backgroundImageWidget = ImageWidget.Cast(createdWidget.FindAnyWidget("FullBackground"));
			if (backgroundImageWidget && m_ConfigRoot.m_aTileBackgrounds && m_ConfigRoot.m_aTileBackgrounds.Count())
				backgroundImageWidget.LoadImageTexture(0, m_ConfigRoot.m_aTileBackgrounds.Get(Math.RandomInt(0, m_ConfigRoot.m_aTileBackgrounds.Count())));

			ImageWidget imageWidget = ImageWidget.Cast(assetImage.FindAnyWidget("Image"));
			if (imageWidget)
			{
				if (!entry.m_Image.IsEmpty())
				{
					imageWidget.LoadImageTexture(0, entry.m_Image);
				}
				else
				{
					imageWidget.SetVisible(false);
				}
			}
		}

		Widget assetInfo = createdWidget.FindAnyWidget("AssetInfo");
		if (assetInfo)
		{
			TextWidget textWidget = TextWidget.Cast(assetInfo.FindAnyWidget("EntityName"));
			if (textWidget)
				textWidget.SetText(entry.m_sTitle);
		}

		return createdWidget;
	}

	//------------------------------------------------------------------------------------------------
	//! sets entry tiles from a sub-category's widget
	// used in SCR_FieldManualSubCategoryScriptedWidgetEventHandler.OnClick
	protected void SetTilesByWidget(Widget widget)
	{
		SCR_FieldManualConfigCategory subCategory = m_mWidgetSubCategoryMap.Get(widget);

		if (!subCategory)
			return;

		if (m_BreadCrumbsComponent && subCategory.m_Parent)
			m_BreadCrumbsComponent.Set(subCategory.m_Parent.m_sTitle, subCategory.m_sTitle);

		SetTiles(subCategory.m_aEntries);
	}

	//------------------------------------------------------------------------------------------------
	//! sets main Field Manual grid tiles from entries
	protected void SetTiles(array<ref SCR_FieldManualConfigEntry> entries)
	{
		if (!m_wMenuGridLayout)
			return;

		SCR_WidgetHelper.RemoveAllChildren(m_wMenuGridLayout);

		if (!entries || entries.Count() < 1)
			return;

		m_mWidgetEntryMap.Clear();

		int column, line;
		Widget assetCard;
		ButtonWidget button;
		foreach (SCR_FieldManualConfigEntry entry : entries)
		{
			assetCard = CreateTileWidget(entry, m_wMenuGridLayout);
			if (!assetCard)
				continue;

			GridSlot.SetColumn(assetCard, column);
			GridSlot.SetRow(assetCard, line);

			column++;
			if (column >= S_TILES_GRID_WIDTH)
			{
				column = 0;
				line += 1;
			}

			button = ButtonWidget.Cast(SCR_WidgetHelper.GetWidgetOrChild(assetCard, "AssetCard0"));
			if (button)
			{
				button.AddHandler(m_EntryButtonEventHandler);
				m_mWidgetEntryMap.Insert(button, entry);
			}
		}
		
		if (m_wGridScrollLayoutWidget)
			m_wGridScrollLayoutWidget.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! sets the current entry to read - can be null (leave reading mode) or any SCR_FieldManualConfigEntry
	protected void SetCurrentEntry(SCR_FieldManualConfigEntry entry)
	{
		if (!m_wPageFrame || !m_wPageEntryTitle)
			return;

		SCR_WidgetHelper.RemoveAllChildren(m_wPageFrame);

		m_bIsInEntryViewMode = entry != null;

		m_wGridScrollLayoutWidget.SetVisible(!m_bIsInEntryViewMode);
		m_wReadingWidget.SetVisible(m_bIsInEntryViewMode);

		if (!m_bIsInEntryViewMode)
		{
			if (m_BreadCrumbsComponent && m_CurrentEntry && m_CurrentEntry.m_Parent && m_CurrentEntry.m_Parent.m_Parent)
				m_BreadCrumbsComponent.Set(m_CurrentEntry.m_Parent.m_Parent.m_sTitle, m_CurrentEntry.m_Parent.m_sTitle);

			m_CurrentEntry = null;
			return;
		}

		if (m_BreadCrumbsComponent && entry.m_Parent && entry.m_Parent.m_Parent)
			m_BreadCrumbsComponent.Set(entry.m_Parent.m_Parent.m_sTitle, entry.m_Parent.m_sTitle, entry.m_sTitle);

		m_CurrentEntry = entry;

		m_wPageEntryTitle.SetText(entry.m_sTitle);

		// entry tab management - disabled for now
/*
		if (m_PageTabView)
		{
			while (m_PageTabView.GetTabCount() > 0)
			{
				m_PageTabView.RemoveTab(0);
			}
			m_PageTabView.AddTab("", entry.m_sTitle);
			m_PageTabView.ShowTab(0, false, false);
		}
// */

		Widget readingWidget = entry.CreateWidget(m_wPageFrame);
		SCR_FieldManualConfigEntry_Weapon weaponEntry = SCR_FieldManualConfigEntry_Weapon.Cast(entry);
		if (weaponEntry)
			FillEntry_Weapon(readingWidget, weaponEntry);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens an entry "from outside", e.g a hint
	void OpenEntry(EFieldManualEntryId entryId)
	{
		if (entryId == EFieldManualEntryId.NONE)
		{
			SetCurrentEntry(null);
			return;
		}

		foreach (SCR_FieldManualConfigEntry entry : m_aAllEntries)
		{
			if (entry.m_eId == entryId)
			{
				m_bOpenedFromOutside = true;
				SetCurrentEntry(entry);
				return;
			}
		}

		SetCurrentEntry(null);
	}

	//------------------------------------------------------------------------------------------------
	//! sets the read entry linked from that widget
	protected void SetCurrentEntryByWidget(Widget widget)
	{
		SCR_FieldManualConfigEntry entry = m_mWidgetEntryMap.Get(widget);
		if (!entry)
			return;

		SetCurrentEntry(entry);
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon(notnull Widget widget, notnull SCR_FieldManualConfigEntry_Weapon entry)
	{
		Widget weaponRender = widget.FindAnyWidget("weaponRender");
		if (true /* remove later */ || !FillEntry_Weapon_Render(RenderTargetWidget.Cast(weaponRender), entry.m_WeaponEntityPath))
			if (weaponRender)
				weaponRender.RemoveFromHierarchy();
		FillEntry_Weapon_Statistics(widget.FindAnyWidget("statsLayout"), entry);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FillEntry_Weapon_Render(RenderTargetWidget renderTarget, ResourceName weaponResourceName)
	{
		if (!renderTarget || !weaponResourceName)
			return false;

		Resource resource = Resource.Load(weaponResourceName);
		IEntity entity = GetGame().SpawnEntityPrefabLocal(resource, null, null);
		if (!entity)
			return false;

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return false;
		
		ItemPreviewManagerEntity manager = world.GetItemPreviewManager();
		if (!manager)
			return false;

		ItemPreviewWidget renderPreview = ItemPreviewWidget.Cast(renderTarget);
		if (!renderPreview)
			return false;

		manager.SetPreviewItem(renderPreview, entity);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics(Widget statsLayout, SCR_FieldManualConfigEntry_Weapon entry)
	{
		if (!statsLayout)
			return;

		SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper = entry.m_WeaponStatsHelper;
		if (!weaponStatsHelper)
		{
			SCR_WidgetHelper.RemoveAllChildren(statsLayout);
			return;
		}

		FillEntry_Weapon_Statistics_Weight(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_Zeroing(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_DefaultZeroing(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_RateOfFire(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_MuzzleVelocity(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_FireModes(statsLayout, weaponStatsHelper);
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_Weight(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsWeight = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsWeight");
		if (!statsWeight || weaponStatsHelper.GetWeight() < 0)
		{
			if (statsWeight)
				statsWeight.RemoveWidgetFromHierarchy();
		}
		else
		{
			statsWeight.SetTranslatedValue("#AR-ValueUnit_Short_Kilograms", weaponStatsHelper.GetWeight().ToString(-1, 2));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_Zeroing(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsZeroing = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsZeroing");
		if (!statsZeroing || !weaponStatsHelper.GetZeroing() || weaponStatsHelper.GetZeroing().IsEmpty())
		{
			if (statsZeroing)
				statsZeroing.RemoveWidgetFromHierarchy();
		}
		else
		{
			array<string> zeroingStr = {};
			array<int> zeroing = weaponStatsHelper.GetZeroing();
			for (int i = 0, size = zeroing.Count(); i < size; i++)
			{
				zeroingStr.Insert(WidgetManager.Translate("#AR-ValueUnit_Short_Meters", zeroing[i]));
			}
			statsZeroing.SetValue(ArrayJoin(zeroingStr));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_DefaultZeroing(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsDefaultZeroing = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsDefaultZeroing");
		if (!statsDefaultZeroing || weaponStatsHelper.GetDefaultZeroing() < 1)
		{
			if (statsDefaultZeroing)
				statsDefaultZeroing.RemoveWidgetFromHierarchy();
		}
		else
		{
			statsDefaultZeroing.SetTranslatedValue("#AR-ValueUnit_Short_Meters", weaponStatsHelper.GetDefaultZeroing().ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_RateOfFire(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsRateOfFire = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsRateOfFire");
		if (!statsRateOfFire || weaponStatsHelper.GetRateOfFire() < 1)
		{
			if (statsRateOfFire)
				statsRateOfFire.RemoveWidgetFromHierarchy();
		}
		else
		{
			statsRateOfFire.SetTranslatedValue("#AR-ValueUnit_Short_RoundsPerMinute", weaponStatsHelper.GetRateOfFire().ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_MuzzleVelocity(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsMuzzleVelocity = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsMuzzleVelocity");
		if (!statsMuzzleVelocity || weaponStatsHelper.m_iMuzzleVelocity < 1)
		{
			if (statsMuzzleVelocity)
				statsMuzzleVelocity.RemoveWidgetFromHierarchy();
		}
		else
		{
			statsMuzzleVelocity.SetTranslatedValue("#AR-ValueUnit_Short_MetersPerSeconds", weaponStatsHelper.m_iMuzzleVelocity.ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_FireModes(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsFireModes = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsFireModes");
		if (!statsFireModes || !weaponStatsHelper.GetFireModes() || weaponStatsHelper.GetFireModes().IsEmpty())
		{
			if (statsFireModes)
				statsFireModes.RemoveWidgetFromHierarchy();
		}
		else
		{
			statsFireModes.SetValue(ArrayJoin(weaponStatsHelper.GetFireModes()));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! one-time call that goes through all categories and entries and adds enabled entries to a one-level array
	// to do: breakdown in smaller methods?
	protected void SetAllEntriesAndParents()
	{
		m_aAllEntries.Clear();
		array<SCR_FieldManualConfigCategory> categoriesToRemove = {};
		array<SCR_FieldManualConfigCategory> subCategoriesToRemove = {};
		array<SCR_FieldManualConfigEntry> entriesToRemove = {};
		foreach (SCR_FieldManualConfigCategory category : m_ConfigRoot.m_aCategories)
		{
			if (!category.m_bEnabled)
			{
				categoriesToRemove.Insert(category);
				continue;
			}

			subCategoriesToRemove.Clear();
			foreach (SCR_FieldManualConfigCategory subCategory : category.m_aCategories) // only two levels are supported by the UI for now
			{
				if (!subCategory.m_bEnabled)
				{
					subCategoriesToRemove.Insert(subCategory);
					continue;
				}

				entriesToRemove.Clear();
				foreach (SCR_FieldManualConfigEntry entry : subCategory.m_aEntries)
				{
					if (!entry.m_bEnabled || entry.m_aContent.IsEmpty())
					{
						entriesToRemove.Insert(entry);
						continue;
					}
					entry.m_Parent = subCategory;
					m_aAllEntries.Insert(entry);
				}
				foreach (SCR_FieldManualConfigEntry entry : entriesToRemove)
				{
					subCategory.m_aEntries.RemoveItemOrdered(entry);
				}

				if (subCategory.m_aEntries.IsEmpty() && subCategory.m_aCategories.IsEmpty())
				{
					subCategoriesToRemove.Insert(subCategory);
					continue;
				}

				subCategory.m_Parent = category;
			}
			foreach (SCR_FieldManualConfigCategory subCategory : subCategoriesToRemove)
			{
				category.m_aCategories.RemoveItemOrdered(subCategory);
			}

			if (category.m_aEntries.IsEmpty() && category.m_aCategories.IsEmpty())
			{
				categoriesToRemove.Insert(category);
			}
		}
		foreach (SCR_FieldManualConfigCategory category : categoriesToRemove)
		{
			m_ConfigRoot.m_aCategories.RemoveItemOrdered(category);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! creates main categories buttons list
	protected void CreateCategoryMenuWidgets()
	{
		if (!m_ConfigRoot || !m_ConfigRoot.m_aCategories || !m_wMenuCategoryList)
			return;

		m_wFirstSubCategoryButton = null;
		m_mWidgetSubCategoryMap.Clear();
		SCR_WidgetHelper.RemoveAllChildren(m_wMenuCategoryList);

		if (m_wMenuTitle)
		{
			m_wMenuTitle.SetText(m_ConfigRoot.m_sTitle);
		}

		Widget subCategoryWidget;
		ButtonWidget button;
		foreach (SCR_FieldManualConfigCategory category : m_ConfigRoot.m_aCategories)
		{
			category.CreateWidget(m_wMenuCategoryList);

			foreach (SCR_FieldManualConfigCategory subCategory : category.m_aCategories)
			{
				subCategoryWidget = subCategory.CreateWidget(m_wMenuCategoryList);
				button = ButtonWidget.Cast(subCategoryWidget);
				if (button)
				{
					button.AddHandler(m_SubCategoryButtonEventHandler);
					m_mWidgetSubCategoryMap.Insert(button, subCategory);

					if (!m_wFirstSubCategoryButton)
						m_wFirstSubCategoryButton = button;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! opens the first sub-category, showing its entries
	protected void OpenFirstSubCategory()
	{
		OnSubCategoryClicked(m_wFirstSubCategoryButton);
		GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, m_wFirstSubCategoryButton, false);
	}

	//------------------------------------------------------------------------------------------------
	//! resets last search value
	// used in SCR_FieldManualSubCategoryScriptedWidgetEventHandler.OnClick
	protected void ResetLastSearch()
	{
		m_sLastSearch = "";
	}

	//------------------------------------------------------------------------------------------------
	//! the search process itself - finds entries and set tiles
	protected void ProcessSearch(SCR_EditBoxSearchComponent searchbox, string search)
	{
		search = search.Trim();

		if (search.IsEmpty())
		{
			ResetLastSearch();
//			if (m_bIsInSearchMode)
//				OpenFirstSubCategory();

			return;
		}

		CloseReadingPanel();

		if (!m_bArmaWarning && search.Contains("ArmA"))
		{
			m_bArmaWarning = true;

			if (m_BreadCrumbsComponent)
				m_BreadCrumbsComponent.Set("#AR-FieldManual_Page_EasterEgg_ArmA_BreadCrumbs");

			SCR_FieldManualConfigEntry_Standard ArmaEntry = new SCR_FieldManualConfigEntry_Standard();
			ArmaEntry.m_sTitle = "#AR-FieldManual_Page_EasterEgg_ArmA_Title";
			SCR_FieldManualPiece_Text ArmaPiece = new SCR_FieldManualPiece_Text();
			ArmaPiece.m_sText = "#AR-FieldManual_Page_EasterEgg_ArmA_Text";
			ArmaEntry.m_aContent.Insert(ArmaPiece);
			SetCurrentEntry(ArmaEntry);

			EditBoxWidget searchEditBox = EditBoxWidget.Cast(GetRootWidget().FindAnyWidget("Searchbar").FindAnyWidget("EditBox"));
			string searchText = searchEditBox.GetText();
			searchText.Replace("ArmA", "Arma");
			searchEditBox.SetText(searchText);

			ResetLastSearch();

			return;
		}

		m_bIsInSearchMode = true;

		string originalSearch = search;
		search.ToLower();
		if (search == m_sLastSearch)
			return;

		m_sLastSearch = search;

		array<ref SCR_FieldManualConfigEntry> entries = {};

		if (!m_sLastSearch.IsEmpty())
		{
			array<int> indexList = {};

			array<string> allEntriesKeys = {};
			foreach (SCR_FieldManualConfigEntry entry : m_aAllEntries)
			{
				allEntriesKeys.Insert(entry.m_sTitle);
			}

			WidgetManager.SearchLocalized(m_sLastSearch, allEntriesKeys, indexList);

			foreach (int index : indexList)
			{
				entries.Insert(m_aAllEntries.Get(index));
			}
		}

		if (m_BreadCrumbsComponent)
		{
			if (entries.IsEmpty())
				m_BreadCrumbsComponent.SetRichFormat(S_SEARCH_NO_RESULT_KEY, originalSearch);
			else
				m_BreadCrumbsComponent.SetRichFormat(S_SEARCH_RESULT_KEY, originalSearch, entries.Count().ToString());
		}

		SetTiles(entries);
	}

	//------------------------------------------------------------------------------------------------
	//! closes the reading panel (sets the current entry to null)
	protected void CloseReadingPanel()
	{
		SetCurrentEntry(null);
	}

	//------------------------------------------------------------------------------------------------
	//! determines what happens when ESC is pressed
	//! - if in reading mode, closes the reading panel
	//! - if in main view, closes the Field Manual
	protected void CloseMenuOrReadingPanel()
	{
		if (m_bIsInEntryViewMode && !m_bOpenedFromOutside)
		{
			CloseReadingPanel();
		}
		else
		{
			Close();
		}
	}

	//------------------------------------------------------------------------------------------------
	//----- STATIC METHODS
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! opens the first entry matching the EFieldManualEntryId enum value
	static SCR_FieldManualUI Open(EFieldManualEntryId entryId)
	{
		SCR_FieldManualUI ui = SCR_FieldManualUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.FieldManualDialog));
		ui.OpenEntry(entryId);
		return ui;
	}

	//------------------------------------------------------------------------------------------------
	//----- my little helper!
	//------------------------------------------------------------------------------------------------
	//! joins strings with a stringtable separator - from 0 to 5 entries
	protected static string ArrayJoin(notnull array<string> entries)
	{
		int count = entries.Count();
		switch (count)
		{
			case 0: return "-";
			case 1: return entries[0];
			case 2: return WidgetManager.Translate("#AR-General_List_Short_2", entries[0], entries[1]);
			case 3: return WidgetManager.Translate("#AR-General_List_Short_3", entries[0], entries[1], entries[2]);
			case 4: return WidgetManager.Translate("#AR-General_List_Short_4", entries[0], entries[1], entries[2], entries[3]);
			case 5: return WidgetManager.Translate("#AR-General_List_Short_5", entries[0], entries[1], entries[2], entries[3], entries[4]);
		}
		Print(string.Format("Too many list entries to join as a string - missing #AR-General_List_Short_%1 translation?", count), LogLevel.WARNING);
		return WidgetManager.Translate("#AR-General_List_Short_5", entries[0], entries[1], entries[2], entries[3], entries[4]);
	}
};
