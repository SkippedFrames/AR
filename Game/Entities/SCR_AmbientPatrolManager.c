#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
[EntityEditorProps(insertable: false)]
class SCR_AmbientPatrolManagerClass : GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_AmbientPatrolManager : GenericEntity
{
	protected static const int CHECK_INTERVAL = 3;				//s, how often should an individual patrol spawn be checked
	protected static const int DESPAWN_TIMEOUT = 10000;			//ms

	protected static const int SPAWN_RADIUS_MIN = 500 * 500;	// Square value for distance checks
	protected static const int SPAWN_RADIUS_MAX = 1000 * 1000;	// Square value for distance checks
	protected static const int DESPAWN_RADIUS_DIFF = 200 * 200;	// Square value for distance checks

	protected static SCR_AmbientPatrolManager s_Instance = null;

	protected ref array<SCR_AmbientPatrolSpawnPointComponent> m_aPatrols = {};
	protected ref array<IEntity> m_aPlayers = {};

	protected int m_iLastAssignedIndex;
	protected int m_iIndexToCheck;
	protected int m_iSpawnDistanceSq;
	protected int m_iDespawnDistanceSq;

	protected float m_fTimer;
	protected float m_fCheckInterval;

	//------------------------------------------------------------------------------------------------
	static SCR_AmbientPatrolManager GetInstance(bool createNew = true)
	{
		if (!s_Instance && createNew)
			s_Instance = SCR_AmbientPatrolManager.Cast(GetGame().SpawnEntity(SCR_AmbientPatrolManager, GetGame().GetWorld()));

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCheckInterval()
	{
		m_fCheckInterval = CHECK_INTERVAL / m_aPatrols.Count();
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
	//! Composes an array of surviving ambient patrols which can be serialized
	int GetRemainingPatrolsInfo(out notnull array<int> remnantsInfo)
	{
		int size;
		SCR_AmbientPatrolManager manager = SCR_AmbientPatrolManager.GetInstance();
		array<SCR_AmbientPatrolSpawnPointComponent> patrols = {};
		manager.GetPatrols(patrols);
		#ifndef AR_CAMPAIGN_TIMESTAMP
		float curTime = Replication.Time();
		#else
		ChimeraWorld world = GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		#endif

		foreach (SCR_AmbientPatrolSpawnPointComponent presence : patrols)
		{
			if (!presence)
				continue;

			SCR_AIGroup grp = presence.GetSpawnedGroup();

			if (presence.GetMembersAlive() < 0 && !grp && !presence.GetIsSpawned())
				continue;

			remnantsInfo.Insert(presence.GetID());

			if (grp)
				size = remnantsInfo.Insert(grp.GetAgentsCount());
			else
			{
				if (presence.GetIsSpawned())
					size = remnantsInfo.Insert(0);
				else
					size = remnantsInfo.Insert(presence.GetMembersAlive());
			}

			// Subtract current time so when data is loaded at scenario start, the delay is correct
			#ifndef AR_CAMPAIGN_TIMESTAMP
			remnantsInfo.Insert(presence.GetRespawnTimestamp() - curTime);
			#else
			remnantsInfo.Insert(presence.GetRespawnTimestamp().DiffMilliseconds(curTime));
			#endif
		}

		return size;
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessSpawnpoint(int spawnpointIndex)
	{
		SCR_AmbientPatrolSpawnPointComponent spawnpoint = m_aPatrols[spawnpointIndex];

		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (!spawnpoint || spawnpoint.GetRespawnTimestamp() > Replication.Time() || (spawnpoint.GetMembersAlive() == 0 && !spawnpoint.GetIsSpawned()))
			return;
		#else
		if (!spawnpoint || (spawnpoint.GetMembersAlive() == 0 && !spawnpoint.GetIsSpawned()))
			return;

		ChimeraWorld world = GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		if (spawnpoint.GetRespawnTimestamp().Greater(currentTime))
			return;
		#endif

#ifdef CONFLICT_SPAWN_ALL_AI
		if (!spawnpoint.GetIsSpawned())
			spawnpoint.SpawnPatrol();

		return;
#endif

		bool playersNear;
		bool playersVeryNear;
		bool playersFar = true;
		vector location = spawnpoint.GetOwner().GetOrigin();
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

					if (distance < SPAWN_RADIUS_MIN)
						playersVeryNear = true;

					break;
				}
			}
		}

		bool isAIOverLimit;
		AIWorld aiWorld = GetGame().GetAIWorld();
		
		if (aiWorld)
		{
			int maxChars = aiWorld.GetAILimit();
			
			if (maxChars <= 0)
				isAIOverLimit = true;
			else
				isAIOverLimit = ((float)aiWorld.GetCurrentAmountOfLimitedAIs() / (float)maxChars) > spawnpoint.GetAILimitThreshold();
		}

		if (!isAIOverLimit && !playersVeryNear)
			spawnpoint.SetIsPaused(false);

		if (playersNear && !spawnpoint.GetIsSpawned() && !spawnpoint.GetIsPaused())
		{
			// Do not spawn the patrol if the AI threshold setting has been reached
			if (isAIOverLimit)
			{
				spawnpoint.SetIsPaused(true);	// Make sure a patrol is not spawned too close to players when AI limit suddenly allows spawning of this group
				return;
			}

			spawnpoint.SpawnPatrol();
			return;
		}

		// Delay is used so dying players don't see the despawn happen
		if (spawnpoint.GetIsSpawned() && playersFar)
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			float despawnT = spawnpoint.GetDespawnTimer();

			if (despawnT == -1)
				spawnpoint.SetDespawnTimer(Replication.Time() + DESPAWN_TIMEOUT);
			else if (Replication.Time() > despawnT)
			#else
			WorldTimestamp despawnT = spawnpoint.GetDespawnTimer();

			if (despawnT == 0)
				spawnpoint.SetDespawnTimer(currentTime.PlusMilliseconds(DESPAWN_TIMEOUT));
			else if (currentTime.Greater(despawnT))
			#endif
				spawnpoint.DespawnPatrol();
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
	void RegisterPatrol(notnull SCR_AmbientPatrolSpawnPointComponent patrol)
	{
		if (m_aPatrols.IsEmpty())
			SetEventMask(EntityEvent.FRAME);

		m_aPatrols.Insert(patrol);
		patrol.SetID(m_iLastAssignedIndex);
		m_iLastAssignedIndex++;
		UpdateCheckInterval();
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterPatrol(notnull SCR_AmbientPatrolSpawnPointComponent patrol)
	{
		m_aPatrols.RemoveItem(patrol);
		m_iIndexToCheck = 0;

		if (!m_aPatrols.IsEmpty())
		{
			UpdateCheckInterval();
			return;
		}

		ClearEventMask(EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	int GetPatrols(out array<SCR_AmbientPatrolSpawnPointComponent> patrols)
	{
		if (patrols)
			return patrols.Copy(m_aPatrols);
		else
			return m_aPatrols.Count();
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

		if (!GetGame().AreGameFlagsSet(EGameFlags.SpawnAI))
		{
			ClearEventMask(EntityEvent.FRAME);
			return;
		}

		m_fTimer += timeSlice;

		if (m_fTimer < m_fCheckInterval)
			return;

		m_fTimer = 0;
		
		// HOTFIX: Don't process spawning at the very start - wait for a save to be applied if it exists
		// Otherwise full-size groups get spawned even if they are marked as eliminated in the save file
		// TODO: Come up with a better solution
		if (GetGame().GetWorld().GetWorldTime() < SCR_GameModeCampaign.BACKEND_DELAY)
			return;

		ProcessSpawnpoint(m_iIndexToCheck);
		m_iIndexToCheck++;

		if (!m_aPatrols.IsIndexValid(m_iIndexToCheck))
			m_iIndexToCheck = 0;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		RefreshPlayerList();

		// Calculate (de)spawn distance based on view distance, have it squared for faster distance calculation
		int fractionOfVD = GetGame().GetViewDistance() * 0.3;
		m_iSpawnDistanceSq = fractionOfVD * fractionOfVD;
		m_iSpawnDistanceSq = Math.Min(SPAWN_RADIUS_MAX, m_iSpawnDistanceSq);
		m_iSpawnDistanceSq = Math.Max(SPAWN_RADIUS_MIN, m_iSpawnDistanceSq);
		m_iDespawnDistanceSq = m_iSpawnDistanceSq + DESPAWN_RADIUS_DIFF;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (!gameMode)
			return;

		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Insert(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_AmbientPatrolManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AmbientPatrolManager()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (!gameMode)
			return;

		gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerKilled().Remove(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Remove(OnPlayerSpawnedOrDeleted);
		gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnected);
	}
}
