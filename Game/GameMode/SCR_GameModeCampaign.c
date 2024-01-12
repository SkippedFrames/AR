#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
class SCR_GameModeCampaignClass : SCR_BaseGameModeClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_GameModeCampaign : SCR_BaseGameMode
{
	[Attribute("1", desc: "Run Conflict automatically at scenario start. If disabled, use RequestStart() method on server.", category: "Campaign")]
	protected bool m_bAutostart;

	[Attribute("1", desc: "Terminate the scenario when Conflict is finished.", category: "Campaign")]
	protected bool m_bTerminateScenario;

	[Attribute("2", desc: "How many control points does a faction need to win", params: "1 inf 1", category: "Campaign")]
	protected int m_iControlPointsThreshold;

	[Attribute("300", desc: "How long does a faction need to hold the control points to win (seconds).", params: "0 inf 1", category: "Campaign")]
	protected float m_fVictoryTimer;

	[Attribute("5", desc: "How many radio operators can act as a mobile spawn point at the same time.", params: "0 inf 1", category: "Campaign")]
	protected int m_iMaxRespawnRadios;

	[Attribute("1000", desc: "Supplies will be autoreplenished in bases until this limit is reached.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuppliesReplenishThreshold;
	
	[Attribute("200", desc: "Supplies will be autoreplenished in bases quickly until this limit is reached.", params: "0 inf 1", category: "Campaign")]
	protected int m_iQuickSuppliesReplenishThreshold;
	
	[Attribute("2", desc: "Supplies income will be multiplied by this number unless the quick replenish threshold has been reached.", params: "1 inf 0.05", category: "Campaign")]
	protected float m_fQuickSuppliesReplenishMultiplier;

	[Attribute("5", desc: "How often are supplies replenished in bases (seconds).", params: "1 inf 1", category: "Campaign")]
	protected int m_iSuppliesArrivalInterval;

	[Attribute("40", desc: "How many supplies are periodically replenished in HQs.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncome;

	[Attribute("4", desc: "How many supplies are periodically replenished in non-HQ bases.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncomeBase;

	[Attribute("4", desc: "How many extra supplies are periodically replenished in non-HQ bases per base connected via radio.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncomeExtra;
	
	[Attribute("600", desc: "The amount of supplies in HQs.", params: "0 inf 1", category: "Campaign")]
	protected int m_iHQStartingSupplies;

	[Attribute("100", desc: "When randomized, the least supplies a base can hold at the start.", params: "0 inf 1", category: "Campaign")]
	protected int m_iMinStartingSupplies;

	[Attribute("500", desc: "When randomized, the most supplies a base can hold at the start.", params: "0 inf 1", category: "Campaign")]
	protected int m_iMaxStartingSupplies;

	[Attribute("25", desc: "The step by which randomized supplies will be added in randomization. Min and Max limits should be divisible by this.", params: "1 inf 1", category: "Campaign")]
	protected int m_iStartingSuppliesInterval;

	[Attribute("US", category: "Campaign")]
	protected FactionKey m_sBLUFORFactionKey;

	[Attribute("USSR", category: "Campaign")]
	protected FactionKey m_sOPFORFactionKey;

	[Attribute("FIA", category: "Campaign")]
	protected FactionKey m_sINDFORFactionKey;

	[Attribute("1", UIWidgets.CheckBox, "Randomized starting supplies in small bases", category: "Campaign")]
	protected bool m_bRandomizeSupplies;

	[Attribute("40", desc: "How much supplies it cost to spawn at base by default?", params: "0 inf 1", category: "Campaign")]
	protected int m_iSpawnCost;

	[Attribute("1200", UIWidgets.EditBox, "The furthest an independent supply depot can be from the nearest base to still be visible in the map.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSupplyDepotIconThreshold;

	[Attribute("{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et", category: "Campaign")]
	protected ResourceName m_sSeekDestroyWaypointPrefab;

	[Attribute("1800", UIWidgets.EditBox, "If suicide is committed more than once in this time (seconds), a respawn penalty is issued.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuicidePenaltyCooldown;

	[Attribute("30", UIWidgets.EditBox, "Stacking extra deploy cooldown after suicide (seconds). Gets deducted over time.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuicideRespawnDelay;

	[Attribute("600", UIWidgets.EditBox, "How often is the post-suicide deploy cooldown penalty deducted (seconds).", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuicideForgiveCooldown;

	static const int MINIMUM_DELAY = 100;
	static const int UI_UPDATE_DELAY = 250;
	static const int MEDIUM_DELAY = 1000;
	static const int DEFAULT_DELAY = 2000;
	static const int BACKEND_DELAY = 25000;

	protected ref ScriptInvoker m_OnFactionAssignedLocalPlayer;
	protected ref ScriptInvoker m_OnStarted;
	protected ref ScriptInvoker m_OnMatchSituationChanged;
	protected ref ScriptInvoker m_OnCallsignOffsetChanged;

	protected ref map<int, Faction> m_mUnprocessedFactionAssignments = new map<int, Faction>();

	protected ref array<SCR_PlayerRadioSpawnPointCampaign> m_aRadioSpawnPoints = {};
	protected ref array<ref SCR_CampaignClientData> m_aRegisteredClients = {};

	protected bool m_bIgnoreMinimumVehicleRank;
	protected bool m_bIsTutorial;
	protected bool m_bMatchOver;
	protected bool m_bWorldPostProcessDone;
	protected bool m_bRemnantsStateLoaded;
	protected bool m_bIsSessionLoadInProgress;

	protected ref SCR_CampaignMilitaryBaseManager m_BaseManager = new SCR_CampaignMilitaryBaseManager(this);

	protected SCR_CampaignStruct m_LoadedData;

	[RplProp(onRplName: "OnStarted")]
	protected bool m_bStarted;

	[RplProp(onRplName: "OnMatchSituationChanged")]
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fVictoryTimestamp;
	#else
	protected WorldTimestamp m_fVictoryTimestamp;
	#endif

	[RplProp(onRplName: "OnMatchSituationChanged")]
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fVictoryPauseTimestamp;
	#else
	protected WorldTimestamp m_fVictoryPauseTimestamp;
	#endif

	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected int m_iWinningFactionId = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;

	[RplProp(onRplName: "OnCallsignOffsetChanged")]
	protected int m_iCallsignOffset = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;

	//------------------------------------------------------------------------------------------------
	//! Triggered when the local player picks a faction
	ScriptInvoker GetOnFactionAssignedLocalPlayer()
	{
		if (!m_OnFactionAssignedLocalPlayer)
			m_OnFactionAssignedLocalPlayer = new ScriptInvoker();

		return m_OnFactionAssignedLocalPlayer;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when Conflict gamemode has started
	ScriptInvoker GetOnStarted()
	{
		if (!m_OnStarted)
			m_OnStarted = new ScriptInvoker();

		return m_OnStarted;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when an event happened which should be communicated to players (i.e. amount of control points held etc.)
	ScriptInvoker GetOnMatchSituationChanged()
	{
		if (!m_OnMatchSituationChanged)
			m_OnMatchSituationChanged = new ScriptInvoker();

		return m_OnMatchSituationChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCallsignOffsetChanged()
	{
		if (!m_OnCallsignOffsetChanged)
			m_OnCallsignOffsetChanged = new ScriptInvoker();

		return m_OnCallsignOffsetChanged;
	}

	//------------------------------------------------------------------------------------------------
	int GetSuppliesReplenishThreshold()
	{
		return m_iSuppliesReplenishThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetQuickSuppliesReplenishThreshold()
	{
		return m_iQuickSuppliesReplenishThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetQuickSuppliesReplenishMultiplier()
	{
		return m_fQuickSuppliesReplenishMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	int GetSuppliesArrivalInterval()
	{
		return m_iSuppliesArrivalInterval;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncome()
	{
		return m_iRegularSuppliesIncome;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncomeBase()
	{
		return m_iRegularSuppliesIncomeBase;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncomeExtra()
	{
		return m_iRegularSuppliesIncomeExtra;
	}

	//------------------------------------------------------------------------------------------------
	int GetSupplyDepotIconThreshold()
	{
		return m_iSupplyDepotIconThreshold;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseManager GetBaseManager()
	{
		return m_BaseManager;
	}

	//------------------------------------------------------------------------------------------------
	float GetVictoryTimer()
	{
		return m_fVictoryTimer;
	}

	//------------------------------------------------------------------------------------------------
	int GetControlPointTreshold()
	{
		return m_iControlPointsThreshold;
	}

	//------------------------------------------------------------------------------------------------
	int GetMinStartingSupplies()
	{
		return m_iMinStartingSupplies;
	}

	//------------------------------------------------------------------------------------------------
	int GetMaxStartingSupplies()
	{
		return m_iMaxStartingSupplies;
	}

	//------------------------------------------------------------------------------------------------
	int GetStartingSuppliesInterval()
	{
		return m_iStartingSuppliesInterval;
	}

	//------------------------------------------------------------------------------------------------
	int GetSpawnCost()
	{
		return m_iSpawnCost;
	}

	//------------------------------------------------------------------------------------------------
	bool WasRemnantsStateLoaded()
	{
		return m_bRemnantsStateLoaded;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSessionLoadInProgress()
	{
		return m_bIsSessionLoadInProgress;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetSeekDestroyWaypointPrefab()
	{
		return m_sSeekDestroyWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	int GetWinningFactionId()
	{
		return m_iWinningFactionId;
	}

	//------------------------------------------------------------------------------------------------
	int GetMaxRespawnRadios()
	{
		return m_iMaxRespawnRadios;
	}

	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetVictoryTimestamp()
	#else
	WorldTimestamp GetVictoryTimestamp()
	#endif
	{
		return m_fVictoryTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetVictoryPauseTimestamp()
	#else
	WorldTimestamp GetVictoryPauseTimestamp()
	#endif
	{
		return m_fVictoryPauseTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	bool IsTutorial()
	{
		return m_bIsTutorial;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsMatchOver()
	{
		return m_bMatchOver;
	}

	//------------------------------------------------------------------------------------------------
	bool HasStarted()
	{
		return m_bStarted;
	}

	//------------------------------------------------------------------------------------------------
	int GetCallsignOffset()
	{
		return m_iCallsignOffset;
	}

	//------------------------------------------------------------------------------------------------
	void OnMatchSituationChanged()
	{
		if (m_OnMatchSituationChanged)
			m_OnMatchSituationChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		// Sync respawn radios & control points amount
		writer.WriteInt(m_BaseManager.GetTargetActiveBasesCount());

		int respawnRadiosBLUFOR = GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetActiveRespawnRadios();
		int respawnRadiosOPFOR = GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetActiveRespawnRadios();

		int controlPointsHeldBLUFOR = GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetControlPointsHeld();
		int controlPointsHeldOPFOR = GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetControlPointsHeld();

		RplId primaryTargetBLUFOR = Replication.FindId(GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetPrimaryTarget());
		RplId primaryTargetOPFOR = Replication.FindId(GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetPrimaryTarget());

		writer.WriteInt(respawnRadiosBLUFOR);
		writer.WriteInt(respawnRadiosOPFOR);

		writer.WriteInt(controlPointsHeldBLUFOR);
		writer.WriteInt(controlPointsHeldOPFOR);

		writer.WriteInt(primaryTargetBLUFOR);
		writer.WriteInt(primaryTargetOPFOR);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		// Sync respawn radios & control points amount
		int activeBasesTotal;

		reader.ReadInt(activeBasesTotal);

		m_BaseManager.SetTargetActiveBasesCount(activeBasesTotal);

		if (m_BaseManager.GetActiveBasesCount() == activeBasesTotal)
			m_BaseManager.OnAllBasesInitialized();

		int respawnRadiosBLUFOR, respawnRadiosOPFOR, controlPointsHeldBLUFOR, controlPointsHeldOPFOR, primaryTargetBLUFOR, primaryTargetOPFOR;

		reader.ReadInt(respawnRadiosBLUFOR);
		reader.ReadInt(respawnRadiosOPFOR);

		reader.ReadInt(controlPointsHeldBLUFOR);
		reader.ReadInt(controlPointsHeldOPFOR);

		reader.ReadInt(primaryTargetBLUFOR);
		reader.ReadInt(primaryTargetOPFOR);

		GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).SetActiveRespawnRadios(respawnRadiosBLUFOR);
		GetFactionByEnum(SCR_ECampaignFaction.OPFOR).SetActiveRespawnRadios(respawnRadiosOPFOR);

		GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).SetControlPointsHeld(controlPointsHeldBLUFOR);
		GetFactionByEnum(SCR_ECampaignFaction.OPFOR).SetControlPointsHeld(controlPointsHeldOPFOR);

		GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(primaryTargetBLUFOR)));
		GetFactionByEnum(SCR_ECampaignFaction.OPFOR).SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(primaryTargetOPFOR)));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_GameModeCampaign GetInstance()
	{
		return SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
	}

	//------------------------------------------------------------------------------------------------
	void SetControlPointsHeld(SCR_CampaignFaction faction, int newCount)
	{
		if (faction.GetControlPointsHeld() == newCount)
			return;

		int index = GetGame().GetFactionManager().GetFactionIndex(faction);

		Rpc(RPC_DoSetControlPointsHeld, index, newCount);
		RPC_DoSetControlPointsHeld(index, newCount)
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoSetControlPointsHeld(int factionIndex, int count)
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		faction.SetControlPointsHeld(count);
		OnMatchSituationChanged();
	}

	//------------------------------------------------------------------------------------------------
	void SetPrimaryTarget(notnull SCR_CampaignFaction faction, SCR_CampaignMilitaryBaseComponent target)
	{
		if (faction.GetPrimaryTarget() == target)
			return;

		int index = GetGame().GetFactionManager().GetFactionIndex(faction);
		RplId targetId = RplId.Invalid();

		if (target)
			targetId = Replication.FindId(target);

		Rpc(RPC_DoSetPrimaryTarget, index, targetId);
		RPC_DoSetPrimaryTarget(index, targetId)
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoSetPrimaryTarget(int factionIndex, int targetId)
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		faction.SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(targetId)));
	}
	
	//------------------------------------------------------------------------------------------------
	void BroadcastMHQFeedback(SCR_EMobileAssemblyStatus msgID, int playerID, int factionID)
	{
		Rpc(RpcDo_BroadcastMHQFeedback, msgID, playerID, factionID);
		
		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_BroadcastMHQFeedback(msgID, playerID, factionID);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_BroadcastMHQFeedback(SCR_EMobileAssemblyStatus msgID, int playerID, int factionID)
	{
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!comp)
			return;
		
		comp.MobileAssemblyFeedback(msgID, playerID, factionID)
	}

	//------------------------------------------------------------------------------------------------
	void RequestStart()
	{
		if (IsProxy())
			return;

		// Start the gamemode after OnWorldPostprocess so bases have time to init properly
		if (m_bWorldPostProcessDone && !m_bStarted)
			Start();
	}

	//------------------------------------------------------------------------------------------------
	protected void Start()
	{
		// Handle player spawnpoints override
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(FindComponent(SCR_PlayerSpawnPointManagerComponent));

		if (playerSpawnPointManager)
		{
			if (m_iMaxRespawnRadios >= 0)
			{
				playerSpawnPointManager.EnablePlayerSpawnPoints(true);
				GetGame().GetCallqueue().CallLater(CheckRadioSpawnpointsSignalCoverage, DEFAULT_DELAY, true);
			}
			else
				playerSpawnPointManager.EnablePlayerSpawnPoints(false);
		}

		// Compose custom bases array from header
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();

		if (!baseManager)
			return;

		float customHQSupplies = m_iHQStartingSupplies;
		bool whitelist = false;
		array<string> customBaseList = {};

		SCR_MissionHeaderCampaign header = SCR_MissionHeaderCampaign.Cast(GetGame().GetMissionHeader());

		if (header)
		{
			whitelist = header.m_bCustomBaseWhitelist;
			customHQSupplies = header.m_iStartingHQSupplies;

			foreach (SCR_CampaignCustomBase customBase : header.m_aCampaignCustomBaseList)
			{
				customBaseList.Insert(customBase.GetBaseName());
			}
		}

		array<SCR_CampaignMilitaryBaseComponent> candidatesForHQ = {};
		array<SCR_CampaignMilitaryBaseComponent> controlPoints = {};
		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);

		string baseName;
		SCR_CampaignMilitaryBaseComponent campaignBase;
		int listIndex;

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase)
				continue;

			// Ignore the base if it's disabled in mission header
			if (header)
			{
				baseName = campaignBase.GetOwner().GetName();
				listIndex = customBaseList.Find(baseName);

				if (listIndex != -1)
				{
					if (!whitelist)
						continue;
				}
				else if (whitelist)
				{
					continue;
				}

				if (whitelist && listIndex != -1)
					campaignBase.ApplyHeaderSettings(header.m_aCampaignCustomBaseList[listIndex]);
			}

			if (!campaignBase.DisableWhenUnusedAsHQ() || !campaignBase.CanBeHQ())
			{
				campaignBase.Initialize();
				m_BaseManager.AddTargetActiveBase();
			}

			if (campaignBase.CanBeHQ())
				candidatesForHQ.Insert(campaignBase);

			if (campaignBase.IsControlPoint())
				controlPoints.Insert(campaignBase);
		}

		m_BaseManager.UpdateBases();

		if (candidatesForHQ.Count() < 2)
		{
			Print("Not enough suitable starting locations found in current setup. Check 'Can Be HQ' attributes in SCR_CampaignMilitaryBaseComponent!", LogLevel.ERROR);
			return;
		}

		// Process HQ selection
		array<SCR_CampaignMilitaryBaseComponent> selectedHQs = {};
		m_BaseManager.SelectHQs(candidatesForHQ, controlPoints, selectedHQs);
		m_BaseManager.SetHQFactions(selectedHQs);

		foreach (SCR_CampaignMilitaryBaseComponent hq : selectedHQs)
		{
			hq.SetAsHQ(true);

			if (customHQSupplies == -1)
				hq.SetStartingSupplies(m_iHQStartingSupplies);
			else
				hq.SetStartingSupplies(customHQSupplies);

			if (!hq.IsInitialized())
			{
				hq.Initialize();
				m_BaseManager.AddTargetActiveBase();
			}
		}

		m_BaseManager.InitializeBases(selectedHQs, m_bRandomizeSupplies);

		if (m_iCallsignOffset == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
		{
			int basesCount = m_BaseManager.GetTargetActiveBasesCount();

			Math.Randomize(-1);
			m_iCallsignOffset = Math.RandomIntInclusive(0, Math.Ceil(basesCount * 0.5));
		}

		Replication.BumpMe();

		array<SCR_SpawnPoint> spawnpoints = SCR_SpawnPoint.GetSpawnPoints();

		foreach (SCR_SpawnPoint spawnpoint : spawnpoints)
		{
			DisableExtraSpawnpoint(spawnpoint);
		}

		SCR_SpawnPoint.Event_SpawnPointAdded.Insert(DisableExtraSpawnpoint);

		// Start periodical checks for winning faction
		GetGame().GetCallqueue().CallLater(CheckForWinner, DEFAULT_DELAY, true);

		SCR_CharacterRankComponent.s_OnRankChanged.Insert(OnRankChanged);
		SCR_AmbientVehiclesManager.GetInstance().GetOnVehicleSpawned().Insert(OnAmbientVehicleSpawned);

		m_bStarted = true;
		Replication.BumpMe();
		m_BaseManager.OnAllBasesInitialized();
		OnStarted();
	}

	//------------------------------------------------------------------------------------------------
	void OnStarted()
	{
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnPointFactionAssigned);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(OnSpawnPointRemoved);

		if (m_OnStarted)
			m_OnStarted.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void OnCallsignOffsetChanged()
	{
		if (m_OnCallsignOffsetChanged)
			m_OnCallsignOffsetChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void DisableExtraSpawnpoint(SCR_SpawnPoint spawnpoint)
	{
		if (spawnpoint.Type() != SCR_SpawnPoint)
			return;

		spawnpoint.SetFaction(null);
	}

	//------------------------------------------------------------------------------------------------
	//! Find out if any faction has won and it's time to end the match
	protected void CheckForWinner()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		#ifndef AR_CAMPAIGN_TIMESTAMP
		float lowestVictoryTimestamp = float.MAX;
		float blockPauseTimestamp;
		float actualVictoryTimestamp;
		#else
		ChimeraWorld world = GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		WorldTimestamp lowestVictoryTimestamp;
		WorldTimestamp blockPauseTimestamp;
		WorldTimestamp actualVictoryTimestamp;
		#endif
		SCR_CampaignFaction winner;

		foreach (Faction faction : factions)
		{
			SCR_CampaignFaction fCast = SCR_CampaignFaction.Cast(faction);

			if (!fCast || !fCast.IsPlayable())
				continue;

			blockPauseTimestamp = fCast.GetPauseByBlockTimestamp();

			#ifndef AR_CAMPAIGN_TIMESTAMP
			if (blockPauseTimestamp == 0)
				actualVictoryTimestamp = fCast.GetVictoryTimestamp();
			else
				actualVictoryTimestamp = Replication.Time() + fCast.GetVictoryTimestamp() - fCast.GetPauseByBlockTimestamp();

			if (actualVictoryTimestamp != 0 && actualVictoryTimestamp < lowestVictoryTimestamp)
			{
				lowestVictoryTimestamp = actualVictoryTimestamp;
				winner = fCast;
			}
			#else
			if (blockPauseTimestamp == 0)
				actualVictoryTimestamp = fCast.GetVictoryTimestamp();
			else
				actualVictoryTimestamp = curTime.PlusMilliseconds(
					fCast.GetVictoryTimestamp().DiffMilliseconds(fCast.GetPauseByBlockTimestamp())
				);

			if (actualVictoryTimestamp != 0)
			{
				if (!winner || actualVictoryTimestamp.Less(lowestVictoryTimestamp))
				{
					lowestVictoryTimestamp = actualVictoryTimestamp;
					winner = fCast;
				}
			}
			#endif
		}

		if (winner)
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			if (lowestVictoryTimestamp <= Replication.Time())
			#else
			if (lowestVictoryTimestamp.LessEqual(curTime))
			#endif
			{
				GetGame().GetCallqueue().Remove(CheckForWinner);
				int winnerId = factionManager.GetFactionIndex(winner);
				RPC_DoEndMatch(winnerId);
				Rpc(RPC_DoEndMatch, winnerId);
				OnMatchSituationChanged();
			}
			else if (factionManager.GetFactionIndex(winner) != m_iWinningFactionId || winner.GetVictoryTimestamp() != m_fVictoryTimestamp || winner.GetPauseByBlockTimestamp() != m_fVictoryPauseTimestamp)
			{
				m_iWinningFactionId = factionManager.GetFactionIndex(winner);
				m_fVictoryTimestamp = winner.GetVictoryTimestamp();
				m_fVictoryPauseTimestamp = winner.GetPauseByBlockTimestamp();
				OnMatchSituationChanged();
				Replication.BumpMe();
			}
		}
		else if (m_iWinningFactionId != -1 || m_fVictoryTimestamp != 0)
		{
			m_iWinningFactionId = -1;
			#ifndef AR_CAMPAIGN_TIMESTAMP
			m_fVictoryTimestamp = 0;
			m_fVictoryPauseTimestamp = 0;
			#else
			m_fVictoryTimestamp = null;
			m_fVictoryPauseTimestamp = null;
			#endif
			OnMatchSituationChanged();
			Replication.BumpMe();
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoEndMatch(int winningFactionId)
	{
		m_bMatchOver = true;

		if (IsProxy())
			return;

		FactionManager fManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		fManager.GetFactionsList(factions);
		Faction winningFaction = fManager.GetFactionByIndex(winningFactionId);

		if (winningFaction)
		{
			foreach (Faction faction : factions)
			{
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(faction);

				if (!f)
					continue;

				if (winningFaction == f)
					f.SendHQMessage(SCR_ERadioMsg.VICTORY, param: winningFactionId);
				else
					f.SendHQMessage(SCR_ERadioMsg.DEFEAT, param: winningFactionId);
			}
		}

		// Match is over, save it so if "Continue" is selected following this the game is not loaded at an end screen
		GetGame().GetSaveManager().Save(ESaveType.AUTO);

		// For the server end the game, replicate to all clients.
		// listening components can react to this by e.g. showing end screen
		if (m_bTerminateScenario)
		{
			SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId: winningFactionId);
			EndGameMode(endData);
		}
	}
	//------------------------------------------------------------------------------------------------
	override void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);

		if (IsProxy() || !GetGame().InPlayMode())
			return;

		if (m_bAutostart)
			Start();

		m_bWorldPostProcessDone = true;
	}

	//------------------------------------------------------------------------------------------------
	FactionKey GetFactionKeyByEnum(SCR_ECampaignFaction faction)
	{
		switch (faction)
		{
			case SCR_ECampaignFaction.INDFOR:
			{
				return m_sINDFORFactionKey;
			};

			case SCR_ECampaignFaction.BLUFOR:
			{
				return m_sBLUFORFactionKey;
			};

			case SCR_ECampaignFaction.OPFOR:
			{
				return m_sOPFORFactionKey;
			};
		}

		return FactionKey.Empty;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetFactionByEnum(SCR_ECampaignFaction faction)
	{
		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return null;

		return SCR_CampaignFaction.Cast(fManager.GetFactionByKey(GetFactionKeyByEnum(faction)));
	}

	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//! Getter for "Rank required" parameter for spawning vehicles.
	// TRUE, if rank requirement is disabled
	bool CanRequestVehicleWithoutRank()
	{
		return m_bIgnoreMinimumVehicleRank;
	}

	//------------------------------------------------------------------------------------------------
	//! Handle the loaded struct; we want to apply it after gamemode has initialized
	void StoreLoadedData(notnull SCR_CampaignStruct struct)
	{
		m_LoadedData = struct;

		if (m_BaseManager.IsBasesInitDone())
			ApplyLoadedData();
		else
			m_BaseManager.GetOnAllBasesInitialized().Insert(ApplyLoadedData);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyLoadedData()
	{
		m_BaseManager.GetOnAllBasesInitialized().Remove(ApplyLoadedData);

		if (!m_LoadedData)
			return;

		// Game was saved after match was over, don't load
		if (m_LoadedData.IsMatchOver())
			return;

		array<ref SCR_CampaignBaseStruct>basesStructs = m_LoadedData.GetBasesStructs();

		// No bases data available for load, something is wrong - terminate
		if (basesStructs.IsEmpty())
			return;

		m_bIsSessionLoadInProgress = true;
		m_BaseManager.LoadBasesStates(basesStructs);

		// We need to wait for all services to spawn before switching the progress bool to false so supplies are not deducted from bases
		GetGame().GetCallqueue().CallLater(EndSessionLoadProgress, DEFAULT_DELAY * 2);

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			m_BaseManager.InitializeSupplyDepotIcons();
			m_BaseManager.HideUnusedBaseIcons();
		}

		m_BaseManager.RecalculateRadioCoverage(GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
		m_BaseManager.RecalculateRadioCoverage(GetFactionByEnum(SCR_ECampaignFaction.OPFOR));

		SCR_TimeAndWeatherHandlerComponent timeHandler = SCR_TimeAndWeatherHandlerComponent.GetInstance();

		// Weather has to be changed after init
		if (timeHandler)
		{
			GetGame().GetCallqueue().Remove(timeHandler.SetupDaytimeAndWeather);
			GetGame().GetCallqueue().CallLater(timeHandler.SetupDaytimeAndWeather, DEFAULT_DELAY, false, m_LoadedData.GetHours(), m_LoadedData.GetMinutes(), m_LoadedData.GetSeconds(), m_LoadedData.GetWeatherState(), true);
		}

		SCR_CampaignTutorialArlandComponent tutorial = SCR_CampaignTutorialArlandComponent.Cast(FindComponent(SCR_CampaignTutorialArlandComponent));

		if (tutorial)
		{
			tutorial.SetResumeStage(m_LoadedData.GetTutorialStage());
			return;
		}

		m_iCallsignOffset = m_LoadedData.GetCallsignOffset();
		Replication.BumpMe();

		LoadRemnantsStates(m_LoadedData.GetRemnantsStructs());
		LoadClientData(m_LoadedData.GetPlayersStructs());

		SCR_CampaignFaction factionBLUFOR = GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignFaction factionOPFOR = GetFactionByEnum(SCR_ECampaignFaction.OPFOR);

		// Delayed spawns to avoid calling them during init
		if (factionBLUFOR && m_LoadedData.GetMHQLocationBLUFOR() != vector.Zero)
			GetGame().GetCallqueue().CallLater(SpawnMobileHQ, DEFAULT_DELAY, false, factionBLUFOR, m_LoadedData.GetMHQLocationBLUFOR(), m_LoadedData.GetMHQRotationBLUFOR());

		if (factionOPFOR && m_LoadedData.GetMHQLocationOPFOR() != vector.Zero)
			GetGame().GetCallqueue().CallLater(SpawnMobileHQ, DEFAULT_DELAY, false, factionOPFOR, m_LoadedData.GetMHQLocationOPFOR(), m_LoadedData.GetMHQRotationOPFOR());

		m_LoadedData = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void EndSessionLoadProgress()
	{
		m_bIsSessionLoadInProgress = false;
	}

	//------------------------------------------------------------------------------------------------
	void StoreRemnantsStates(out notnull array<ref SCR_CampaignRemnantInfoStruct> outEntries)
	{
		SCR_AmbientPatrolManager manager = SCR_AmbientPatrolManager.GetInstance();

		if (!manager)
			return;

		array<int> remnantsInfo = {};

		for (int i = 0, count = manager.GetRemainingPatrolsInfo(remnantsInfo); i < count; i++)
		{
			SCR_CampaignRemnantInfoStruct struct = new SCR_CampaignRemnantInfoStruct();
			struct.SetID(remnantsInfo[i]);
			struct.SetMembersAlive(remnantsInfo[i + 1]);
			struct.SetRespawnTimer(remnantsInfo[i + 2]);
			outEntries.Insert(struct);
			i += 2;
		}
	}

	//------------------------------------------------------------------------------------------------
	void LoadRemnantsStates(notnull array<ref SCR_CampaignRemnantInfoStruct> entries)
	{
		SCR_AmbientPatrolManager manager = SCR_AmbientPatrolManager.GetInstance();
		array<SCR_AmbientPatrolSpawnPointComponent> patrols = {};
		manager.GetPatrols(patrols);
		#ifdef AR_CAMPAIGN_TIMESTAMP
		ChimeraWorld world = GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		#endif

		foreach (SCR_AmbientPatrolSpawnPointComponent presence : patrols)
		{
			if (!presence)
				continue;

			foreach (SCR_CampaignRemnantInfoStruct info : entries)
			{
				if (info.GetID() == presence.GetID())
				{
					presence.SetMembersAlive(info.GetMembersAlive());
					#ifndef AR_CAMPAIGN_TIMESTAMP
					presence.SetRespawnTimestamp(info.GetRespawnTimer());
					#else
					presence.SetRespawnTimestamp(curTime.PlusMilliseconds(info.GetRespawnTimer()));
					#endif
				}
			}
		}

		m_bRemnantsStateLoaded = true;
	}

	//------------------------------------------------------------------------------------------------
	// Triggered each time player built a composition
	void OnStructureBuilt(SCR_CampaignMilitaryBaseComponent base, SCR_EditableEntityComponent entity, bool add)
	{
		if (IsTutorial())
		{
			SCR_CampaignTutorialArlandComponent tutorial = SCR_CampaignTutorialArlandComponent.Cast(FindComponent(SCR_CampaignTutorialArlandComponent));

			if (tutorial)
				tutorial.OnStructureBuilt(base, entity.GetOwner());
			else
				SCR_CampaignTutorialArlandComponent.GetOnStructureBuilt().Invoke(base, entity.GetOwner());
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetIsTutorial(bool isTutorial)
	{
		m_bIsTutorial = isTutorial;

		if (m_bIsTutorial)
			SCR_PopUpNotification.SetFilter(SCR_EPopupMsgFilter.TUTORIAL);
		else
			SCR_PopUpNotification.SetFilter(SCR_EPopupMsgFilter.ALL);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetActiveRespawnRadios(int factionIndex, int count)
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		faction.SetActiveRespawnRadios(count);
	}

	//------------------------------------------------------------------------------------------------
	void AddActiveRespawnRadio(notnull SCR_CampaignFaction faction, SCR_PlayerRadioSpawnPointCampaign processedSpawnpoint = null)
	{
		if (processedSpawnpoint)
		{
			if (m_aRadioSpawnPoints.Contains(processedSpawnpoint))
				return;

			m_aRadioSpawnPoints.Insert(processedSpawnpoint);
		}

		int index = GetGame().GetFactionManager().GetFactionIndex(faction);
		int count = faction.GetActiveRespawnRadios() + 1;

		Rpc(RPC_DoSetActiveRespawnRadios, index, count);
		RPC_DoSetActiveRespawnRadios(index, count);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveActiveRespawnRadio(notnull SCR_CampaignFaction faction, SCR_PlayerRadioSpawnPointCampaign processedSpawnpoint = null)
	{
		if (processedSpawnpoint)
		{
			int index = m_aRadioSpawnPoints.Find(processedSpawnpoint);

			if (index == -1)
				return;

			m_aRadioSpawnPoints.Remove(index);
		}

		int index = GetGame().GetFactionManager().GetFactionIndex(faction);
		int previousActiveRadios = faction.GetActiveRespawnRadios();

		Rpc(RPC_DoSetActiveRespawnRadios, index, previousActiveRadios - 1);
		RPC_DoSetActiveRespawnRadios(index, previousActiveRadios - 1);

		// Check all players for radios if limit is no longer maxed, activate a dormant one
		if (previousActiveRadios == GetMaxRespawnRadios())
			ReactivatePlayerSpawnpoint(faction, processedSpawnpoint);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks all players if they're carrying a long range radio, activates a spawnpoint on the first valid one found
	void ReactivatePlayerSpawnpoint(notnull SCR_CampaignFaction faction, SCR_PlayerRadioSpawnPointCampaign processedSpawnpoint)
	{
		array<SCR_SpawnPoint> allSpawnpoints = SCR_SpawnPoint.GetSpawnPoints();
		PlayerManager pMan = GetGame().GetPlayerManager();

		foreach (SCR_SpawnPoint spawnpoint : allSpawnpoints)
		{
			if (spawnpoint == processedSpawnpoint)
				continue;

			SCR_PlayerRadioSpawnPointCampaign conflictSpawnpoint = SCR_PlayerRadioSpawnPointCampaign.Cast(spawnpoint);

			if (!conflictSpawnpoint || conflictSpawnpoint.GetFactionKey() == faction.GetFactionKey())
				continue;

			SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pMan.GetPlayerControlledEntity(conflictSpawnpoint.GetPlayerID()));

			if (!player)
				continue;

			CharacterControllerComponent charController = player.GetCharacterController();

			if (!charController || charController.IsDead())
				continue;

			Faction playerFaction = player.GetFaction();

			if (!playerFaction || playerFaction != faction)
				continue;

			EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(player.FindComponent(EquipedLoadoutStorageComponent));

			if (!loadoutStorage)
				continue;

			IEntity backpack = loadoutStorage.GetClothFromArea(LoadoutBackpackArea);

			if (!backpack || !backpack.FindComponent(SCR_RadioComponent))
				continue;

			if (!m_BaseManager.IsEntityInFactionRadioSignal(player, playerFaction))
				continue;

			BaseLoadoutClothComponent loadoutCloth = BaseLoadoutClothComponent.Cast(backpack.FindComponent(BaseLoadoutClothComponent));

			if (loadoutCloth && loadoutCloth.GetAreaType().IsInherited(LoadoutBackpackArea))
			{
				conflictSpawnpoint.ActivateSpawnPointPublic();

				if (faction.GetActiveRespawnRadios() == GetMaxRespawnRadios())
					return;
				else
					continue;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Identify radio operators inside or outside of radio range, enable or disable their spawpoints accordingly
	protected void CheckRadioSpawnpointsSignalCoverage()
	{
		array<SCR_SpawnPoint> spawnpoints = SCR_SpawnPoint.GetSpawnPoints();
		SCR_CampaignFactionManager factionM = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!factionM)
			return;

		foreach (SCR_SpawnPoint spawnpoint : spawnpoints)
		{
			SCR_PlayerRadioSpawnPointCampaign spawnpointC = SCR_PlayerRadioSpawnPointCampaign.Cast(spawnpoint);

			if (!spawnpointC)
				continue;

			if (spawnpointC.GetFlags() & EntityFlags.STATIC)
				continue;

			if (spawnpointC.GetOrigin() == vector.Zero)
				continue;

			Faction faction = spawnpointC.GetCachedFaction();

			if (!faction)
				continue;

			IEntity owner = GetGame().GetPlayerManager().GetPlayerControlledEntity(spawnpointC.GetPlayerID());
			bool isRenegade = SCR_CharacterRankComponent.GetCharacterRank(owner) == SCR_ECharacterRank.RENEGADE;

			if (isRenegade)
			{
				spawnpointC.DeactivateSpawnPointPublic();
				continue;
			}

			bool isInRange = m_BaseManager.IsEntityInFactionRadioSignal(spawnpointC, faction);

			if (isInRange)
				spawnpointC.SetFaction(faction);
			else
				spawnpointC.SetFaction(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyClientData(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!pc)
			return;

		SCR_CampaignClientData clientData = GetClientData(playerId);

		if (!clientData)
			return;

		bool allowFactionLoad = true;

#ifdef ENABLE_DIAG
		if (SCR_RespawnComponent.Diag_IsCLISpawnEnabled())
			allowFactionLoad = false;
#endif

		// Automatically apply the client's previous faction
		int forcedFaction = clientData.GetFactionIndex();

		if (allowFactionLoad && forcedFaction != -1)
		{
			SCR_PlayerFactionAffiliationComponent fac = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));

			if (fac)
			{
				Faction faction = GetGame().GetFactionManager().GetFactionByIndex(forcedFaction);
				fac.RequestFaction(faction);
			}
		}

		int xp;
		SCR_PlayerXPHandlerComponent handlerXP = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

		if (handlerXP)
			xp = handlerXP.GetPlayerXP();

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));

		if (comp)
			comp.AwardXP(playerId, SCR_EXPRewards.UNDEFINED, 1, false, clientData.GetXP() - xp);
	}

	//------------------------------------------------------------------------------------------------
	void LoadClientData(notnull array<ref SCR_CampaignPlayerStruct> data)
	{
		m_aRegisteredClients.Clear();

		foreach (SCR_CampaignPlayerStruct playerData : data)
		{
			SCR_CampaignClientData clientData = new SCR_CampaignClientData();

			clientData.SetID(playerData.GetID());
			clientData.SetXP(playerData.GetXP());
			clientData.SetFactionIndex(playerData.GetFactionIndex());

			m_aRegisteredClients.Insert(clientData);
		}
	}

	//------------------------------------------------------------------------------------------------
	void WriteAllClientsData()
	{
		array<int> pcList = {};

		for (int i = 0, playersCount = GetGame().GetPlayerManager().GetPlayers(pcList); i < playersCount; i++)
		{
			PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(pcList[i]);

			if (!pc)
				continue;

			int ID = pc.GetPlayerId();
			WriteClientData(ID, pc: pc);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Save object with player's current data
	protected void WriteClientData(int playerID, bool disconnecting = false, PlayerController pc = null)
	{
		SCR_CampaignClientData clientData = GetClientData(playerID);

		if (!clientData)
			return;

		if (!pc)
			pc = GetGame().GetPlayerManager().GetPlayerController(playerID);

		if (!pc)
			return;

		SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

		if (!comp)
			return;

		// Set data readable from PlayerController
		clientData.SetXP(comp.GetPlayerXP());
	}

	//------------------------------------------------------------------------------------------------
	void SpawnMobileHQ(notnull SCR_CampaignFaction faction, vector pos, vector rot)
	{
		if (faction.GetMobileAssembly())
			return;

		EntitySpawnParams params = EntitySpawnParams();
		GetWorldTransform(params.Transform);
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(rot, params.Transform);
		params.Transform[3] = pos;

		IEntity MHQ = GetGame().SpawnEntityPrefab(Resource.Load(faction.GetMobileHQPrefab()), null, params);

		if (!MHQ)
			return;

		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(MHQ.FindComponent(BaseRadioComponent));

		if (radioComponent && radioComponent.TransceiversCount() > 0)
		{
			radioComponent.SetPower(false);
			radioComponent.GetTransceiver(0).SetFrequency(faction.GetFactionRadioFrequency());
			radioComponent.SetEncryptionKey(faction.GetFactionRadioEncryptionKey());
		}

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(MHQ.FindComponent(SlotManagerComponent));

		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		foreach (EntitySlotInfo slot : slots)
		{
			IEntity truckBed = slot.GetAttachedEntity();

			if (!truckBed)
				continue;

			SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));

			if (mobileAssemblyComponent)
			{
				mobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(faction));
				mobileAssemblyComponent.UpdateRadioCoverage();
				mobileAssemblyComponent.Deploy(SCR_EMobileAssemblyStatus.DEPLOYED);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetClientsData(notnull out array<ref SCR_CampaignClientData> dataArray)
	{
		int count;

		foreach (SCR_CampaignClientData data : m_aRegisteredClients)
		{
			count++;
			dataArray.Insert(data);
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Get corresponding client data, create new object if not found
	SCR_CampaignClientData GetClientData(int playerId)
	{
		if (playerId == 0)
			return null;

		string playerIdentity = SCR_CampaignPlayerStruct.GetPlayerIdentity(playerId);

		if (playerIdentity == string.Empty)
			return null;

		SCR_CampaignClientData clientData;

		// Check if the client is reconnecting
		for (int i = 0, clientsCount = m_aRegisteredClients.Count(); i < clientsCount; i++)
		{
			if (m_aRegisteredClients[i].GetID() == playerIdentity)
			{
				clientData = m_aRegisteredClients[i];
				break;
			}
		}

		if (!clientData)
		{
			clientData = new SCR_CampaignClientData;
			clientData.SetID(playerIdentity);
			m_aRegisteredClients.Insert(clientData);
		}

		return clientData;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAmbientVehicleSpawned(SCR_AmbientVehicleSpawnPointComponent spawnpoint, Vehicle vehicle)
	{
		SCR_HelicopterDamageManagerComponent helicopterDamageManager = SCR_HelicopterDamageManagerComponent.Cast(vehicle.FindComponent(SCR_HelicopterDamageManagerComponent));

		// Ignore non-helicopter vehicles
		if (!helicopterDamageManager)
			return;

		array<HitZone> hitZones = {};
		helicopterDamageManager.GetAllHitZones(hitZones);
		vector transform[3];
		transform[0] = vehicle.GetOrigin();
		transform[1] = vector.Forward;
		transform[2] = vector.Up;

		// Damage the engine and hull
		foreach (HitZone hitZone : hitZones)
		{
			if (!hitZone.IsInherited(SCR_EngineHitZone) && !hitZone.IsInherited(SCR_FlammableHitZone))
				continue;

			helicopterDamageManager.HandleDamage(EDamageType.TRUE, hitZone.GetMaxHealth() * 0.75, transform, vehicle, hitZone, Instigator.CreateInstigator(null), null, -1, -1);
		}

		array<SCR_FuelManagerComponent> fuelManagers = {};
		array<BaseFuelNode> fuelNodes = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(vehicle, fuelManagers);

		// Remove all fuel
		foreach (SCR_FuelManagerComponent fuelManager : fuelManagers)
		{
			fuelNodes.Clear();
			fuelManager.GetFuelNodesList(fuelNodes);

			foreach (BaseFuelNode fuelNode : fuelNodes)
			{
				fuelNode.SetFuel(0.0);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRankChanged(SCR_ECharacterRank oldRank, SCR_ECharacterRank newRank, notnull IEntity owner, bool silent)
	{
		if (silent)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner);
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetPlayerFaction(playerId));

		if (!faction)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		if (!factionManager)
			return;

		SCR_RankIDCampaign rank = SCR_RankIDCampaign.Cast(factionManager.GetRankByID(newRank));

		if (!rank)
			return;

		SCR_ERadioMsg radio;

		if (newRank < oldRank && !rank.IsRankRenegade())
			radio = SCR_ERadioMsg.DEMOTION;
		else
			radio = rank.GetRadioMsg();

		faction.SendHQMessage(radio, calledID: playerId, public: false, param: newRank)
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));

		if (playerController)
		{
			SCR_PlayerFactionAffiliationComponent playerFactionAff = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));

			if (playerFactionAff)
			{
				playerFactionAff.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse_O);
				playerFactionAff.GetOnPlayerFactionResponseInvoker_S().Insert(OnPlayerFactionResponse_S);
			}
		}

		super.OnPlayerRegistered(playerId);

		if (!playerController)
			return;

		// Normally this is done in OnPlayerAuditSuccess, but in SP the callback is not triggered
		if (RplSession.Mode() == RplMode.None && !m_bIsTutorial)
			ApplyClientData(playerId);

		// See HandleOnFactionAssigned()
		if (SCR_PlayerController.GetLocalPlayerId() == 0)
			return;

		int key;

		for (int i = 0, count = m_mUnprocessedFactionAssignments.Count(); i < count; i++)
		{
			key = m_mUnprocessedFactionAssignments.GetKey(i);

			if (key == SCR_PlayerController.GetLocalPlayerId())
			{
				ProcessFactionAssignment(m_mUnprocessedFactionAssignments.Get(key));
				m_mUnprocessedFactionAssignments.Remove(key);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int iPlayerID)
	{
		super.OnPlayerAuditSuccess(iPlayerID);

		// Apply data with a delay so client's game has time to initialize and register faction setting
		if (RplSession.Mode() != RplMode.None)
			GetGame().GetCallqueue().CallLater(ApplyClientData, MINIMUM_DELAY, false, iPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		GetTaskManager().OnPlayerDisconnected(playerId);
		WriteClientData(playerId, true);
		m_BaseManager.OnPlayerDisconnected(playerId)
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		super.OnPlayerSpawnFinalize_S(requestComponent, handlerComponent, data, entity);

		PlayerController pc = requestComponent.GetPlayerController();

		if (!pc)
			return;

		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));

		if (campaignNetworkComponent)
			campaignNetworkComponent.OnPlayerAliveStateChanged(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnOnPoint_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity entity, SCR_SpawnPointSpawnData spawnPointData)
	{
		super.OnPlayerSpawnOnPoint_S(requestComponent, handlerComponent, entity, spawnPointData);

		// Award XP for the owner of the respawn radio (if applicable)
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));

		if (compXP)
		{
			SCR_SpawnPoint spawnpoint = spawnPointData.GetSpawnPoint();

			if (spawnpoint)
			{
				SCR_PlayerSpawnPoint playerSpawnpoint = SCR_PlayerSpawnPoint.Cast(spawnpoint);
				SCR_DeployableSpawnPoint radioSpawnpoint = SCR_DeployableSpawnPoint.Cast(spawnpoint);

				if (playerSpawnpoint)
				{
					compXP.AwardXP(playerSpawnpoint.GetPlayerID(), SCR_EXPRewards.SPAWN_PROVIDER);
				}
				else if (radioSpawnpoint)
				{
					SCR_BaseDeployableSpawnPointComponent comp = radioSpawnpoint.GetDeployableSpawnPointComponent();
				
					if (comp)
					{
						int playerId = comp.GetItemOwnerID();
						
						// Don't award XP if player respawns on their own spawnpoint
						if (playerId != GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity))
							compXP.AwardXP(playerId, SCR_EXPRewards.SPAWN_PROVIDER);
					}
				}
			}
		}

		IEntity parent = spawnPointData.GetSpawnPoint().GetParent();
		SCR_CampaignMilitaryBaseComponent base;

		// Find spawnpoint's parent base, deduct supplies if applicable
		while (parent)
		{
			base = SCR_CampaignMilitaryBaseComponent.Cast(parent.FindComponent(SCR_CampaignMilitaryBaseComponent));

			if (base)
				break;

			parent = parent.GetParent();
		}

		if (!base)
			return;

		if (!base.IsHQ())
			base.AddSupplies(-base.GetBaseSpawnCost());

		// Location popup for player
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(requestComponent.GetPlayerId());

		if (playerController)
		{
			SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));

			if (campaignNetworkComponent)
				campaignNetworkComponent.RespawnLocationPopup(base.GetCallsign());
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnSpawnPointFactionAssigned(SCR_SpawnPoint spawnpoint)
	{
		IEntity owner = spawnpoint.GetParent();

		if (owner)
		{
			SCR_CampaignMilitaryBaseComponent parentBase = SCR_CampaignMilitaryBaseComponent.Cast(owner.FindComponent(SCR_CampaignMilitaryBaseComponent));

			if (parentBase)
				parentBase.OnSpawnPointFactionAssigned(spawnpoint.GetFactionKey())
		}

		if (IsProxy())
			return;

		// Handle amount of active respawn radios
		SCR_PlayerRadioSpawnPointCampaign spawnpointC = SCR_PlayerRadioSpawnPointCampaign.Cast(spawnpoint);

		if (!spawnpointC)
			return;

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(spawnpointC.GetFactionKey()));

		if (faction)
		{
			if (faction.GetActiveRespawnRadios() >= GetMaxRespawnRadios())
				spawnpointC.DeactivateSpawnPointPublic();
			else
				AddActiveRespawnRadio(faction, spawnpointC);
		}
		else
		{
			SCR_CampaignFaction previousFaction = SCR_CampaignFaction.Cast(spawnpointC.GetCachedFaction());

			if (previousFaction)
				RemoveActiveRespawnRadio(previousFaction, spawnpointC);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnSpawnPointRemoved(SCR_SpawnPoint spawnpoint)
	{
		if (IsProxy())
			return;

		// If radio spawnpoint has been removed, free up a slot for it
		SCR_PlayerRadioSpawnPointCampaign spawnpointC = SCR_PlayerRadioSpawnPointCampaign.Cast(spawnpoint);

		if (!spawnpointC)
			return;

		FactionKey faction = spawnpointC.GetFactionKey();

		if (faction.IsEmpty())
			return;

		RemoveActiveRespawnRadio(SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByKey(faction)), spawnpointC);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);

		if (IsProxy())
			return;

		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!pc)
			return;

		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
		
		if (campaignNetworkComponent)
		{
			campaignNetworkComponent.OnPlayerAliveStateChanged(false);
			campaignNetworkComponent.ResetSavedSupplies();
		}

		UpdateRespawnPenalty(playerId);

		if (playerId == killer.GetInstigatorPlayerID())
			OnSuicide(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Handles forgiving of post-suicide deploy timer penalties
	protected void UpdateRespawnPenalty(int playerId)
	{
		if (m_iSuicideRespawnDelay == 0 || m_iSuicideForgiveCooldown == 0)
			return;

		SCR_CampaignClientData clientData = GetClientData(playerId);

		if (!clientData)
			return;

		float respawnPenalty = clientData.GetRespawnPenalty();

		if (respawnPenalty == 0)
			return;

		float curTime = GetGame().GetWorld().GetWorldTime();
		float penaltyCooldownMs = (float)m_iSuicideForgiveCooldown * 1000;
		float timeSinceLastDeduction = curTime - clientData.GetLastPenaltyDeductionTimestamp();
		float penaltiesForgiven = Math.Floor(timeSinceLastDeduction / penaltyCooldownMs);

		if (penaltiesForgiven < 1)
			return;

		clientData.SetLastPenaltyDeductionTimestamp(curTime);
		float forgivenPenalty = (float)m_iSuicideRespawnDelay * penaltiesForgiven;
		clientData.SetRespawnPenalty(respawnPenalty - forgivenPenalty);

		array<Managed> timers = {};
		FindComponents(SCR_RespawnTimerComponent, timers);

		foreach (Managed timer : timers)
		{
			// Skip this specific type as it's handled separately for radio spawns
			if (timer.Type() == SCR_TimedSpawnPointComponent)
				continue;

			SCR_RespawnTimerComponent timerCast = SCR_RespawnTimerComponent.Cast(timer);

			if (!timerCast)
				continue;

			timerCast.SetRespawnTime(playerId, timerCast.GetRespawnTime() - respawnPenalty - forgivenPenalty);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSuicide(int playerId)
	{
		// Don't issue penalties in WB so it doesn't interfere with debugging etc.
#ifdef WORKBENCH
		return;
#endif
#ifdef NO_SUICIDE_PENALTY
		return;
#endif
		if (m_bIsTutorial)
			return;

		if (m_iSuicideRespawnDelay == 0)
			return;
		
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		// Do not process suicide if player was unconscious upon death
		if (pc)
		{
			SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.Cast(pc.FindComponent(SCR_CampaignFeedbackComponent));
			
			if (comp && !comp.IsConscious())
				return;
		}

		SCR_CampaignClientData clientData = GetClientData(playerId);

		if (!clientData)
			return;

		float respawnPenalty = clientData.GetRespawnPenalty();
		float lastSuicideTimestamp = clientData.GetLastSuicideTimestamp();
		float curTime = GetGame().GetWorld().GetWorldTime();
		clientData.SetLastSuicideTimestamp(curTime);

		if (lastSuicideTimestamp == 0)
			return;

		float timeSinceLastSuicide = curTime - lastSuicideTimestamp;
		float penaltyCooldownMs = (float)m_iSuicidePenaltyCooldown * 1000;

		// Last suicide happened long enough time ago, don't issue a penalty
		if (timeSinceLastSuicide > penaltyCooldownMs)
			return;

		float addedPenalty = m_iSuicideRespawnDelay;

		clientData.SetLastPenaltyDeductionTimestamp(curTime);
		clientData.SetRespawnPenalty(respawnPenalty + addedPenalty);

		array<Managed> timers = {};
		FindComponents(SCR_RespawnTimerComponent, timers);

		foreach (Managed timer : timers)
		{
			// Skip this specific type as it's handled separately for radio spawns
			if (timer.Type() == SCR_TimedSpawnPointComponent)
				continue;

			SCR_RespawnTimerComponent timerCast = SCR_RespawnTimerComponent.Cast(timer);

			if (!timerCast)
				continue;

			timerCast.SetRespawnTime(playerId, timerCast.GetRespawnTime() + respawnPenalty + addedPenalty);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Award additional XP for enemies killed in friendly bases
	override void OnControllableDestroyed(IEntity entity, IEntity killerEntity, notnull Instigator instigator)
	{
		super.OnControllableDestroyed(entity, killerEntity, instigator);

		if (IsProxy())
			return;

		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));

		if (!compXP)
			return;

		// Ignore AI or NONE instigators
		if (instigator.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;

		int killerId = instigator.GetInstigatorPlayerID();

		SCR_ChimeraCharacter victimCharacter = SCR_ChimeraCharacter.Cast(entity);

		if (!victimCharacter)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		Faction factionKiller = Faction.Cast(factionManager.GetPlayerFaction(killerId));
		if (!factionKiller)
			return;

		//Friendly kills are not to be rewarded
		if (factionKiller.IsFactionFriendly(victimCharacter.GetFaction()))
			return;

		vector victimPos = victimCharacter.GetOrigin();

		SCR_CampaignMilitaryBaseComponent nearestBase = m_BaseManager.FindClosestBase(victimPos);

		if (!nearestBase)
			return;

		//this awards additional XP to base defenders, so if the instigator is not in his own base, there should be no reward
		if (nearestBase.GetFaction() != factionKiller)
			return;

		if (vector.DistanceXZ(victimPos, nearestBase.GetOwner().GetOrigin()) > nearestBase.GetRadius())
			return;

		compXP.AwardXP(killerId, SCR_EXPRewards.CUSTOM_1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionResponse_S(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (!response)
			return;

		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(component.GetPlayerController());

		if (!playerController)
			return;

		int playerID = playerController.GetPlayerId();

		// Save faction selected in client's data
		SCR_CampaignClientData clientData;
		clientData = GetClientData(playerID);

		if (clientData && faction)
			clientData.SetFactionIndex(factionManager.GetFactionIndex(faction));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionResponse_O(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (!response)
			return;

		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(component.GetPlayerController());

		if (!playerController)
			return;

		int playerID = playerController.GetPlayerId();

		// When a faction is being assigned to the client automatically by server, playerId might not yet be registered
		// In that case, this saves the connecting player's data and processes them later in local OnPlayerRegistered()
		if (SCR_PlayerController.GetLocalPlayerId() == playerID)
			ProcessFactionAssignment(faction);
		else
			m_mUnprocessedFactionAssignments.Set(playerID, faction);
	}

	//------------------------------------------------------------------------------------------------
	//! See HandleOnFactionAssigned
	protected void ProcessFactionAssignment(Faction assignedFaction)
	{
		m_BaseManager.SetLocalPlayerFaction(SCR_CampaignFaction.Cast(assignedFaction));

		if (m_OnFactionAssignedLocalPlayer)
			m_OnFactionAssignedLocalPlayer.Invoke(assignedFaction);

		// Delayed call so tasks are properly initialized
		if (IsProxy())
			GetGame().GetCallqueue().CallLater(m_BaseManager.UpdateTaskBases, DEFAULT_DELAY, false, assignedFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when an entity is spawned by an EntitySpawnerComponent
	void OnEntityRequested(notnull IEntity spawnedEntity, IEntity user, SCR_Faction faction)
	{
		if (IsProxy() || !spawnedEntity.IsInherited(Vehicle))
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		int playerId = playerManager.GetPlayerIdFromControlledEntity(user);

		if (playerId == 0)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));

		if (!playerController)
			return;

		SCR_CampaignNetworkComponent networkComp = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));

		if (!networkComp)
			return;

		ChimeraWorld world = spawnedEntity.GetWorld();
		#ifndef AR_CAMPAIGN_TIMESTAMP
		networkComp.SetLastRequestTimestamp(Replication.Time());
		#else
		networkComp.SetLastRequestTimestamp(world.GetServerTimestamp());
		#endif

		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(spawnedEntity.FindComponent(BaseRadioComponent));

		// Assign faction radio frequency
		if (radioComponent)
		{
			BaseTransceiver transceiver = radioComponent.GetTransceiver(0);

			if (transceiver)
			{
				radioComponent.SetPower(false);
				transceiver.SetFrequency(faction.GetFactionRadioFrequency());
				radioComponent.SetEncryptionKey(faction.GetFactionRadioEncryptionKey());
			}
		}

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(spawnedEntity.FindComponent(SlotManagerComponent));

		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		IEntity truckBed;
		SCR_CampaignSuppliesComponent suppliesComponent;
		SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent;
		EventHandlerManagerComponent eventHandlerManager;

		// Handle Conflict-specific vehicles
		foreach (EntitySlotInfo slot : slots)
		{
			if (!slot)
				continue;

			truckBed = slot.GetAttachedEntity();

			if (!truckBed)
				continue;

			mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));

			// Mobile HQ
			if (mobileAssemblyComponent)
			{
				mobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(faction));
				networkComp.SendVehicleSpawnHint(EHint.CONFLICT_MOBILE_HQ);
			}
		}
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);

		// Cheat menu
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP, 0);
			PlayerController pc = GetGame().GetPlayerController();

			if (pc)
			{
				SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

				if (comp)
					comp.CheatRank();
			}
		}

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN, 0);
			PlayerController pc = GetGame().GetPlayerController();

			if (pc)
			{
				SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

				if (comp)
					comp.CheatRank(true);
			}
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	void SCR_GameModeCampaign(IEntitySource src, IEntity parent)
	{
		// Attributes check
		if (m_sBLUFORFactionKey == FactionKey.Empty)
			Print("SCR_GameModeCampaign: Empty BLUFOR faction key!", LogLevel.ERROR);

		if (m_sOPFORFactionKey == FactionKey.Empty)
			Print("SCR_GameModeCampaign: Empty OPFOR faction key!", LogLevel.ERROR);

		if (m_sINDFORFactionKey == FactionKey.Empty)
			Print("SCR_GameModeCampaign: Empty INDFOR faction key!", LogLevel.ERROR);

		if (!GetGame().InPlayMode())
			return;

		// Cheat menu
#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MENU, "Conflict", "");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_INSTANT_BUILDING, "", "Instant composition spawning", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP, "", "Promotion", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN, "", "Demotion", "Conflict");
		SetFlags(EntityFlags.ACTIVE, false);
		ConnectToDiagSystem();
#endif

		// Parameters override from header
		SCR_MissionHeaderCampaign header = SCR_MissionHeaderCampaign.Cast(GetGame().GetMissionHeader());

		if (header)
		{
			m_bIgnoreMinimumVehicleRank = header.m_bIgnoreMinimumVehicleRank;

			int suppliesMax = header.m_iMaximumBaseSupplies;
			int suppliesMin = header.m_iMinimumBaseSupplies;
			int respawnRadiosCount = header.m_iMaximumRespawnRadios;
			int controlPointsLimit = header.m_iControlPointsCap;
			int victoryTimeout = header.m_fVictoryTimeout;

			if (suppliesMax != -1)
				m_iMaxStartingSupplies = suppliesMax;

			if (suppliesMin != -1)
				m_iMinStartingSupplies = suppliesMin;

			if (respawnRadiosCount != -1)
				m_iMaxRespawnRadios = respawnRadiosCount;

			if (controlPointsLimit != -1)
				m_iControlPointsThreshold = controlPointsLimit;

			if (victoryTimeout != -1)
				m_fVictoryTimer = victoryTimeout;
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_GameModeCampaign()
	{
		DisconnectFromDiagSystem();

		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(OnSpawnPointFactionAssigned);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Remove(OnSpawnPointRemoved);

		SCR_AmbientVehiclesManager manager = SCR_AmbientVehiclesManager.GetInstance(false);

		if (manager)
			manager.GetOnVehicleSpawned().Remove(OnAmbientVehicleSpawned);
	}
}
