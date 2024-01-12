// todo@lk: figure out what to call this, journal is probably not the best choice, idiot
class SCR_MapJournalUI : SCR_MapUIBaseComponent
{
	protected SCR_JournalSetupConfig m_JournalConfig;

	[Attribute("EntryList")]
	protected string m_sEntryList;
	protected Widget m_wEntryList;

	[Attribute("EntryLayout")]
	protected string m_sEntryLayout;
	protected Widget m_wEntryLayout;

	protected ref array<SCR_MapJournalUIButton> m_aEntries = {};
	protected int m_iCurrentEntryId = -1;

	[Attribute("JournalFrame", desc: "Root frame widget name")]
	protected string m_sRootWidgetName;
	protected Widget m_wJournalFrame;

	[Attribute("exclamationCircle", desc: "Toolmenu imageset quad name")]
	protected string m_sToolMenuIconName;

	protected SCR_MapToolMenuUI m_ToolMenu;
	protected SCR_MapToolEntry m_ToolMenuEntry;

	protected SCR_PlayerFactionAffiliationComponent m_PlyFactionAffilComp;
	
	[Attribute("MapTaskListFrame", desc: "Map task list frame widget name")]
	protected string m_sMapTaskListFrame;
	
	[Attribute("faction", desc: "Map task list imageset quad name")]
	protected string m_sTaskListToolMenuIconName;

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		SCR_RespawnBriefingComponent briefingComp = SCR_RespawnBriefingComponent.GetInstance();
		if (!briefingComp)
			return;
		
		if (!briefingComp.LoadJournalConfig())
			return;

		m_JournalConfig = briefingComp.GetJournalSetup();
		if (!m_JournalConfig)
			return;
	
		m_ToolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (m_ToolMenu)
		{
			m_ToolMenuEntry = m_ToolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, m_sToolMenuIconName, 1);
			m_ToolMenuEntry.m_OnClick.Insert(ToggleVisible);
			m_ToolMenuEntry.SetEnabled(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		if (!m_JournalConfig)
			return;

		m_wJournalFrame = m_RootWidget.FindAnyWidget(m_sRootWidgetName);
		
		m_wEntryList = m_RootWidget.FindAnyWidget(m_sEntryList);
		if (!m_wEntryList)
			return;

		m_wEntryLayout = m_RootWidget.FindAnyWidget(m_sEntryLayout);
		if (!m_wEntryLayout)
			return;

		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;

		m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!m_PlyFactionAffilComp)
			return;

		m_PlyFactionAffilComp.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse);

		GetJournalForPlayer();		
		
		ToggleVisible();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionResponse(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (response)
		{
			m_wEntryLayout.SetVisible(false);
			GetJournalForPlayer();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle visibility for the journal
	protected void ToggleVisible()
	{
		if (!m_wJournalFrame || !m_ToolMenu)
			return;
		
		array<ref SCR_MapToolEntry> menuEntries = {};
		menuEntries = m_ToolMenu.GetMenuEntries();
		if (!menuEntries && menuEntries.IsEmpty())
			return;
		
		foreach (SCR_MapToolEntry toolEntry : menuEntries)
		{
			if (toolEntry.GetImageSet() != m_sTaskListToolMenuIconName)
				continue;
			
			Widget mapTaskListFrame = m_RootWidget.FindAnyWidget(m_sMapTaskListFrame);
			if (mapTaskListFrame && mapTaskListFrame.IsVisible())
			{
				mapTaskListFrame.SetVisible(false);
				toolEntry.SetActive(false);
				
				SCR_UITaskManagerComponent m_UITaskManager = SCR_UITaskManagerComponent.GetInstance();
				if (m_UITaskManager)
					m_UITaskManager.Action_TasksClose();
			}
		}

		bool visible = m_wJournalFrame.IsVisible();
		m_wJournalFrame.SetVisible(!visible);
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.SetActive(!visible);
	}

	//------------------------------------------------------------------------------------------------
	protected void GetJournalForPlayer()
	{
		Widget child = m_wEntryList.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			delete child;
			child = sibling;
		}

		m_iCurrentEntryId = -1;

		FactionKey factionKey = FactionKey.Empty;
		if (m_PlyFactionAffilComp)
		{
			Faction plyFaction = m_PlyFactionAffilComp.GetAffiliatedFaction();
			if (plyFaction)
				factionKey = plyFaction.GetFactionKey();
		}

		SCR_JournalConfig factionJournal = m_JournalConfig.GetJournalConfig(factionKey);
		if (!factionJournal)
			return;

		array<ref SCR_JournalEntry> journalEntries = factionJournal.GetEntries();
		for (int entryId = 0; entryId < journalEntries.Count(); ++entryId)
		{
			SCR_JournalEntry entry = journalEntries[entryId];
			Widget e = GetGame().GetWorkspace().CreateWidgets(entry.GetEntryButtonLayout(), m_wEntryList);
			SCR_MapJournalUIButton entryBtn = SCR_MapJournalUIButton.Cast(e.FindHandler(SCR_MapJournalUIButton));
			entryBtn.SetContent(entry.GetEntryName());
			entryBtn.SetEntry(entry, entryId);
			entryBtn.m_OnClicked.Insert(ShowEntry);
			m_aEntries.Insert(entryBtn);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowEntry(SCR_MapJournalUIButton journalBtn)
	{
		if (journalBtn.GetId() == m_iCurrentEntryId)
		{
			m_wEntryLayout.SetVisible(!m_wEntryLayout.IsVisible());
		}
		else
		{
			Widget child = m_wEntryLayout.GetChildren();
			if (child)
			{
				delete child;
			}

			m_iCurrentEntryId = journalBtn.GetId();
			journalBtn.ShowEntry(m_wEntryLayout);
			m_wEntryLayout.SetVisible(true);
		}
	}
};

class SCR_MapJournalUIButton : SCR_ButtonComponent
{
	protected SCR_JournalEntry m_Entry;
	protected int m_iEntryId;

	//------------------------------------------------------------------------------------------------
	void SetEntry(SCR_JournalEntry entry, int id)
	{
		m_Entry = entry;
		m_iEntryId = id;
	}

	//------------------------------------------------------------------------------------------------
	void ShowEntry(Widget target)
	{
		m_Entry.SetEntryLayoutTo(target);
	}

	//------------------------------------------------------------------------------------------------
	int GetId()
	{
		return m_iEntryId;
	}
};
