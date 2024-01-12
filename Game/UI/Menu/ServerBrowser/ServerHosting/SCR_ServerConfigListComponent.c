/*!
Script component for handling server config editing UI 

*/

class SCR_ServerConfigListComponent : SCR_ConfigListComponent
{
	[Attribute("{7E4D962E3084CD77}Configs/ServerBrowser/ServerHosting/Dialogs/ServerConfigDialogs.conf", UIWidgets.ResourceAssignArray)]
	protected ResourceName m_sDialogs;
	
	protected const string NAME_ENTRY = "name";
	protected const string FILE_NAME_ENTRY = "fileName";
	protected const string SCENARIO_SELECTION_ENTRY = "scenarioId";
	protected const string SCENARIO_MOD_SELECTION_ENTRY = "scenarioModId";
	protected const string PLAYER_LIMIT_ENTRY = "maxPlayers";
	protected const string BATTLEYE = "battlEye";
	protected const string CROSSPLAY = "crossPlatform";
	protected const string SIMPLE_PORT = "publicPortSimple";
	
	// Properties content
	protected const string SERVER_NAME_BASE = "%1 %2";
	protected const string FILE_NAME_BASE = "Config_%1";
	protected const string DEFAULT_SCENARIO = "{ECC61978EDCC2B5A}Missions/23_Campaign.conf";
	
	protected SCR_WidgetListEntryEditBox m_NameEdit;
	protected SCR_WidgetListEntryEditBox m_SimplePortEdit;
	protected SCR_WidgetListEntrySelection m_ScenarioSelect;
	protected SCR_WidgetListEntrySelection m_ScenarioModSelect;
	protected SCR_WidgetListEntrySelection m_BattleyeSelect;
	protected SCR_WidgetListEntrySelection m_CrossplaySelect;
	
	protected SCR_WidgetListEntrySlider m_PlayerListSlider;
	
	protected bool m_bNameEdited;
	protected bool m_bFileNameEdited;
	
	// Mod + scenarios that mod is overriding
	protected ref array<ref SCR_ScenarioSources> m_aScenarioSources = {};

