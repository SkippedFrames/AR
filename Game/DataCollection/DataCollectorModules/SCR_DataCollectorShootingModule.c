[BaseContainerProps()]
class SCR_DataCollectorShootingModule : SCR_DataCollectorModule
{
	//TODO: Remove this map
	protected ref map<int, IEntity> m_mTrackedPossibleShooters = new map<int, IEntity>();

	//------------------------------------------------------------------------------------------------
	protected override void AddInvokers(IEntity player)
	{
		super.AddInvokers(player);
		if (!player)
			return;

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));

		if (!compartmentAccessComponent)
			return;

		compartmentAccessComponent.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
		compartmentAccessComponent.GetOnCompartmentLeft().Insert(OnCompartmentLeft);

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (!eventHandlerManager)
			return;

		eventHandlerManager.RegisterScriptHandler("OnProjectileShot", this, OnWeaponFired);
		eventHandlerManager.RegisterScriptHandler("OnGrenadeThrown", this, OnGrenadeThrown);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWeaponFired(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerID);

		//In the future we will use Weapon.GetWeaponType() and Weapon.GetWeaponSubtype() to determine the weapon shot and add it to the player's profile
		//For now, simply count a shot
		playerData.AddStat(SCR_EDataStats.SHOTS, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGrenadeThrown(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (!weapon)
			return;

		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerID);

		//Not counting smoke grenade 'cause it deals no damage. TO DO: Count it as a different specialization operation
		if (weapon.GetWeaponType() == EWeaponType.WT_SMOKEGRENADE)
			return;

		playerData.AddStat(SCR_EDataStats.GRENADES_THROWN, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected override void RemoveInvokers(IEntity player)
	{
		super.RemoveInvokers(player);
		if (!player)
			return;

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));

		if (!compartmentAccessComponent)
			return;

		compartmentAccessComponent.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
		compartmentAccessComponent.GetOnCompartmentLeft().Remove(OnCompartmentLeft);

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (!eventHandlerManager)
			return;

		eventHandlerManager.RemoveScriptHandler("OnProjectileShot", this, OnWeaponFired);
		eventHandlerManager.RemoveScriptHandler("OnGrenadeThrown", this, OnGrenadeThrown);
	}

	//Players who enter a vehicle do not need to be tracked as possible shooters, unless using a turret
	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);

		//Turrets can shoot
		if (!compartment || !compartment.GetOccupant() || SCR_CompartmentAccessComponent.GetCompartmentType(compartment) == ECompartmentType.Turret)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		int playerID = playerManager.GetPlayerIdFromControlledEntity(compartment.GetOccupant());

		m_mTrackedPossibleShooters.Remove(playerID);
	}

	//Players who leave a vehicle can be shooters
	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;

		if (!compartment.GetOccupant())
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		int playerID = playerManager.GetPlayerIdFromControlledEntity(compartment.GetOccupant());
		if (playerID == 0) // Non-player character
			return;

		m_mTrackedPossibleShooters.Insert(playerID, compartment.GetOccupant());
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		controlledEntity = m_mTrackedPossibleShooters.Get(playerID);
		super.OnPlayerDisconnected(playerID, controlledEntity);

		m_mTrackedPossibleShooters.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerID, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerID, controlledEntity);
		m_mTrackedPossibleShooters.Insert(playerID, controlledEntity);
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);

		if (to)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(to);
			m_mTrackedPossibleShooters.Insert(playerID, to);
		}
		else if (from)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(from);
			m_mTrackedPossibleShooters.Remove(playerID);
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	override void OnAIKilled(IEntity AIEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnAIKilled(AIEntity, killerEntity, killer);
		
		//This method adds a kill no matter the mean by which the AI was killed.
		//The name of the module is a little bit misleading		
		if (killer.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;

		SCR_ChimeraCharacter AIEntityChar = SCR_ChimeraCharacter.Cast(AIEntity);
		if (!AIEntityChar)
			return;
		
		int killerId = killer.GetInstigatorPlayerID();
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		Faction factionKiller = Faction.Cast(factionManager.GetPlayerFaction(killerId));
		if (!factionKiller)
			return;
		
		SCR_PlayerData killerData = GetGame().GetDataCollector().GetPlayerData(killerId);

		//Add an AI kill. Find if friendly or unfriendly
		if (factionKiller.IsFactionFriendly(AIEntityChar.GetFaction()))
			killerData.AddStat(SCR_EDataStats.FRIENDLY_AI_KILLS);
		else
			killerData.AddStat(SCR_EDataStats.AI_KILLS);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
		m_mTrackedPossibleShooters.Remove(playerId);

		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerId);
		playerData.AddStat(SCR_EDataStats.DEATHS);
		
		if (killer.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;
		
		SCR_ChimeraCharacter playerEntityChar = SCR_ChimeraCharacter.Cast(playerEntity);
		if (!playerEntityChar)
			return;
		
		int killerId = killer.GetInstigatorPlayerID();

		// Suicide?
		if (playerId == killerId)
			return;

		SCR_PlayerData killerData = GetGame().GetDataCollector().GetPlayerData(killerId);

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		Faction factionKiller = Faction.Cast(factionManager.GetPlayerFaction(killerId));
		if (!factionKiller)
			return;

		//Add a kill. Find if friendly or unfriendly
		if (factionKiller.IsFactionFriendly(playerEntityChar.GetFaction()))
			killerData.AddStat(SCR_EDataStats.FRIENDLY_KILLS);
		else
			killerData.AddStat(SCR_EDataStats.KILLS);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeTick)
	{
		//If there's no data collector, do nothing
		if (!GetGame().GetDataCollector())
			return;

		m_fTimeSinceUpdate += timeTick;

		if (m_fTimeSinceUpdate < m_fUpdatePeriod)
			return;

		SCR_PlayerData playerData;
		int playerId;

		for (int i = m_mTrackedPossibleShooters.Count() - 1; i >= 0; i--)
		{

			playerId = m_mTrackedPossibleShooters.GetKey(i);
			playerData = GetGame().GetDataCollector().GetPlayerData(playerId);

			//DEBUG display
#ifdef ENABLE_DIAG
			if (m_StatsVisualization)
			{
				m_StatsVisualization.Get(SCR_EShootingModuleStats.DEATHS).SetText((playerData.GetStat(SCR_EDataStats.DEATHS) - playerData.GetStat(SCR_EDataStats.DEATHS, false)).ToString());
				m_StatsVisualization.Get(SCR_EShootingModuleStats.PLAYERKILLS).SetText((playerData.GetStat(SCR_EDataStats.KILLS) - playerData.GetStat(SCR_EDataStats.KILLS, false)).ToString());
				m_StatsVisualization.Get(SCR_EShootingModuleStats.AIKILLS).SetText((playerData.GetStat(SCR_EDataStats.AI_KILLS) - playerData.GetStat(SCR_EDataStats.AI_KILLS, false)).ToString());
				m_StatsVisualization.Get(SCR_EShootingModuleStats.FRIENDLYPLAYERKILLS).SetText((playerData.GetStat(SCR_EDataStats.FRIENDLY_KILLS) - playerData.GetStat(SCR_EDataStats.FRIENDLY_KILLS, false)).ToString());
				m_StatsVisualization.Get(SCR_EShootingModuleStats.FRIENDLYAIKILLS).SetText((playerData.GetStat(SCR_EDataStats.FRIENDLY_AI_KILLS) - playerData.GetStat(SCR_EDataStats.FRIENDLY_AI_KILLS, false)).ToString());
				m_StatsVisualization.Get(SCR_EShootingModuleStats.BULLETSSHOT).SetText((playerData.GetStat(SCR_EDataStats.SHOTS) - playerData.GetStat(SCR_EDataStats.SHOTS, false)).ToString());
				m_StatsVisualization.Get(SCR_EShootingModuleStats.GRENADESTHROWN).SetText((playerData.GetStat(SCR_EDataStats.GRENADES_THROWN) - playerData.GetStat(SCR_EDataStats.GRENADES_THROWN, false)).ToString());
			}
#endif
		}
		m_fTimeSinceUpdate = 0;
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void CreateVisualization()
	{
		super.CreateVisualization();
		if (!m_StatsVisualization)
			return;

		CreateEntry("Deaths: ", 0, SCR_EShootingModuleStats.DEATHS);
		CreateEntry("Player Kills: ", 0, SCR_EShootingModuleStats.PLAYERKILLS);
		CreateEntry("AI Kills: ", 0, SCR_EShootingModuleStats.AIKILLS);
		CreateEntry("Friendly Player Kills: ", 0, SCR_EShootingModuleStats.FRIENDLYPLAYERKILLS);
		CreateEntry("Friendly AI Kills: ", 0, SCR_EShootingModuleStats.FRIENDLYAIKILLS);
		CreateEntry("Bullets Shot: ", 0, SCR_EShootingModuleStats.BULLETSSHOT);
		CreateEntry("Grenades Thrown: ", 0, SCR_EShootingModuleStats.GRENADESTHROWN);
	}
#endif
};

#ifdef ENABLE_DIAG
enum SCR_EShootingModuleStats
{
	DEATHS,
	PLAYERKILLS,
	AIKILLS,
	FRIENDLYPLAYERKILLS,
	FRIENDLYAIKILLS,
	BULLETSSHOT,
	GRENADESTHROWN
};
#endif
