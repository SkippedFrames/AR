//------------------------------------------------------------------------------------------------
void MarkerPlacedInvoker(int posX, int posY, bool isPublic);	// world pos X, world pos Y, is visible to everyone or just the player who placed it
typedef func MarkerPlacedInvoker;

//------------------------------------------------------------------------------------------------
//! Markers UI map component 
class SCR_MapMarkersUI : SCR_MapUIBaseComponent
{
	[Attribute("#AR-MapMarker_ParentCategory", UIWidgets.Auto, "Menu category name" )]
	protected string m_sCategoryName;
	
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", UIWidgets.ResourceNamePicker, desc: "Icons imageset", params: "imageset" )]
	protected ResourceName m_sIconImageset;
	
	[Attribute("scenarios", UIWidgets.Auto, "Category icon quad" )]
	protected string m_sCategoryIconName;
	
	[Attribute("{46C46D97D1FE6241}UI/layouts/Map/MapMarkerEditBox.layout", UIWidgets.ResourceNamePicker, desc: "Edit box dialog when placing custom marker", params: "layout" )]
	protected ResourceName m_sEditBoxLayout;
	
	[Attribute("{DEA2D3B788CDCB4F}UI/layouts/Map/MapIconSelectorEntry.layout", UIWidgets.ResourceNamePicker, desc: "Icon selection entry layout", params: "layout" )]
	protected ResourceName m_sSelectorIconEntry;
	
	[Attribute("{8A5D43FC8AC6C171}UI/layouts/Map/MapColorSelectorEntry.layout", UIWidgets.ResourceNamePicker, desc: "Color selection entry layout", params: "layout" )]
	protected ResourceName m_sSelectorColorEntry;
	
	[Attribute("cancel", UIWidgets.Auto, "Delete icon quad" )]
	protected string m_sDeleteIconName;
	
	[Attribute("20", UIWidgets.Auto, "Icon selector entries per line" )]
	protected int m_iIconsPerLine;

	protected const int USERID_EDITBOX = 1000;	// unique id set to editbox allowing us to find it in case there are other editboxes
	protected const string ICON_ENTRY = "IconEntry";
	protected const string COLOR_ENTRY = "ColorEntry";
	protected const string ICON_SELECTOR = "IconSelector";
	protected const string COLOR_SELECTOR = "ColorSelector";
	protected const ResourceName SELECTOR_LINE = "{CF8EC7A0D310A8D9}UI/layouts/Map/MapColorSelectorLine.layout";
	
	protected ref Color BACKGROUND_DEFAULT = new Color(4,4,4,255);
	protected ref Color BACKGROUND_SELECTED = new Color(16,16,16,255);
 	
	protected int m_bIsDelayed;										// used to delay input context for marker dialog by one frame so it doesnt trigger inputs used to open it
	protected int m_iIconEntryCount;								// how many icon entries are within the current tab
	protected int m_iIconLines;								
	protected SCR_MapMarkerEntryPlaced m_PlacedMarkerConfig;		// saved entry for custom text placed markers
	protected Widget m_MarkerEditRoot;
	protected Widget m_IconSelector;
	protected ImageWidget m_wMarkerPreview;
	protected ImageWidget m_wMarkerPreviewGlow;
	protected TextWidget m_wMarkerPreviewText;
	protected SCR_TabViewComponent m_TabComponent;
	protected SCR_EditBoxComponent m_EditBoxComp;
	protected SCR_SliderComponent m_SliderComp;
	
	// Placed marker attributes
	protected int m_iWantedIconEntry;
	protected int m_iSelectedIconID;							// used for selecting icon when edit dialog is confirmed
	protected int m_iSelectedColorID;
	protected float m_fRotation;
	protected string m_sEditBoxText; 
	protected SCR_ButtonBaseComponent m_SelectedIconButton;		// used for (un)coloring and selecting proper button when navigating on controller
	protected SCR_ButtonBaseComponent m_SelectedColorButton;
	
	protected SCR_MapMarkerManagerComponent m_MarkerMgr;
	protected SCR_SelectionMenuCategoryEntry m_RootCategoryEntry;
	protected SCR_SelectionMenuEntry m_MarkerRemoveEntry;
	protected SCR_MapMarkerBase m_RemovableMarker;
	
	protected ref ScriptInvokerBase<MarkerPlacedInvoker> m_OnCustomMarkerPlaced = new ScriptInvokerBase<MarkerPlacedInvoker>();
	
