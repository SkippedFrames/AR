//------------------------------------------------------------------------------------------------
class SCR_RespawnSystemComponentClass : RespawnSystemComponentClass
{
};

//! Scripted implementation that handles spawning and respawning of players.
//! Should be attached to a GameMode entity.
[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class SCR_RespawnSystemComponent : RespawnSystemComponent
{
	[Attribute(category: "Respawn System")]
	protected ref SCR_SpawnLogic m_SpawnLogic;
	
	[Attribute("1", uiwidget: UIWidgets.CheckBox, category: "Respawn System")]
	protected bool m_bEnableRespawn;
	
	[Attribute("1.5", desc: "Delay (in seconds) for opening deploy menu after death.")]
	protected float m_fDeployMenuOpenDelay;
	
	[Attribute("{A39BE59EB6F41125}Configs/Respawn/SpawnPointRequestResultInfoConfig.conf", desc: "Holds a config of all reasons why a specific spawnpoint can be disabled")]
	protected ResourceName m_sSpawnPointRequestResultInfoHolder;
	
	protected ref SCR_SpawnPointRequestResultInfoConfig m_SpawnPointRequestResultInfoHolder;

	// Instance of this component
	private static SCR_RespawnSystemComponent s_Instance = null;

	// The parent of this entity which should be a gamemode
	protected SCR_BaseGameMode m_pGameMode;
	// Parent entity's rpl component
	protected RplComponent m_pRplComponent;

	// Preload
	protected ref SimplePreload m_Preload;

	protected ref ScriptInvoker Event_OnRespawnEnabledChanged = new ref ScriptInvoker();

	SCR_BaseSpawnPointRequestResultInfo GetSpawnPointRequestResultInfo(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response, SCR_SpawnData data)
	{
		if (!m_SpawnPointRequestResultInfoHolder)
			return null;
		
		return m_SpawnPointRequestResultInfoHolder.GetFirstValidRequestResultInfo(requestComponent, response, data);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns an instance of RespawnSystemComponent
	static SCR_RespawnSystemComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode pGameMode = GetGame().GetGameMode();
			if (pGameMode)
				s_Instance = SCR_RespawnSystemComponent.Cast(pGameMode.FindComponent(SCR_RespawnSystemComponent));
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns either a valid Faction of local player character or null
	[Obsolete("Use SCR_FactionManager.SGetLocalPlayerFaction instead")]
	static Faction GetLocalPlayerFaction(IEntity player = null)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(player);
		if (character)
			return character.GetFaction();

		return SCR_FactionManager.SGetLocalPlayerFaction();
	}

	//------------------------------------------------------------------------------------------------
	//! Access to replication component
	RplComponent GetRplComponent()
	{
		return m_pRplComponent;
	}

	//------------------------------------------------------------------------------------------------
	// Called when a spawn is requested
	// Asks the gamemode with PickPlayerSpawnPoint query expecting to get a spawn point
	// at which the player should be spawned
	[Obsolete("Use SCR_SpawnHandlerComponent instead.")]
	protected override GenericEntity RequestSpawn(int playerId)
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	// Perform respawn using custom prefab
	// NOTE: temporary hack for 14546, please use with caution
	// TODO(koudelkaluk): remove me once there's a proper solution
	[Obsolete("Use SCR_RespawnComponent.RequestSpawn instead!")]
	GenericEntity CustomRespawn(int playerId, string prefab, vector position, vector rotation = vector.Zero)
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Verifies that provided loadout belongs to player assigned faction if faction manager is used.
	*/
	[Obsolete("Utilize SCR_LoadoutManager instead!")]
	bool CanSetLoadout(int playerId, int loadoutIndex)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	// Called from SCR_RespawnComponent
	[Obsolete("Utilize SCR_PlayerLoadoutComponent instead!")]
	void DoSetPlayerLoadout(int playerId, int loadoutIndex)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Verifies that provided faction can be assigned to provided player.
	*/
	[Obsolete("Use SCR_PlayerFactionAffiliationComponent instead")]
	bool CanSetFaction(int playerId, int factionIndex)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Verifies that provided spawnPoint belongs to player assigned faction if faction manager is used.
	*/
	[Obsolete("Spawn points are no longer assigned and are utilized directly through SCR_SpawnPointSpawnHandlerComponent")]
	bool CanSetSpawnPoint(int playerId, RplId spawnPointId)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	// Called from SCR_RespawnComponent
	[Obsolete("Spawn points are no longer assigned and are utilized directly through SCR_SpawnPointSpawnHandlerComponent")]
	void DoSetPlayerSpawnPoint(int playerId, RplId spawnPointIdentity)
	{
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_FactionManager.SGetFactionPlayerCount instead.")]
	int GetFactionPlayerCount(Faction faction)
	{
		return SCR_FactionManager.SGetFactionPlayerCount(faction);
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_LoadoutManager.SGetLoadoutPlayerCount instead")]
	int GetLoadoutPlayerCount(SCR_BasePlayerLoadout loadout)
	{
		return SCR_LoadoutManager.SGetLoadoutPlayerCount(loadout);
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_PlayerLoadoutComponent.RequestLoadout instead")]
	void SetPlayerLoadout(int playerId, int loadoutIndex)
	{
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_PlayerFactionAffiliationComponent instead.")]
	void SetPlayerFaction(int playerId, int factionIndex)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Returns faction corresponding to the provided index or null if none exists
	[Obsolete("Use SCR_FactionManager instead.")]
	Faction GetFactionByIndex(int factionIndex)
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Return faction index or -1 if not existant
	[Obsolete("Use SCR_FactionManager instead.")]
	int GetFactionIndex(Faction faction)
	{
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Try to get player faction from PlayerRespawnInfo
	//! If faction index is within valid bounds, return Faction otherwise null
	[Obsolete("Use SCR_FactionManager.SGetPlayerFaction instead")]
	Faction GetPlayerFaction(int playerId)
	{
		return SCR_FactionManager.SGetPlayerFaction(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns loadout corresponding to the provided index or null if none exists
	[Obsolete("Utilize SCR_LoadoutManager instead!")]
	SCR_BasePlayerLoadout GetLoadoutByIndex(int loadoutIndex)
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Try to get player loadout from PlayerRespawnInfo
	//! If loadout index is within valid bounds, return Faction otherwise null
	[Obsolete("Use SCR_LoadoutManager.SGetPlayerLoadout instead")]
	SCR_BasePlayerLoadout GetPlayerLoadout(int playerId)
	{
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Return faction index or -1 if not existant
	[Obsolete("Utilize SCR_LoadoutManager instead!")]
	int GetLoadoutIndex(SCR_BasePlayerLoadout loadout)
	{
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! UI management
	static MenuBase OpenRespawnMenu()
	{
		MenuManager pMenuManager = GetGame().GetMenuManager();
		if (!pMenuManager)
			return null;

		return pMenuManager.OpenMenu(ChimeraMenuPreset.RespawnSuperMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Close all menus operated by Respawn System
	static void CloseRespawnMenu()
	{
		MenuManager pMenuManager = GetGame().GetMenuManager();
		if (!pMenuManager)
			return;

		MenuBase menu = pMenuManager.FindMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu);
		if (menu)
			pMenuManager.CloseMenu(menu);
	}

	//------------------------------------------------------------------------------------------------
	//! Simple getter for other
	[Obsolete("...")]
	static bool IsRespawnMenuOpened()
	{
		MenuManager pMenuManager = GetGame().GetMenuManager();
		if (!pMenuManager)
			return false;

		return (pMenuManager.FindMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu) != null);
	}

	//------------------------------------------------------------------------------------------------
	//! Close all menus operated by Respawn System
	[Obsolete("...")]
	static void ToggleRespawnMenu()
	{
		if (IsRespawnMenuOpened())
			CloseRespawnMenu();
		else
			OpenRespawnMenu();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set respawn enabled Server only
	\param enableSpawning set respawn enabled or not
	*/
	void ServerSetEnableRespawn(bool enableSpawning)
	{
		if (!Replication.IsServer() || enableSpawning == m_bEnableRespawn)
			return;

		SetEnableRespawnBroadcast(enableSpawning);
		Rpc(SetEnableRespawnBroadcast, enableSpawning);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetEnableRespawnBroadcast(bool enableSpawning)
	{
		m_bEnableRespawn = enableSpawning;
		Event_OnRespawnEnabledChanged.Invoke(m_bEnableRespawn);
	}


	//------------------------------------------------------------------------------------------------
	/*!
	Get respawn enabled
	\return bool m_bEnableRespawn
	*/
	bool IsRespawnEnabled()
	{
		return m_bEnableRespawn;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns whether faction change is allowed by the game mode.
		\return True if allowed.
	*/
	bool IsFactionChangeAllowed()
	{
		return m_pGameMode.IsFactionChangeAllowed();
	}

	/*!
	Get on respawn changed script invoker
	\return ScriptInvoker Event_OnRespawnEnabledChanged which is called when server enables or disables respawn
	*/
	ScriptInvoker GetOnRespawnEnabledChanged()
	{
		return Event_OnRespawnEnabledChanged;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority only:
			Whenever a SCR_SpawnHandlerComponent receives a request from SCR_SpawnRequestComponent that needs to
			verify whether a player can spawn in addition to the SCR_SpawnHandlerComponent logic (per-case logic),
			this method is called to allow handling logic on a global scale.

			\param requestComponent The player request component (instigator).
			\param handlerComponent The handler that passes the event to this manager.
			\param data The data passed from the request
			\param[out] result Reason why respawn is disabled. Note that if returns true the reason will always be OK

			\return true If request is allowed, false otherwise.
	*/
	bool CanRequestSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::CanRequestSpawn_S(playerId: %2, handler: %2, data: %3)", Type().ToString(),
					requestComponent.GetPlayerId(),
					handlerComponent,
					data);
		#endif

		if (!m_bEnableRespawn)
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_SPAWNING_DISABLED;
			return false;
		}
			

		return m_pGameMode.CanPlayerSpawn_S(requestComponent, handlerComponent, data, result);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority only:
			During the spawn process (after validation pass), the SCR_SpawnHandlerComponent can opt to prepare
			spawned entity. This process first happens on affiliated SCR_SpawnHandlerComponent and if it succeeds,
			it additionally raises this method, which can prepare entity on a global scale. (E.g. game mode logic)
			Preparation can still fail (e.g. desire to seat a character, but an error occurs) and by returning false
			the sender is informed of such failure and can respond accordingly.
			\param requestComponent Instigator of the request.
			\param handlerComponent Handler that processed the request.
			\param data The payload of the request.
			\param entity Spawned (or generally assigned) entity to be prepared.
			\return True on success (continue to next step), fail on failure (terminate spawn process).
	*/
	bool PreparePlayerEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		return m_pGameMode.PreparePlayerEntity_S(requestComponent, handlerComponent, data, entity);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority only:
			During the spawn process the SCR_SpawnHandlerComponent can opt to handle changes of previous (and next) controlled
			(or newly spawned) entity for the given player. Such process additionally raises this method, which can handle
			entity changes on a global scale. (E.g. game mode logic).
			\param requestComponent Instigator of the request.
			\param handlerComponent Handler that processed the request.
			\param previousEntity Previously controlled entity. (May be null)
			\param newEntity Entity to be controlled.
			\param data The payload of the request.
	*/
	void OnPlayerEntityChange_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity previousEntity, IEntity newEntity, SCR_SpawnData data)
	{
		m_pGameMode.OnPlayerEntityChanged_S(requestComponent.GetPlayerId(), previousEntity, newEntity);
		m_SpawnLogic.OnPlayerEntityChanged_S(requestComponent.GetPlayerId(), previousEntity, newEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Authority only:
			Whenever a request to spawn is denied by the authority, this callback is raised.			
	*/
	void OnSpawnPlayerEntityFailure_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity entity, SCR_SpawnData data, SCR_ESpawnResult reason)
	{
		m_pGameMode.OnSpawnPlayerEntityFailure_S(requestComponent, handlerComponent, entity, data, reason);		
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority only:
			Whenever a SCR_SpawnHandlerComponent processes a spawn request and finished the finalization stage
			(awaits finalization, passes control to client) this method is called. This is the final step in the respawn
			process and after this point the owner of SCR_SpawnRequestComponent is spawned.
			\param requestComponent Instigator of the request.
			\param handlerComponent Handler that processed the request.
			\param data The payload of the request.
			\param entity Spawned (or generally assigned) entity.
	*/
	void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		m_pGameMode.OnPlayerSpawnFinalize_S(requestComponent, handlerComponent, data, entity);
		m_SpawnLogic.OnPlayerSpawned_S(requestComponent.GetPlayerId(), entity);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerRegistered_S(int playerId)
	{
		m_SpawnLogic.OnPlayerRegistered_S(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected_S(int playerId, KickCauseCode cause, int timeout)
	{
		m_SpawnLogic.OnPlayerDisconnected_S(playerId, cause, timeout);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerKilled_S(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		m_SpawnLogic.OnPlayerKilled_S(playerId, playerEntity, killerEntity, killer);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerDeleted_S(int playerId)
	{
		m_SpawnLogic.OnPlayerDeleted_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	float GetDeployMenuOpenDelay_ms()
	{
		return m_fDeployMenuOpenDelay * 1000;
	}

	//------------------------------------------------------------------------------------------------
	bool CanOpenDeployMenu()
	{
		return m_SpawnLogic && m_SpawnLogic.Type() == SCR_MenuSpawnLogic;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SpawnLogic GetSpawnLogic()
	{
		return m_SpawnLogic;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		m_SpawnPointRequestResultInfoHolder = SCR_ConfigHelperT<SCR_SpawnPointRequestResultInfoConfig>.GetConfigObject(m_sSpawnPointRequestResultInfoHolder);
		if (!m_SpawnPointRequestResultInfoHolder)
			Print("'SCR_RespawnSystemComponent' has no valid m_SpawnPointRequestResultInfoHolder! This means the disabled reason cannot be disabled spawn point!", LogLevel.ERROR);
		
		m_pGameMode = SCR_BaseGameMode.Cast(owner);
		if (!m_pGameMode)
			Print("SCR_RespawnSystemComponent has to be attached to a SCR_BaseGameMode (or inherited) entity!", LogLevel.ERROR);
		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (!m_SpawnLogic)
			Print("SCR_RespawnSystemComponent is missing SCR_SpawnLogic!", LogLevel.ERROR);
		

		if (GetGame().InPlayMode())
		{
			m_SpawnLogic.OnInit(this);
			
			// Validate faction manager
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (!factionManager)
			{
				string text = string.Format("No %1 found in the world, %2 might not work as intended!",
						SCR_FactionManager, SCR_RespawnSystemComponent);
				Print(text, LogLevel.WARNING);
			}

			// Validate loadout manager
			SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
			if (!loadoutManager)
			{
				string text = string.Format("No %1 found in the world, %2 might not work as intended!",
						SCR_LoadoutManager, SCR_RespawnSystemComponent);
				Print(text, LogLevel.WARNING);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override event bool OnRplSave(ScriptBitWriter w)
	{
		w.WriteBool(m_bEnableRespawn);

		return super.OnRplSave(w);
	}

	//------------------------------------------------------------------------------------------------
	protected override event bool OnRplLoad(ScriptBitReader r)
	{
		bool enableRespawn;
		r.ReadBool(enableRespawn);

		SetEnableRespawnBroadcast(enableRespawn);

		return super.OnRplLoad(r);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RespawnSystemComponent()
	{
		s_Instance = null;
		m_pRplComponent = null;
	}
};
