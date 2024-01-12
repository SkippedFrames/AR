[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Briefing screen shown in respawn menu.")]
class SCR_RespawnBriefingComponentClass : SCR_BaseGameModeComponentClass
{
};

class SCR_RespawnBriefingComponent : SCR_BaseGameModeComponent
{
	[Attribute("")]
	protected ResourceName m_sJournalConfigPath;
	protected ref SCR_JournalSetupConfig m_JournalConfig;

	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	
	[Attribute("{324E923535DCACF8}UI/Textures/DeployMenu/Briefing/conflict_HintBanner_1_UI.edds", desc: "background shown when briefing has no hints")]
	protected ResourceName m_SimpleBriefingBackground;
	
	[Attribute()]
	protected ref array<ref SCR_UIInfo> m_aGameModeHints;

	[Attribute()]
	protected ref array<ref SCR_BriefingVictoryCondition> m_aWinConditions;

	protected ref ScriptInvoker m_OnBriefingChanged = new ScriptInvoker(); 
	
	protected bool m_bWasShown = false;
	
	protected ref array<ref Tuple3<FactionKey, string, string>> m_aBriefingStrings = {};
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		
		writer.WriteInt(m_aBriefingStrings.Count());
		foreach (Tuple3<FactionKey, string, string> briefingString : m_aBriefingStrings)
		{
			writer.WriteString(briefingString.param1);
			writer.WriteString(briefingString.param2);
			writer.WriteString(briefingString.param3);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int count;
		FactionKey factionKey;
		string customEntryName;
		string newText;
		
		reader.ReadInt(count);
		for (int i = 0; i < count; i++)
		{
			reader.ReadString(factionKey);
			reader.ReadString(customEntryName);
			reader.ReadString(newText);
			RewriteEntry(factionKey, customEntryName, newText);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void RewriteEntryMain(FactionKey factionKey, string customEntryName, string newText)
	{
		RewriteEntry(factionKey, customEntryName, newText);
		Rpc(RpcDo_RewriteEntry, factionKey, customEntryName, newText);
		
		m_aBriefingStrings.Insert(new Tuple3<FactionKey, string, string>(factionKey, customEntryName, newText));
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_RewriteEntry(FactionKey factionKey, string customEntryName, string newText)
	{
		RewriteEntry(factionKey, customEntryName, newText);
	}
	
	//------------------------------------------------------------------------------------------------
	void RewriteEntry(FactionKey factionKey, string customEntryName, string newText)
	{
		if (!m_JournalConfig)
			LoadJournalConfig();
		
		if (!m_JournalConfig)
			return;

		SCR_JournalConfig journalConfig = m_JournalConfig.GetJournalConfig(factionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{

			if (journalEntry.GetCustomEntryName() != customEntryName)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}
		
		if (targetJournalEntry)
			targetJournalEntry.SetEntryText(newText);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_JournalSetupConfig GetJournalSetup()
	{
		if (!m_JournalConfig)
			LoadJournalConfig();
		
		return m_JournalConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetConfig()
	{
		m_JournalConfig = null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool LoadJournalConfig()
	{
		if (m_JournalConfig)
			return true;
		
		if (m_sJournalConfigPath.GetPath().IsEmpty())
		{
			Print("Journal config path is empty!", LogLevel.WARNING);
			return false;
		}

		Resource holder = BaseContainerTools.LoadContainer(m_sJournalConfigPath);
		if (!holder || !holder.IsValid())
			return false;

		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
			return false;

		m_JournalConfig = SCR_JournalSetupConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!m_JournalConfig)
		{
			Print("Journal config couldn't be created!", LogLevel.WARNING);
			return false;
		}

		return true;
	}
	
	/*!
	\return Local instance of the briefing component.
	*/
	static SCR_RespawnBriefingComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return SCR_RespawnBriefingComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_RespawnBriefingComponent));
		else
			return null;
	}
	
	/*!
	\return Briefing UI info
	*/
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
	
	/*!
	\return Simple briefing background image
	*/
	ResourceName GetSimpleBriefingBackground()
	{
		return m_SimpleBriefingBackground;
	}

	ResourceName GetJournalConfigPath()
	{
		return m_sJournalConfigPath;
	}

	//------------------------------------------------------------------------------------------------
	int GetGameModeHints(out array<ref SCR_UIInfo> hints)
	{
		hints = m_aGameModeHints;

		return m_aGameModeHints.Count();
	}

	//------------------------------------------------------------------------------------------------
	int GetWinConditions(out array<ref SCR_BriefingVictoryCondition> conditions)
	{
		conditions = m_aWinConditions;

		return m_aWinConditions.Count();
	}

	/*!
	\return Event called when info changes, so that respawn menu can be updated (to be override by inherited classes)
	*/
	ScriptInvoker GetOnBriefingChanged()
	{
		return m_OnBriefingChanged;
	}

	//------------------------------------------------------------------------------------------------	
	void SetBriefingShown(bool shown = true)
	{
		m_bWasShown = shown;
	}

	//------------------------------------------------------------------------------------------------
	bool GetWasBriefingShown()
	{
		return m_bWasShown;
	}
};

[BaseContainerProps()]
class SCR_BriefingVictoryCondition
{
	[Attribute("1", UIWidgets.ComboBox, "Type of victory condition", "", ParamEnumArray.FromEnum(ETaskIconType))]
	protected ETaskIconType victoryCondition;
	
	[Attribute()]
	protected string name;
	
	[Attribute()]
	protected string description;
	
	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return name;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDescription()
	{
		return description;
	}
	
	//------------------------------------------------------------------------------------------------
	ETaskIconType GetConditionType()
	{
		return victoryCondition;
	}
};