	protected ref map<SCR_ButtonBaseComponent, int> m_mIconIDs = new map<SCR_ButtonBaseComponent, int>();		// map icon buttons to config ids
	protected ref map<SCR_ButtonBaseComponent, int> m_mColorIDs = new map<SCR_ButtonBaseComponent, int>();	// map color buttons to config ids
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<MarkerPlacedInvoker> GetOnCustomMarkerPlaced()
	{
		return m_OnCustomMarkerPlaced; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether the marker is owned by the local player
	static bool IsOwnedMarker(notnull SCR_MapMarkerBase marker)
	{
		PlayerController localController = GetGame().GetPlayerController();
		if (localController)
		{
			if (marker.GetMarkerOwnerID() == localController.GetPlayerId())
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn a placed marker 
	protected void CreateMarkerPlaced(SCR_MapMarkerMenuEntry entry)
	{		
		if (entry.GetMarkerType() == SCR_EMapMarkerType.PLACED_CUSTOM)
		{
			CreateMarkerEditDialog();
		}		
		else if (entry.GetMarkerType() == SCR_EMapMarkerType.PLACED_MILITARY)			
		{
			float wX, wY;
			m_MapEntity.GetMapCenterWorldPosition(wX, wY);
			
			SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
			marker.SetType(entry.GetMarkerType());
			marker.SetWorldPos(wX, wY);
			marker.SetMarkerConfigID(entry.GetMarkerConfigID());
			m_MarkerMgr.InsertStaticMarker(marker);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create custom marker dialog
	//! \param tabID is ID of selected tabWidget tab, if not set first is default
	//! \param focusedIconEntry is ID of selected icon, if not set first is default
	protected void CreateMarkerEditDialog(int tabID = 0, int selectedIconEntry = -1, int selectedColorEntry = -1)
	{
		if (m_MarkerEditRoot)
			CleanupMarkerEditWidget();
		
		m_iWantedIconEntry = selectedIconEntry;
		
		m_MarkerEditRoot = GetGame().GetWorkspace().CreateWidgets(m_sEditBoxLayout, m_RootWidget);
		
		float screenX, screenY;
		m_MapEntity.GetMapWidget().GetScreenSize(screenX, screenY);
		FrameSlot.SetPos(m_MarkerEditRoot, GetGame().GetWorkspace().DPIUnscale(screenX * 0.5), GetGame().GetWorkspace().DPIUnscale(screenY * 0.5));
		
		m_wMarkerPreview = ImageWidget.Cast(m_MarkerEditRoot.FindAnyWidget("MarkerIcon"));
		m_wMarkerPreviewGlow = ImageWidget.Cast(m_MarkerEditRoot.FindAnyWidget("MarkerIconGlow"));
		m_wMarkerPreviewText = TextWidget.Cast(m_MarkerEditRoot.FindAnyWidget("MarkerText"));
		
		InitColorIcons(selectedColorEntry);
		
		Widget sliderRotation = m_MarkerEditRoot.FindAnyWidget("SliderRoot");
		m_SliderComp = SCR_SliderComponent.Cast(sliderRotation.FindHandler(SCR_SliderComponent));
		m_SliderComp.m_OnChanged.Insert(OnSliderChanged);
		
		Widget categoryTab = m_MarkerEditRoot.FindAnyWidget("MarkerEditTab");
		m_TabComponent = SCR_TabViewComponent.Cast(categoryTab.FindHandler(SCR_TabViewComponent));
		
		array<ref SCR_MarkerIconCategory> categoriesArr = m_PlacedMarkerConfig.GetIconCategories();
		foreach (SCR_MarkerIconCategory category : categoriesArr)
		{
			m_TabComponent.AddTab(string.Empty, category.m_sName, identifier: category.m_sIdentifier);
		}
		
		m_TabComponent.m_OnChanged.Insert(OnTabChanged);
		m_TabComponent.ShowTab(tabID, true, false);
		
		Widget editBoxRoot = m_MarkerEditRoot.FindAnyWidget("EditBoxRoot");
		editBoxRoot.FindAnyWidget("EditBox").SetUserID(USERID_EDITBOX);
		m_EditBoxComp = SCR_EditBoxComponent.Cast(editBoxRoot.FindHandler(SCR_EditBoxComponent));
		m_EditBoxComp.m_OnTextChange.Insert(OnEditBoxTextChanged);
		m_EditBoxComp.SetValue(string.Empty);
		
		SCR_InputButtonComponent confirmComp = SCR_InputButtonComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ButtonPublic").FindHandler(SCR_InputButtonComponent));
		confirmComp.m_OnClicked.Insert(OnEditConfirmed);
		
		confirmComp = SCR_InputButtonComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ButtonPrivate").FindHandler(SCR_InputButtonComponent));
		confirmComp.m_OnClicked.Insert(OnEditConfirmedPrivate);
		
		confirmComp = SCR_InputButtonComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ButtonCancel").FindHandler(SCR_InputButtonComponent));
		confirmComp.m_OnClicked.Insert(CleanupMarkerEditWidget);
			
		FocusWidget(m_SelectedIconButton.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FocusWidget(Widget widget)
	{
		GetGame().GetWorkspace().SetFocusedWidget(widget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init colors
	protected void InitColorIcons(int selectedColorEntry)
	{
		array<ref SCR_MarkerColorEntry> colorsArr = m_PlacedMarkerConfig.GetColorEntries();
		Widget colorSelector = m_MarkerEditRoot.FindAnyWidget(COLOR_SELECTOR);		
		Widget colorSelectorLine = colorSelector.FindAnyWidget("ColorSelectorLine");
		SCR_ButtonImageComponent firstColorEntry;
		
		m_mColorIDs.Clear();
		m_SelectedColorButton = null;
		
		foreach (int i, SCR_MarkerColorEntry colorEntry : colorsArr)
		{			
			Widget button = GetGame().GetWorkspace().CreateWidgets(m_sSelectorColorEntry, colorSelectorLine);
			button.SetName(COLOR_ENTRY + i.ToString());
			SCR_ButtonImageComponent buttonComp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
			buttonComp.GetImageWidget().SetColor(colorEntry.GetColor());
			buttonComp.m_OnClicked.Insert(OnColorEntryClicked);
			buttonComp.m_OnFocus.Insert(OnColorEntryFocused);
			
			m_mColorIDs.Insert(buttonComp, i);
			
			if (!firstColorEntry)
				firstColorEntry = buttonComp;
		}
		
		if (selectedColorEntry == -1)
		{
			OnColorEntryClicked(firstColorEntry);
		}
		else
		{
			SCR_ButtonBaseComponent buttonComp = m_mColorIDs.GetKeyByValue(selectedColorEntry);
			if (buttonComp)
				OnColorEntryClicked(buttonComp);
			else 
				OnColorEntryClicked(firstColorEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init icons by category
	protected void InitCategoryIcons(SCR_TabViewContent tabContent)
	{
		m_mIconIDs.Clear();
		m_SelectedIconButton = null;
		
		string imageset, imagesetGlow, quad;
		
		m_IconSelector = m_MarkerEditRoot.FindAnyWidget(ICON_SELECTOR);		
		array<ref SCR_MarkerIconEntry> iconsArr = m_PlacedMarkerConfig.GetIconEntries();
				
		Widget child = m_IconSelector.GetChildren();
		while (child)
		{
			child.RemoveFromHierarchy();
			child = m_IconSelector.GetChildren();
		}
		
		Widget iconSelectorLine = GetGame().GetWorkspace().CreateWidgets(SELECTOR_LINE, m_IconSelector);
		m_iIconEntryCount = 0;
		m_iIconLines = 1;
				
		SCR_ButtonImageComponent firstEntry;
		
		foreach (int i, SCR_MarkerIconEntry iconEntry : iconsArr)
		{
			if (iconEntry.m_sCategoryIdentifier != tabContent.m_sTabIdentifier)
				continue;
			
			m_iIconEntryCount++;
			if (m_iIconEntryCount > m_iIconsPerLine * m_iIconLines)
			{
				iconSelectorLine = GetGame().GetWorkspace().CreateWidgets(SELECTOR_LINE, m_IconSelector);
				m_iIconLines++;
			}

			Widget button = GetGame().GetWorkspace().CreateWidgets(m_sSelectorIconEntry, iconSelectorLine);
			button.SetName(ICON_ENTRY + m_iIconEntryCount.ToString());
			SCR_ButtonImageComponent buttonComp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
			m_mIconIDs.Insert(buttonComp, i);
			if (!firstEntry)
				firstEntry = buttonComp;
			
			iconEntry.GetIconResource(imageset, imagesetGlow, quad);
			buttonComp.SetImage(imageset, quad);
			buttonComp.m_OnClicked.Insert(OnIconEntryClicked);
			buttonComp.m_OnFocus.Insert(OnIconEntryFocused);
		}
		
		if (m_iWantedIconEntry == -1)
		{
			OnIconEntryClicked(firstEntry);
		}
		else
		{
			SCR_ButtonBaseComponent buttonComp = m_mIconIDs.GetKeyByValue(m_iWantedIconEntry);
			if (buttonComp)
				OnIconEntryClicked(buttonComp);
			else 
				OnIconEntryClicked(firstEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init radial menu marker entries along with their visual previews
	protected void InitFactionPlacedMarkers(SCR_MapMarkerConfig markerConfig, SCR_MapRadialUI radialUI)
	{
		SCR_MapMarkerEntryMilitary milConf = SCR_MapMarkerEntryMilitary.Cast(markerConfig.GetMarkerEntryConfigByType(SCR_EMapMarkerType.PLACED_MILITARY));
		if (!milConf)
			return;
		
		array<ref SCR_MarkerMilitaryFactionEntry> milFactionEntries = milConf.GetMilitaryFactionEntries();
		array<ref SCR_MarkerMilitaryEntry> milEntries = milConf.GetMilitaryEntries();
		
		if (milFactionEntries.IsEmpty() || milEntries.IsEmpty())
			return;
		
		foreach (int i, SCR_MarkerMilitaryFactionEntry milFaction : milFactionEntries)
		{			
			SCR_MapMarkerMenuCategory categoryEntry = new SCR_MapMarkerMenuCategory();
			categoryEntry.SetMarkerType(SCR_EMapMarkerType.PLACED_MILITARY);
			categoryEntry.SetLayout();
			categoryEntry.SetSymbolProps(milFaction.GetFactionIdentity(), milFaction.GetColor());
			radialUI.InsertCustomRadialCategory(categoryEntry, m_RootCategoryEntry);
			
			foreach (SCR_MarkerMilitaryEntry milEntry : milEntries)
			{
				SCR_MapMarkerMenuEntry menuEntry = new SCR_MapMarkerMenuEntry();
				menuEntry.SetMarkerType(SCR_EMapMarkerType.PLACED_MILITARY);
				menuEntry.SetLayout();
				menuEntry.SetName(milEntry.GetDescription());
				menuEntry.GetOnPerform().Insert(OnEntryPerformed);
				menuEntry.SetSymbolProps(milFaction.GetFactionIdentity(), milFaction.GetColor(), milEntry.GetDimension(), milEntry.GetIcons(), milEntry.GetAmplifier());
						
				menuEntry.SetMarkerConfigID(i * 1000 + milEntry.GetEntryID());

				radialUI.InsertCustomRadialEntry(menuEntry, categoryEntry);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create local markers
	protected void CreateLocalMarkers()
	{
		array<ref SCR_MapMarkerBase> markers = m_MarkerMgr.GetLocalMarkers();
		foreach (SCR_MapMarkerBase marker : markers)
		{
			marker.OnCreateMarker();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create static markers
	protected void CreateStaticMarkers()
	{
		array<ref SCR_MapMarkerBase> markersSimple = m_MarkerMgr.GetStaticMarkers();
		for (int i; i < markersSimple.Count(); i++)
		{
			if (!markersSimple.IsIndexValid(i))
				continue;
			
			SCR_MapMarkerBase marker = markersSimple[i];
			Faction markerFaction = SCR_FactionManager.SGetPlayerFaction(marker.GetMarkerOwnerID());
			Faction localFaction = SCR_FactionManager.SGetLocalPlayerFaction();
			if ( (marker.GetMarkerOwnerID() != GetGame().GetPlayerController().GetPlayerId()) && (!localFaction || localFaction.IsFactionEnemy(markerFaction)))
			{
				if (Replication.IsServer())				// if server, enemy markers have to be kept for sync but are disabled
					marker.SetServerDisabled(true);
				else 									// if client, enemy marker can be safely removed
				{
					markersSimple.RemoveItem(marker);
					i--;
					continue;
				}
			}
			
			marker.OnCreateMarker();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create dynamic markers
	protected void CreateDynamicMarkers()
	{
		array<SCR_MapMarkerEntity> markersDynamic = m_MarkerMgr.GetDynamicMarkers();
		foreach (SCR_MapMarkerEntity marker : markersDynamic)
		{
			marker.OnCreateMarker();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Remove callback from radial menu
	protected void RemoveMarkerMenu()
	{
		RemoveOwnedMarker(m_RemovableMarker);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Attemt to remove marker, only works if owned
	protected void RemoveOwnedMarker(SCR_MapMarkerBase marker)
	{		
		if (!marker)
			return;
		
		if (marker.GetMarkerID() == -1)		// basic
			m_MarkerMgr.RemoveLocalMarker(marker);
		else 
			m_MarkerMgr.RemoveStaticMarker(marker);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Marker edit widget removal
	protected void CleanupMarkerEditWidget()
	{
		if (m_MarkerEditRoot)
			m_MarkerEditRoot.RemoveFromHierarchy();
		
		m_bIsDelayed = false;
	}
	
	//------------------------------------------------------------------------------------------------
	// EVENTS
	//------------------------------------------------------------------------------------------------
	//! SCR_TabViewComponent event
	protected void OnTabChanged(SCR_TabViewComponent tabView, Widget widget, int index)
	{
		SCR_TabViewContent tab = tabView.GetEntryContent(index);
		
		InitCategoryIcons(tab);
		m_SliderComp.SetValue(0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnColorEntryClicked(SCR_ButtonBaseComponent component)
	{		
		if (m_SelectedColorButton)
		{
			m_SelectedColorButton.SetBackgroundColors(BACKGROUND_DEFAULT);
			m_SelectedColorButton.ColorizeBackground(false);
		}
		
		component.SetBackgroundColors(BACKGROUND_SELECTED);
		component.ColorizeBackground(false);	// this will color the button to hover color for KBM
		m_SelectedColorButton = component;
		m_iSelectedColorID = m_mColorIDs.Get(component);
		
		m_wMarkerPreview.SetColor(m_PlacedMarkerConfig.GetColorEntry(m_iSelectedColorID));
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnColorEntryFocused(Widget rootW)
	{
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;
		
		SCR_ButtonBaseComponent buttonComp = SCR_ButtonBaseComponent.Cast(rootW.FindHandler(SCR_ButtonBaseComponent));
		if (buttonComp)
			OnColorEntryClicked(buttonComp);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnIconEntryClicked(notnull SCR_ButtonBaseComponent component)
	{		
		if (m_SelectedIconButton)
		{
			m_SelectedIconButton.SetBackgroundColors(BACKGROUND_DEFAULT);
			m_SelectedIconButton.ColorizeBackground(false);
		}
		
		component.SetBackgroundColors(BACKGROUND_SELECTED);
		component.ColorizeBackground(false);	// this will color the button to hover color for KBM
		m_SelectedIconButton = component;
		m_iSelectedIconID = m_mIconIDs.Get(component);
		
		ResourceName imageset, imagesetGlow;
		string quad;
		m_PlacedMarkerConfig.GetIconEntry(m_iSelectedIconID, imageset, imagesetGlow, quad);
		
		m_wMarkerPreview.LoadImageFromSet(0, imageset, quad);
		m_wMarkerPreviewGlow.LoadImageFromSet(0, imagesetGlow, quad);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnIconEntryFocused(Widget rootW)
	{
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;
		
		SCR_ButtonBaseComponent buttonComp = SCR_ButtonBaseComponent.Cast(rootW.FindHandler(SCR_ButtonBaseComponent));
		if (buttonComp)
			OnIconEntryClicked(buttonComp);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_EditBoxComponent event
	protected void OnEditBoxTextChanged(string text)
	{
		m_wMarkerPreviewText.SetText(text);
		m_sEditBoxText = text;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_SliderComponent event
	protected void OnSliderChanged(SCR_SliderComponent slider, float value)
	{
		m_wMarkerPreview.SetRotation(value);
		m_wMarkerPreviewGlow.SetRotation(value);
		m_fRotation = value;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonTextComponent event
	protected void OnEditConfirmed(SCR_InputButtonComponent button)
	{
		OnInsertMarker(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonTextComponent event
	protected void OnEditConfirmedPrivate(SCR_InputButtonComponent button)
	{
		OnInsertMarker(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInsertMarker(bool isPublic)
	{
		float wX, wY;
		m_MapEntity.GetMapCenterWorldPosition(wX, wY);
		
		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
		marker.SetWorldPos(wX, wY);
		marker.SetRotation(m_fRotation);
		marker.SetColorEntry(m_iSelectedColorID);
		marker.SetIconEntry(m_iSelectedIconID);
		marker.SetCustomText(m_sEditBoxText);
		
		if (isPublic)
			m_MarkerMgr.InsertStaticMarker(marker);
		else
			m_MarkerMgr.InsertLocalMarker(marker);
		
		m_OnCustomMarkerPlaced.Invoke(wX, wY, isPublic);
		
		CleanupMarkerEditWidget();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapRadialUI event
	protected void OnRadialMenuInit()
	{
		SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
		if (!markerConfig)
			return;
		
		SCR_MapRadialUI radialUI = SCR_MapRadialUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapRadialUI));

		m_RootCategoryEntry = radialUI.AddRadialCategory(m_sCategoryName);
		m_RootCategoryEntry.SetIcon(m_sIconImageset, m_sCategoryIconName);
		
		array<ref SCR_MapMarkerEntryConfig> entryConfigs = markerConfig.GetMarkerEntryConfigs();
		
		foreach (SCR_MapMarkerEntryConfig entry : entryConfigs)		// menu entries
		{			
			if (entry.GetMarkerType() == SCR_EMapMarkerType.PLACED_MILITARY)
			{
				InitFactionPlacedMarkers(markerConfig, radialUI);
			}
			else if (entry.GetMarkerType() == SCR_EMapMarkerType.PLACED_CUSTOM)
			{
				SCR_MapMarkerEntryPlaced entryPlaced = SCR_MapMarkerEntryPlaced.Cast(entry);
				
				SCR_MapMarkerMenuEntry menuEntry = new SCR_MapMarkerMenuEntry();
				menuEntry.SetMarkerType(SCR_EMapMarkerType.PLACED_CUSTOM);
				menuEntry.SetName(entryPlaced.GetMenuDescription());
				menuEntry.GetOnPerform().Insert(OnEntryPerformed);
				menuEntry.SetIcon(entryPlaced.GetMenuImageset(), entryPlaced.GetMenuIcon());
	
				radialUI.InsertCustomRadialEntry(menuEntry, m_RootCategoryEntry);
			}
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_RadialMenuController event
	protected void OnRadialMenuOpen(SCR_RadialMenuController controller)
	{		
		SCR_MapRadialUI radialUI = SCR_MapRadialUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapRadialUI));
		if (m_MarkerRemoveEntry)
		{
			radialUI.RemoveRadialEntry(m_MarkerRemoveEntry);
			m_RemovableMarker = null;
		}
		
		// delete marker button
		array<Widget> widgets = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
		
		SCR_MapMarkerWidgetComponent markerComp;
		foreach ( Widget widget : widgets )
		{
			markerComp = SCR_MapMarkerWidgetComponent.Cast(widget.FindHandler(SCR_MapMarkerWidgetComponent));	
			if (!markerComp)
				continue;
						
			SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
			if (marker)
			{
				if (!IsOwnedMarker(marker) || (marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM && marker.GetType() != SCR_EMapMarkerType.PLACED_MILITARY))
					continue;
				
				m_MarkerRemoveEntry = radialUI.AddRadialEntry("#AR-MapMarker_DeleteHint");
				m_MarkerRemoveEntry.SetIcon(m_sIconImageset, m_sDeleteIconName);
				m_MarkerRemoveEntry.GetOnPerform().Insert(RemoveMarkerMenu);
				
				m_RemovableMarker = marker;
				
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapMarkerMenuEntry event
	protected void OnEntryPerformed(SCR_SelectionMenuEntry entry)
	{
		SCR_MapMarkerMenuEntry markerEntry = SCR_MapMarkerMenuEntry.Cast(entry);
		CreateMarkerPlaced(markerEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Quick open input for marker category
	protected void OnInputQuickMarkerMenu(float value, EActionTrigger reason)
	{
		SCR_MapRadialUI mapRadial = SCR_MapRadialUI.GetInstance();
		if (!mapRadial)
			return;
		
		mapRadial.GetRadialController().OnInputOpen();
		mapRadial.GetRadialController().GetRadialMenu().PerformEntry(m_RootCategoryEntry);
	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Marker delete quickbind
	protected void OnInputMarkerDelete(float value, EActionTrigger reason)
	{
		array<Widget> widgets = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
		
		SCR_MapMarkerWidgetComponent markerComp;
		foreach ( Widget widget : widgets )
		{
			markerComp = SCR_MapMarkerWidgetComponent.Cast(widget.FindHandler(SCR_MapMarkerWidgetComponent));	
			if (!markerComp)
				continue;
						
			SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
			if (marker)
			{
				if (!IsOwnedMarker(marker) || (marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM && marker.GetType() != SCR_EMapMarkerType.PLACED_MILITARY))
					continue;
				
				RemoveOwnedMarker(marker);
			}
				
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Marker select
	protected void OnInputMapSelect(float value, EActionTrigger reason)
	{
		array<Widget> widgets = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
		
		SCR_MapMarkerWidgetComponent markerComp;
		foreach ( Widget widget : widgets )
		{
			markerComp = SCR_MapMarkerWidgetComponent.Cast(widget.FindHandler(SCR_MapMarkerWidgetComponent));	
			if (!markerComp)
				continue;
						
			SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
			if (!marker)
				continue;
		
			if (IsOwnedMarker(marker) && marker.GetType() == SCR_EMapMarkerType.PLACED_CUSTOM)
			{	
				int wPos[2];
				float screenX, screenY;
				marker.GetWorldPos(wPos);
				m_MapEntity.WorldToScreen(wPos[0], wPos[1], screenX, screenY);
				m_MapEntity.PanSmooth(screenX, screenY, 0.1);
								
				CreateMarkerEditDialog(m_PlacedMarkerConfig.GetIconCategoryID(marker.GetIconEntry()), marker.GetIconEntry(), marker.GetColorEntry());
																
				m_EditBoxComp.SetValue(marker.GetCustomText());
				m_SliderComp.SetValue(marker.GetRotation());
				
				RemoveOwnedMarker(marker);
				
				break;
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Menu select/confirm bind
	protected void OnInputMenuConfirm(float value, EActionTrigger reason)
	{
		if (!m_bIsDelayed)
			return;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focused || focused.GetUserID() == USERID_EDITBOX)
			return;
		
		if (m_MarkerEditRoot)
			OnInsertMarker(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Menu alternative confirm bind
	protected void OnInputMenuConfirmAlter(float value, EActionTrigger reason)
	{
		if (m_MarkerEditRoot)
			OnInsertMarker(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Menu back/escape bind
	protected void OnInputMenuBack(float value, EActionTrigger reason)
	{
		if (m_MarkerEditRoot)
			CleanupMarkerEditWidget();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputMenuDown(float value, EActionTrigger reason)
	{
		string name;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (focused)
			name = focused.GetName();
		
		if (name.Contains(ICON_ENTRY))
			FocusWidget(m_SelectedColorButton.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputMenuUp(float value, EActionTrigger reason)
	{
		string name;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focused)
			return;
		
		name = focused.GetName();
		
		if (name.Contains(COLOR_ENTRY))
			FocusWidget(m_SelectedIconButton.GetRootWidget());
		else if (focused.GetUserID() == USERID_EDITBOX)
			FocusWidget(m_SelectedColorButton.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputMenuLeft(float value, EActionTrigger reason)
	{
		string name;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (focused)
			name = focused.GetName();
		
		if (!name.Contains(ICON_ENTRY))
			return;
		
		int pos = name.ToInt(offset: 9);
		int target = pos - 1;
		
		if (target < 1)
			target = m_iIconEntryCount;
		
		Widget w = m_IconSelector.FindAnyWidget(ICON_ENTRY + target);
		if (w)
			FocusWidget(w);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputMenuRight(float value, EActionTrigger reason)
	{
		string name;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (focused)
			name = focused.GetName();
		
		if (!name.Contains(ICON_ENTRY))
			return;
		
		int pos = name.ToInt(offset: 9);
		int target = pos + 1;
		
		if (target > m_iIconEntryCount)
			target = 1;
		
		Widget w = m_IconSelector.FindAnyWidget(ICON_ENTRY + target);
		if (w)
			FocusWidget(w);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapToolInteractionUI event
	protected void OnDragWidget(Widget widget)
	{
		SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
		if (marker)
		{
			if (!IsOwnedMarker(marker) || marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM)
				return;
			
			marker.SetDragged(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapToolInteractionUI event
	protected void OnDragEnd(Widget widget)
	{
		SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
		if (marker)
		{
			if (!IsOwnedMarker(marker) || marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM)
				return;
			
			marker.SetDragged(false);
			vector pos = FrameSlot.GetPos(widget);

			float wX, wY;
			m_MapEntity.ScreenToWorld(GetGame().GetWorkspace().DPIScale(pos[0]), GetGame().GetWorkspace().DPIScale(pos[1]), wX, wY);
			
			SCR_MapMarkerBase markerNew = new SCR_MapMarkerBase();
			markerNew.SetType(marker.GetType());
			markerNew.SetWorldPos(wX, wY);
			markerNew.SetColorEntry(marker.GetColorEntry());
			markerNew.SetIconEntry(marker.GetIconEntry());
			markerNew.SetCustomText(marker.GetCustomText());
			markerNew.SetRotation(marker.GetRotation());
			
			int markerID = marker.GetMarkerID();
			RemoveOwnedMarker(marker);
			
			if (markerID != -1)
				m_MarkerMgr.InsertStaticMarker(markerNew);
			else
				m_MarkerMgr.InsertLocalMarker(markerNew);
			
			m_OnCustomMarkerPlaced.Invoke(wX, wY, marker.GetMarkerID() != -1);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		m_MarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
		if (!markerConfig)
			return;
		
		m_PlacedMarkerConfig = SCR_MapMarkerEntryPlaced.Cast(markerConfig.GetMarkerEntryConfigByType(SCR_EMapMarkerType.PLACED_CUSTOM));
		
		CreateLocalMarkers();
		CreateStaticMarkers();
		CreateDynamicMarkers();
		
		m_MarkerMgr.EnableUpdate(true);		// run frame update manager side
		
		GetGame().GetInputManager().AddActionListener("MapQuickMarkerMenu", EActionTrigger.DOWN, OnInputQuickMarkerMenu);
		GetGame().GetInputManager().AddActionListener("MapMarkerDelete", EActionTrigger.DOWN, OnInputMarkerDelete);
		GetGame().GetInputManager().AddActionListener("MapSelect", EActionTrigger.DOWN, OnInputMapSelect);
		GetGame().GetInputManager().AddActionListener("MenuSelect", EActionTrigger.DOWN, OnInputMenuConfirm);
		GetGame().GetInputManager().AddActionListener("MenuRefresh", EActionTrigger.DOWN, OnInputMenuConfirmAlter);
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnInputMenuBack);
		GetGame().GetInputManager().AddActionListener("MenuDown", EActionTrigger.DOWN, OnInputMenuDown);
		GetGame().GetInputManager().AddActionListener("MenuUp", EActionTrigger.DOWN, OnInputMenuUp);
		GetGame().GetInputManager().AddActionListener("MenuRight", EActionTrigger.DOWN, OnInputMenuRight);
		GetGame().GetInputManager().AddActionListener("MenuLeft", EActionTrigger.DOWN, OnInputMenuLeft);
		
		if ( SCR_MapToolInteractionUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolInteractionUI)) )	// if dragging available, add callback
		{
			SCR_MapToolInteractionUI.GetOnDragWidgetInvoker().Insert(OnDragWidget);
			SCR_MapToolInteractionUI.GetOnDragEndInvoker().Insert(OnDragEnd);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{				
		CleanupMarkerEditWidget();
		GetGame().GetInputManager().RemoveActionListener("MapQuickMarkerMenu", EActionTrigger.DOWN, OnInputQuickMarkerMenu);
		GetGame().GetInputManager().RemoveActionListener("MapMarkerDelete", EActionTrigger.DOWN, OnInputMarkerDelete);
		GetGame().GetInputManager().RemoveActionListener("MapSelect", EActionTrigger.DOWN, OnInputMapSelect);
		GetGame().GetInputManager().RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnInputMenuConfirm);
		GetGame().GetInputManager().RemoveActionListener("MenuRefresh", EActionTrigger.DOWN, OnInputMenuConfirmAlter);
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnInputMenuBack);
		GetGame().GetInputManager().RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnInputMenuDown);
		GetGame().GetInputManager().RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnInputMenuUp);
		GetGame().GetInputManager().RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnInputMenuRight);
		GetGame().GetInputManager().RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnInputMenuLeft);
		
		m_MarkerMgr.EnableUpdate(false);
		super.OnMapClose(config);
	}

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		SCR_MapRadialUI radialMenu = SCR_MapRadialUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapRadialUI));
		if (radialMenu)
		{
			radialMenu.GetOnMenuInitInvoker().Insert(OnRadialMenuInit);
			radialMenu.GetRadialController().GetOnInputOpen().Insert(OnRadialMenuOpen);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (m_MarkerEditRoot)
		{
			if (!m_bIsDelayed)
			{
				m_bIsDelayed = true;
				return;
			}
			
			GetGame().GetInputManager().ActivateContext("MapMarkerEditContext");
		}
	}
}
