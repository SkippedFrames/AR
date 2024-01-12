//! Component responsible for requesting and visualization of available loadouts in deploy menu.
class SCR_LoadoutRequestUIComponent : SCR_DeployRequestUIBaseComponent
{
	[Attribute("LoadoutList", desc: "List for available player's loadouts' buttons")]
	protected string m_sLoadoutList;
	protected GridLayoutWidget m_wLoadoutList;

	[Attribute("LoadoutPreview")]
	protected string m_sLoadoutPreview;
	protected Widget m_wLoadoutPreview;

	[Attribute("LoadoutIcon")]
	protected string m_sLoadoutIcon;
	protected ImageWidget m_wLoadoutIcon;

	[Attribute("LoadoutName")]
	protected string m_sLoadoutName;
	protected TextWidget m_wLoadoutName;

	[Attribute("{39D815C843414C76}UI/layouts/Menus/DeployMenu/LoadoutButton.layout", desc: "Layout for loadout button, has to have SCR_LoadoutButton attached to it.")]
	protected ResourceName m_sLoadoutButton;

	[Attribute("Selector")]
	protected string m_sLoadoutSelector;
	protected Widget m_wLoadoutSelector;

	[Attribute("LoadoutSelectorRoot")]
	protected string m_sRoot;

	protected SCR_LoadoutGallery m_LoadoutSelector;

	protected SCR_LoadoutManager m_LoadoutManager;
	protected SCR_PlayerLoadoutComponent m_PlyLoadoutComp;
	protected SCR_LoadoutPreviewComponent m_PreviewComp;
	protected SCR_ArsenalManagerComponent m_ArsenalManagerComp;
	protected SCR_PlayerFactionAffiliationComponent m_PlyFactionAffilComp;
	protected IEntity m_PreviewedEntity;

	protected Widget m_wLoadouty;
	protected Widget m_wSupplies;
	protected RichTextWidget m_wSuppliesText;

	protected const int LOADOUTS_PER_ROW = 2;
	
	protected ref ScriptInvokerInt m_OnPlayerEntryFocused;
	protected ref ScriptInvokerWidget m_OnPlayerEntryFocusLost;

	//----------------------------------------------------------------------------------------------
	int GetLoadoutCost(SCR_BasePlayerLoadout playerLoadout)
	{
		int cost;
		
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return 0;
			
		SCR_Faction faction = SCR_Faction.Cast(m_PlyFactionAffilComp.GetAffiliatedFaction());
		if (!faction)
			return 0;
			
		ResourceName loadoutResource = playerLoadout.GetLoadoutResource();
		if (!loadoutResource)
			return 0;
			
		Resource resource = Resource.Load(loadoutResource);
		if (!resource)
			return 0;
			
		SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.CHARACTER, resource.GetResource().GetResourceName(), faction);
		if (!entry)
			return 0;
			
