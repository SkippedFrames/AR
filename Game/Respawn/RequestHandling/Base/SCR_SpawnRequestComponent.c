//------------------------------------------------------------------------------------------------
class SCR_SpawnRequestComponentClass : ScriptComponentClass
{
	static override bool DependsOn(string className)
	{		
		if (className == "SCR_RespawnComponentClass")
			return true;
		
		return false;
	}
};

//------------------------------------------------------------------------------------------------
/*!
	SCR_SpawnRequestComponent <-> SCR_SpawnHandlerComponent

	SCR_SpawnRequestComponent allows communication between client and the authority,
	SCR_SpawnHandlerComponent handles the requests on authority as desired.

	The respawn request component is the client (but can be issued by authority) component of the respawn process which
	sends respawn requests to be process by the authority.
	The process is further streamlined via the usage of a unified parent manager, the SCR_RespawnComponent.
*/
class SCR_SpawnRequestComponent : ScriptComponent
{
	private SCR_PlayerController m_PlayerController;
	private SCR_RespawnComponent m_RespawnComponent;
	private SCR_SpawnLockComponent m_LockComponent;
	private RplComponent m_RplComponent;

	private SCR_SpawnHandlerComponent m_HandlerComponent;
	private bool m_bIsPreloading;

	PlayerController GetPlayerController()
	{
		return m_PlayerController;
	}
	
	int GetPlayerId()
	{
		return m_PlayerController.GetPlayerId();
	}

	protected SCR_RespawnComponent GetRespawnComponent()
	{
		return m_RespawnComponent;
	}

	SCR_SpawnHandlerComponent GetHandlerComponent()
	{
		return m_HandlerComponent;
	}

	/*!
		Returns the lock component on owner PlayerController (if any) that is used to lock
		requests until they are resolved.
	*/
	protected SCR_SpawnLockComponent GetLock()
	{
		return m_LockComponent;
	}

