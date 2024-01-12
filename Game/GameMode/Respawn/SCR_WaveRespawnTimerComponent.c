#include "scripts/Game/config.c"
[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Handles respawn timers for players.")]
class SCR_WaveRespawnTimerComponentClass: SCR_RespawnTimerComponentClass
{
};

#define RESPAWN_TIMER_COMPONENT_DEBUG

//------------------------------------------------------------------------------------------------
//! Must be attached to a GameMode
// TODO@AS: Revisit after changes in timer comp
class SCR_WaveRespawnTimerComponent : SCR_RespawnTimerComponent
{	
	//! Respawn timers per faction
	[RplProp()]
	protected ref array<ref SCR_RespawnTimer> m_aFactionRespawnTimers = new ref array<ref SCR_RespawnTimer>();

	[RplProp()]
	protected ref array<int> m_aAllowedPlayers = new array<int>();

	[RplProp()]
	protected ref array<int> m_aWaitingPlayers = new array<int>();
	
	#ifdef RESPAWN_TIMER_COMPONENT_DEBUG
	override void DrawDebugInfo()
	{
		super.DrawDebugInfo();

		DbgUI.Begin("Wave Respawn Diag");
		{
			// Players ready to respawn
			DbgUI.Text("Allowed Players:");
			PlayerManager playerManager = GetGame().GetPlayerManager();
			foreach (int playerId : m_aAllowedPlayers)
			{
				string playerText = string.Format("%1: %2", playerId, playerManager.GetPlayerName(playerId));
				DbgUI.Text(playerText);
			}

			// Players waiting in wave
			DbgUI.Text("Waiting Players:");
			foreach (int playerId : m_aWaitingPlayers)
			{
				string playerText = string.Format("%1: %2", playerId, playerManager.GetPlayerName(playerId));
				DbgUI.Text(playerText);
			}

			#ifndef AR_RESPAWN_TIMER_TIMESTAMP
			float timeNow = GetCurrentTime();
			#else
			WorldTimestamp timeNow = GetCurrentTime();
			#endif
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			
			if (factionManager)
			{
				// Respawn timers
				DbgUI.Text("Faction Timers:");
				int index = 0;
				foreach (SCR_RespawnTimer respawnTimer : m_aFactionRespawnTimers)
				{
					Faction affiliatedFaction = factionManager.GetFactionByIndex(index);
					if (affiliatedFaction)
					{
						string factionText = string.Format("%1: %2s (%3)", index, respawnTimer.GetRemainingTime(timeNow), affiliatedFaction.GetFactionName());
						DbgUI.Text(factionText);
					}

					index++;
				}
			}

		}
		DbgUI.End();
	}
	#endif

	//------------------------------------------------------------------------------------------------
	override bool GetCanPlayerSpawn(int playerID)
	{
		return m_aAllowedPlayers.Contains(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override int GetPlayerRemainingTime(int playerID)
	{
		// Ready to respawn
		if (m_aAllowedPlayers.Contains(playerID))
			return 0;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return 0;

		// Waiting in next wave
		Faction faction = factionManager.GetPlayerFaction(playerID);
		int factionIndex = factionManager.GetFactionIndex(faction);
		if (factionIndex < 0)
			return 0;

		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		float timeNow = GetCurrentTime();
		#else
		WorldTimestamp timeNow = GetCurrentTime();
		#endif
		return m_aFactionRespawnTimers[factionIndex].GetRemainingTime(timeNow);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
		
		int allowedIndex = m_aAllowedPlayers.Find(playerId);
		if (allowedIndex != -1)
		{
			m_aAllowedPlayers.Remove( allowedIndex );
		}

		int waitingIndex = m_aWaitingPlayers.Find(playerId);
		if (waitingIndex != -1)
		{
			m_aWaitingPlayers.Remove( waitingIndex );
		}

		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player gets killed.
		\param playerId PlayerId of victim player.
		\param player Entity of victim player if any.
		\param killerEntity Entity of killer instigator if any.
		\param killer Instigator of the kill
	*/
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
		if (!m_aWaitingPlayers.Contains(playerId))
		{
			m_aWaitingPlayers.Insert(playerId);
			Replication.BumpMe();
		}
	}
	override void OnPlayerDeleted(int playerId, IEntity player)
	{
		OnPlayerKilled(playerId, player, null, Instigator.CreateInstigator(null));
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		if (!m_aWaitingPlayers.Contains(playerId))
		{
			m_aWaitingPlayers.Insert(playerId);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*
		Broadcasts that sets timer duration for provided faction.
		\param duration Duration of respawn timer.
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	#ifndef AR_RESPAWN_TIMER_TIMESTAMP
	void RpcDo_StartFactionTimer(int factionIndex, float rplTime)
	#else
	void RpcDo_StartFactionTimer(int factionIndex, WorldTimestamp rplTime)
	#endif
	{
		SCR_RespawnTimer timer = m_aFactionRespawnTimers[factionIndex];
		timer.Start(rplTime);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!IsMaster())
			return;
				
		bool shouldBump;
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		float timeNow = GetCurrentTime();
		#else
		WorldTimestamp timeNow = GetCurrentTime();
		#endif
		// Update timers
		int timersCount = m_aFactionRespawnTimers.Count();
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		for (int i = 0; i < timersCount; i++)
		{
			SCR_RespawnTimer timer = m_aFactionRespawnTimers[i];
			if (timer)
			{
				// If timer is finished, allow spawn for current wave of players
				if (timer.IsFinished(timeNow))
				{
					// Add each waiting player of given faction into the "allow players" list
					for (int playerIndex = m_aWaitingPlayers.Count() -1; playerIndex >= 0; playerIndex--)
					{
						int playerId = m_aWaitingPlayers[playerIndex];
						Faction playerFaction = factionManager.GetPlayerFaction(playerId);
						if (playerFaction)
						{
							int factionIndex = factionManager.GetFactionIndex(playerFaction);
							if (factionIndex == i)
							{
								// It would be nicer to use set<T>, but rpl codec is missing for that object
								if (!m_aAllowedPlayers.Contains(playerId))
									m_aAllowedPlayers.Insert(playerId);

								// Remove from waiting list, we are in ready list already
								m_aWaitingPlayers.Remove(playerIndex);
								
								// Make sure to propagate changes
								shouldBump = true;
							}
						}
					}
					
					// Start timer
					RpcDo_StartFactionTimer(i, timeNow);
					Rpc(RpcDo_StartFactionTimer, i, timeNow);
				}
			}
		}
		
		if (shouldBump)
			Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Initialize this component.
	*/
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		
		// Fetch neccessary components
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	/*
		Initialize this component with data from FactionsManager.
	*/
	override void EOnInit(IEntity owner)
	{
		
		// Fill list of respawns with default data
		int factionsCount;
		array<Faction> factions = {};
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionsCount = factionManager.GetFactionsList(factions);

		// Prefill list with data
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		float timeNow = GetCurrentTime();
		#else
		WorldTimestamp timeNow = GetCurrentTime();
		#endif
		for (int i = 0; i < factionsCount; i++)
		{
			ref SCR_RespawnTimer timer = new SCR_RespawnTimer();
			timer.SetDuration(m_fRespawnTime);
			#ifndef AR_RESPAWN_TIMER_TIMESTAMP
			timer.Start(timeNow - m_fRespawnTime);
			#else
			timer.Start(timeNow.PlusSeconds(-m_fRespawnTime));
			#endif
			m_aFactionRespawnTimers.Insert(timer);
		}

		Replication.BumpMe();
	}
};
