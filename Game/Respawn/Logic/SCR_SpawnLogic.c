//------------------------------------------------------------------------------------------------
/*
	Authority:
		Object responsible for defining respawn logic.

		This object receives callbacks from parent SCR_RespawnSystemComponent that can be used
		to either spawn the player on the authority side or just notify the remote player that
		they can process to spawn, or any combination based on derived implementations.
*/
[BaseContainerProps(category: "Respawn")]
class SCR_SpawnLogic
{
	//------------------------------------------------------------------------------------------------
	void OnInit(SCR_RespawnSystemComponent owner)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerRegistered_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerRegistered_S(playerId: %2)", Type().ToString(), playerId);
		#endif

		SCR_RespawnComponent respawnComponent = GetPlayerRespawnComponent_S(playerId);
		respawnComponent.GetOnRespawnRequestInvoker_S().Insert(OnPlayerSpawnRequest_S);
		respawnComponent.GetOnRespawnResponseInvoker_S().Insert(OnPlayerSpawnResponse_S);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected_S(int playerId, KickCauseCode cause, int timeout)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerDisconnected_S(playerId: %2)", Type().ToString(), playerId);
		#endif

		SCR_RespawnComponent respawnComponent = GetPlayerRespawnComponent_S(playerId);
		respawnComponent.GetOnRespawnRequestInvoker_S().Remove(OnPlayerSpawnRequest_S);
		respawnComponent.GetOnRespawnResponseInvoker_S().Remove(OnPlayerSpawnResponse_S);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawnRequest_S(SCR_SpawnRequestComponent requestComponent)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerSpawnRequest_S(playerId: %2)", Type().ToString(), requestComponent.GetPlayerId());
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawnResponse_S(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerSpawnResponse_S(playerId: %2, response: %3)",
			Type().ToString(), requestComponent.GetPlayerId(), typename.EnumToString(SCR_ESpawnResult, response));
		#endif

		if (response != SCR_ESpawnResult.OK)
			OnPlayerSpawnFailed_S(requestComponent.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawnFailed_S(int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawned_S(int playerId, IEntity entity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerSpawned_S(playerId: %2, entity: %3)", Type().ToString(), playerId, entity);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerEntityChanged_S(int playerId, IEntity previousEntity, IEntity newEntity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerEntityChanged_S(playerId: %2, previousEntity: %3, newEntity: %4)",
			Type().ToString(), playerId, previousEntity, newEntity);
		#endif

		if (!newEntity)
			OnPlayerEntityLost_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerKilled_S(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerKilled_S(playerId: %2, playerEntity: %3, killerEntity: %4, killerId: %5)",
			Type().ToString(), playerId, playerEntity, killerEntity, killer.GetInstigatorPlayerID());
		#endif

		OnPlayerEntityLost_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDeleted_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerDeleted_S(playerId: %2)", Type().ToString(), playerId);
		#endif

		OnPlayerEntityLost_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called whenever provided player loses controlled entity, this can occur e.g.
		when a player dies or their entity is deleted.
	*/
	protected void OnPlayerEntityLost_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerEntityLost_S(playerId: %2)", Type().ToString(), playerId);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Notify the target player that they are ready for spawn. Useful for cases of manual spawning,
		e.g. when user should open respawn menu and similar.
	*/
	protected void NotifyPlayerReadyForSpawn(int playerId)
	{
		GetPlayerRespawnComponent_S(playerId).NotifyReadyForSpawn_S();
	}
	
	//------------------------------------------------------------------------------------------------
	/*
		Probe the SCR_ReconnectComponent for player of given playerId.
		If player is eligible for respawn using the reconnection method, true is returned.
	*/
	bool IsEligibleForReconnection(int playerId)
	{
		SCR_ReconnectComponent reconnectComponent = SCR_ReconnectComponent.GetInstance();
		if (!reconnectComponent || !reconnectComponent.IsReconnectEnabled())
			return false;
		
		SCR_EReconnectState recState = reconnectComponent.IsInReconnectList(playerId);
		if (recState == SCR_EReconnectState.NOT_RECONNECT)
			return false;
		
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (controller)
		{
			SCR_ReconnectSynchronizationComponent syncComp = SCR_ReconnectSynchronizationComponent.Cast(controller.FindComponent(SCR_ReconnectSynchronizationComponent));
			if (syncComp)
				syncComp.CreateReconnectDialog(recState);
		}
		
		return recState == SCR_EReconnectState.ENTITY_AVAILABLE;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Resolves spawn using the SCR_ReconnectComponent for player of given playerId.
		If such player is eligible for spawning this way, action is taken and true is
		returned on success (entity given over), false otherwise.
		\param playerId Player
		\param assignedEntity Returned entity if successful
	*/
	protected bool ResolveReconnection(int playerId, out IEntity assignedEntity)
	{
		if (!IsEligibleForReconnection(playerId))
			return false;
		
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::ResolveReconnection(playerId: %2)", Type().ToString(), playerId);
		#endif
			
		SCR_ReconnectComponent reconnectComponent = SCR_ReconnectComponent.GetInstance();
		assignedEntity = reconnectComponent.ReturnControlledEntity(playerId);
		return assignedEntity != null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_RespawnComponent GetPlayerRespawnComponent_S(int playerId)
	{
		return SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
	}

	//------------------------------------------------------------------------------------------------
	SCR_RespawnComponent GetLocalPlayerRespawnComponent()
	{
		return SCR_RespawnComponent.Cast(GetGame().GetPlayerController().GetRespawnComponent());
	}

	//------------------------------------------------------------------------------------------------
	SCR_PlayerFactionAffiliationComponent GetPlayerFactionComponent_S(int playerId)
	{
		return SCR_PlayerFactionAffiliationComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId).FindComponent(SCR_PlayerFactionAffiliationComponent));
	}

	//------------------------------------------------------------------------------------------------
	SCR_PlayerLoadoutComponent GetPlayerLoadoutComponent_S(int playerId)
	{
		return SCR_PlayerLoadoutComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId).FindComponent(SCR_PlayerLoadoutComponent));
	}
};
