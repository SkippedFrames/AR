#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
class SCR_SeizingComponentClass : SCR_MilitaryBaseLogicComponentClass
{
	[Attribute("{59A6F1EBC6C64F79}Prefabs/Logic/SeizingTrigger.et", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sTriggerPrefab;

	//------------------------------------------------------------------------------------------------
	ResourceName GetTriggerPrefab()
	{
		return m_sTriggerPrefab;
	}
}

//------------------------------------------------------------------------------------------------
#ifdef AR_CAMPAIGN_TIMESTAMP
void OnTimerChangeFn(WorldTimestamp newStart, WorldTimestamp newEnd);
typedef func OnTimerChangeFn;
typedef ScriptInvokerBase<OnTimerChangeFn> OnTimerChangeInvoker;

//------------------------------------------------------------------------------------------------
#endif
class SCR_SeizingComponent : SCR_MilitaryBaseLogicComponent
{
	[Attribute("100")]
	protected int m_iRadius;

	[Attribute("7", desc: "Units in a vehicle (most probably aircraft) above this altitude will be ignored.")]
	protected int m_iMaximumAltitude;

	[Attribute("10")]
	protected float m_fMaximumSeizingTime;

	[Attribute("6")]
	protected float m_fMinimumSeizingTime;

	[Attribute("5", "How many characters need to be seizing at once to achieve the minimum seizing time.")]
	protected int m_iMaximumSeizingCharacters;

	[Attribute("0", desc: "How long after respawn is player able to start seizing or defending.")]
	protected float m_fRespawnCooldownPeriod;

	[Attribute("0", desc: "When checked, the seizing timer will decrease gradually when the seizing is interrupted.")]
	protected bool m_bGradualTimerReset;

	[Attribute("0", desc: "Allow seizing for playable factions only.")]
	protected bool m_bIgnoreNonPlayableAttackers;

	[Attribute("0", desc: "Allow defending for playable factions only.")]
	protected bool m_bIgnoreNonPlayableDefenders;

	[Attribute("1")]
	protected bool m_bShowNotifications;

	[Attribute(ENotification.AREA_SEIZING_DONE_FRIENDLIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eCapturedByFriendliesNotification;

	[Attribute(ENotification.AREA_SEIZING_DONE_ENEMIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eCapturedByEnemiesNotification;

	[RplProp(onRplName: "OnSeizingTimestampChanged")]
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fSeizingStartTimestamp;
	#else
	protected WorldTimestamp m_fSeizingStartTimestamp;
	#endif

	[RplProp(onRplName: "OnSeizingTimestampChanged")]
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fSeizingEndTimestamp;
	#else
	protected WorldTimestamp m_fSeizingEndTimestamp;
	#endif

	static const float TRIGGER_CHECK_PERIOD_IDLE = 3;
	static const float TRIGGER_CHECK_PERIOD_ACTIVE = 1;

	protected ref ScriptInvoker m_OnCaptureStart;
	protected ref ScriptInvoker m_OnCaptureInterrupt;
	protected ref ScriptInvoker m_OnCaptureFinish;
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected ref ScriptInvoker m_OnTimerChange;
	#else
	protected ref OnTimerChangeInvoker m_OnTimerChange;
	#endif

	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fInterruptedCaptureTimestamp;
	#else
	protected WorldTimestamp m_fInterruptedCaptureTimestamp;
	#endif
	protected float m_fCurrentSeizingTime;
	protected float m_fInterruptedCaptureDuration;
	protected SCR_Faction m_PrevailingFaction;
	protected SCR_Faction m_PrevailingFactionPrevious;
	protected BaseGameTriggerEntity m_Trigger;
	protected bool m_bQueryFinished = true;
	protected bool m_bEnabled = true;
	protected RplComponent m_RplComponent;
	protected bool m_bCharacterPresent;
	protected SCR_FactionAffiliationComponent m_FactionControl;
	protected int m_iSeizingCharacters;
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected ref map<int, float> m_mSpawnTimers = new map<int, float>();
	#else
	protected ref map<int, WorldTimestamp> m_mSpawnTimers = new map<int, WorldTimestamp>();
	#endif

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	int GetRadius()
	{
		return m_iRadius;
	}

	//------------------------------------------------------------------------------------------------
	void AllowNotifications(bool allow)
	{
		m_bShowNotifications = allow;
	}

	//------------------------------------------------------------------------------------------------
	bool NotificationsAllowed()
	{
		return m_bShowNotifications;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCaptureStart()
	{
		if (!m_OnCaptureStart)
			m_OnCaptureStart = new ScriptInvoker();

		return m_OnCaptureStart;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCaptureInterrupt()
	{
		if (!m_OnCaptureInterrupt)
			m_OnCaptureInterrupt = new ScriptInvoker();

		return m_OnCaptureInterrupt;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCaptureFinish()
	{
		if (!m_OnCaptureFinish)
			m_OnCaptureFinish = new ScriptInvoker();

		return m_OnCaptureFinish;
	}

	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	ScriptInvoker GetOnTimerChange()
	#else
	OnTimerChangeInvoker GetOnTimerChange()
	#endif
	{
		if (!m_OnTimerChange)
			#ifndef AR_CAMPAIGN_TIMESTAMP
			m_OnTimerChange = new ScriptInvoker();
			#else
			m_OnTimerChange = new OnTimerChangeInvoker();
			#endif

		return m_OnTimerChange;
	}

	//------------------------------------------------------------------------------------------------
	protected void EvaluatePrevailingFaction()
	{
		float delay;

		// Switch to idle mode when nobody is in the area
		// Evaluation for all bases should not be done at once, randomize the timer a bit
		if (m_bCharacterPresent)
			delay = Math.RandomFloatInclusive(TRIGGER_CHECK_PERIOD_ACTIVE, TRIGGER_CHECK_PERIOD_ACTIVE + (TRIGGER_CHECK_PERIOD_ACTIVE * 0.2));
		else
			delay = Math.RandomFloatInclusive(TRIGGER_CHECK_PERIOD_IDLE, TRIGGER_CHECK_PERIOD_IDLE + (TRIGGER_CHECK_PERIOD_IDLE * 0.2));

		GetGame().GetCallqueue().CallLater(EvaluatePrevailingFaction, delay * 1000);

		if (!m_bQueryFinished)
			return;

		m_Trigger.GetOnQueryFinished().Insert(OnQueryFinished);
		m_Trigger.QueryEntitiesInside();
		m_bQueryFinished = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnQueryFinished(BaseGameTriggerEntity trigger)
	{
		m_bQueryFinished = true;

		array<IEntity> presentEntities = {};
		int presentEntitiesCnt = m_Trigger.GetEntitiesInside(presentEntities);
		m_bCharacterPresent = presentEntitiesCnt != 0;

		// Nobody is here, no need to evaluate
		if (!m_bCharacterPresent)
		{
			if (m_PrevailingFaction)
			{
				m_PrevailingFactionPrevious = m_PrevailingFaction;
				m_PrevailingFaction = null;
				OnPrevailingFactionChanged();
			}

			return;
		}

		map<SCR_Faction, int> factionsPresence = new map<SCR_Faction, int>();
		SCR_Faction evaluatedEntityFaction;

		int factionCnt;

		// Go through all entities and check their factions
		for (int i = 0; i < presentEntitiesCnt; i++)
		{
			evaluatedEntityFaction = EvaluateEntityFaction(presentEntities[i]);

			if (!evaluatedEntityFaction)
				continue;

			factionCnt = factionsPresence.Get(evaluatedEntityFaction);

			// If faction is not yet registered, do it now - otherwise just increase its presence counter
			if (factionCnt == 0)
				factionsPresence.Insert(evaluatedEntityFaction, 1);
			else
				factionsPresence.Set(evaluatedEntityFaction, factionCnt + 1);
		}

		SCR_Faction prevailingFaction;
		int highestAttackingPresence;
		int highestDefendingPresence;
		int curSeizingCharacters;
		int presence;

		// Evaluate the highest attacking presence
		for (int i = 0, cnt = factionsPresence.Count(); i < cnt; i++)
		{
			// Non-playable attackers are not allowed
			if (m_bIgnoreNonPlayableAttackers && !factionsPresence.GetKey(i).IsPlayable())
				continue;

			presence = factionsPresence.GetElement(i);

			if (presence > highestAttackingPresence)
			{
				highestAttackingPresence = presence;
				prevailingFaction = factionsPresence.GetKey(i);
			}
			else if (presence == highestAttackingPresence)	// When 2 or more factions have the same presence, none should prevail
			{
				prevailingFaction = null;
			}
		}

		// Evaluate the highest defending presence
		if (prevailingFaction)
		{
			for (int i = 0, cnt = factionsPresence.Count(); i < cnt; i++)
			{
				// Non-playable defenders are not allowed
				if (m_bIgnoreNonPlayableDefenders && !factionsPresence.GetKey(i).IsPlayable())
					continue;

				// This faction is already considered attacking
				if (factionsPresence.GetKey(i) == prevailingFaction)
					continue;

				highestDefendingPresence = Math.Max(factionsPresence.GetElement(i), highestDefendingPresence);
			}

			// Get net amount of players effectively seizing (clamp for max attackers attribute)
			if (prevailingFaction && highestAttackingPresence > highestDefendingPresence)
			{
				curSeizingCharacters = Math.Min(highestAttackingPresence - highestDefendingPresence, m_iMaximumSeizingCharacters);
			}
			else
			{
				prevailingFaction = null;
				curSeizingCharacters = 0;
			}
		}

		if (prevailingFaction != m_PrevailingFaction)
		{
			m_iSeizingCharacters = curSeizingCharacters;
			m_PrevailingFactionPrevious = m_PrevailingFaction;
			m_PrevailingFaction = prevailingFaction;
			OnPrevailingFactionChanged();
		}
		else if (prevailingFaction && curSeizingCharacters != m_iSeizingCharacters)
		{
			m_iSeizingCharacters = curSeizingCharacters;
			RefreshSeizingTimer();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ent);

		if (!char)
			return null;

		if (char.IsInVehicle() && SCR_TerrainHelper.GetHeightAboveTerrain(char.GetOrigin()) > m_iMaximumAltitude)
			return null;

		CharacterControllerComponent charControl = CharacterControllerComponent.Cast(char.FindComponent(CharacterControllerComponent));

		if (charControl && charControl.IsDead())
			return null;

		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());

		if (damageMan && damageMan.GetIsUnconscious())
			return null;

		// Handle after-respawn cooldown
		if (m_fRespawnCooldownPeriod > 0)
		{
			int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);

			if (playerId != 0 && m_mSpawnTimers.Contains(playerId))
			{
				#ifndef AR_CAMPAIGN_TIMESTAMP
				if (m_mSpawnTimers.Get(playerId) > Replication.Time())
				#else
				ChimeraWorld world = GetOwner().GetWorld();
				if (m_mSpawnTimers.Get(playerId).Greater(world.GetServerTimestamp()))
				#endif
					return null;
				else
					m_mSpawnTimers.Remove(playerId)
			}
		}

		SCR_Faction entityFaction = SCR_Faction.Cast(char.GetFaction());

		return entityFaction;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrevailingFactionChanged()
	{
		if (m_FactionControl.GetAffiliatedFaction() == m_PrevailingFaction || !m_PrevailingFaction)
		{
			if (m_fSeizingEndTimestamp != 0)
			{
				#ifndef AR_CAMPAIGN_TIMESTAMP
				m_fInterruptedCaptureTimestamp = Replication.Time();
				m_fInterruptedCaptureDuration = m_fInterruptedCaptureTimestamp - m_fSeizingStartTimestamp;
				m_fSeizingEndTimestamp = 0;
				m_fSeizingStartTimestamp = 0;
				#else
				ChimeraWorld world = GetOwner().GetWorld();
				m_fInterruptedCaptureTimestamp = world.GetServerTimestamp();
				m_fInterruptedCaptureDuration = m_fInterruptedCaptureTimestamp.DiffMilliseconds(m_fSeizingStartTimestamp);
				m_fSeizingEndTimestamp = null;
				m_fSeizingStartTimestamp = null;
				#endif
				int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFactionPrevious);
				Rpc(RpcDo_OnCaptureInterrupt, factionIndex);
				RpcDo_OnCaptureInterrupt(factionIndex);
			}
		}
		else
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			m_fSeizingStartTimestamp = Replication.Time();
			#else
			ChimeraWorld world = GetOwner().GetWorld();
			m_fSeizingStartTimestamp = world.GetServerTimestamp();
			#endif
			RefreshSeizingTimer();
			int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFaction);
			Rpc(RpcDo_OnCaptureStart, factionIndex);
			RpcDo_OnCaptureStart(factionIndex);
		}

		OnSeizingTimestampChanged();
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSeizingTimestampChanged()
	{
		if (m_OnTimerChange)
			m_OnTimerChange.Invoke(m_fSeizingStartTimestamp, m_fSeizingEndTimestamp);

		if (IsProxy())
			return;

		// Run EOnFrame only if timer is actually ticking
		if (m_fSeizingStartTimestamp == 0 && m_fSeizingEndTimestamp == 0)
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
		else
			SetEventMask(GetOwner(), EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshSeizingTimer()
	{
		float seizingTimeVar = m_fMaximumSeizingTime - m_fMinimumSeizingTime;
		float deduct;

		if (m_iMaximumSeizingCharacters > 1)	// Avoid division by 0
		{
			float deductPerPlayer = seizingTimeVar / (m_iMaximumSeizingCharacters - 1);
			deduct = deductPerPlayer * (m_iSeizingCharacters - 1);
		}

		#ifndef AR_CAMPAIGN_TIMESTAMP
		m_fSeizingEndTimestamp = m_fSeizingStartTimestamp + ((m_fMaximumSeizingTime - deduct) * 1000);
		#else
		m_fSeizingEndTimestamp = m_fSeizingStartTimestamp.PlusSeconds(m_fMaximumSeizingTime - deduct);
		#endif

		if (m_bGradualTimerReset && m_fInterruptedCaptureDuration != 0)
			HandleGradualReset();

		Replication.BumpMe();
		OnSeizingTimestampChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! When capture started after getting interrupted, take into account the time spent on it
	protected void HandleGradualReset()
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		float timeSinceInterrupt = m_fSeizingStartTimestamp - m_fInterruptedCaptureTimestamp;

		if (timeSinceInterrupt < m_fInterruptedCaptureDuration)
		{
			float diff = m_fInterruptedCaptureDuration - timeSinceInterrupt;
			m_fSeizingStartTimestamp = m_fSeizingStartTimestamp - diff;
			m_fSeizingEndTimestamp = m_fSeizingEndTimestamp - diff;
		}

		m_fInterruptedCaptureDuration = 0;
		m_fInterruptedCaptureTimestamp = 0;
		#else
		float timeSinceInterrupt = m_fSeizingStartTimestamp.DiffMilliseconds(m_fInterruptedCaptureTimestamp);

		if (timeSinceInterrupt < m_fInterruptedCaptureDuration)
		{
			float diff = m_fInterruptedCaptureDuration - timeSinceInterrupt;
			m_fSeizingStartTimestamp = m_fSeizingStartTimestamp.PlusMilliseconds(-diff);
			m_fSeizingEndTimestamp = m_fSeizingEndTimestamp.PlusMilliseconds(-diff);
		}

		m_fInterruptedCaptureDuration = 0;
		m_fInterruptedCaptureTimestamp = null;
		#endif
	}

	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetSeizingStartTimestamp()
	#else
	WorldTimestamp GetSeizingStartTimestamp()
	#endif
	{
		return m_fSeizingStartTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetSeizingEndTimestamp()
	#else
	WorldTimestamp GetSeizingEndTimestamp()
	#endif
	{
		return m_fSeizingEndTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_OnCaptureStart(int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		if (m_OnCaptureStart)
			m_OnCaptureStart.Invoke(faction, this);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_OnCaptureInterrupt(int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		if (m_OnCaptureInterrupt)
			m_OnCaptureInterrupt.Invoke(faction, this);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_OnCaptureFinish(int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		if (m_OnCaptureFinish)
			m_OnCaptureFinish.Invoke(faction, this);

		UpdateFlagsInHierarchy(faction);

		if (m_bShowNotifications)
			NotifyPlayerInRadius(faction);

		if (!IsProxy())
		{
			if (m_FactionControl.GetAffiliatedFaction() != faction)
				m_FactionControl.SetAffiliatedFaction(faction);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		if (!controlledEntity)
			return;

		// Player did not spawn in the area, ignore them
		if (vector.DistanceSqXZ(controlledEntity.GetOrigin(), GetOwner().GetOrigin()) > (m_iRadius * m_iRadius))
			return;

		#ifndef AR_CAMPAIGN_TIMESTAMP
		m_mSpawnTimers.Set(playerId, Replication.Time() + (m_fRespawnCooldownPeriod * 1000))
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		m_mSpawnTimers.Set(playerId, world.GetServerTimestamp().PlusSeconds(m_fRespawnCooldownPeriod));
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFlagsInHierarchy(notnull SCR_Faction faction)
	{
		array<IEntity> queue = {GetOwner()};
		SCR_FlagComponent flag;
		IEntity processedEntity;
		IEntity nextInHierarchy;

		while (!queue.IsEmpty())
		{
			processedEntity = queue[0];
			queue.Remove(0);

			flag = SCR_FlagComponent.Cast(processedEntity.FindComponent(SCR_FlagComponent));

			if (flag)
				flag.ChangeMaterial(faction.GetFactionFlagMaterial());

			nextInHierarchy = processedEntity.GetChildren();

			while (nextInHierarchy)
			{
				queue.Insert(nextInHierarchy);
				nextInHierarchy = nextInHierarchy.GetSibling();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetIsLocalPlayerPresent()
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());

		if (!player)
			return false;

		return (vector.DistanceSqXZ(player.GetOrigin(), GetOwner().GetOrigin()) <= (m_iRadius * m_iRadius));
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyPlayerInRadius(notnull SCR_Faction faction)
	{
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (!playerFaction)
			return;

		if (!GetIsLocalPlayerPresent())
			return;

		if (playerFaction == faction)
			SCR_NotificationsComponent.SendLocal(m_eCapturedByFriendliesNotification);
		else
			SCR_NotificationsComponent.SendLocal(m_eCapturedByEnemiesNotification);
	}

	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		if (m_FactionControl)
			return m_FactionControl.GetAffiliatedFaction();
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBaseFactionChanged(Faction faction)
	{
		super.OnBaseFactionChanged(faction);
		m_PrevailingFaction = null;
	}

	//------------------------------------------------------------------------------------------------
	void Disable()
	{
		m_bEnabled = false;

		if (m_Trigger)
			delete m_Trigger;

		ClearEventMask(GetOwner(), EntityEvent.FRAME);

		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			if (!base)
				continue;

			base.UnregisterLogicComponent(this);
		}

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (gameMode)
			gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawned);

		GetGame().GetCallqueue().Remove(EvaluatePrevailingFaction);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		// All functionality is server-side
		if (IsProxy())
			return;

		if (!m_bEnabled)
			return;

		// Attributes check
		if (m_iRadius <= 0)
		{
			Print("SCR_SeizingComponent: Invalid area radius (" + m_iRadius + ")! Terminating...", LogLevel.ERROR);
			return;
		}

		if (m_fMaximumSeizingTime < 0 || m_fMinimumSeizingTime < 0 || m_fMaximumSeizingTime < m_fMinimumSeizingTime)
		{
			Print("SCR_SeizingComponent: Invalid seizing time setting (" + m_fMinimumSeizingTime + ", " + m_fMaximumSeizingTime + ")! Terminating...", LogLevel.ERROR);
			return;
		}

		if (m_iMaximumSeizingCharacters <= 0)
		{
			Print("SCR_SeizingComponent: Invalid maximum seizing characters (" + m_iMaximumSeizingCharacters + ")! Terminating...", LogLevel.ERROR);
			return;
		}

		if (!GetGame().InPlayMode())
			return;

		SCR_SeizingComponentClass componentData = SCR_SeizingComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return;

		m_FactionControl = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));

		if (!m_FactionControl)
		{
			Print("SCR_SeizingComponent: Owner is missing SCR_FactionAffiliationComponent! Terminating...", LogLevel.ERROR);
			return;
		}

		Resource triggerResource = Resource.Load(componentData.GetTriggerPrefab());

		if (!triggerResource)
		{
			Print("SCR_SeizingComponent: Trigger resource failed to load! Terminating...", LogLevel.ERROR);
			return;
		}

		// Spawn the trigger locally on server
		m_Trigger = BaseGameTriggerEntity.Cast(GetGame().SpawnEntityPrefabLocal(triggerResource, GetGame().GetWorld()));

		if (!m_Trigger)
		{
			Print("SCR_SeizingComponent: Trigger failed to spawn! Terminating...", LogLevel.ERROR);
			return;
		}

		m_Trigger.SetSphereRadius(m_iRadius);
		owner.AddChild(m_Trigger, -1);

		// Register after-respawn cooldown method
		if (m_fRespawnCooldownPeriod > 0)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

			if (gameMode)
				gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		}

		GetGame().GetCallqueue().CallLater(EvaluatePrevailingFaction, Math.RandomFloatInclusive(TRIGGER_CHECK_PERIOD_IDLE, TRIGGER_CHECK_PERIOD_IDLE + (TRIGGER_CHECK_PERIOD_IDLE * 0.2)) * 1000);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (m_fSeizingEndTimestamp == 0 || Replication.Time() < m_fSeizingEndTimestamp)
			return;

		m_fSeizingEndTimestamp = 0;
		m_fSeizingStartTimestamp = 0;
		#else
		if (m_fSeizingEndTimestamp == 0)
			return;

		ChimeraWorld world = owner.GetWorld();
		if (world.GetServerTimestamp().Less(m_fSeizingEndTimestamp))
			return;

		m_fSeizingEndTimestamp = null;
		m_fSeizingStartTimestamp = null;
		#endif
		OnSeizingTimestampChanged();

		Replication.BumpMe();

		if (m_FactionControl.GetAffiliatedFaction() != m_PrevailingFaction)
		{
			int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFaction);
			Rpc(RpcDo_OnCaptureFinish, factionIndex);
			RpcDo_OnCaptureFinish(factionIndex);
		}
		else
		{
			int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFactionPrevious);
			Rpc(RpcDo_OnCaptureInterrupt, factionIndex);
			RpcDo_OnCaptureInterrupt(factionIndex);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SeizingComponent()
	{
		if (m_Trigger)
			delete m_Trigger;

		m_mSpawnTimers.Clear();
		m_mSpawnTimers = null;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (gameMode)
			gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawned);

		GetGame().GetCallqueue().Remove(EvaluatePrevailingFaction);
	}
}
