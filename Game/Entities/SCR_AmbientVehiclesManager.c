#include "scripts/Game/config.c"
void OnAmbientVehicleSpawnedDelegate(SCR_AmbientVehicleSpawnPointComponent spawnpoint, Vehicle vehicle);

typedef func OnAmbientVehicleSpawnedDelegate;

typedef ScriptInvokerBase<OnAmbientVehicleSpawnedDelegate> OnAmbientVehicleSpawnedInvoker;

//------------------------------------------------------------------------------------------------
[EntityEditorProps(insertable: false)]
class SCR_AmbientVehiclesManagerClass : GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_AmbientVehiclesManager : GenericEntity
{
	protected static const int CHECK_INTERVAL = 3;					//s, how often should an individual spawnpoint be checked
	protected static const int DESPAWN_TIMEOUT = 10000;				//ms
	protected static const int PARKED_THRESHOLD = 5;				//m, how far away can the spawned vehicle moved before being considered used

	protected static const int SPAWN_RADIUS_MIN_SQ = 500 * 500;		// Square value for distance checks
	protected static const int SPAWN_RADIUS_MAX_SQ = 1000 * 1000;	// Square value for distance checks
	protected static const int DESPAWN_RADIUS_DIFF_SQ = 200 * 200;	// Square value for distance checks
	
	protected static SCR_AmbientVehiclesManager s_Instance = null;
	
	protected ref array<SCR_AmbientVehicleSpawnPointComponent> m_aSpawnpoints = {};
	protected ref array<IEntity> m_aPlayers = {};

	protected ref OnAmbientVehicleSpawnedInvoker m_OnVehicleSpawned;

	protected int m_iLastAssignedIndex;
	protected int m_iIndexToCheck;
	protected int m_iSpawnDistanceSq;
	protected int m_iDespawnDistanceSq;

	protected float m_fTimer;
	protected float m_fCheckInterval;
	
