//------------------------------------------------------------------------------------------------
class SCR_GameModeCleanSweepClass: SCR_BaseGameModeClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_GameModeCleanSweep : SCR_BaseGameMode
{
	const int INVALID_AREA_INDEX = -1;
	const string ENEMY_PRESENCE_ELIMINATED_TEXT = "Enemy presence eliminated!\nNew area is being selected.\nPlease wait.";
	
	[Attribute("6")]
	int m_iMinAiGroups;
	
	[Attribute("10")]
	int m_iMaxAiGroups;
	
	[Attribute("1.5")]
	float m_fAiGroupsPerPlayer;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "What group type should spawn")]
	ResourceName m_GroupType;
	
	[Attribute(defvalue: "{EBD2A7DA0A3C6E17}UI/layouts/Menus/CleanSweep/CleanSweepAreaSelection.layout", desc: "Layout for area selection")]
	ResourceName m_AreaSelectionLayout;
	
	[Attribute("0")]
	bool m_bSwapSides;

	Widget m_wRoot;
	Widget m_wAreaSelectionWidget;
	TextWidget m_wText;
	
	ref array<IEntity> m_aEnemySoldiers = new ref array<IEntity>;
	ref array<SCR_AIGroup> m_aGroups;
	ref array<SCR_SpawnPoint> m_aEnemySpawnPoints;
	ref array<SCR_SpawnPoint> m_aPlayerSpawnPoints;
	AIWaypoint m_AttackWP;
	
	int m_iGameMasterID = -1;
	
	[RplProp(onRplName: "OnAreaChanged")]
	int areaID = INVALID_AREA_INDEX;
	
	//------------------------------------------------------------------------------------------------
	void OnAreaChanged()
	{
		if (m_wAreaSelectionWidget)
			m_wAreaSelectionWidget.RemoveFromHierarchy();
		
		if (!m_aEnemySpawnPoints)
			m_aEnemySpawnPoints = new ref array<SCR_SpawnPoint>();
		else
			m_aEnemySpawnPoints.Clear();
		
		if (!m_aPlayerSpawnPoints)
			m_aPlayerSpawnPoints = new ref array<SCR_SpawnPoint>();
		else
			m_aPlayerSpawnPoints.Clear();
		
		if (m_RplComponent && m_RplComponent.IsProxy())
			return;
		
		// Spawn setup:
		// Select random spawn point
		
		ref array<SCR_CleanSweepArea> activeAreas = new ref array<SCR_CleanSweepArea>();
		
		int activeAreasCount = GetActiveAreas(activeAreas);
		
		if (activeAreasCount < 1)
		{
			Print("CleanSweeup has not enough active playable areas. GameMode will not function.", LogLevel.ERROR);
			return;
		}
		
		SCR_CleanSweepArea area = activeAreas[areaID];
		if (!area)
			return;
		
		// Move waypoint to the area
		AIWaypoint wp = AIWaypoint.Cast(GetWorld().FindEntityByName("WP1"));
		if (wp)
			wp.SetOrigin(area.GetOrigin());
		
		vector centerPos = area.GetOrigin();
		float rangeSq = area.m_Range * area.m_Range;
		array<IEntity> entities = new array<IEntity>();
		array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPoints();
		GetWorld().GetActiveEntities(entities);
		
		for (int i = spawnPoints.Count() - 1; i >= 0; i--)
		{
			SCR_SpawnPoint spawnPoint = spawnPoints[i];
			if (vector.DistanceSqXZ(centerPos, spawnPoint.GetOrigin()) < rangeSq)
			{
				FactionKey faction = spawnPoint.GetFactionKey();
				if (faction == "USSR")
				{
					if (!m_bSwapSides)
						m_aEnemySpawnPoints.Insert(spawnPoint);
					else
						m_aPlayerSpawnPoints.Insert(spawnPoint);
				}
				if (faction == "US")
				{
					if (!m_bSwapSides)
						m_aPlayerSpawnPoints.Insert(spawnPoint);
					else
						m_aEnemySpawnPoints.Insert(spawnPoint);
				}
			}
		}
		
		for (int i = entities.Count() - 1; i >= 0; i--)
		{
			// Vehicle spawn component ??
			SCR_VehicleSpawner vehicleSpawner = SCR_VehicleSpawner.Cast(entities[i].FindComponent(SCR_VehicleSpawner));
			if (!vehicleSpawner)
				continue;
			
			if (vector.DistanceSqXZ(centerPos, entities[i].GetOrigin()) < rangeSq)
				vehicleSpawner.PerformSpawn();
		}
		
		SpawnEnemies();
		m_bAutoPlayerRespawn = true;
		
		RespawnPlayers();
	}
	
	//------------------------------------------------------------------------------------------------
	void RespawnPlayers()
	{
		array<int> players = new array<int>();
		int count = GetGame().GetPlayerManager().GetPlayers(players);
		for (int i = count - 1; i >= 0; i--)
		{
			PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(players[i]);
			
			if (!playerController)
				continue;
			
			SCR_CleanSweepNetworkComponent networkComponent = SCR_CleanSweepNetworkComponent.Cast(playerController.FindComponent(SCR_CleanSweepNetworkComponent));
			if (!networkComponent)
				continue;
			
			networkComponent.CommitSuicide();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSwapSides(bool swapSides)
	{
		m_bSwapSides = swapSides;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetArea(int targetAreaID /*= areaID this is error*/)
	{
		areaID = targetAreaID;
		OnAreaChanged();
		Replication.BumpMe();
		
		if (m_RplComponent && !m_RplComponent.IsProxy())
			GetGame().GetCallqueue().CallLater(CheckActiveAreaState, 1, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckActiveAreaState()
	{
		ChimeraCharacter character;
		DamageManagerComponent damageManager;
		for (int i = m_aEnemySoldiers.Count() -1; i >= 0; i--)
		{
			if (!m_aEnemySoldiers[i])
				m_aEnemySoldiers.Remove(i);
			else
			{
				character = ChimeraCharacter.Cast(m_aEnemySoldiers[i]);
				if (!character)
				{
					m_aEnemySoldiers.Remove(i);
					continue;
				}
				
				damageManager = character.GetDamageManager();
				if (!damageManager)
					return;
				
				if (damageManager.GetState() != EDamageState.DESTROYED)
					return;
			}
		}
		
		if (m_RplComponent && !m_RplComponent.IsProxy())
		{
			ReplicatedShowHint(0, 5);
			
			if (RplSession.Mode() != RplMode.Dedicated)
				ShowHint(0, 5);
			
			areaID = INVALID_AREA_INDEX;
			Replication.BumpMe();
			GetGame().GetCallqueue().Remove(CheckActiveAreaState);
			ShowAreaSelectionToGameMaster();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (RplSession.Mode() != RplMode.Dedicated && areaID == INVALID_AREA_INDEX && m_wAreaSelectionWidget)
		{
			InputManager inputManager = GetGame().GetInputManager();
			inputManager.ActivateContext("MenuContext");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ReplicatedShowHint(int hintID, float showTime)
	{
		Rpc(RPC_ShowHint, hintID, showTime);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_ShowHint(int hintID, float time)
	{
		ShowHint(hintID, time);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowHint(int hintID, float showTime)
	{
		if (!m_wText || !m_wRoot)
			return;
		
		switch (hintID)
		{
			case 0:
			{
				m_wText.SetText(ENEMY_PRESENCE_ELIMINATED_TEXT);
				break;
			}
		}
		
		AnimateWidget.Opacity(m_wRoot, 1, 1);
		
		ScriptCallQueue queue = GetGame().GetCallqueue(); 
		queue.CallLater(this.HideHint, showTime * 1000);
	}
	
	//------------------------------------------------------------------------------------------------
	void HideHint()
	{
		if (m_wRoot)
			AnimateWidget.Opacity(m_wRoot, 0, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnEnemies()
	{
		if (m_RplComponent && m_RplComponent.IsProxy())
			return;
		
		if (m_aEnemySpawnPoints.Count() == 0)
			return;
		
		// Clean up groups
		for (int i = m_aGroups.Count() - 1; i >= 0; i--)
		{
			delete m_aGroups[i];
		}
		m_aGroups.Clear();
		m_aEnemySoldiers.Clear();
		
		int enemyGroupsCount = Math.ClampInt((Math.Ceil(GetGame().GetPlayerManager().GetPlayerCount() * m_fAiGroupsPerPlayer)), m_iMinAiGroups, m_iMaxAiGroups);
		for (int i = 0; i < enemyGroupsCount; i++)
		{
			RandomGenerator generator = new RandomGenerator;
			generator.SetSeed(Math.RandomInt(0,100));

			SCR_SpawnPoint spawnPoint = m_aEnemySpawnPoints.GetRandomElement();
			if (!spawnPoint)
				return;
			
			vector position = generator.GenerateRandomPointInRadius(0, 2, spawnPoint.GetOrigin());
			position[1] = spawnPoint.GetOrigin()[1];
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = position;
			
			Resource res = Resource.Load(m_GroupType);
			SCR_AIGroup newGrp = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
			if (!newGrp)
				continue;
			m_aGroups.Insert(newGrp);
			
			array<AIAgent> agents = new array<AIAgent>;
			
			newGrp.GetAgents(agents);
			foreach (AIAgent agent : agents)
			{
				if (agent)
					m_aEnemySoldiers.Insert(agent.GetControlledEntity());
			}
			
			if (m_AttackWP)
				newGrp.AddWaypoint(m_AttackWP);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Called after a player gets killed.
		\param playerId PlayerId of victim player.
		\param player Entity of victim player if any.
		\param killerEntity Entity of killer instigator if any.
		\param killer instigator of the kill
	*/
	protected override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
		m_aEnemySoldiers.RemoveItemOrdered(playerEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetPlayersCenter()
	{
		array<int> players = new array<int>();
		int count = GetGame().GetPlayerManager().GetPlayers(players);
		
		vector center = vector.Zero;
		int countedEntities = 0;
		
		for (int i = count - 1; i >= 0; i--)
		{
			IEntity controlledEntity = SCR_PossessingManagerComponent.GetPlayerMainEntity(players[i]);
			if (!controlledEntity)
				continue;
			
			vector entityPosition = controlledEntity.GetOrigin();
			center[0] = center[0] + entityPosition[0];
			center[1] = center[1] + entityPosition[1];
			center[2] = center[2] + entityPosition[2];
			
			countedEntities++;
		}
		
		if (countedEntities != 0)
		{
			center[0] = center[0] / countedEntities;
			center[1] = center[1] / countedEntities;
			center[2] = center[2] / countedEntities;
		}
		
		return center;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActiveAreas(array<SCR_CleanSweepArea> activeAreas)
	{
		int count = 0;
		
		array<SCR_CleanSweepArea> areas = SCR_CleanSweepArea.s_aInstances;
		if (!areas)
			return count;
		
		int areasCount = areas.Count();
		for (int i = 0; i < areasCount; i++)
		{
			SCR_CleanSweepArea area = areas[i];
			if (!area)
				continue;
			
			if (!area.m_Active)
				continue;
			
			activeAreas.Insert(area);
			count++;
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	void InitializeServer()
	{
		m_AttackWP = AIWaypoint.Cast(GetWorld().FindEntityByName("WP1"));
		m_aGroups = new ref array<SCR_AIGroup>;
		m_aPlayerSpawnPoints = new ref array<SCR_SpawnPoint>;
		m_aEnemySpawnPoints = new ref array<SCR_SpawnPoint>;
	}
	
	//------------------------------------------------------------------------------------------------
	PlayerController GetGameMaster()
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(m_iGameMasterID);
		if (!playerController)
			return PickNewGameMaster();
		
		return playerController;
	}
	
	//------------------------------------------------------------------------------------------------
	PlayerController PickNewGameMaster(int exclude = -1)
	{
		PlayerController playerController;
		
		array<int> players = {};
		int count = GetGame().GetPlayerManager().GetPlayers(players);
		for (int i = 0; i < count; i++)
		{
			if (players[i] == exclude)
				continue;
			
			playerController = GetGame().GetPlayerManager().GetPlayerController(players[i]);
			if (playerController)
			{
				m_iGameMasterID = players[i];
				break;
			}
		}
		
		return playerController;
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowAreaSelectionToGameMaster()
	{
		PlayerController playerController = GetGameMaster();
		if (!playerController)
				return;
		
		SCR_CleanSweepNetworkComponent networkComponent = SCR_CleanSweepNetworkComponent.Cast(playerController.FindComponent(SCR_CleanSweepNetworkComponent));
		if (!networkComponent)
			return;
		
		networkComponent.ShowAreaSelectionScreen();
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowAreaSelectionUI()
	{
		if (areaID != INVALID_AREA_INDEX || m_AreaSelectionLayout.GetPath() == string.Empty)
			return;
		
		if (!m_wAreaSelectionWidget)
			m_wAreaSelectionWidget = GetGame().GetWorkspace().CreateWidgets(m_AreaSelectionLayout);
		
		XComboBoxWidget selectionBox = XComboBoxWidget.Cast(m_wAreaSelectionWidget.FindAnyWidget("SelectionBox"));
		if (!selectionBox)
			return;
		
		array<SCR_CleanSweepArea> activeAreas = {};
		
		int count = GetActiveAreas(activeAreas);
		
		for (int i = 0; i < count; i++)
		{
			selectionBox.AddItem(activeAreas[i].GetName());
		}
		
		selectionBox.SetCurrentItem(0);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		Replication.BumpMe();
		if (areaID != INVALID_AREA_INDEX)
			return;
		
		ShowAreaSelectionToGameMaster();
		//GetGame().GetCallqueue().CallLater(ShowAreaSelectionToGameMaster, 10000, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);
		
		Replication.BumpMe();
		if (areaID != INVALID_AREA_INDEX || m_iGameMasterID != playerId)
			return;
		
		PickNewGameMaster(m_iGameMasterID);
		ShowAreaSelectionToGameMaster();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_bAutoPlayerRespawn = false;
		
		if (m_RplComponent && !m_RplComponent.IsProxy())
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerRegistered().Insert(OnPlayerRegistered);
		}
		
		m_wRoot = GetGame().GetWorkspace().CreateWidgets("{DE9F713BE2C5D190}UI/layouts/HUD/HintFrame.layout");
		m_wText = TextWidget.Cast(m_wRoot.FindAnyWidget("MessageText"));
		TextWidget title = TextWidget.Cast(m_wRoot.FindAnyWidget("TitleText"));
		if (title)
			title.SetText("CLEAN SWEEP");
		
		m_wRoot.SetOpacity(0);
		
		if (m_RplComponent && !m_RplComponent.IsProxy())
			InitializeServer();
	}
};
