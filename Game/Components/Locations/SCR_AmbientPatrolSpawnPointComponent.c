#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
class SCR_AmbientPatrolSpawnPointComponentClass: ScriptComponentClass
{
	[Attribute("{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et", UIWidgets.ResourceNamePicker, "Cycle waypoint to be used for waypoints in hierarchy.", "et")]
	protected ResourceName m_sCycleWaypointPrefab;
	
	[Attribute("{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", UIWidgets.ResourceNamePicker, "Waypoint to be used if no waypoints are found in hierarchy.", "et")]
	protected ResourceName m_sDefaultWaypointPrefab;
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetCycleWaypointPrefab()
	{
		return m_sCycleWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultWaypointPrefab()
	{
		return m_sDefaultWaypointPrefab;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_AmbientPatrolSpawnPointComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EGroupType))]
	protected SCR_EGroupType m_eGroupType;
	
	[Attribute("0")]
	protected bool m_bPickRandomGroupType;
	
	[Attribute("0", UIWidgets.EditBox, "How often will the group respawn. (seconds, 0 = no respawn)", "0 inf 1")]
	protected int m_iRespawnPeriod;
	
	[Attribute("0.95", desc: "If (CurrentAIs / AILimit) > this value, the group will not be spawned.",  params: "0 0.95 0.01")]
	protected float m_fAILimitThreshold;
	
	protected bool m_bSpawned;
	protected bool m_bPaused;
	
	protected int m_iID;
	protected int m_iMembersAlive = -1;
	
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fRespawnTimestamp;
	protected float m_fDespawnTimer = -1;
	#else
	protected WorldTimestamp m_fRespawnTimestamp;
	protected WorldTimestamp m_fDespawnTimer;
	#endif
	
	protected AIWaypoint m_Waypoint;
	
	protected ResourceName m_sPrefab;
	
	protected SCR_AIGroup m_Group;
	
	protected Faction m_SavedFaction;
	
	//------------------------------------------------------------------------------------------------
	void SetID(int ID)
	{
		m_iID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetID()
	{
		return m_iID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMembersAlive(int count)
	{
		m_iMembersAlive = count;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMembersAlive()
	{
		return m_iMembersAlive;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsSpawned(bool spawned)
	{
		m_bSpawned = spawned;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsSpawned()
	{
		return m_bSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Pause this spawnpoint so a group will not be spawned unless players leave and re-enter the area
	void SetIsPaused(bool paused)
	{
		m_bPaused = paused;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsPaused()
	{
		return m_bPaused;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAILimitThreshold()
	{
		return m_fAILimitThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetDespawnTimer(float time)
	#else
	void SetDespawnTimer(WorldTimestamp time)
	#endif
	{
		m_fDespawnTimer = time;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetDespawnTimer()
	#else
	WorldTimestamp GetDespawnTimer()
	#endif
	{
		return m_fDespawnTimer;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetRespawnTimestamp(float timestamp)
	#else
	void SetRespawnTimestamp(WorldTimestamp timestamp)
	#endif
	{
		m_fRespawnTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetRespawnTimestamp()
	#else
	WorldTimestamp GetRespawnTimestamp()
	#endif
	{
		return m_fRespawnTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetSpawnedGroup()
	{
		return m_Group;
	}
	
	//------------------------------------------------------------------------------------------------
	AIWaypoint GetWaypoint()
	{
		return m_Waypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	protected ResourceName GetRandomPrefabByProbability(notnull SCR_EntityCatalog entityCatalog, notnull array<SCR_EntityCatalogEntry> data)
	{
		float highestProbability;
		array<ResourceName> prefabs = {};
		array<int> eligiblePrefabIds = {};
		array<float> probabilities = {};
		SCR_EntityCatalogEntry catalogEntry;
		SCR_EntityCatalogAmbientPatrolData patrolData;
		float probability;
		
		for (int i = 0, count = data.Count(); i < count; i++)
		{
			int catalogIndex = data[i].GetCatalogIndex();
			catalogEntry = entityCatalog.GetCatalogEntry(catalogIndex);
			
			if (!catalogEntry)
				continue;
			
			patrolData = SCR_EntityCatalogAmbientPatrolData.Cast(catalogEntry.GetEntityDataOfType(SCR_EntityCatalogAmbientPatrolData));
			
			if (!patrolData)
				continue;
			
			probability = patrolData.GetProbabilityOfPresence();
			
			prefabs.Insert(catalogEntry.GetPrefab());
			probabilities.Insert(probability);
			
			if (probability > highestProbability)
				highestProbability = probability
		}
		
		if (prefabs.IsEmpty())
			return ResourceName.Empty;
		
		Math.Randomize(-1);
		float rand = Math.RandomFloat(0, highestProbability);
		
		for (int i = 0, count = probabilities.Count(); i < count; i++)
		{
			if (probabilities[i] >= rand)
				eligiblePrefabIds.Insert(i);
		}
		
		if (eligiblePrefabIds.IsEmpty())
			return ResourceName.Empty;
		
		return prefabs[eligiblePrefabIds.GetRandomElement()];
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Update(SCR_Faction faction)
	{
		if (!m_Waypoint)
			PrepareWaypoints();
		
		m_SavedFaction = faction;
		
		if (!faction)
			return;
		
		SCR_EntityCatalog entityCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.GROUP);
		
		if (!entityCatalog)
			return;
		
		array<SCR_EntityCatalogEntry> data = {};
		entityCatalog.GetEntityListWithData(SCR_EntityCatalogAmbientPatrolData, data);
		
		if (m_bPickRandomGroupType)
		{
			m_sPrefab = GetRandomPrefabByProbability(entityCatalog, data);
			return;
		}
		
		SCR_EntityCatalogEntry catalogEntry;
		SCR_EntityCatalogAmbientPatrolData patrolData;
		
		for (int i = 0, count = data.Count(); i < count; i++)
		{
			int catalogIndex = data[i].GetCatalogIndex();
			catalogEntry = entityCatalog.GetCatalogEntry(catalogIndex);
			
			if (!catalogEntry)
				continue;
			
			patrolData = SCR_EntityCatalogAmbientPatrolData.Cast(catalogEntry.GetEntityDataOfType(SCR_EntityCatalogAmbientPatrolData));
			
			if (!patrolData)
				continue;
			
			if (patrolData.GetGroupType() != m_eGroupType)
				continue;
			
			m_sPrefab = catalogEntry.GetPrefab();
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected AIWaypointCycle SpawnCycleWaypoint(notnull EntitySpawnParams params)
	{
		array<AIWaypoint> waypoints = {};
		array<IEntity> queue = {GetOwner()};
		AIWaypoint waypoint;
		IEntity processedEntity;
		IEntity nextInHierarchy;
		
		while (!queue.IsEmpty())
		{
			processedEntity = queue[0];
			queue.Remove(0);
			
			waypoint = AIWaypoint.Cast(processedEntity);
			
			if (waypoint)
				waypoints.Insert(waypoint);
			
			nextInHierarchy = processedEntity.GetChildren();
			
			while (nextInHierarchy)
			{
				queue.Insert(nextInHierarchy);
				nextInHierarchy = nextInHierarchy.GetSibling();
			}
		}
		
		if (waypoints.IsEmpty())
			return null;
		
		if (waypoints.Count() == 1)
		{
			m_Waypoint = waypoints[0];
			return null;
		}
		
		SCR_AmbientPatrolSpawnPointComponentClass componentData = SCR_AmbientPatrolSpawnPointComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return null;
		
		Resource waypointResource = Resource.Load(componentData.GetCycleWaypointPrefab());
		
		if (!waypointResource || !waypointResource.IsValid())
			return null;
		
		AIWaypointCycle wp = AIWaypointCycle.Cast(GetGame().SpawnEntityPrefab(waypointResource, null, params));
		
		if (!wp)
		{
			Print("SCR_AmbientPatrolSpawnPointComponent: AIWaypointCycle cast of m_sCycleWaypointPrefab has failed!", LogLevel.ERROR);
			return null;
		}
		
		wp.SetWaypoints(waypoints);
		
		return wp;
	}
	
	//------------------------------------------------------------------------------------------------
	void PrepareWaypoints()
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = GetOwner().GetOrigin();
		
		AIWaypointCycle predefinedWaypoint = SpawnCycleWaypoint(params);
		
		if (predefinedWaypoint)
		{
			m_Waypoint = predefinedWaypoint;
			return;
		}
		else if (m_Waypoint)
		{
			return;
		}
		
		SCR_AmbientPatrolSpawnPointComponentClass componentData = SCR_AmbientPatrolSpawnPointComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return;
		
		Resource waypointResource = Resource.Load(componentData.GetDefaultWaypointPrefab());
		
		if (!waypointResource || !waypointResource.IsValid())
			return;
		
		AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(waypointResource, null, params));
		
		if (!wp)
			return;
		
		m_Waypoint = wp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnPatrol()
	{
		SCR_FactionAffiliationComponent comp = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		
		if (!comp)
			return;
		
		SCR_Faction faction = SCR_Faction.Cast(comp.GetAffiliatedFaction());
		
		if (!faction)
			faction = SCR_Faction.Cast(comp.GetDefaultAffiliatedFaction());
		
		if (faction != m_SavedFaction || m_iRespawnPeriod > 0)
			Update(faction);
		
		m_bSpawned = true;
		
		if (m_sPrefab.IsEmpty())
			return;
		
		Resource prefab = Resource.Load(m_sPrefab);
		
		if (!prefab || !prefab.IsValid())
			return;
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = GetOwner().GetOrigin();
		Math.Randomize(-1);
		
		if (m_iRespawnPeriod == 0 && m_Waypoint && Math.RandomFloat01() >= 0.5)
		{
			AIWaypointCycle cycleWP = AIWaypointCycle.Cast(m_Waypoint);
			
			if (cycleWP)
			{
				array<AIWaypoint> waypoints = {};
				cycleWP.GetWaypoints(waypoints);
				params.Transform[3] = waypoints.GetRandomElement().GetOrigin();
			}
		}
		
		m_Group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(prefab, null, params));
		
		if (!m_Group)
			return;
		
		if (!m_Group.GetSpawnImmediately())
		{
			if (m_iMembersAlive > 0)
				m_Group.SetMaxUnitsToSpawn(m_iMembersAlive);
			
			m_Group.SpawnUnits();
		}
		
		m_Group.AddWaypoint(m_Waypoint);
		
		if (m_iRespawnPeriod != 0)
			m_Group.GetOnAgentRemoved().Insert(OnAgentRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	void DespawnPatrol()
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		m_fDespawnTimer = -1;
		#else
		m_fDespawnTimer = null;
		#endif
		m_bSpawned = false;
		
		if (!m_Group)
		{
			m_iMembersAlive = 0;
			return;
		}
		
		array<AIAgent> units = {};
		m_Group.GetAgents(units);
		int count = m_Group.GetAgentsCount();
		m_iMembersAlive = count;
		RplComponent.DeleteRplEntity(m_Group, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAgentRemoved()
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (!m_Group || m_Group.GetAgentsCount() > 0 || m_fRespawnTimestamp >= Replication.Time())
			return;
		#else
		if (!m_Group || m_Group.GetAgentsCount() > 0)
			return;

		ChimeraWorld world = GetOwner().GetWorld();
		if (m_fRespawnTimestamp.GreaterEqual(world.GetServerTimestamp()))
			return;
		#endif
		
		// Set up respawn timestamp, convert s to ms, reset original group size
		#ifndef AR_CAMPAIGN_TIMESTAMP
		m_fRespawnTimestamp = (Replication.Time() + (m_iRespawnPeriod * 1000));
		#else
		m_fRespawnTimestamp = world.GetServerTimestamp().PlusSeconds(m_iRespawnPeriod);
		#endif
		m_iMembersAlive = -1;
		m_bSpawned = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		
		if (!factionComponent)
		{
			Print("SCR_AmbientPatrolSpawnPointComponent: SCR_FactionAffiliationComponent not found on owner entity. Patrol spawning will not be available.", LogLevel.WARNING);
			return;
		}
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode() || Replication.IsClient())
			return;
		
		SCR_AmbientPatrolManager.GetInstance().RegisterPatrol(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AmbientPatrolSpawnPointComponent()
	{
		SCR_AmbientPatrolManager manager = SCR_AmbientPatrolManager.GetInstance(false);
		
		if (manager)
			manager.UnregisterPatrol(this);
	}
};