	/*!
		When we ask the server for spawning eligibility, to prevent uneccessary transfers and unnecessary boilerplate,
		we store the current pending data until a response is received. The data is validated when sent during actual
		respawn request anyway, so in worst case scenario, we receive a response that spawn data is invalid.
	*/
	private ref SCR_SpawnData m_ConfirmationPendingData;

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the type of THandlerComponent component this TRequestComponent is linked to.
		Each request sent through a TRequestComponent is always processed via a THandlerComponent.
	*/
	typename GetHandlerType()
	{
		return SCR_SpawnHandlerComponent;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the type of TData this TRequestComponent is linked to.
		Each request sent through a TRequestComponent accepts and handles a TData payload.
	*/
	typename GetDataType()
	{
		return SCR_SpawnData;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsOwner()
	{
		return !m_RplComponent || m_RplComponent.IsOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return m_RplComponent && m_RplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	/*
		Initializes the component by finding necessary dependencies.
	*/
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		m_PlayerController = SCR_PlayerController.Cast(owner);
		if (!m_PlayerController)
		{
			Print(string.Format("%1 is not attached in %2 hierarchy! (%1 should be a child of %3!)",
				Type().ToString(), SCR_PlayerController, SCR_RespawnComponent),
				LogLevel.ERROR);
		}
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent)
		{
			Print(string.Format("%1 could not find %2!",
				Type().ToString(), RplComponent),
				LogLevel.ERROR);
		}

		m_RespawnComponent = SCR_RespawnComponent.Cast(owner.FindComponent(SCR_RespawnComponent));
		if (!m_RespawnComponent)
		{
			Print(string.Format("%1 could not find %2!",
				Type().ToString(), SCR_RespawnComponent),
				LogLevel.ERROR);
		}

		m_LockComponent = SCR_SpawnLockComponent.Cast(owner.FindComponent(SCR_SpawnLockComponent));
		if (!m_LockComponent)
		{
			Print(string.Format("%1 could not find %2!",
				Type().ToString(), SCR_SpawnLockComponent),
				LogLevel.ERROR);
		}

		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystem)
		{
			Print(string.Format("%1 could not find %2!",
				Type().ToString(), SCR_RespawnSystemComponent),
				LogLevel.ERROR);
		}

		m_HandlerComponent = SCR_SpawnHandlerComponent.Cast(respawnSystem.FindComponent(GetHandlerType()));
		if (!m_HandlerComponent)
		{
			Print(string.Format("%1 could not find %2!",
				Type().ToString(), GetHandlerType().ToString()),
				LogLevel.ERROR);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Sends an ask request to the authority to find out whether spawn with provided data is valid.
		Returned value does NOT correspond to the actual response. For reponse see available events.
		\return Returns true if the request was sent from this client properly, false otherwise.
	*/
	sealed bool CanRequestRespawn(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::CanRequestRespawn(data: %2)", Type().ToString(), data);
		#endif

		SCR_SpawnLockComponent lock = GetLock();
		if (lock && !lock.TryLock(this, false))
		{
			//Debug.Error("Caught request on locked player!"); //~ No need to error as it simply returns on locked. As requests can be made in various situations
			return false;
		}
		
		// Notify that request began
		if (IsOwner())
			m_RespawnComponent.GetOnCanRespawnRequestInvoker_O().Invoke(this, data);
		if (!IsProxy())
			m_RespawnComponent.GetOnCanRespawnRequestInvoker_S().Invoke(this, data);

		// Now that we ensured that there will be no more outgoing requests,
		// store the user constructed data to be passed onto response later on
		m_ConfirmationPendingData = data;

		// Send the ask request
		bool success = DoCanRequestRespawn(m_ConfirmationPendingData);

		// Notify self on local failure, early-reject.
		if (!success)
			Rpc_SendCanResponse_O(SCR_ESpawnResult.BAD_REQUEST);

		return success;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Request is first handled on the sending side (player or authority depending on context).
		This method can return false and will immediately raise a SCR_ESpawnResult.BAD_REQUEST response.
		\return True in case request was sucessfully dispatched, false otherwise.
	*/
	protected bool DoCanRequestRespawn(SCR_SpawnData data)
	{
		Debug.Error("Not implemented!");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority call to dispatch provided data to corresponding SCR_SpawnHandlerComponent.
		Handles responses when handler is not found or on request success internally.
	*/
	protected void ProcessCanRequest_S(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::ProcessCanRequest_S(data: %2)", Type().ToString(), data);
		#endif

		// Server lock
		SCR_SpawnLockComponent lock = GetLock();
		if (lock && !lock.TryLock(this, true))
		{
			Debug.Error("Caught request on locked player!");
			return;
		}

		SCR_RespawnComponent respawnComponent = GetRespawnComponent();
		respawnComponent.GetOnCanRespawnRequestInvoker_S().Invoke(this, data);

		// Find handler
		SCR_SpawnHandlerComponent handler = GetHandlerComponent();
		if (!handler)
		{
			Print("GameMode does not support this method of spawning!", LogLevel.WARNING);
			SendCanResponse_S(SCR_ESpawnResult.UNSUPPORTED_SPAWN_METHOD, data);
			return;
		}

		// Dispatch message
		SCR_ESpawnResult response = handler.CanHandleRequest_S(this, data);
		SendCanResponse_S(response, data);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority:
			Sends a response from the server to the owner about current can-respawn request.
			\param response The result of the request.
			\param data The data the request was instigated with.
	*/
	protected void SendCanResponse_S(SCR_ESpawnResult response, SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::SendCanResponse_S(resp: %2, data: %3)", Type().ToString(), typename.EnumToString(SCR_ESpawnResult, response), data);
		#endif

		// End server request
		SCR_SpawnLockComponent lock = GetLock();
		if (lock)
		{
			lock.Unlock(this, true);
			lock.Unlock(this, false);
		}
		
		// Notify authority
		SCR_RespawnComponent respawnComponent = GetRespawnComponent();
		respawnComponent.GetOnCanRespawnResponseInvoker_S().Invoke(this, response, data);

		// Send client response
		Rpc(Rpc_SendCanResponse_O, response);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Owner:
			Processes received response about current spawn process from the authority, dispatches events.
		\param response The result of the respawning action.
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Rpc_SendCanResponse_O(SCR_ESpawnResult response)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::Rpc_SendCanResponse_O(resp: %2)", Type().ToString(), typename.EnumToString(SCR_ESpawnResult, response));
		#endif
		
		// End client request
		SCR_SpawnLockComponent lock = GetLock();
		if (lock)
		{
			lock.Unlock(this, false);
		}
		
		// Notify
		SCR_RespawnComponent respawnComponent = GetRespawnComponent();		
		respawnComponent.GetOnCanRespawnResponseInvoker_O().Invoke(this, response, m_ConfirmationPendingData);

		// Release stored data
		m_ConfirmationPendingData = null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Requests respawn with provided data.
		! Note that even successful request can still be denied by the authority!
		\return True in case success was sent from the client, false otherwise (early reject).
	*/
	sealed bool RequestRespawn(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::RequestRespawn(data: %2)", Type().ToString(), data);
		#endif

		// Lock client
		SCR_SpawnLockComponent lock = GetLock();
		if (lock && !lock.TryLock(this, false))
		{
			Print("Caught request spawn on locked player!", LogLevel.WARNING);
			return false;
		}
		
		// Notify that request began
		if (IsOwner())
			m_RespawnComponent.GetOnRespawnRequestInvoker_O().Invoke(this);
		if (!IsProxy())
			m_RespawnComponent.GetOnRespawnRequestInvoker_S().Invoke(this);

		// Request
		bool success = DoRequestRespawn(data);

		// Notify self on local failure, early-reject.
		if (!success)
			Rpc_SendResponse_O(SCR_ESpawnResult.BAD_REQUEST);

		return success;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Request is first handled on the sending side (player or authority depending on context).
		This method can return false and will immediately raise a SCR_ESpawnResult.BAD_REQUEST response.
		\return True in case request was sucessfully dispatched, false otherwise.
	*/
	protected bool DoRequestRespawn(SCR_SpawnData data)
	{
		Debug.Error("Not implemented!");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority call to dispatch provided data to corresponding SCR_SpawnHandlerComponent.
		Handles responses when handler is not found or on request success internally.
	*/
	protected void ProcessRequest_S(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::ProcessRequest_S(data: %2)", Type().ToString(), data);
		#endif

		// Server lock
		SCR_SpawnLockComponent lock = GetLock();
		if (lock && !lock.TryLock(this, true))
		{
			Debug.Error("Caught request on locked player!");
			return;
		}

		// Find handler
		SCR_SpawnHandlerComponent handler = GetHandlerComponent();
		if (!handler)
		{
			Print("GameMode does not support this method of spawning!", LogLevel.WARNING);
			SendResponse_S(SCR_ESpawnResult.UNSUPPORTED_SPAWN_METHOD, data);
			return;
		}

		// Dispatch message
		IEntity spawnedEntity;
		SCR_ESpawnResult response = handler.HandleRequest_S(this, data, spawnedEntity);

		// Entity was not spawned correctly, notify the requester and no finalization
		if (response != SCR_ESpawnResult.OK)
		{
			SendResponse_S(response, data);
			return;
		}
		
		// For entities that can technically be controlled by an AI agent, notify the agent
		// that such entity is the result of a spawning process and is pending for a player
		SCR_ChimeraAIAgent agent = FindAIAgent(spawnedEntity);
		if (agent)
			agent.SetPlayerPending_S(m_PlayerController.GetPlayerId());

		// Editor needs to be aware of whether the pending character is for a player or not in order to update the Editor Budget during spawning.
		// TODO: Remove this after the Editor's Respawn Sytem refactor.
		SCR_EditableCharacterComponent editorCharacter = SCR_EditableCharacterComponent.Cast(spawnedEntity.FindComponent(SCR_EditableCharacterComponent));
		if (editorCharacter)
			editorCharacter.SetIsPlayerPending(m_PlayerController.GetPlayerId());

		// Entity was spawned, so we can await finalization.
		SendFinalizationBegin_S();
		OnFinalizeBegin_S(handler, data, spawnedEntity);
		GetGame().GetCallqueue().CallLater(AwaitFinalization_S, 0, true, handler, data, spawnedEntity);
	}

	//------------------------------------------------------------------------------------------------
	/*
		Authority callback when finalization of spawn has began.
	*/
	protected void OnFinalizeBegin_S(SCR_SpawnHandlerComponent handler, SCR_SpawnData data, IEntity spawnedEntity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnFinalizeBegin_S(handler: %1, data: %2, entity: %3)", Type().ToString(),
					handler, data, spawnedEntity);
		#endif
		handler.OnFinalizeBegin_S(this, data, spawnedEntity);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority callback to await finalization.
	*/
	protected void AwaitFinalization_S(SCR_SpawnHandlerComponent handler, SCR_SpawnData data, IEntity spawnedEntity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::AwaitFinalization_S(handler: %2, data: %3, entity: %4)", Type().ToString(),
					handler, data, spawnedEntity);
		#endif

		if (CanFinalize_S(handler, data, spawnedEntity))
		{
			FinalizeRequest_S(handler, data, spawnedEntity);
			GetGame().GetCallqueue().Remove(AwaitFinalization_S);
			
			// For entities that were previously marked as pending player
			// spawn, clear their state as the process is done
			SCR_ChimeraAIAgent agent = FindAIAgent(spawnedEntity);
			if (agent)
				agent.SetPlayerPending_S(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns whether finalization can occur.
	*/
	protected bool CanFinalize_S(SCR_SpawnHandlerComponent handler, SCR_SpawnData data, IEntity spawnedEntity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::CanFinalize_S(handler: %2, data: %3, entity: %4)", Type().ToString(),
					handler, data, spawnedEntity);
		#endif
		return handler.CanFinalize_S(this, data, spawnedEntity);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority call to finalize respawn, after ProcessRequest_S (AwaitFinalization_S respectrively) has finished.
		It is delayed to allow certain jobs (end of frame) to finish, before passing the ownership to the client.
	*/
	protected void FinalizeRequest_S(SCR_SpawnHandlerComponent handler, SCR_SpawnData data, IEntity spawnedEntity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::FinalizeRequest_S(handler: %1, data: %2, entity: %3)", Type().ToString(),
					handler, data, spawnedEntity);
		#endif
		SCR_ESpawnResult response = handler.FinalizeRequest_S(this, data, spawnedEntity);
		SendResponse_S(response, data);
		
		// Authority-side notification for systems to understand a spawn occurred
		if (response != SCR_ESpawnResult.OK)
			return;
		
		m_RespawnComponent.NotifySpawn(spawnedEntity);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority:
			Sends a response from the server to the owner about current respawn request.
		\param response The result of the respawning action.
		\param data The data the request was instigated with.
	*/
	protected void SendResponse_S(SCR_ESpawnResult response, SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::SendResponse_S(resp: %2, data: %3)", Type().ToString(), typename.EnumToString(SCR_ESpawnResult, response), data);
		#endif

		// End server request
		SCR_SpawnLockComponent lock = GetLock();
		if (lock)
		{
			lock.Unlock(this, true);
			lock.Unlock(this, false);
		}

		// Notify manager
		SCR_RespawnComponent respawnComponent = GetRespawnComponent();
		respawnComponent.GetOnRespawnResponseInvoker_S().Invoke(this, response, data);

		// Send client response
		Rpc(Rpc_SendResponse_O, response);
	}
	
	/*!
		Send a notification from the authority that the finalization has started.
	*/
	protected void SendFinalizationBegin_S()
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::SendFinalizationBegin_S()", Type().ToString());
		#endif
		
		Rpc(Rpc_OnFinalizationBegin_O);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Rpc_OnFinalizationBegin_O()
	{
		SCR_RespawnComponent respawnComponent = GetRespawnComponent();
		respawnComponent.GetOnRespawnFinalizeBeginInvoker_O().Invoke(this);
	}

	/*
		Preload handling
	*/
	bool IsPreloading()
	{
		return m_bIsPreloading;
	}

	void StartSpawnPreload(vector position)
	{
		NotifyPreloadStarted_S();
		if (m_PlayerController)		
			RplComponent.InsertMPObserver(m_PlayerController.GetRplIdentity(), position[0], position[2]);
		Rpc(Rpc_StartPreload_O, position);
	}

	protected void NotifyPreloadStarted_S()
	{
		m_bIsPreloading = true;
		Rpc(Rpc_NotifyPreloadStarted_S);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_NotifyPreloadStarted_S()
	{
		m_bIsPreloading = true;
	}

	protected void NotifyPreloadFinished_S()
	{
		m_bIsPreloading = false;
		Rpc(Rpc_NotifyPreloadFinished_S);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_NotifyPreloadFinished_S()
	{
		m_bIsPreloading = false;
		if (m_PlayerController)
			RplComponent.RemoveMPObserver(m_PlayerController.GetRplIdentity());
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Rpc_StartPreload_O(vector position)
	{
		SCR_BaseGameMode gm = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gm.GetOnPreloadFinished().Insert(NotifyPreloadFinished_S);
		gm.StartSpawnPreload(position);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Owner:
			Processes received response about current spawn process from the authority, dispatches events.
		\param response The result of the respawning action.
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Rpc_SendResponse_O(SCR_ESpawnResult response)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::Rpc_SendResponse_O(resp: %2)", Type().ToString(), typename.EnumToString(SCR_ESpawnResult, response));
		#endif

		// End client request
		SCR_SpawnLockComponent lock = GetLock();
		if (lock)
			lock.Unlock(this, false);
		
		// Notify client
		SCR_RespawnComponent respawnComponent = GetRespawnComponent();
		respawnComponent.GetOnRespawnResponseInvoker_O().Invoke(this, response, m_ConfirmationPendingData);
		SCR_RespawnComponent.SGetOnLocalPlayerSpawned().Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Tries to find AI agent for provided entity.
	*/
	protected SCR_ChimeraAIAgent FindAIAgent(IEntity entity)
	{
		if (entity)
		{
			AIControlComponent controlComponent = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
			if (controlComponent)
			{
				SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(controlComponent.GetControlAIAgent());
				return agent;
			}
		}
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Destructor.
	*/
	void ~SCR_SpawnRequestComponent()
	{
		if(m_PlayerController)
			RplComponent.RemoveMPObserver(m_PlayerController.GetRplIdentity());
	}

	#ifdef ENABLE_DIAG
	/*!
		Draws diagnostics for this respawn component.
	*/
	void DrawDiag()
	{
		if (!m_Diag)
			m_Diag = CreateDiag();
		if (m_Diag)
			m_Diag.DrawDbgUI(GetPlayerController());
	}
	protected ref SCR_BaseRespawnDiag m_Diag;
	protected ref SCR_BaseRespawnDiag CreateDiag()
	{
		return null;
	}
	#endif
};

#ifdef ENABLE_DIAG
/*!
	Base diagnostic utilities for respawn components.
*/
class SCR_BaseRespawnDiag
{
	void DrawDbgUI(PlayerController playerController);
};

/*!
	Diagnostic utility class for provided request component type.
*/
class SCR_RespawnDiag<Class TReqComponent> : SCR_BaseRespawnDiag
{
	protected TReqComponent m_RequestComponent;

	override void DrawDbgUI(PlayerController playerController)
	{
		if (m_RequestComponent == null)
			m_RequestComponent = TReqComponent.Cast(playerController.FindComponent(TReqComponent));

		if (!m_RequestComponent)
			return;

		int playerId = m_RequestComponent.GetPlayerId();
		string label = string.Format("%1 [playerId: %2, playerName: %3])", 
			m_RequestComponent.Type().ToString(),
			playerId,
			GetGame().GetPlayerManager().GetPlayerName(playerId)
		);

		DbgUI.Begin(label);
		{
			DrawContent();
		}
		DbgUI.End();
	}

	protected void DrawContent()
	{
		SCR_SpawnHandlerComponent handlerComponent = m_RequestComponent.GetHandlerComponent();
		if (handlerComponent != null)
			DbgUI.Text(string.Format("Handler: %1", handlerComponent));
		else
			DbgUI.Text("Handler: Not found!");

		DbgUI.Text(string.Format("Data: %1", m_RequestComponent.GetDataType()));

		if (DbgUI.Button("Can Request"))
			OnAskPressed();

		if (DbgUI.Button("Do Request"))
			OnRequestPressed();
	}

	protected void OnAskPressed()
	{
		SCR_SpawnData data = CreateData();
		m_RequestComponent.CanRequestRespawn(data);
	}

	protected void OnRequestPressed()
	{
		SCR_SpawnData data = CreateData();
		m_RequestComponent.RequestRespawn(data);
	}

	protected SCR_SpawnData CreateData()
	{
		Debug.Error("Not implemented");
		return null;
	}
};
#endif