	// Invokers
	protected ref ScriptInvoker<string> m_OnScenarioSelected;
	protected ref ScriptInvoker<string> m_OnPortChanged;

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnScenarioSelected(string itemId)
	{
		if (m_OnScenarioSelected)
			m_OnScenarioSelected.Invoke(itemId);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnScenarioSelected()
	{
		if (!m_OnScenarioSelected)
			m_OnScenarioSelected = new ScriptInvoker();

		return m_OnScenarioSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnPortChanged()
	{
		if (!m_OnPortChanged)
			m_OnPortChanged = new ScriptInvoker();

		return m_OnPortChanged;
	}
	
	//-------------------------------------------------------------------------------------------
	// Overriden widget api
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (GetGame().InPlayMode())
			FillScenarios();
		
		m_wScrollWidget = ScrollLayoutWidget.Cast(w);
	}

	//-------------------------------------------------------------------------------------------
	// Protected 
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	//! Fill scenario selection with list of available scenarios
	protected void FillScenarios()
	{
		// Find widgets
		m_NameEdit = SCR_WidgetListEntryEditBox.Cast(FindEntry(NAME_ENTRY));
		
		m_SimplePortEdit = SCR_WidgetListEntryEditBox.Cast(FindEntry(SIMPLE_PORT));
		
		m_ScenarioSelect = SCR_WidgetListEntrySelection.Cast(FindEntry(SCENARIO_SELECTION_ENTRY));
		
		m_ScenarioModSelect = SCR_WidgetListEntrySelection.Cast(FindEntry(SCENARIO_MOD_SELECTION_ENTRY));
		
		m_PlayerListSlider = SCR_WidgetListEntrySlider.Cast(FindEntry(PLAYER_LIMIT_ENTRY));
		
		m_BattleyeSelect = SCR_WidgetListEntrySelection.Cast(FindEntry(BATTLEYE));
		
		m_CrossplaySelect = SCR_WidgetListEntrySelection.Cast(FindEntry(CROSSPLAY));
		
		// Setup names 
		if (m_NameEdit)
			m_NameEdit.GetEditBoxComponent().m_OnChanged.Insert(OnNameChanged);
		
		// Simple port 
		if (m_SimplePortEdit)
			m_SimplePortEdit.GetEditBoxComponent().m_OnChanged.Insert(OnSimplePortChanged);
		
		// Scenarios
		if (!m_ScenarioSelect)
			return;
		
		// Fill scenario sources 
		FillScenarioSources();
		
		// Fill scenario options 
		array<ref SCR_LocalizedProperty> options = {};
		
		for (int i = 0, count = m_aScenarioSources.Count(); i < count; i++)
		{
			MissionWorkshopItem scenario = m_aScenarioSources[i].m_Scenario;
			if (!scenario)
				return;
			
			SCR_LocalizedProperty scenarioOption = new SCR_LocalizedProperty(scenario.Name(), scenario.Id());
			options.Insert(scenarioOption);
		}
		
		m_ScenarioSelect.SetOptions(options);
		
		// Callback 
		m_ScenarioSelect.GetSelectionComponent().m_OnChanged.Insert(OnSelectScenario);
		
		// Select default scenario
		int defaultId = SetSelectedScenarioById(DEFAULT_SCENARIO);
		m_ScenarioSelect.SelectOption(defaultId);
		OnSelectScenario(null, defaultId);
		
		// Platform specific setup 
		ConsoleSetup();
	}
	
	//-------------------------------------------------------------------------------------------
	protected void FillScenarioSources()
	{
		// Get all scenarios 
		array<MissionWorkshopItem> allMissions = {};
		GetGame().GetBackendApi().GetWorkshop().GetPageScenarios(allMissions, 0, 4096);
		
		// Filter scenarios 
		array<MissionWorkshopItem> missions = {};
		
		for (int i = 0, count = allMissions.Count(); i < count; i++)
		{
			// Exclude single player 
			if (allMissions[i].GetPlayerCount() == 1)
				continue;
			
			// Exclude broken mod scenarios
			if (allMissions[i].GetOwner())
			{
				SCR_WorkshopItem item = SCR_AddonManager.GetInstance().Register(allMissions[i].GetOwner());
				 
				if (item && item.GetAnyDependencyMissing() || item.GetCorrupted())
					continue;
			}
			
			missions.Insert(allMissions[i]);
		}
		
		// Order scenarios 
		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(missions);
		
		// Add scenarios and sorces to map 
		for (int i = 0, count = missions.Count(); i < count; i++)
		{
			InsertScenarioToMap(missions[i], missions[i].GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Insert scenario and sorces to scenario sources mod map 
	//! If scenario name is reapeate, insert source mod in to existing scenario 
	protected void InsertScenarioToMap(MissionWorkshopItem scenario, WorkshopItem modOwner)
	{
		// Scenario map structure  - key: scenario (SCR_LocalizedProperty), value: sources (array SCR_LocalizedProperty)
		
		// Scenario id is in map - there stil could be repeating names 
		int scenarioIndex = -1;
		
		for (int i = 0, count = m_aScenarioSources.Count(); i < count; i++)
		{		
			MissionWorkshopItem mission = m_aScenarioSources[i].m_Scenario;
				
			// Scenario structure - label: name, propertyName: id	
			if (mission && scenario.Id() == mission.Id())
			{
				//scenario = m_aScenarioSources[i].m_Scenario;
				scenarioIndex = i;
				break;
			}
		}
		
		// Scenario is in map 
		if (scenarioIndex != -1)
		{
			// Add source to existing scenario
			m_aScenarioSources[scenarioIndex].m_aMods.Insert(modOwner);
		}
		else
		{
			// Add new 
			m_aScenarioSources.Insert(new SCR_ScenarioSources(scenario, {modOwner}));
		}
	}
	
	//-------------------------------------------------------------------------------------------
	//! Return default scenario id
	protected int SetSelectedScenarioById(string scenarioId)
	{
		array<ref SCR_LocalizedProperty> scenarios = {};
		m_ScenarioSelect.GetOptions(scenarios);
		
		for (int i = 0, count = m_aScenarioSources.Count(); i < count; i++)
		{
			// Property name = scenario id
			if (scenarios[i].m_sPropertyName == scenarioId)
			{
				return i;
			}
		}

		// Fallback
		return 0;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Apply console specific properties
	protected void ConsoleSetup()
	{
		// Check platform 
		if (!GetGame().IsPlatformGameConsole())
			return;
		
		// Default values 
		m_BattleyeSelect.SetValue("0");
		m_CrossplaySelect.SetValue("1");
		
		// Hide limited on consoles 
		m_BattleyeSelect.GetEntryRoot().SetVisible(false);
		m_CrossplaySelect.GetEntryRoot().SetVisible(false);
	}
	
	//-------------------------------------------------------------------------------------------
	// Public
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	void SelectScenario(notnull MissionWorkshopItem scenario)
	{
		array<MissionWorkshopItem> missionItemsAll = new array<MissionWorkshopItem>;
		GetGame().GetBackendApi().GetWorkshop().GetPageScenarios(missionItemsAll, 0, 4096);
		
		if (m_ScenarioSelect)
		{
			int selected = SetSelectedScenarioById(scenario.Id());
			
			m_ScenarioSelect.SelectOption(selected);
			OnSelectScenario(SCR_ComboBoxComponent.Cast(m_ScenarioSelect.GetSelectionComponent()), selected);
		}
	}
	
	//-------------------------------------------------------------------------------------------
	MissionWorkshopItem GetSelectedScenario()
	{
		if (!m_ScenarioSelect)
			return null;
	
		array<MissionWorkshopItem> missionItemsAll = new array<MissionWorkshopItem>;
		GetGame().GetBackendApi().GetWorkshop().GetPageScenarios(missionItemsAll, 0, 4096);
		
		for (int i = 0, count = missionItemsAll.Count(); i < count; i++)
		{
			SCR_LocalizedProperty selected = m_ScenarioSelect.GetSelectedOption();
			string name = selected.m_sPropertyName;
			
			if (missionItemsAll[i].Id() == name)
			{
				return missionItemsAll[i];
			}
		}
		
		return null;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Return currently selected scenario owner mod as workshop item
	WorkshopItem GetScenarioOwnerMod()
	{
		if (!m_ScenarioSelect)
			return null;
	
		array<MissionWorkshopItem> missionItemsAll = new array<MissionWorkshopItem>;
		GetGame().GetBackendApi().GetWorkshop().GetPageScenarios(missionItemsAll, 0, 4096);
		
		// Check current selection 
		SCR_LocalizedProperty property = m_ScenarioModSelect.GetSelectedOption();
		
		if (!property)
			return null;
		
		// Select owner mod
		for (int i = 0, count = missionItemsAll.Count(); i < count; i++)
		{
			WorkshopItem ownerItem = missionItemsAll[i].GetOwner();
			if (!ownerItem)
				continue;
			
			if (ownerItem.Id() == property.m_sPropertyName)
			{
				return ownerItem;
			}
		}
		
		return null;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Fill all entries with values from given DS config 
	void FillFromDSConfig(notnull SCR_DSConfig config)
	{
		// Game
		FindEntry("name").SetValue(config.game.name);
		FindEntry("maxPlayers").SetValue(config.game.maxPlayers.ToString());
		FindEntry("password").SetValue(config.game.password);
		FindEntry("passwordAdmin").SetValue(config.game.passwordAdmin);
		FindEntry("visible").SetValue(SCR_JsonApiStructHandler.BoolToString(config.game.visible));
		FindEntry("crossPlatform").SetValue(SCR_JsonApiStructHandler.BoolToString(config.game.crossPlatform));
		
		// Game properties
		SCR_DSGameProperties gamePropertiesSCr = SCR_DSGameProperties.Cast(config.game.gameProperties);
		
		if (gamePropertiesSCr)
		{
			FindEntry("battlEye").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.battlEye));
			FindEntry("disableThirdPerson").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.disableThirdPerson));
			FindEntry("VONDisableUI").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.VONDisableUI));
			FindEntry("VONDisableDirectSpeechUI").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.VONDisableDirectSpeechUI));
			FindEntry("VONCanTransmitCrossFaction").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.VONCanTransmitCrossFaction));
			FindEntry("serverMaxViewDistance").SetValue(gamePropertiesSCr.serverMaxViewDistance.ToString());
			FindEntry("networkViewDistance").SetValue(gamePropertiesSCr.networkViewDistance.ToString());
			FindEntry("serverMinGrassDistance").SetValue(gamePropertiesSCr.serverMinGrassDistance.ToString());
		}

		// Scenario 
		m_ScenarioSelect.SelectOption(SetSelectedScenarioById(config.game.scenarioId));
	}
	
	//-------------------------------------------------------------------------------------------
	//! Scroll to the selected entry
	void ScrollToTheEntry(notnull Widget w)
	{
		// Entry position
		float entryX, entryY;
		w.GetScreenPos(entryX, entryY);
		
		// Vertical list size
		float listW, listH;
		m_wList.GetScreenSize(listW, listH);
		
		// Vertical list size
		float listX, listY;
		m_wList.GetScreenPos(listX, listY);
		
		// Scroll (view) size
		float scrollW, scrollH; 
		m_wScrollWidget.GetScreenSize(scrollW, scrollH);
		
		// Scroll to entry
		float outView = listH - scrollH; // What size can't be seen 
		float relPos = entryY - listY;

		float pos = 1 - (outView - relPos) / outView;
		m_wScrollWidget.SetSliderPos(0, pos);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPort(string port)
	{
		m_SimplePortEdit.SetValue(port);
	}
	
	//-------------------------------------------------------------------------------------------
	// Callbacks 
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	protected void OnNameChanged(SCR_EditBoxComponent edit, string text)
	{
		m_bNameEdited = !text.IsEmpty();
	}
	
	//-------------------------------------------------------------------------------------------	
	protected void OnSimplePortChanged(SCR_EditBoxComponent edit, string text)
	{
		if (m_OnPortChanged)
			m_OnPortChanged.Invoke(text);
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnFileNameChanged(SCR_EditBoxComponent edit, string text)
	{
		m_bFileNameEdited = !text.IsEmpty();
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnSelectScenario(SCR_ComboBoxComponent comboBox, int selected)
	{
		if (!m_ScenarioSelect || !m_ScenarioModSelect)
			return;
		
		int selecteScenario = m_ScenarioSelect.GetSelectedOption();
		array<ref SCR_LocalizedProperty> ownerMods = {};
		
		int count = 0;
		
		if (m_aScenarioSources[selected].m_aMods)
			count = m_aScenarioSources[selected].m_aMods.Count();
		
		for (int i = 0; i < count; i++)
		{
			WorkshopItem mod = m_aScenarioSources[selected].m_aMods[i];
			if (!mod)
				continue;
			
			string name = m_aScenarioSources[selected].m_aMods[i].Name();
			string id = m_aScenarioSources[selected].m_aMods[i].Id();
			
			ownerMods.Insert(new SCR_LocalizedProperty(name, id));
		}
		
		// Is vanilla?
		if (!ownerMods || ownerMods.IsEmpty() || ownerMods[0] == null)
		{
			// Display source mod as Arma Reforger
			ownerMods.Clear();
			ownerMods.Insert(new SCR_LocalizedProperty("#AR-Editor_Attribute_OverlayLogo_Reforger", ""));
		}
		
		m_ScenarioModSelect.SetOptions(ownerMods);
		m_ScenarioModSelect.SelectOption(0);
		
		// Enable selection interactivity only if scenario has multiple sources 
		m_ScenarioModSelect.SetInteractive(ownerMods.Count() > 1);
		
		// Update properties
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		if (!m_bNameEdited)
			GenerateName(scenario);
		
		if (!m_bFileNameEdited)
			GenerateFileName(scenario);
		
		SetupPlayerLimit(scenario.GetPlayerCount());
		
		InvokeOnScenarioSelected(scenario.Id());
	}
	
	//-------------------------------------------------------------------------------------------
	//! Generate server name to the name editbox in case name wasn't edited
	protected void GenerateName(notnull MissionWorkshopItem scenario)
	{
		if (!m_NameEdit)
			return;
		
		string name = string.Format(SERVER_NAME_BASE, SCR_Global.GetProfileName(), scenario.Name());
		
		m_NameEdit.SetValue(name);
		m_NameEdit.ClearInvalidInput();
	}
	
	protected const string CHAR_BLACK_LIST = "<>:\/\|?*.";
	
	//-------------------------------------------------------------------------------------------
	//! Generate server name to the name editbox in case name wasn't edited
	string GenerateFileName(notnull MissionWorkshopItem scenario)
	{
		string scenarioTranslation = WidgetManager.Translate(scenario.Name());
		string fileName =  string.Format(FILE_NAME_BASE, scenarioTranslation);
		fileName.Replace(" ", "");
		
		// Remove special characters 
		for (int i = 0, count = CHAR_BLACK_LIST.Length(); i < count; i++)
		{
			fileName.Replace(CHAR_BLACK_LIST[i], "");
		}
		
		return fileName;
	}
	
	//-------------------------------------------------------------------------------------------
	protected void SetupPlayerLimit(int limit)
	{
		if (!m_PlayerListSlider)
			return;
		
		m_PlayerListSlider.SetRange(1, limit);
		
		// Set value 
		int value = m_PlayerListSlider.ValueAsString().ToInt();
		
		if (value > limit)
			m_PlayerListSlider.SetValue(limit.ToString());
	}
	
	//-------------------------------------------------------------------------------------------
	// API
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	ResourceName GetDialogs()
	{
		return m_sDialogs;
	}
};

//! Stores scenario with scenario source
//-------------------------------------------------------------------------------------------
class SCR_ScenarioSources
{
	MissionWorkshopItem m_Scenario;
	
	ref array<WorkshopItem> m_aMods;
	
	void SCR_ScenarioSources(MissionWorkshopItem scenario, array<WorkshopItem> mods)
	{
		m_Scenario = scenario;
		m_aMods = mods;
	}
}

