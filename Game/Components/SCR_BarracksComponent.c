[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "Component to be used with barrack compositions, handing unit spawning.", color: "0 0 255 255")]
class SCR_BarracksComponentClass: SCR_ServicePointComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BarracksComponent: SCR_ServicePointComponent
{
	
	[Attribute("{D4A1576757665D02}Prefabs/Groups/Campaign/Group_US_Defenders.et", UIWidgets.ResourceNamePicker, "Defending Group Prefab", "et")]
	protected ResourceName m_sGroupPrefab;
	
	[Attribute("1", UIWidgets.Auto, "Number of Groups to be spawned")]
	protected int m_iAIGroups;
	
	[Attribute("{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", UIWidgets.ResourceNamePicker, "Defend waypoint prefab", "et")]
	protected ResourceName m_sDefendWaypointPrefab;
	
	[Attribute("150", UIWidgets.Auto, "Time between unit respawns")]
	protected float m_fRespawnDelay;
	
	[Attribute("5", UIWidgets.Auto, "Offset from spawn position")]
	protected float m_fSpawnOffset;
	
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, desc: "Local spawn position offset coords from owner", params: "inf inf purpose=coords space=entity")]
	protected vector m_vSpawnPosition;
	
	protected RplComponent m_RplComponent;
	protected bool m_bEnableSpawning = true;
	protected ref array <ref SCR_AIBarracksGroup> m_aAIgroups = {};
	protected ref array <SCR_AIGroup> m_aOldGroups = {};
	
	protected static const int SPAWN_RADIUS_MIN = Math.Pow(500, 2);
	protected static const int SPAWN_RADIUS_MAX = Math.Pow(1000, 2);
	protected static const int GROUP_HANDLER_DELAY = 1000;		
	//------------------------------------------------------------------------------------------------
	float GetRespawnDelay()
	{
		return m_fRespawnDelay;
	}
	//------------------------------------------------------------------------------------------------
	void EnableSpawning(bool enableSpawning)
	{
		m_bEnableSpawning = enableSpawning;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupPrefab(ResourceName groupPrefab)
	{
		//unassign previous groups
		SCR_AIGroup aiGroup;
		foreach (SCR_AIBarracksGroup grp : m_aAIgroups)
		{
			aiGroup = grp.GetGroup();
			if (!aiGroup)
				continue;
			
			m_aOldGroups.Insert(aiGroup);
			grp.SetGroup(null);
		}
		
		m_sGroupPrefab = groupPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupCount(int count)
	{
		m_iAIGroups = count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns position parameters for spawning 
	protected EntitySpawnParams GetSpawnParameters(SCR_AIBarracksGroup barrackGrp)
	{
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		GetOwner().GetTransform(params.Transform);
		
		vector offset;
		foreach (SCR_AIBarracksGroup group : m_aAIgroups)
		{
			if (group == barrackGrp)
				break;
			
			offset[2] = offset[2] + m_fSpawnOffset;
		}
		
		params.Transform[3] = params.Transform[3] + offset;
		
		vector spawnposMat[4];
		spawnposMat[3] = m_vSpawnPosition;
		
		Math3D.MatrixMultiply4(params.Transform, spawnposMat, params.Transform);
		return params;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles group spawning and despawning
	protected void HandleGroups()
	{
		// despawn AI units if no player is in vicinity
		if (!PlayerDistanceCheck())
		{
			foreach (SCR_AIBarracksGroup grp : m_aAIgroups)
			{
				if (!grp.GetIsDespawned())
					DespawnGroup(grp);
			}
			
			// Delete old, untracked groups, which lasted from previous base owner
			array<AIAgent> units = new array<AIAgent>();
			foreach (SCR_AIGroup oldGrp : m_aOldGroups)
			{
				if (!oldGrp)
					continue;
				
				oldGrp.GetAgents(units);
		
				for (int i = units.Count() - 1; i >= 0; i--)
				{
					oldGrp.RemoveAIEntityFromGroup(units[i].GetControlledEntity());
					delete units[i].GetControlledEntity();
					delete units[i];
				}
			}
			
			m_bEnableSpawning = true;
			return;
		}
		
		if (!m_bEnableSpawning)
			return;
		
		// Creating new groups
		CreateNewGroups();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creating new groups and refilling current ones
	protected void CreateNewGroups()
	{
		if (m_aAIgroups.Count() != m_iAIGroups)
		{
			SCR_AIBarracksGroup grp;
			for (int i, cnt = m_iAIGroups - m_aAIgroups.Count(); i < cnt; i++)
			{
				grp = new SCR_AIBarracksGroup;
				m_aAIgroups.Insert(grp);
			}
		}
		
		foreach (SCR_AIBarracksGroup grp : m_aAIgroups)
		{
			if (!grp.GetGroup())
			{
				CreateGroup(grp);
				grp.GetOnUnitRemoved().Insert(SetRefillRequests);
			}
			
			FillGroup(grp);
			EnableSpawning(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true, if there is player around barracks
	protected bool PlayerDistanceCheck()
	{
		array<int> players = {};
		int playersCount = GetGame().GetPlayerManager().GetPlayers(players);
		
		float spawnDistanceSq = Math.Clamp(Math.Pow(GetGame().GetViewDistance() / 2, 2), SPAWN_RADIUS_MAX, SPAWN_RADIUS_MIN);
		float despawnDistanceSq = Math.Clamp(Math.Pow(GetGame().GetViewDistance() / 2, 2), SPAWN_RADIUS_MIN, SPAWN_RADIUS_MAX);
		IEntity playerEntity;
		float dist;
		vector origin = GetOwner().GetOrigin();
		
		for (int i = 0; i < playersCount; i++)
		{
			playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(players[i]);
			
			if (!playerEntity)
				continue;

			dist = vector.DistanceSq(playerEntity.GetOrigin(), origin);
			
			if (dist < spawnDistanceSq)
			{
				return true;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateGroup(SCR_AIBarracksGroup barrackGrp)
	{
		if (IsProxy())
			return;
		
		EntitySpawnParams params = GetSpawnParameters(barrackGrp);
		
		Resource res = Resource.Load(m_sGroupPrefab);
		if (!res.IsValid())
    		return;
		
		Resource wpRes = Resource.Load(m_sDefendWaypointPrefab);
		if (!wpRes.IsValid())
    		return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
		if (!group)
			return;
		
		OnGroupSpawned(group);
		
		SCR_DefendWaypoint wp = SCR_DefendWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(wpRes, null, params));
		if (!wp)
			return;
		
		group.AddWaypoint(wp);
		
		//Set group invokers
		group.GetOnAgentRemoved().Insert(barrackGrp.OnRemove);
		group.GetOnAgentAdded().Insert(barrackGrp.OnSpawn);
		
		barrackGrp.SetGroup(group);
		barrackGrp.SetGroupSize(group.m_aUnitPrefabSlots.Count());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillGroup(SCR_AIBarracksGroup barrackGrp)
	{
		SCR_AIGroup group = barrackGrp.GetGroup();
		if (!group)
			return;
		
		int filling = barrackGrp.GetUnitRefillCount();
		for (int index = 0; index < filling; index++)
		{
			SpawnUnit(group.m_aUnitPrefabSlots.GetRandomElement(), barrackGrp);	
		}
		
		barrackGrp.SetDespawned(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGroupSpawned(notnull SCR_AIGroup grp)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetRefillRequests(SCR_AIBarracksGroup grp)
	{
		if (grp.GetRefillRequested())
			return;
		
		GetGame().GetCallqueue().CallLater(GroupRefillRequest, m_fRespawnDelay * 1000, false, grp);
		grp.SetRefillRequested(true);
	}
	//------------------------------------------------------------------------------------------------
	protected void GroupRefillRequest(SCR_AIBarracksGroup grp)
	{
		grp.SetRefillRequested(false);
		EnableSpawning(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn Units with given ResourceName
	protected void SpawnUnit(ResourceName unitResource, SCR_AIBarracksGroup barrackGrp)
	{
		if (IsProxy())
			return;
		
		SCR_AIGroup group = barrackGrp.GetGroup();
		if (!group)
			return;
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = group.GetOrigin();
		Resource res = Resource.Load(unitResource);
		
		int index = group.GetAgentsCount();
		
		AIFormationDefinition formationDefinition;
		AIFormationComponent formationComponent = AIFormationComponent.Cast(group.FindComponent(AIFormationComponent));
		if (formationComponent)
			formationDefinition = formationComponent.GetFormation();
		
		if (formationDefinition)		
			params.Transform[3] = group.CoordToParent(formationDefinition.GetOffsetPosition(index));
		else
			params.Transform[3] = group.CoordToParent(Vector(index, 0, 0));
		
		vector angles = Math3D.MatrixToAngles(params.Transform);
		angles[0] = GetOwner().GetAngles()[1];
		Math3D.AnglesToMatrix(angles, params.Transform);
		
		IEntity ai = GetGame().SpawnEntityPrefab(res, null, params);
		if (!ai)
			return;
		
		group.AddAIEntityToGroup(ai);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used to despawn Groups based on Distance
	protected void DespawnGroup(SCR_AIBarracksGroup grp)
	{
		SCR_AIGroup group = grp.GetGroup();
		if (!group)
			return;
		
		grp.SetDespawned(true);
		
		array<AIAgent> units = new array<AIAgent>();
		group.GetAgents(units);
		
		for (int i = units.Count() - 1; i >= 0; i--)
		{
			group.RemoveAIEntityFromGroup(units[i].GetControlledEntity());
			delete units[i].GetControlledEntity();
			delete units[i];
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		if (!GetGame().InPlayMode())
			return;
		
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
		
		if (IsProxy())
			return;
		
		GetGame().GetCallqueue().CallLater(HandleGroups, GROUP_HANDLER_DELAY, true);
	}
	
	//------------------------------------------------------------------------------------------------
	// PostInit
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BarracksComponent()
	{
		GetGame().GetCallqueue().Remove(HandleGroups);
		GetGame().GetCallqueue().Remove(GroupRefillRequest);
		
		SCR_AIGroup aiGroup;
		foreach(SCR_AIBarracksGroup grp : m_aAIgroups)
		{
			aiGroup = grp.GetGroup();
			if (aiGroup)
				SCR_EntityHelper.DeleteEntityAndChildren(grp.GetGroup());
		}
	}
};

//------------------------------------------------------------------------------------------------
class SCR_AIBarracksGroup
{
	protected ref ScriptInvoker Event_OnUnitRemoved;
	
	protected SCR_AIGroup m_Group;
 	protected int m_iRefillCount = -1;
	protected int m_iSize;
	protected bool m_bRefillRequested;
	protected bool m_bIsDespawned;
	
	ScriptInvoker GetOnUnitRemoved()
	{
		if (!Event_OnUnitRemoved)
			Event_OnUnitRemoved = new ScriptInvoker;
		
		return Event_OnUnitRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetUnitsAlive()
	{
		if (!m_Group)
			return 0;
		
		return m_Group.GetAgentsCount();
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetRefillRequested()
	{
		return m_bRefillRequested;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRefillRequested(bool requested)
	{
		m_bRefillRequested = requested;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsDespawned()
	{
		return m_bIsDespawned;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetUnitRefillCount()
	{
		return m_iSize - GetUnitsAlive();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetGroup()
	{
		return m_Group;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroup(SCR_AIGroup group)
	{
		m_Group = group;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDespawned(bool despawned)
	{
		m_bIsDespawned = despawned;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupSize(int groupSize)
	{
		m_iSize = groupSize;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGroupSize()
	{
		return m_iSize;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSpawn()
	{
		if (m_iSize == 0)
			m_iSize = m_Group.m_aUnitPrefabSlots.Count();
		
		if (m_iRefillCount != -1)
			m_iRefillCount--;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRemove(AIAgent agent)
	{
		if (m_bIsDespawned)
			return;
		
		Event_OnUnitRemoved.Invoke(this);
		
		if (GetUnitsAlive() < 1)
			m_iRefillCount = 0;
	}
}
