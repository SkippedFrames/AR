//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "GameScripted/Respawn/Handlers", description: "Allows the respawn system to utilize spawning on SpawnPoint(s). Requires a SCR_SpawnPointRespawnComponent attached to PlayerController.")]
class SCR_SpawnPointSpawnHandlerComponentClass : SCR_SpawnHandlerComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_SpawnPointSpawnHandlerComponent : SCR_SpawnHandlerComponent
{
	protected SCR_PlayerSpawnPointManagerComponent m_PlayerSpawnPointManager;

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_PlayerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(owner.FindComponent(SCR_PlayerSpawnPointManagerComponent));
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Verifies provided data.
	*/
	protected override bool ValidateData_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data)
	{
		if (!super.ValidateData_S(requestComponent, data))
			return false;

		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		if (!spawnPointData)
			return false;

		// Spawn point must exist
		SCR_SpawnPoint spawnPoint = spawnPointData.GetSpawnPoint();
		if (!spawnPoint)
			return false;

		// If player spawn points are used, verify against such type
		if (m_PlayerSpawnPointManager)
		{
			// Of player spawn point type, but spawning of this kind is disallowed
			SCR_PlayerSpawnPoint playerSpawnPoint = SCR_PlayerSpawnPoint.Cast(spawnPoint);
			if (playerSpawnPoint && !m_PlayerSpawnPointManager.IsPlayerSpawnPointsEnabled())
				return false;
		}

		return true;
	}
	
	/*!
		Handles request, ensuring spawn points logic is valid.
	*/
	override SCR_ESpawnResult HandleRequest_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, out IEntity spawnedEntity)
	{
		// Handle request as usual
		SCR_ESpawnResult result = super.HandleRequest_S(requestComponent, data, spawnedEntity);
		if (result != SCR_ESpawnResult.OK)
		{
			// But in case of failure, clear pending reservation
			SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
			SCR_SpawnPoint spawnPoint = spawnPointData.GetSpawnPoint();
			if (spawnPoint)
				spawnPoint.ClearReservationFor_S(requestComponent.GetPlayerId());
		}
		
		return result;
	}

	/*!
		Authority side check for whether provided request can be processed on provided spawn point.
	*/
	protected override bool CanRequestSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, out SCR_ESpawnResult result)
	{
		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		SCR_SpawnPoint spawnPoint = spawnPointData.GetSpawnPoint();
		if (!spawnPoint)
		{
			result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED;
			return false;
		}
			
		
		// Cannot reserve = will not be able to spawn
		if (!spawnPoint.CanReserveFor_S(requestComponent.GetPlayerId(), result))
		{
			//result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED;
			return false;
		}
			

		return super.CanRequestSpawn_S(requestComponent, data, result);
	}

	/*!
		Prepare an entity on the server side prior to passing ownership.
		For example in this case character can have items added, can be seated in vehicle, etc.
	*/
	protected override bool PrepareEntity_S(SCR_SpawnRequestComponent requestComponent, IEntity entity, SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::PrepareEntity_S(playerId: %2, entity: %3, data: %4)", Type().ToString(),
					requestComponent.GetPlayerController().GetPlayerId(),
					entity,
					data);
		#endif

		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		SCR_SpawnPoint spawnPoint = spawnPointData.GetSpawnPoint();
		if (!spawnPoint.PrepareSpawnedEntity_S(requestComponent, data, entity))
			return false;

		return super.PrepareEntity_S(requestComponent, entity, data);
	}

	/*!
		Handle locking of spawn point and spawn the entity.
	*/
	protected override SCR_ESpawnResult SpawnEntity_S(SCR_SpawnRequestComponent requestComponent, notnull SCR_SpawnData data, out IEntity spawnedEntity)
	{
		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		SCR_SpawnPoint spawnPoint = spawnPointData.GetSpawnPoint();
		int playerId = requestComponent.GetPlayerId();

		// Lock spawn point
		if (!spawnPoint.ReserveFor_S(playerId))
			return SCR_ESpawnResult.SPAWN_NOT_ALLOWED;
		
		// Process spawn
		SCR_ESpawnResult result = super.SpawnEntity_S(requestComponent, data, spawnedEntity);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called periodically to ask whether finalization can be finished.
		\return True to finalize request, resulting in FinalizeRequest_S call, false to await further.
	*/
	override bool CanFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		SCR_SpawnPoint spawnPoint = spawnPointData.GetSpawnPoint();
		if (!super.CanFinalize_S(requestComponent, data, entity))
			return false;

		return spawnPoint.CanFinalizeSpawn_S(requestComponent, data, entity);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called when request is finalized.
		\return True to finalize request.
	*/
	override void OnFinalizeDone_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		SCR_SpawnPoint spawnPoint = spawnPointData.GetSpawnPoint();
		spawnPoint.OnFinalizeSpawnDone_S(requestComponent, data, entity);
		
		// Clear reservation on spawn complete
		spawnPoint.ClearReservationFor_S(requestComponent.GetPlayerId());
	}
};
