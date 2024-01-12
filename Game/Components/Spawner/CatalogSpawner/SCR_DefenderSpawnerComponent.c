#include "scripts/Game/config.c"
[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "Component to be used with barrack compositions, handing unit spawning.", color: "0 0 255 255")]
class SCR_DefenderSpawnerComponentClass : SCR_SlotServiceComponentClass
{
	[Attribute(defvalue: "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", UIWidgets.ResourceNamePicker, desc: "Default waypoint prefab", "et", category: "Defender Spawner")]
	protected ResourceName m_sDefaultWaypointPrefab;

	[Attribute(defvalue :"{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et", UIWidgets.ResourceNamePicker, desc: "Default group to be initially assigned to created units", "et", category: "Defender Spawner")]
	protected ResourceName m_sDefaultGroupPrefab;

	[Attribute(defvalue: "{FFF9518F73279473}PrefabsEditable/Auto/AI/Waypoints/E_AIWaypoint_Move.et", UIWidgets.ResourceNamePicker, "Move waypoint prefab", "et", category: "Defender Spawner")]
	protected ResourceName m_sMoveWaypointPrefab;
	
	[Attribute(defvalue: "2.5", params: "0 inf", desc: "Completion radius of initial move waypoint", "et", category: "Defender Spawner")]
	protected float m_fMoveWaypointCompletionRadius;
	
	[Attribute(defvalue: "75", params: "0 inf", desc: "Radius of default waypoint", "et", category: "Defender Spawner")]
	protected float m_fDefaultWaypointCompletionRadius;