		SCR_EntityCatalogSpawnerData data = SCR_EntityCatalogSpawnerData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!data)
			return 0;
	
		cost = data.GetSupplyCost();
		
		return cost;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot = w.FindAnyWidget(m_sRoot);

		m_LoadoutManager = GetGame().GetLoadoutManager();
		if (!m_LoadoutManager)
		{
			Print("Loadout manager is missing in the world! Deploy menu won't work correctly.", LogLevel.ERROR);
			return;	
		}

		if (m_LoadoutManager)
			m_LoadoutManager.GetOnMappedPlayerLoadoutInfoChanged().Insert(OnLoadoutsChanged); // todo@lk: update loadout list
		
		m_wLoadoutIcon = ImageWidget.Cast(w.FindAnyWidget(m_sLoadoutIcon));
		m_wLoadoutName = TextWidget.Cast(w.FindAnyWidget(m_sLoadoutName));
		m_wLoadoutList = GridLayoutWidget.Cast(w.FindAnyWidget(m_sLoadoutList));

		m_wLoadouty = w.FindAnyWidget("Loadouty");
		
		m_wExpandButtonName = TextWidget.Cast(w.FindAnyWidget(m_sExpandButtonName));
		m_wExpandButtonIcon = ImageWidget.Cast(w.FindAnyWidget(m_sExpandButtonIcon));
		m_wExpandButton = w.FindAnyWidget(m_sExpandButton);

		SCR_ArsenalManagerComponent.GetArsenalManager(m_ArsenalManagerComp);
		if (m_ArsenalManagerComp)
			m_ArsenalManagerComp.GetOnLoadoutUpdated().Insert(UpdateLoadouts);

		if (m_wExpandButton && m_wExpandButton.IsVisible())
		{
			SCR_ButtonBaseComponent expandBtn = SCR_ButtonBaseComponent.Cast(m_wExpandButton.FindHandler(SCR_ButtonBaseComponent));
			expandBtn.m_OnClicked.Insert(ToggleCollapsed);
			GetOnListCollapse().Insert(OnListExpand);
		}		
		
		m_wLoadoutSelector = w.FindAnyWidget(m_sLoadoutSelector);
		if (m_wLoadoutSelector)
			m_LoadoutSelector = SCR_LoadoutGallery.Cast(m_wLoadoutSelector.FindHandler(SCR_LoadoutGallery));

		if (m_LoadoutSelector)
		{
			m_LoadoutSelector.GetOnLoadoutClicked().Insert(RequestPlayerLoadout);
			m_LoadoutSelector.GetOnLoadoutHovered().Insert(SetLoadoutPreview);
		}

		m_wLoadoutPreview = w.FindAnyWidget(m_sLoadoutPreview);
		if (m_wLoadoutPreview)
			m_PreviewComp = SCR_LoadoutPreviewComponent.Cast(m_wLoadoutPreview.FindHandler(SCR_LoadoutPreviewComponent));

		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			m_PlyLoadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
			m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (GetPlayerLoadout())
			{
				if (m_wExpandButtonName)
					m_wExpandButtonName.SetText(GetPlayerLoadout().GetLoadoutName());	
				SetLoadoutPreview(GetPlayerLoadout());
			}
		}
		
		if (m_wLoadouty)
			m_wSupplies = m_wLoadouty.FindAnyWidget("w_Supplies");
		
		if (m_wSupplies)
			m_wSuppliesText = RichTextWidget.Cast(m_wSupplies.FindAnyWidget("SuppliesText"));
	}
	
	protected override void ToggleCollapsed()
	{
		if (m_wExpandButton && m_wExpandButton.IsVisible())
		{
			bool visible = !IsExpanded();
			SetExpanded(visible);
			GetOnListCollapse().Invoke(this, visible);
		}
	}
	
	override void SetExpanded(bool expanded)
	{
		m_wLoadouty.SetVisible(expanded);
	}
	
	protected override bool IsExpanded()
	{
		return m_wLoadouty.IsVisible();
	}	
	
	protected void OnLoadoutsChanged(SCR_BasePlayerLoadout loadout, int newCount)
	{
	}

	//! Called from deploy menu when player's loadout is assigned.
	void OnPlayerLoadoutAssigned(SCR_PlayerLoadoutComponent component)
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(btn);
			if (loadoutBtn && loadoutBtn.GetPlayerId() == component.GetPlayerId())
			{
				loadoutBtn.SetLoadout(component.GetLoadout());
				break;
			}
		}
	}

	//! Show available loadouts in the loadout selector.
	void ShowAvailableLoadouts(Faction faction)
	{
		if (!m_LoadoutManager)
			return;

		ResetPlayerLoadoutPreview();

		if (!m_LoadoutSelector)
			return;

		m_LoadoutSelector.ClearAll();

		array<ref SCR_BasePlayerLoadout> availableLoadouts = {};
		m_LoadoutManager.GetPlayerLoadoutsByFaction(faction, availableLoadouts);

		SCR_PlayerArsenalLoadout arsenalLoadout = null;
		foreach (SCR_BasePlayerLoadout loadout : availableLoadouts)
		{
			if (!loadout.IsLoadoutAvailableClient())
				continue;

			if (loadout.IsInherited(SCR_PlayerArsenalLoadout))
				arsenalLoadout = SCR_PlayerArsenalLoadout.Cast(loadout);

			m_LoadoutSelector.AddItem(loadout, loadout.IsLoadoutAvailableClient());
		}
		
		if (!availableLoadouts.IsEmpty())
		{
			if (arsenalLoadout && GetPlayerLoadout() != arsenalLoadout)
				m_PlyLoadoutComp.RequestLoadout(arsenalLoadout);
			else if (!GetPlayerLoadout())
				m_PlyLoadoutComp.RequestLoadout(availableLoadouts[0]);
		}

		RefreshLoadoutPreview();		
	}

	//! Fill the loadout list with players' loadouts.
	void ShowPlayerLoadouts(array<int> playerIds, int slotCount = -1)
	{
		if (!m_wLoadoutList || !m_LoadoutManager)
			return;

		SetListVisible(true);

		ClearLoadoutList();
		
		if (!playerIds || playerIds.IsEmpty())
			return;

		for (int i = 0; i < playerIds.Count(); ++i)
		{
			int pid = playerIds[i];
			SCR_BasePlayerLoadout playerLoadout = m_LoadoutManager.GetPlayerLoadout(pid);
			CreatePlayerLoadoutButton(playerLoadout, pid, i);
		}
		
		for (int i = playerIds.Count(); i < slotCount; ++i)
		{
			CreateEmptySlot(i);
		}
	}

	//! Removed all buttons from the loadout list.
	protected void ClearLoadoutList()
	{
		Widget child = m_wLoadoutList.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}
		
		m_aButtons.Clear();
	}

	// Create a non-interactive loadout button with player's name.
	protected void CreatePlayerLoadoutButton(SCR_BasePlayerLoadout loadout, int pid, int order)
	{
		Widget name = GetGame().GetWorkspace().CreateWidgets(m_sLoadoutButton, m_wLoadoutList);
		SCR_LoadoutButton buttonComp = SCR_LoadoutButton.Cast(name.FindHandler(SCR_LoadoutButton));

		buttonComp.SetLoadout(loadout);
		buttonComp.SetPlayer(pid);
		buttonComp.SetSelected(pid == GetGame().GetPlayerController().GetPlayerId());
		buttonComp.SetEnabled(loadout.IsLoadoutAvailableClient());

		buttonComp.m_OnFocus.Insert(OnButtonFocused);
		buttonComp.m_OnFocusLost.Insert(OnButtonFocusLost);
		buttonComp.m_OnMouseEnter.Insert(OnButtonFocused);
		buttonComp.m_OnMouseLeave.Insert(OnMouseLeft);

		int cnt = m_aButtons.Insert(buttonComp);
		
		GridSlot.SetColumn(name, order % LOADOUTS_PER_ROW);
		GridSlot.SetRow(name, order / LOADOUTS_PER_ROW);
	}

	//! Create an empty slot in the loadout grid.
	protected void CreateEmptySlot(int order)
	{
		Widget slot = GetGame().GetWorkspace().CreateWidgets(m_sLoadoutButton, m_wLoadoutList);
		SCR_DeployButtonBase handler = SCR_DeployButtonBase.Cast(slot.FindHandler(SCR_DeployButtonBase));

		handler.SetShouldUnlock(false);
		m_aButtons.Insert(handler);
		
		GridSlot.SetColumn(slot, order % LOADOUTS_PER_ROW);
		GridSlot.SetRow(slot, order / LOADOUTS_PER_ROW);
	}

	protected void UpdateLoadouts(int playerID, bool hasValidLoadout)
	{
		array<ref SCR_BasePlayerLoadout> availableLoadouts = {};
		m_LoadoutManager.GetPlayerLoadoutsByFaction(m_PlyFactionAffilComp.GetAffiliatedFaction(), availableLoadouts);
		foreach (SCR_BasePlayerLoadout loadout : availableLoadouts)
		{
			SCR_LoadoutButton loadoutBtn = m_LoadoutSelector.GetButtonForLoadout(loadout);
			if (loadoutBtn)
				loadoutBtn.SetEnabled(loadout.IsLoadoutAvailableClient());
		}
	}

	//! Send a loadout request when clicking on a loadout button.
	protected void RequestPlayerLoadout(SCR_LoadoutButton loadoutBtn)
	{
		SCR_BasePlayerLoadout loadout = loadoutBtn.GetLoadout();
		if (!loadout)
			return;

		Lock(loadoutBtn);
		SetLoadoutPreview(loadout);
		
		if (m_wExpandButtonName)
			m_wExpandButtonName.SetText(loadout.GetLoadoutName());			
		
		m_PlyLoadoutComp.RequestLoadout(loadout);
	}

	protected void OnButtonFocused(Widget w)
	{
		SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(w.FindHandler(SCR_LoadoutButton));
		if (!loadoutBtn)
			return;
		
		m_OnButtonFocused.Invoke();
		
		if (m_OnPlayerEntryFocused)
			m_OnPlayerEntryFocused.Invoke(loadoutBtn.GetPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonFocusLost(Widget w)
	{
		if (m_OnPlayerEntryFocusLost)
			m_OnPlayerEntryFocusLost.Invoke(w);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnPlayerEntryFocused()
	{
		if (!m_OnPlayerEntryFocused)
			m_OnPlayerEntryFocused = new ScriptInvokerInt();
		
		return m_OnPlayerEntryFocused;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerWidget GetOnPlayerEntryFocusLost()
	{
		if (!m_OnPlayerEntryFocusLost)
			m_OnPlayerEntryFocusLost = new ScriptInvokerWidget();
		
		return m_OnPlayerEntryFocusLost;
	}
	
	//! Send a request to assign a random faction loadout.
	void RequestRandomLoadout(Faction faction)
	{
		array<ref SCR_BasePlayerLoadout> loadouts = {};
		m_LoadoutManager.GetPlayerLoadoutsByFaction(faction, loadouts);
		SCR_BasePlayerLoadout rndLoadout = loadouts.GetRandomElement();
		m_PlyLoadoutComp.RequestLoadout(loadouts.GetRandomElement());
		GetGame().GetCallqueue().CallLater(SetLoadoutPreview, 10, false, rndLoadout);
	}

	void SetSelected(SCR_PlayerLoadoutComponent component)
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(btn);
			if (loadoutBtn)
				loadoutBtn.SetSelected(loadoutBtn.GetPlayerId() == component.GetPlayerId());
		}
		
		if (m_LoadoutSelector)
			m_LoadoutSelector.SetSelected(component.GetLoadout());
	}

	//! Set a loadout shown in the preview widget.
	protected void SetLoadoutPreview(SCR_BasePlayerLoadout loadout)
	{
		PreviewRenderAttributes attributes;

		if (m_PreviewComp && loadout)
		{
			if (m_wLoadoutName)
				m_wLoadoutName.SetText(loadout.GetLoadoutName());			

			Resource res = Resource.Load(loadout.GetLoadoutResource());
			IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_CharacterInventoryStorageComponent");
			if (source)
			{
				BaseContainer container = source.GetObject("Attributes");
				ItemAttributeCollection collection = ItemAttributeCollection.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
				if (collection)
					attributes = PreviewRenderAttributes.Cast(collection.FindAttribute(SCR_CharacterInventoryPreviewAttributes));
			}

			m_PreviewedEntity = m_PreviewComp.SetPreviewedLoadout(loadout, attributes);
			m_wLoadoutPreview.SetVisible(true);
		}
	}

	//! Refreshes current loadout preview widget.
	void RefreshLoadoutPreview()
	{
		SCR_BasePlayerLoadout loadout = m_PlyLoadoutComp.GetLoadout();
		if (!loadout)
			return;

		SetLoadoutPreview(loadout);

		if (m_LoadoutSelector)
			m_LoadoutSelector.SetSelected(loadout);

		if (m_wLoadoutName && loadout)
			m_wLoadoutName.SetText(loadout.GetLoadoutName());

		if (m_wExpandButtonName && loadout)
			m_wExpandButtonName.SetText(loadout.GetLoadoutName());

		if (m_wExpandButtonIcon && loadout)
			m_wExpandButtonIcon.LoadImageTexture(0, GetUIInfo(loadout).GetIconPath());
		
		if (m_wSuppliesText && loadout)
			m_wSuppliesText.SetText(string.ToString(GetLoadoutCost(loadout)));
	}

	//! Get local player's assigned loadout.
	SCR_BasePlayerLoadout GetPlayerLoadout()
	{
		return m_PlyLoadoutComp.GetAssignedLoadout();
	}

	//! Add a loadout into the loadout selector.
	protected void AddLoadout(SCR_BasePlayerLoadout loadout)
	{
		if (loadout)
			m_LoadoutSelector.AddItem(loadout);
	}

	protected void ResetPlayerLoadoutPreview()
	{
		if (m_wLoadoutPreview)
			m_wLoadoutPreview.SetVisible(false);

		if (m_wLoadoutName)
			m_wLoadoutName.SetText("#AR-DeployMenu_SelectLoadout");
	}

	void SetListVisible(bool visible)
	{
		if (m_wLoadoutList)
			m_wLoadoutList.SetVisible(visible);
	}

	void ShowLoadoutSelector(bool show)
	{
		m_wRoot.SetVisible(show);
	}
	
	SCR_EditableEntityUIInfo GetUIInfo(SCR_BasePlayerLoadout loadout)
	{
		if (!loadout)
			return null;

		Resource res = Resource.Load(loadout.GetLoadoutResource());
		IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
		if (source)
		{
			BaseContainer container = source.GetObject("m_UIInfo");
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

			return info;
		}

		return null;
	}

	override void SetListWidget(Widget list)
	{
		if (m_wLoadoutList)
		{
//			SetListVisible(false);
			ClearLoadoutList();		
		}
		
		m_wLoadoutList = GridLayoutWidget.Cast(list);
	}

	bool IsSelectorFocused()
	{
		return m_LoadoutSelector && m_LoadoutSelector.GetFocused();
	}
};

//------------------------------------------------------------------------------------------------
class SCR_LoadoutButton : SCR_DeployButtonBase
{
	[Attribute("PlayerName")]
	protected string m_sPlayerName;
	protected TextWidget m_wPlayerName;

	[Attribute("Leader")]
	protected string m_sLeaderText;
	protected Widget m_wLeaderText;

	protected SCR_BasePlayerLoadout m_Loadout;
	protected int m_iPlayerId = -1;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wPlayerName = TextWidget.Cast(w.FindAnyWidget(m_sPlayerName));
		m_wLeaderText = w.FindAnyWidget(m_sLeaderText);
		m_wElements = w.FindAnyWidget(m_sElements);
		
		if (m_wLeaderText)
			SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(OnLeaderChanged);
	}
	
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(OnLeaderChanged);
	}
	
	//! Assign loadout associated with this button.
	void SetLoadout(SCR_BasePlayerLoadout loadout)
	{
		m_Loadout = loadout;
		if (!loadout)
			return;

		if (GetUIInfo())
			SetImage(GetUIInfo().GetIconPath());
	}

	//! Set player id associated with this button and update the visuals.
	void SetPlayer(int pid)
	{
		m_iPlayerId = pid;
		SetPlayerName(GetGame().GetPlayerManager().GetPlayerName(pid));

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup group = groupManager.GetPlayerGroup(pid);
		if (!group)
			return;

		SetIsLeader(pid == m_iPlayerId && group.IsPlayerLeader(pid));
	}

	//! Update visuals when group leader changes.
	protected void OnLeaderChanged(int groupId, int pid)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup group = groupManager.FindGroup(groupId);
		if (!group)
			return;

		SetIsLeader(pid == m_iPlayerId && group.IsPlayerLeader(pid));
	}

	protected void SetIsLeader(bool leader)
	{
		if (m_wLeaderText)
			m_wLeaderText.SetVisible(leader);
	}

	/*!
	Get player id associated with this button.
	\return player id
	*/
	int GetPlayerId()
	{
		return m_iPlayerId;
	}
	
	protected void SetPlayerName(string name)
	{
		if (m_wPlayerName)
			m_wPlayerName.SetText(name);
	}

	/*!
	Get the loadout associated with this button.
	\return player loadout
	*/
	SCR_BasePlayerLoadout GetLoadout()
	{
		return m_Loadout;
	}

	/*!
	Get UI info of this button's loadout.
	\return UI info
	*/
	SCR_EditableEntityUIInfo GetUIInfo()
	{
		Resource res = Resource.Load(m_Loadout.GetLoadoutResource());
		IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
		if (source)
		{
			BaseContainer container = source.GetObject("m_UIInfo");
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

			return info;
		}

		return null;
	}
};
