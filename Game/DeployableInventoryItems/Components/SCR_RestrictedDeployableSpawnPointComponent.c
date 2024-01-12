[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_RestrictedDeployableSpawnPointComponentClass : SCR_BaseDeployableSpawnPointComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! Deployable spawn point with configurable conditions
class SCR_RestrictedDeployableSpawnPointComponent : SCR_BaseDeployableSpawnPointComponent
{
	[Attribute(defvalue: "1")]
	protected int m_iMaxSpawnPointsPerGroup;

	[Attribute(defvalue: "10")]
	protected int m_iMaxRespawns;

	[Attribute(defvalue: "1", desc: "Check if bases are nearby before deploying")]
	protected bool m_bQueryBases;

	[Attribute(defvalue: "0", desc: "Check if spawn points are nearby before deploying")]
	protected bool m_bQuerySpawnPoints;

	[Attribute(defvalue: "0", desc: "Check if characters are nearby before deploying")]
	protected bool m_bQueryCharacters;

	[Attribute(defvalue: "300.0", desc: "Query radius for military bases")]
	protected float m_fQueryRadiusBases;

	[Attribute(defvalue: "250.0", desc: "Query radius for other existing spawn points")]
	protected float m_fQueryRadiusSpawnPoints;

	[Attribute(defvalue: "100.0", desc: "Query radius for enemy characters")]
	protected float m_fQueryRadiusCharacters;

	[Attribute(defvalue: "0", desc: "Query all military bases, not just Main Operating Bases")]
	protected bool m_bQueryAllBases;

	[Attribute(defvalue: "0")]
	protected bool m_bIgnoreEnemyBases;

	[Attribute(defvalue: "0")]
	protected bool m_bIgnoreEnemySpawnPoints;

	[Attribute(defvalue: "0")]
	protected bool m_bIgnoreEnemyCharacters;

	[Attribute(defvalue: "0")]
	protected bool m_bUnlockActionsForEnemyFactions;

	[Attribute(defvalue: "0")]
	protected bool m_bAllowAllGroupsToSpawn;

	[Attribute(defvalue: "0")]
	protected bool m_bUnlockActionsForAllGroups;

	[RplProp()]
	protected int m_iGroupID = -1;

	protected bool m_bSpawnLimitReached;

	protected int m_iRespawnCount;

	protected bool m_bCanBeDeployed;

	//------------------------------------------------------------------------------------------------
	//! Returns true when there are no entities in the specified area that could prevent deploying
	protected bool CanDeploy(notnull array<IEntity> entities, notnull Faction spawnPointFaction, vector spawnPointOrigin)
	{
		if (entities.IsEmpty())
			return true;

		float queryRadius = 100; // Default 100 meters
		bool isBase, isSpawnPoint, isCharacter;

		if (SCR_MilitaryBaseComponent.Cast(entities[0].FindComponent(SCR_MilitaryBaseComponent)))
		{
			queryRadius = m_fQueryRadiusBases;
			isBase = true;
		}
		else if (SCR_SpawnPoint.Cast(entities[0]))
		{
			queryRadius = m_fQueryRadiusSpawnPoints;
			isSpawnPoint = true;
		}
		else if (SCR_ChimeraCharacter.Cast(entities[0]))
		{
			queryRadius = m_fQueryRadiusCharacters;
			isCharacter = true;
		}

		float radiusSq = queryRadius * queryRadius;

		foreach (IEntity e : entities)
		{
			float distanceToItemSq = (spawnPointOrigin - e.GetOrigin()).LengthSq();
			bool isWithinRadius = distanceToItemSq < radiusSq;

			if (!isWithinRadius)
				continue;

			SCR_Faction faction = SCR_Faction.Cast(SCR_Faction.GetEntityFaction(e));
			if (!faction)
				return false;

			bool isFriendlyFaction = faction.DoCheckIfFactionFriendly(spawnPointFaction);

			if (isBase)
			{
				if (isFriendlyFaction)
					return false;

				if (!isFriendlyFaction && !m_bIgnoreEnemyBases)
					return false;
			}
			else if (isSpawnPoint)
			{
				if (isFriendlyFaction)
					return false;

				if (!isFriendlyFaction && !m_bIgnoreEnemySpawnPoints)
					return false;
			}
			else if (isCharacter)
			{
				if (!isFriendlyFaction && !m_bIgnoreEnemyCharacters)
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanActionBeShown(notnull IEntity userEntity, bool checkCanDeploy, bool checkFaction = true, bool checkGroupID = true)
	{
		if (checkCanDeploy && !m_bDeployableSpawnPointsEnabled)
			return false;

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(userEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation)
			return false;

		Faction affiliatedFaction = factionAffiliation.GetAffiliatedFaction();
		if (!affiliatedFaction)
			return false;

		FactionKey userFactionKey = affiliatedFaction.GetFactionKey();
		if (!userFactionKey)
			return false;

		if (userFactionKey != m_FactionKey && !m_bUnlockActionsForEnemyFactions && checkFaction)
			return false;

		SCR_PossessingManagerComponent possessingManagerComp = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManagerComp)
			return false;

		int userID = possessingManagerComp.GetIdFromControlledEntity(userEntity);

		SCR_PlayerControllerGroupComponent playerControllerGroupComp = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(userID);
		if (!playerControllerGroupComp)
			return false;

		int userGroupID = playerControllerGroupComp.GetGroupID();

		if (userFactionKey == m_FactionKey && userGroupID != m_iGroupID && !m_bUnlockActionsForAllGroups && checkGroupID)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if deploying is possible first, then deploy at the end of the timed action
	void TryDeploy(IEntity userEntity)
	{
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp || rplComp.IsProxy())
			return;

		SCR_PossessingManagerComponent possessingManagerComp = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManagerComp)
			return;

		if (!userEntity)
			return;

		int userID = possessingManagerComp.GetIdFromControlledEntity(userEntity);

		SCR_PlayerControllerGroupComponent playerControllerGroupComp = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(userID);
		if (!playerControllerGroupComp)
			return;

		m_iGroupID = playerControllerGroupComp.GetGroupID();
		if (m_iGroupID < 0)
			m_iGroupID = 0;

		Replication.BumpMe();

		// Check if deployed item limit for user group has been reached
		array<SCR_RestrictedDeployableSpawnPointComponent> deployedSpawnPointsInGroup = {};

		foreach (SCR_BaseDeployableSpawnPointComponent spawnPointComp : s_aActiveDeployedSpawnPoints)
		{
			SCR_RestrictedDeployableSpawnPointComponent restrictedSpawnPointComp = SCR_RestrictedDeployableSpawnPointComponent.Cast(spawnPointComp);

			if (!restrictedSpawnPointComp)
				continue;

			if (restrictedSpawnPointComp.GetGroupID() == m_iGroupID)
				deployedSpawnPointsInGroup.Insert(restrictedSpawnPointComp);
		}

		if (deployedSpawnPointsInGroup.Count() >= m_iMaxSpawnPointsPerGroup)
		{
			SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_DEPLOYED_SPAWNPOINT_LIMIT, deployedSpawnPointsInGroup.Count(), m_iMaxSpawnPointsPerGroup);
			return;
		}

		array<IEntity> entities = {};
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey(m_FactionKey));

		// Check for nearby bases and prevent deploy if there are any
		if (m_bQueryBases)
		{
			SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();
			if (!baseManager)
				return;

			array<SCR_MilitaryBaseComponent> baseComponents = {};

			baseManager.GetBases(baseComponents);

			foreach (SCR_MilitaryBaseComponent baseComponent : baseComponents)
			{
				if (m_bQueryAllBases)
				{
					entities.Insert(baseComponent.GetOwner());
					continue;
				}

				SCR_CampaignMilitaryBaseComponent campaignBaseComponent = SCR_CampaignMilitaryBaseComponent.Cast(baseComponent);
				if (!campaignBaseComponent || !campaignBaseComponent.IsHQ())
					continue;

				entities.Insert(baseComponent.GetOwner());
			}

			if (!CanDeploy(entities, faction, GetOwner().GetOrigin()))
			{
				if (m_bQueryAllBases)
					SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_NEARBY_BASE);
				else
					SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_NEARBY_HQ);

				return;
			}
		}

		// Check for nearby spawnpoints and prevent deploy if there are any
		if (m_bQuerySpawnPoints)
		{
			entities = {};

			array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPoints();

			foreach (SCR_SpawnPoint spawnPoint : spawnPoints)
			{
				if (!SCR_PlayerRadioSpawnPoint.Cast(spawnPoint))
					entities.Insert(spawnPoint);
			}

			if (!CanDeploy(entities, faction, GetOwner().GetOrigin()))
			{
				SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_NEARBY_SPAWNPOINT);
				return;
			}
		}

		// Check for nearby enemy characters and prevent deploy if there are any
		if (m_bQueryCharacters)
		{
			entities = {};

			array<SCR_ChimeraCharacter> characters = SCR_CharacterRegistrationComponent.GetChimeraCharacters();
			if (!characters)
				return;

			foreach (SCR_ChimeraCharacter character : characters)
			{
				if (character == userEntity || character.GetCharacterController().IsDead())
					continue;

				entities.Insert(character);
			}

			if (!CanDeploy(entities, faction, GetOwner().GetOrigin()))
			{
				SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_NEARBY_PLAYERS);
				return;
			}
		}

		m_bCanBeDeployed = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if deploy is possible, then call super.Deploy()
	override void Deploy(IEntity userEntity)
	{
		if (!m_bCanBeDeployed)
			return;

		super.Deploy(userEntity);

		if (!m_SpawnPoint)
			return;

		SCR_RestrictedDeployableSpawnPoint restrictedSpawnPoint = SCR_RestrictedDeployableSpawnPoint.Cast(m_SpawnPoint);
		if (!restrictedSpawnPoint)
			return;

		restrictedSpawnPoint.SetRespawnCount(m_iRespawnCount);
		restrictedSpawnPoint.SetMaxRespawns(m_iMaxRespawns);

		restrictedSpawnPoint.SetAllowAllGroupsToSpawn(m_bAllowAllGroupsToSpawn);
		restrictedSpawnPoint.SetGroupID(m_iGroupID);
	}

	//------------------------------------------------------------------------------------------------
	//! Cache respawn count; then call super.Dismantle()
	override void Dismantle(IEntity userEntity = null)
	{
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
			if (!rplComp || rplComp.IsProxy())
				return;

		SCR_RestrictedDeployableSpawnPoint restrictedSpawnPoint = SCR_RestrictedDeployableSpawnPoint.Cast(m_SpawnPoint);
		if (!restrictedSpawnPoint)
			return;

		m_iRespawnCount = restrictedSpawnPoint.GetRespawnCount();

		m_iGroupID = -1; // reset groupID
		Replication.BumpMe();

		m_bCanBeDeployed = false;

		super.Dismantle(userEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Display amount of respawns left - called from SCR_ShowItemInfo.PerformAction
	void ShowInfo(notnull IEntity userEntity)
	{
		SCR_PossessingManagerComponent possessingManagerComp = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManagerComp)
			return;

		int userID = possessingManagerComp.GetIdFromControlledEntity(userEntity);
		int respawnsLeft = m_iMaxRespawns - m_iRespawnCount;

		if (!m_bIsDeployed || m_bAllowAllGroupsToSpawn)
		{
			SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_DISPLAY_RESPAWN_COUNT, respawnsLeft, m_iMaxRespawns);
			return;
		}

		SCR_PlayerControllerGroupComponent playerControllerGroupComp = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(userID);
		if (!playerControllerGroupComp)
			return;

		int userGroupID = playerControllerGroupComp.GetGroupID();

		if (userGroupID != m_iGroupID)
		{
			SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_DISPLAY_GROUP, m_iGroupID);
			return;
		}

		SCR_NotificationsComponent.SendToPlayer(userID, ENotification.DEPLOYABLE_SPAWNPOINTS_DISPLAY_RESPAWN_COUNT, respawnsLeft, m_iMaxRespawns);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanDeployBeShown(notnull IEntity userEntity)
	{
		if (!CanActionBeShown(userEntity, true, checkGroupID : false))
			return false;

		return !m_bIsDeployed;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanDismantleBeShown(notnull IEntity userEntity)
	{
		if (!CanActionBeShown(userEntity, false))
			return false;

		return m_bIsDeployed;
	}

	//------------------------------------------------------------------------------------------------
	bool CanInfoBeShown(notnull IEntity userEntity)
	{
		return CanActionBeShown(userEntity, !m_bIsDeployed, checkGroupID : false);
	}

	//------------------------------------------------------------------------------------------------
	bool GetAllowAllGroupsToSpawn()
	{
		return m_bAllowAllGroupsToSpawn;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIgnoreEnemyCharacters()
	{
		return m_bIgnoreEnemyCharacters;
	}

	//------------------------------------------------------------------------------------------------
	int GetGroupID()
	{
		return m_iGroupID;
	}

	//------------------------------------------------------------------------------------------------
	float GetQueryRadiusCharacters()
	{
		return m_fQueryRadiusCharacters;
	}

	//------------------------------------------------------------------------------------------------
	void SetRespawnCount(int respawnCount)
	{
		m_iRespawnCount = respawnCount;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupRemoved(SCR_AIGroup group)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup itemGroup = groupsManager.FindGroup(m_iGroupID);

		if (group == itemGroup)
			Dismantle(); // Dismantle item if removed group is the same as the item group; otherwise deployed item would be unusable
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		groupsManager.GetOnPlayableGroupRemoved().Insert(OnGroupRemoved);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RestrictedDeployableSpawnPointComponent()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		groupsManager.GetOnPlayableGroupRemoved().Remove(OnGroupRemoved);
	}
}