	//------------------------------------------------------------------------------------------------
	ResourceName GetMoveWaypointPrefab()
	{
		return m_sMoveWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultWaypointPrefab()
	{
		return m_sDefaultWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultGroupPrefab()
	{
		return m_sDefaultGroupPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMoveWaypointCompletionRadius()
	{
		return m_fMoveWaypointCompletionRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDefaultWaypointCompletionRadius()
	{
		return m_fDefaultWaypointCompletionRadius;
	}
};

void OnDefenderGroupSpawnedDelegate(SCR_DefenderSpawnerComponent spawner, SCR_AIGroup group);
typedef func OnDefenderGroupSpawnedDelegate;
typedef ScriptInvokerBase<OnDefenderGroupSpawnedDelegate> OnDefenderGroupSpawnedInvoker;

//------------------------------------------------------------------------------------------------
//! Service providing group of defenders defined in faction. Requires SCR_EnableDefenderAction on ActionManager for players to manage its functionality.
class SCR_DefenderSpawnerComponent : SCR_SlotServiceComponent
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Defender Spawner", desc: "Allowed labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aAllowedLabels;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Defender Spawner", desc: "Ignored labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIgnoredLabels;

	[Attribute(defvalue: "300", uiwidget: UIWidgets.Auto, category: "Defender Spawner", desc: "Time in seconds between unit respawns")]
	protected float m_fRespawnDelay;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, category: "Defender Spawner", desc: "Enable spawning"), RplProp()]
	protected bool m_bEnableSpawning;

	[Attribute(desc: "Enables supplies usage.", category: "Supplies")]
	protected bool m_bSuppliesConsumptionEnabled;
	
	[Attribute(params: "0 inf", desc: "Custom supplies value.", category: "Supplies")]
	protected int m_iCustomSupplies;
	
	protected RplComponent m_RplComponent;
	protected SCR_CampaignSuppliesComponent m_SupplyComponent; //TODO: Temporary until supply sandbox rework
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_SpawnerAIGroupManagerComponent m_GroupSpawningManager;
	#ifndef AR_DEFENDER_SPAWN_TIMESTAMP
	protected float m_fNextRespawnTime;
	#else
	protected WorldTimestamp m_fNextRespawnTime;
	#endif
	protected SCR_EntityCatalogEntry m_GroupEntry;
	protected SCR_AIGroup m_AIgroup;
	protected AIWaypoint m_Waypoint;
	protected int m_iDespawnedGroupMembers;
	protected SCR_Faction m_CurrentFaction;
	protected ref ScriptInvoker m_OnSpawnerOwningFactionChanged = new ScriptInvoker(); //Invokes with new and old factions assigned to this spawner
	protected ref array<ref Tuple2<AIWaypoint, SCR_AIGroup>> m_aGroupWaypoints = {};
	protected ref array<ResourceName> m_aRefillQueue;
	protected ref array<SCR_AIGroup> m_aPreviousGroups;
	
	protected ref OnDefenderGroupSpawnedInvoker m_OnDefenderGroupSpawned;

	protected static const int SPAWN_CHECK_INTERVAL = 1000;
	protected static const int SPAWN_RADIUS_MIN = Math.Pow(500, 2);
	protected static const int SPAWN_RADIUS_MAX = Math.Pow(1000, 2);
	
	//------------------------------------------------------------------------------------------------
	SCR_Faction GetCurrentFaction()
	{
		return m_CurrentFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns invoker called on faction change of spawner. Invokes with new faction to be assigned and previous faction
	ScriptInvoker GetOnFactionChanged()
	{
		if (!m_OnSpawnerOwningFactionChanged)
			m_OnSpawnerOwningFactionChanged = new ScriptInvoker();

		return m_OnSpawnerOwningFactionChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	OnDefenderGroupSpawnedInvoker GetOnDefenderGroupSpawned()
	{
		if (!m_OnDefenderGroupSpawned)
			m_OnDefenderGroupSpawned = new OnDefenderGroupSpawnedInvoker();

		return m_OnDefenderGroupSpawned;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign custom waypoint to be used by defender group
	void SetWaypoint(SCR_AIWaypoint wp)
	{
		if (wp)
			m_Waypoint = wp;
	}

	//------------------------------------------------------------------------------------------------
	//! Set Defender group data from Entity catalog on owning faction
	protected void AssignDefenderGroupDataFromOwningFaction()
	{
		if (!m_CurrentFaction)
			return;
		
		SCR_EntityCatalog catalog = m_CurrentFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.GROUP);
		if (!catalog)
		{
			Print("SCR_Faction Config lacks catalog of GROUP type. SCR_DefenderSpawnerComponent won't work without it", LogLevel.ERROR);
			return;
		}

		array<SCR_EntityCatalogEntry> entityEntries = {};
		array<typename> includedDataClasses = {};
		includedDataClasses.Insert(SCR_EntityCatalogSpawnerData); //~ The Data the entity must have

		catalog.GetFullFilteredEntityList(entityEntries, m_aAllowedLabels, m_aIgnoredLabels, includedDataClasses, null, false);

		//Only one entry of defender group should be used. Rest will be ditched, at least for now
		if (!entityEntries.IsEmpty())
			m_GroupEntry = entityEntries[0];
	}

	//------------------------------------------------------------------------------------------------
	//! Assign supply component to handle supplies of spawner. Currently uses Campaign specific supplies (temporarily)
	void AssignSupplyComponent(notnull SCR_CampaignSuppliesComponent supplyComp)
	{
		m_SupplyComponent = supplyComp;
		
		//Resets timer, so new soldiers are spawned, if spawning is enabled.
		#ifndef AR_DEFENDER_SPAWN_TIMESTAMP
		m_fNextRespawnTime = 0;
		#else
		m_fNextRespawnTime = null;
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Get supply component assigned to spawner, temporary

	SCR_CampaignSuppliesComponent GetSpawnerSupplyComponent()
	{
		return m_SupplyComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Get available supplies
	float GetSpawnerSupplies()
	{
		if (!m_ResourceComponent)
			return m_iCustomSupplies;
		
		SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
			
		if (!consumer)
			return 0.0;
		
		if (m_RplComponent && !m_RplComponent.IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(consumer);
		
		return consumer.GetAggregatedResourceValue();
	}

	//------------------------------------------------------------------------------------------------
	//! Set the spawner supplies value
	void AddSupplies(float value)
	{
		if (m_ResourceComponent)
		{
			SCR_ResourceInteractor interactor;
			
			if (value >= 0)
			{
				SCR_ResourceGenerator generator = m_ResourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				
				if (!generator)
					return;
				
				interactor = generator;
				
				generator.RequestGeneration(value);
			}
			else
			{
				SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				
				if (!consumer)
					return;
				
				interactor = consumer;
				
				consumer.RequestConsumtion(-value);
			}
		}

		m_iCustomSupplies = m_iCustomSupplies + value;
		if (m_iCustomSupplies < 0)
			m_iCustomSupplies = 0;
				
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if spawner is currently activated
	bool IsSpawningEnabled()
	{
		return m_bEnableSpawning;
	}

	//------------------------------------------------------------------------------------------------
	//! Enable spawning of defender groups
	//! \param enable bool to enable or disable spawning
	//! \param playerID id of player using this action
	void EnableSpawning(bool enable, int playerID)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
		if (!playerController)
			return;

		SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(playerController.GetControlledEntity());
		if (!chimeraCharacter || chimeraCharacter.GetFaction() != m_CurrentFaction)
			return;

		m_bEnableSpawning = enable;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn Units with given ResourceName and assign it to group
	//! \param unitResource resource name of unit to be spawned
	//! \param consumeSupplies if false, spawning this unit won't consume supplies. Is used for respawning units, which were previously despawned to save performance
	protected void SpawnUnit(ResourceName unitResource, bool consumeSupplies = true)
	{
		if (IsProxy())
			return;
		
		if (!m_GroupEntry || !m_AIgroup || !m_AIgroup.m_aUnitPrefabSlots || m_AIgroup.m_aUnitPrefabSlots.IsEmpty())
			return;
		
		SCR_EntityCatalogSpawnerData entityData = SCR_EntityCatalogSpawnerData.Cast(m_GroupEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!entityData)
			return;
		
		SCR_EntitySpawnerSlotComponent slot = GetFreeSlot();
		if (!slot)
			return;
		
		// Calculate unit spawn cost from overall group member count
		int spawnCost;
		if (m_AIgroup.m_aUnitPrefabSlots.Count() > 0)
			spawnCost = entityData.GetSupplyCost() / m_AIgroup.m_aUnitPrefabSlots.Count();

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		slot.GetOwner().GetTransform(spawnParams.Transform);
		Resource res = Resource.Load(unitResource);

		IEntity ai = GetGame().SpawnEntityPrefab(res, null, spawnParams);
		if (!ai)
			return;

		if (m_aRefillQueue)
			m_aRefillQueue.RemoveItem(unitResource);
		
		// consume supplies, if their consumption is enabled and revents spawning, if there is not enough supplies available.		
		if (consumeSupplies && m_bSuppliesConsumptionEnabled && GetSpawnerSupplies() >= spawnCost)
			AddSupplies(-spawnCost);

		AIControlComponent control = AIControlComponent.Cast(ai.FindComponent(AIControlComponent));
		if (control)
			control.ActivateAI();
		
		AIAgent agent = control.GetAIAgent();
		if (!agent)
			return;
		
		//Remove group waypoint to reduce performance impact due to new members being added to it.
		if (m_Waypoint)
			m_AIgroup.RemoveWaypoint(m_Waypoint);
		
		SCR_EntityLabelPointComponent rallyPointEntity = slot.GetRallyPoint();
		if (rallyPointEntity)
		{
			// Create temporary group for unit and later rally point waypoint
			SCR_AIGroup group = CreateTemporaryGroup();
			if (!group)
			{
				m_AIgroup.AddAgent(agent);
				return;
			}
		
			SCR_AIWaypoint wp = CreateRallyPointWaypoint(rallyPointEntity);
			if (!wp)
			{
				m_AIgroup.AddAgent(agent);
				return;
			}
			
			if (!m_aGroupWaypoints)
				m_aGroupWaypoints = {};
			
			group.AddAgent(agent);
			group.AddWaypoint(wp);
			m_aGroupWaypoints.Insert(new Tuple2<AIWaypoint, SCR_AIGroup>(wp, group));
			group.GetOnWaypointCompleted().Insert(OnGroupWaypointFinished);
			group.GetOnAgentRemoved().Insert(OnAIAgentRemoved);
		}
		else
		{
			// Rally point not found, add spawned ai directly to controlled group
			m_AIgroup.AddAgent(agent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAIAgentRemoved(SCR_AIGroup group, AIAgent ai)
	{
		foreach (int index, Tuple2<AIWaypoint, SCR_AIGroup> groupWaypoint : m_aGroupWaypoints)
		{
			if (groupWaypoint.param2 != group)
				continue;
			
			m_aGroupWaypoints.Remove(index);
			break;
		}
		
		if (m_aGroupWaypoints.IsEmpty())
			m_aGroupWaypoints = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGroupWaypointFinished(notnull AIWaypoint wp)
	{	
		SCR_AIGroup group;
		int groupIndex;
		
		foreach (int i, Tuple2<AIWaypoint, SCR_AIGroup> groupWaypoint : m_aGroupWaypoints)
		{
			if (groupWaypoint.param1 != wp)
				continue;
			
			group = groupWaypoint.param2;
			groupIndex = i;
			break;
		}
		
		if (!group)
			return;

		group.GetOnAgentRemoved().Remove(OnAIAgentRemoved);
		
		array<AIAgent> agents = {};
		group.GetAgents(agents);

		if (!m_AIgroup)
			return;
		
		foreach (AIAgent agent : agents)
		{	
			m_AIgroup.AddAgent(agent);
		}

		m_aGroupWaypoints.Remove(groupIndex);
		if (m_aGroupWaypoints.IsEmpty())
			m_aGroupWaypoints = null;
		
		if ((m_AIgroup.GetAgentsCount() == m_AIgroup.m_aUnitPrefabSlots.Count()) || (!m_aGroupWaypoints))
			m_AIgroup.AddWaypoint(GetWaypoint());
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_AIGroup CreateTemporaryGroup()
	{
		SCR_DefenderSpawnerComponentClass prefabData = SCR_DefenderSpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;
		
		Resource groupRes = Resource.Load(prefabData.GetDefaultGroupPrefab());
		if (!groupRes.IsValid())
			return null;
			
		return SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(groupRes, GetGame().GetWorld()));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns waypoint created as rally point for spawned unit
	protected SCR_AIWaypoint CreateRallyPointWaypoint(notnull SCR_EntityLabelPointComponent rallyPoint)
	{
		SCR_DefenderSpawnerComponentClass prefabData = SCR_DefenderSpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;

		EntitySpawnParams wpParams = new EntitySpawnParams();
		wpParams.TransformMode = ETransformMode.WORLD;
		rallyPoint.GetOwner().GetTransform(wpParams.Transform);

		Resource wpRes = Resource.Load(prefabData.GetMoveWaypointPrefab());
		if (!wpRes.IsValid())
			return null;
		
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(wpRes, null, wpParams));
		
		//Set completion radius
		if (wp)
			wp.SetCompletionRadius(prefabData.GetMoveWaypointCompletionRadius());
		
		return wp;
	}

	//------------------------------------------------------------------------------------------------
	//!Returns free slot
	SCR_EntitySpawnerSlotComponent GetFreeSlot()
	{
		foreach (SCR_EntitySpawnerSlotComponent slot : m_aChildSlots)
		{
			if (!slot.IsOccupied())
				return slot;
		}

		foreach (SCR_EntitySpawnerSlotComponent slot : m_aNearSlots)
		{
			if (!slot.IsOccupied())
				return slot;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Refills AI's in defender group
	protected void ReinforceGroup()
	{
		array<ResourceName> unitPrefabs = {};
		unitPrefabs.Copy(m_AIgroup.m_aUnitPrefabSlots);

		array<AIAgent> outAgents = {};
		m_AIgroup.GetAgents(outAgents);

		foreach (AIAgent agent : outAgents)
		{
			IEntity controlledEntity = agent.GetControlledEntity();
			if (!controlledEntity)
				return;

			EntityPrefabData prefabData = controlledEntity.GetPrefabData();
			if (!prefabData)
				return;

			ResourceName aiResource = prefabData.GetPrefabName();
			unitPrefabs.RemoveItem(aiResource);
		}

		IEntity leader;
		if (m_aGroupWaypoints)
		{
			foreach (Tuple2<AIWaypoint, SCR_AIGroup> groups : m_aGroupWaypoints)
			{
				if (groups.param2)
					leader = groups.param2.GetLeaderEntity();
				
				if (!leader)
					continue;
				
				EntityPrefabData prefabData = leader.GetPrefabData();
				if (!prefabData)
					continue;
				
				unitPrefabs.RemoveItem(prefabData.GetPrefabName());
			}
		}
		
		m_aRefillQueue = unitPrefabs;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn defender AI group from faction prefab. Also handles price per spawned unit.
	//! Should some units be despawned because of player distance, spawn price will be reduced by it
	protected void SpawnGroup()
	{
		if (IsProxy())
			return;

		if (!m_GroupEntry)
			return;

		SCR_EntityCatalogSpawnerData entityData = SCR_EntityCatalogSpawnerData.Cast(m_GroupEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!entityData)
			return;

		float supplies = GetSpawnerSupplies();
		int spawnCost = entityData.GetSupplyCost();
		if (supplies < entityData.GetSupplyCost() && m_bSuppliesConsumptionEnabled)
			return;

		SCR_EntitySpawnerSlotComponent slot = GetFreeSlot();
		if (!slot)
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		slot.GetOwner().GetTransform(params.Transform);

		Resource res = Resource.Load(m_GroupEntry.GetPrefab());
		if (!res.IsValid())
			return;

		SCR_AIGroup.IgnoreSpawning(true);
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
		if (!group || group.m_aUnitPrefabSlots.Count() == 0)
		{
			SCR_AIGroup.IgnoreSpawning(false);
			return;
		}

		m_AIgroup = group;
		
		if (m_OnDefenderGroupSpawned)
			m_OnDefenderGroupSpawned.Invoke(this, group);
		
		ReinforceGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected AIWaypoint GetWaypoint()
	{
		if (m_Waypoint)
			return m_Waypoint;
		
		SCR_EntitySpawnerSlotComponent slot = GetFreeSlot();
		if (!slot)
			return null;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		
		SCR_EntityLabelPointComponent rallyPoint = slot.GetRallyPoint();
		//If rally point is available, create waypoint on it, otherwise use slot position
		if (rallyPoint)
			rallyPoint.GetOwner().GetTransform(params.Transform);
		else
			slot.GetOwner().GetTransform(params.Transform);

		SCR_DefenderSpawnerComponentClass prefabData = SCR_DefenderSpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;

		Resource wpRes = Resource.Load(prefabData.GetDefaultWaypointPrefab());
		if (!wpRes.IsValid())
			return null;

		m_Waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(wpRes, null, params));
		if (!m_Waypoint)
			return null;
		
		m_Waypoint.SetCompletionRadius(prefabData.GetDefaultWaypointCompletionRadius());
		
		return m_Waypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true, if there is player around barracks
	protected bool PlayerDistanceCheck()
	{
		array<int> players = {};
		int playersCount = GetGame().GetPlayerManager().GetPlayers(players);

		float spawnDistanceSq = Math.Clamp(Math.Pow(GetGame().GetViewDistance() * 0.5, 2), SPAWN_RADIUS_MAX, SPAWN_RADIUS_MIN);
		float despawnDistanceSq = Math.Clamp(Math.Pow(GetGame().GetViewDistance() * 0.5, 2), SPAWN_RADIUS_MIN, SPAWN_RADIUS_MAX);
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
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Manages automatic group spawning
	protected void HandleGroup()
	{
		#ifndef AR_DEFENDER_SPAWN_TIMESTAMP
		float replicationTime = Replication.Time();
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp replicationTime = world.GetServerTimestamp();
		#endif
		
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (!aiWorld)
			return;
		
		//If another Ai would be over limit, stop requesting and prevent players from enabling it again.
		if ((aiWorld.GetCurrentAmountOfLimitedAIs() + 1) >= aiWorld.GetAILimit())
		{
			if (m_bEnableSpawning)
			{
				m_bEnableSpawning = false; //disable spawner
				Replication.BumpMe();
				
				m_GroupSpawningManager.SetIsAtAILimit(true); //block requesting action 
			}
			
			return;
		}
		
		bool distanceCheck = PlayerDistanceCheck();

		if (m_bEnableSpawning && distanceCheck)
		{
			// Reinforce existing or create new defender group. Also handles respawning of despawned defenders
			#ifndef AR_DEFENDER_SPAWN_TIMESTAMP
			if (m_iDespawnedGroupMembers > 0 || (replicationTime > m_fNextRespawnTime))
			{
				m_fNextRespawnTime = replicationTime + (m_fRespawnDelay * 1000);
			#else
			if (m_iDespawnedGroupMembers > 0 || replicationTime.Greater(m_fNextRespawnTime))
			{
				m_fNextRespawnTime = replicationTime.PlusSeconds(m_fRespawnDelay);
			#endif
				if (m_AIgroup && (m_AIgroup.m_aUnitPrefabSlots.Count() != m_AIgroup.GetAgentsCount()))
					ReinforceGroup();

				if (!m_AIgroup)
					SpawnGroup();
			}
			
			//Spawn queued units
			if (m_aRefillQueue && !m_aRefillQueue.IsEmpty())
			{
				if (m_iDespawnedGroupMembers > 0)
				{
					SpawnUnit(m_aRefillQueue[0], false);
					m_iDespawnedGroupMembers--;
				}
				else
				{
					SpawnUnit(m_aRefillQueue[0]);
				}
			}
		}
		
		//Despawn units that are too far away from players
		if (!distanceCheck)
		{
			if (m_AIgroup)
			{
				m_iDespawnedGroupMembers = m_AIgroup.GetAgentsCount();
				SCR_EntityHelper.DeleteEntityAndChildren(m_AIgroup);
			}
			
			//If leftover groups from previous owners remains alive, they will be despawned
			if (m_aPreviousGroups)
			{
				foreach (SCR_AIGroup aiGroup : m_aPreviousGroups)
				{
					SCR_EntityHelper.DeleteEntityAndChildren(aiGroup);
				}
				
				m_aPreviousGroups = null;
			}
			
			if (m_aGroupWaypoints)
			{
				foreach (Tuple2<AIWaypoint, SCR_AIGroup> groupWaypoint : m_aGroupWaypoints)
				{
					SCR_EntityHelper.DeleteEntityAndChildren(groupWaypoint.param2);
					m_iDespawnedGroupMembers++;
				}
				
				m_aGroupWaypoints = null;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
		if (faction == m_CurrentFaction)
			return;
		
		//if there is already a group from current faction, cache it for despawn
		if (m_AIgroup)
		{
			if (!m_aPreviousGroups)
				m_aPreviousGroups = new array<SCR_AIGroup>;
			
			m_aPreviousGroups.Insert(m_AIgroup);
			m_AIgroup = null;
		}
		
		SCR_Faction newFaction = SCR_Faction.Cast(faction);
		
		m_GroupEntry = null;

		SCR_Faction oldFaction = m_CurrentFaction;
		m_CurrentFaction = newFaction;
		
		AssignDefenderGroupDataFromOwningFaction();
		m_OnSpawnerOwningFactionChanged.Invoke(newFaction, oldFaction);
	}

	//------------------------------------------------------------------------------------------------
	protected void AssignInitialFaction()
	{
		if (!m_FactionControl)
			return;

		Faction faction = m_FactionControl.GetAffiliatedFaction();
		if (!faction)
			faction = m_FactionControl.GetDefaultAffiliatedFaction();

		m_CurrentFaction = SCR_Faction.Cast(faction);
		if (!m_CurrentFaction)
			m_CurrentFaction = SCR_Faction.Cast(m_FactionControl.GetDefaultAffiliatedFaction());

		if (m_CurrentFaction)
			AssignDefenderGroupDataFromOwningFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		super.EOnInit(owner);

		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
			return;

		AssignInitialFaction();
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;	
			
		m_GroupSpawningManager = SCR_SpawnerAIGroupManagerComponent.Cast(gameMode.FindComponent(SCR_SpawnerAIGroupManagerComponent));
		if (!m_GroupSpawningManager)
		{
			Print("SCR_DefenderSpawnerComponent requires SCR_SpawnerAIGroupManagerComponent attached to gamemode to work properly!", LogLevel.ERROR);	
			return;
		}	
			
		if (IsProxy())
			return;

		//Setup group handling (spawning and refilling). This delay also prevents potential JIP replication error on clients
		GetGame().GetCallqueue().CallLater(HandleGroup, SPAWN_CHECK_INTERVAL, true);
	}
		
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DefenderSpawnerComponent()
	{
		if (m_AIgroup)
			SCR_EntityHelper.DeleteEntityAndChildren(m_AIgroup);

		if (m_aGroupWaypoints)
		{
			foreach (Tuple2<AIWaypoint, SCR_AIGroup> groupWaypoint : m_aGroupWaypoints)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(groupWaypoint.param2);
			}
		}
			
		GetGame().GetCallqueue().Remove(HandleGroup);
	}
};