	//------------------------------------------------------------------------------------------------
	static SCR_AmbientVehiclesManager GetInstance(bool createNew = true)
	{
		if (!s_Instance && createNew)
			s_Instance = SCR_AmbientVehiclesManager.Cast(GetGame().SpawnEntity(SCR_AmbientVehiclesManager, GetGame().GetWorld()));

		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	OnAmbientVehicleSpawnedInvoker GetOnVehicleSpawned()
	{
		if (!m_OnVehicleSpawned)
			m_OnVehicleSpawned = new OnAmbientVehicleSpawnedInvoker();

		return m_OnVehicleSpawned;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCheckInterval()
	{
		m_fCheckInterval = CHECK_INTERVAL / m_aSpawnpoints.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshPlayerList()
	{
		m_aPlayers.Clear();
		array<int> playerIds = {};
		PlayerManager pc = GetGame().GetPlayerManager();
		int playersCount = pc.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			IEntity player = pc.GetPlayerControlledEntity(playerId);

			if (!player)
				continue;

			CharacterControllerComponent comp = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));

			if (!comp || comp.IsDead())
				continue;

			m_aPlayers.Insert(player);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawnedOrDeleted(int playerId, IEntity player)
	{
		RefreshPlayerList();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessSpawnpoint(int spawnpointIndex)
	{
		SCR_AmbientVehicleSpawnPointComponent spawnpoint = m_aSpawnpoints[spawnpointIndex];

		if (!spawnpoint || spawnpoint.GetIsDepleted())
			return;

		Vehicle spawnedVeh = spawnpoint.GetSpawnedVehicle();
		#ifndef AR_CAMPAIGN_TIMESTAMP
		float currentTime = Replication.Time();
		#else
		ChimeraWorld world = GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		#endif

		if (!spawnedVeh)
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			int respawnTimestamp = spawnpoint.GetRespawnTimestamp();
			#else
			WorldTimestamp respawnTimestamp = spawnpoint.GetRespawnTimestamp();
			#endif
			
			// Respawn timer is ticking
			#ifndef AR_CAMPAIGN_TIMESTAMP
			if (respawnTimestamp > currentTime)
			#else
			if (respawnTimestamp.Greater(currentTime))
			#endif
				return;
			
			if (spawnpoint.GetIsFirstSpawnDone())
			{
				int respawnPeriod = spawnpoint.GetRespawnPeriod();
				
				// Vehicle has been deleted, setup respawn timer if enabled
				if (respawnTimestamp == 0 && respawnPeriod > 0)
				{
					#ifndef AR_CAMPAIGN_TIMESTAMP
					spawnpoint.SetRespawnTimestamp(currentTime + (respawnPeriod * 1000));
					#else
					spawnpoint.SetRespawnTimestamp(currentTime.PlusSeconds(respawnPeriod));
					#endif
					return;
				}
			}
		}

		vector location = spawnpoint.GetOwner().GetOrigin();

		// Only handle vehicles which are still on their spawnpoints
		if (spawnedVeh && vector.DistanceXZ(spawnedVeh.GetOrigin(), location) > PARKED_THRESHOLD)
		{
			// Non-respawning spawnpoints get depleted once the vehicle leaves the spawnpoint
			if (spawnpoint.GetRespawnPeriod() <= 0)
				spawnpoint.SetIsDepleted(true);
			
			return;
		}

		bool playersNear = false;
		bool playersFar = true;
		int distance;

		// Define if any player is close enough to spawn or if all players are far enough to despawn
		foreach (IEntity player : m_aPlayers)
		{
			if (!player)
				continue;

			distance = vector.DistanceSq(player.GetOrigin(), location);

			if (distance < m_iDespawnDistanceSq)
			{
				playersFar = false;

				if (distance < m_iSpawnDistanceSq)
				{
					playersNear = true;
					break;
				}
			}
		}

		if (!spawnedVeh && playersNear)
		{
			Vehicle vehicle = spawnpoint.SpawnVehicle();

			if (vehicle && m_OnVehicleSpawned)
				m_OnVehicleSpawned.Invoke(spawnpoint, vehicle);

			return;
		}

		// Delay is used so dying players don't see the despawn happen
		if (spawnedVeh && playersFar)
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			float despawnT = spawnpoint.GetDespawnTimer();

			if (despawnT == -1)
				spawnpoint.SetDespawnTimer(currentTime + DESPAWN_TIMEOUT);
			else if (currentTime > despawnT)
			#else
			WorldTimestamp despawnT = spawnpoint.GetDespawnTimer();

			if (despawnT == 0)
				spawnpoint.SetDespawnTimer(currentTime.PlusMilliseconds(DESPAWN_TIMEOUT));
			else if (currentTime.Greater(despawnT))
			#endif
				spawnpoint.DespawnVehicle();
		}
		else
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			spawnpoint.SetDespawnTimer(-1);
			#else
			spawnpoint.SetDespawnTimer(null);
			#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	void RegisterSpawnpoint(notnull SCR_AmbientVehicleSpawnPointComponent spawnpoint)
	{
		if (m_aSpawnpoints.IsEmpty())
			SetEventMask(EntityEvent.FRAME);
	
		m_aSpawnpoints.Insert(spawnpoint);
		spawnpoint.SetID(m_iLastAssignedIndex);
		m_iLastAssignedIndex++;
		UpdateCheckInterval();
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterSpawnpoint(notnull SCR_AmbientVehicleSpawnPointComponent spawnpoint)
	{
		m_aSpawnpoints.RemoveItem(spawnpoint);
		m_iIndexToCheck = 0;

		if (!m_aSpawnpoints.IsEmpty())
		{
			UpdateCheckInterval();
			return;
		}

		ClearEventMask(EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	int GetSpawnpoints(out array<SCR_AmbientVehicleSpawnPointComponent> spawnpoints)
	{
		if (spawnpoints)
			return spawnpoints.Copy(m_aSpawnpoints);
		else
			return m_aSpawnpoints.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerId, KickCauseCode cause = KickCauseCode.NONE, int timeout = -1)
	{
		RefreshPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		RefreshPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		if (!GetGame().AreGameFlagsSet(EGameFlags.SpawnVehicles))
		{
			ClearEventMask(EntityEvent.FRAME);
			return;
		}

		m_fTimer += timeSlice;

		if (m_fTimer < m_fCheckInterval)
			return;

		m_fTimer = 0;

		ProcessSpawnpoint(m_iIndexToCheck);
		m_iIndexToCheck++;

		if (!m_aSpawnpoints.IsIndexValid(m_iIndexToCheck))
			m_iIndexToCheck = 0;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		RefreshPlayerList();

		// Calculate (de)spawn distance based on view distance, have it squared for faster distance calculation
		int fractionVD = GetGame().GetViewDistance() * 0.3;
		m_iSpawnDistanceSq = fractionVD * fractionVD;
		m_iSpawnDistanceSq = Math.Min(SPAWN_RADIUS_MAX_SQ, m_iSpawnDistanceSq);
		m_iSpawnDistanceSq = Math.Max(SPAWN_RADIUS_MIN_SQ, m_iSpawnDistanceSq);
		m_iDespawnDistanceSq = m_iSpawnDistanceSq + DESPAWN_RADIUS_DIFF_SQ;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (!gameMode)
			return;

		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Insert(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AmbientVehiclesManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AmbientVehiclesManager()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (!gameMode)
			return;

		gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerKilled().Remove(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Remove(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnected);
	}
};
