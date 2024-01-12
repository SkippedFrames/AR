#include "scripts/Game/config.c"
[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Handles respawn timers for players.")]
class SCR_RespawnTimerComponentClass: SCR_BaseGameModeComponentClass
{
};

#define RESPAWN_TIMER_COMPONENT_DEBUG

//------------------------------------------------------------------------------------------------
//! Must be attached to a GameMode
class SCR_RespawnTimerComponent : SCR_BaseGameModeComponent
{
	[Attribute("10", UIWidgets.EditBox, "Default time in seconds that a player has to wait after dead to respawn.", category: "Respawn Timers")]
	protected float m_fRespawnTime;

	//------------------------------------------------------------------------------------------------
	/*!
		Map of respawn timers of individual players.
			key: PlayerId of target player
			val: RespawnTimer object
	*/
	protected ref map<int, ref SCR_RespawnTimer> m_mRespawnTimers = new map<int, ref SCR_RespawnTimer>();

	//! RplComponent attached to this component's owner entity
	protected RplComponent m_RplComponent;

	#ifdef RESPAWN_TIMER_COMPONENT_DEBUG
	protected static bool s_DebugRegistered = false;
	#endif

	//------------------------------------------------------------------------------------------------
	/*!
		Serialize state of this component on the server.
	*/
	override bool RplSave(ScriptBitWriter writer)
    {
		writer.WriteFloat(m_fRespawnTime);
        writer.WriteInt(m_mRespawnTimers.Count());
		foreach (int playerId, SCR_RespawnTimer timer : m_mRespawnTimers)
		{
			writer.WriteInt(playerId);
			timer.RplSave(writer);
		}

        return true;
    }

	//------------------------------------------------------------------------------------------------
	/*!
		Deserialize state of this component on the client.
	*/
    override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadFloat(m_fRespawnTime);

		int count;
		reader.ReadInt(count);

		for (int i = 0; i < count; i++)
		{
			int playerId;
			reader.ReadInt(playerId);

			// Create new instance
			if (!m_mRespawnTimers.Contains(playerId))
				m_mRespawnTimers.Insert(playerId, new SCR_RespawnTimer());

			// Load instance data
			m_mRespawnTimers[playerId].RplLoad(reader);
		}

        return true;
    }

	//------------------------------------------------------------------------------------------------
	/*!
		Are we the master of this component's RplComponent node?
	*/
	protected bool IsMaster()
	{
		return (!m_RplComponent || m_RplComponent.IsMaster());
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns time in seconds after world start. Synchronized to clients.
	*/
	#ifndef AR_RESPAWN_TIMER_TIMESTAMP
	protected float GetCurrentTime()
	#else
	protected WorldTimestamp GetCurrentTime()
	#endif
	{
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		float time = Replication.Time();
		return time * 0.001; // ms to s
		#else
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return null;
		
		return world.GetServerTimestamp();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void SetRespawnTime(int playerID, float respawnTime)
	{
		// Server only
		if (!IsMaster())
			return;

		// Set duration and replicate to all clients
		RpcDo_SetDuration_BC(playerID, respawnTime);
		Rpc(RpcDo_SetDuration_BC, playerID, respawnTime);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerEnqueued(int playerID)
	{
		return m_mRespawnTimers.Contains(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Only relevant to server and local player.
	*/
	bool GetCanPlayerSpawn(int playerID)
	{
		if (!IsPlayerEnqueued(playerID))
		{
			Print("Provided playerId: (" + playerID + ") in SCR_RespawnTimerComponent was invalid!", LogLevel.ERROR);
			return false;
		}

		return m_mRespawnTimers[playerID].IsFinished(GetCurrentTime());
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerRemainingTime(int playerID)
	{
		if (!m_mRespawnTimers.Contains(playerID))
		{
			Print("Provided playerId: (" + playerID + ") in SCR_RespawnTimerComponent was invalid!", LogLevel.ERROR);
			return false;
		}

		float remainingTime = m_mRespawnTimers[playerID].GetRemainingTime(GetCurrentTime());
		return Math.Ceil(remainingTime);
	}

	/*!
		Start respawn timer for provided entity if a player controlled it.
	*/
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);

		// Invalid playerID
		if (playerId <= 0)
			return;

		// TODO@AS: Propagate change to owner
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		float rplTime = GetCurrentTime();
		#else
		WorldTimestamp rplTime = GetCurrentTime();
		#endif

		// Notify all clients, fire locally
		RpcDo_StartTimer_BC(playerId, rplTime);
		Rpc(RpcDo_StartTimer_BC, playerId, rplTime);
	}
	override void OnPlayerDeleted(int playerId, IEntity player)
	{
		OnPlayerKilled(playerId, player, null, Instigator.CreateInstigator(null));
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Broadcasts that starts timer for provided player.
		\param playerId PlayerId of target player.
		\param rplTime Synchronized network time.
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	#ifndef AR_RESPAWN_TIMER_TIMESTAMP
	private void RpcDo_StartTimer_BC(int playerId, float rplTime)
	#else
	private void RpcDo_StartTimer_BC(int playerId, WorldTimestamp rplTime)
	#endif
	{
		m_mRespawnTimers[playerId].Start(rplTime);
	}

	//------------------------------------------------------------------------------------------------
	/*
		Broadcasts that sets timer duration for provided player.
		\param playerId PlayerId of target player.
		\param duration Duration of respawn timer.
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_SetDuration_BC(int playerId, float duration)
	{
		m_mRespawnTimers[playerId].SetDuration(duration);
	}

	//------------------------------------------------------------------------------------------------
	/*
		Broadcasts that sets timer duration for ALL players.
		\param duration Duration of respawn timer.
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_SetDurationAll_BC(float duration)
	{
		foreach (SCR_RespawnTimer timer : m_mRespawnTimers)
			timer.SetDuration(duration);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Register provided client's respawn timer.
	*/
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);

		// Prepare timer
		ref SCR_RespawnTimer respawnTimer = new SCR_RespawnTimer();

		// For first spawn to be already finished even when time == 0
		respawnTimer.SetDuration(m_fRespawnTime);
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		respawnTimer.Start(-m_fRespawnTime);
		#else
		WorldTimestamp startTime;
		startTime = startTime.PlusSeconds(-m_fRespawnTime);
		respawnTimer.Start(startTime);
		#endif

		if (!m_mRespawnTimers.Contains(playerId))
			m_mRespawnTimers.Insert(playerId, respawnTimer);
	}

	//------------------------------------------------------------------------------------------------
	//TODO: Editor team would like to set respawn time for Factions specific
	void SetRespawnTime(float respawnTime)
	{
		// Set default respawn time value
		m_fRespawnTime = respawnTime;

		// Propagate changes to existing timers for both self and all clients
		RpcDo_SetDurationAll_BC(respawnTime);
		Rpc(RpcDo_SetDurationAll_BC, respawnTime);
	}

	//------------------------------------------------------------------------------------------------
	float GetRespawnTime()
	{
		return m_fRespawnTime;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		// Do nothing for now,
		// TODO@AS: If needed, remove unused respawn timers?
	}
	
	#ifdef RESPAWN_TIMER_COMPONENT_DEBUG
	protected void DrawDebugInfo()
	{
		ArmaReforgerScripted game = GetGame();
			// Prepare strings to display
			const string header = "[PlayerID] [PlayerName] [Timer] [IsFinished] [TimerObject]";
			const string tableFmt = "%1 | %2 | %3 | %4 | %5";

			// Find local pyl
			int localId = -1;
			PlayerController playerController = game.GetPlayerController();
			if (playerController)
			localId = playerController.GetPlayerId();

			DbgUI.Begin("Respawn Timer Component Diag");
			DbgUI.Text("RplTime: " + GetCurrentTime() );
			DbgUI.Spacer(10);
			DbgUI.Text( header );

			#ifndef AR_RESPAWN_TIMER_TIMESTAMP
			float timeNow = GetCurrentTime();
			#else
			WorldTimestamp timeNow = GetCurrentTime();
			#endif
			PlayerManager playerManager = GetGame().GetPlayerManager();
			foreach (int playerId, SCR_RespawnTimer timer : m_mRespawnTimers)
			{
				string isFinished = "False";
				if (timer.IsFinished(timeNow))
					isFinished = "True";

				string playerIdentifier = playerId.ToString();
				string playerName = playerManager.GetPlayerName(playerId);
				string time = timer.GetRemainingTime(timeNow).ToString();
				string obj = timer.ToString();


				// if (playerId == localId)
				string text = string.Format(tableFmt, playerIdentifier, playerName, time, isFinished, obj);
				if (playerId == localId)
					text = " > " + text;

				DbgUI.Text(text);
				DbgUI.Spacer(4);
			}

			DbgUI.End();
	}
	#endif

	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);

		#ifdef RESPAWN_TIMER_COMPONENT_DEBUG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RESPAWN_TIMER_COMPONENT))
		{
			DrawDebugInfo();
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		ConnectToDiagSystem(owner);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromDiagSystem(owner);
		
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_RespawnTimerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef RESPAWN_TIMER_COMPONENT_DEBUG
		if (!s_DebugRegistered)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RESPAWN_TIMER_COMPONENT, "", "Respawn Timer", "GameMode");
			s_DebugRegistered = true;
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RespawnTimerComponent()
	{
		#ifdef RESPAWN_TIMER_COMPONENT_DEBUG
		if (s_DebugRegistered)
		{
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RESPAWN_TIMER_COMPONENT );
			s_DebugRegistered = false;
		}
		#endif
	}
};
