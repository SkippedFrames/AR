//------------------------------------------------------------------------------------------------
//! Data class for reconnecting players
class SCR_ReconnectData
{
	int m_iPlayerId;	
	IEntity m_ReservedEntity;	// entity of the returning player
	
	//------------------------------------------------------------------------------------------------
	void SCR_ReconnectData(int playerId, IEntity entity)
	{
		m_iPlayerId = playerId;
		m_ReservedEntity = entity;
	}
};

//------------------------------------------------------------------------------------------------
//! State of a reconnecting player
enum SCR_EReconnectState
{
	NOT_RECONNECT,
	ENTITY_AVAILABLE,
	ENTITY_DISCARDED
};

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_ReconnectComponentClass : SCR_BaseGameModeComponentClass
{
	//------------------------------------------------------------------------------------------------
	static override bool DependsOn(string className)
	{
		if (className == "RplComponentClass")
			return true;
		
		return false;
	}
};

//------------------------------------------------------------------------------------------------
//! Takes care of managing player reconnects in case of involuntary disconnect
//! Authority only component attached to gamemode prefab
class SCR_ReconnectComponent : SCR_BaseGameModeComponent
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable reconnect functionality for this gamemode")]
	bool m_bEnableReconnect;
	
	static SCR_ReconnectComponent s_Instance;
	
	protected bool m_bIsInit;			// whether check for connection to backend happenned 
	protected bool m_bIsReconEnabled;	
	protected ref array<ref SCR_ReconnectData> m_ReconnectPlayerList = new array<ref SCR_ReconnectData>();
	
	protected ref ScriptInvoker<SCR_ReconnectData> m_OnAddedToReconnectList = new ScriptInvoker();
	protected ref ScriptInvoker<SCR_ReconnectData> m_OnPlayerReconnect = new ScriptInvoker();  
	
	//------------------------------------------------------------------------------------------------
	static SCR_ReconnectComponent GetInstance() 
	{ 
		return s_Instance; 
	}
		
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAddedToList()
	{ 
		return m_OnAddedToReconnectList; 
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnReconnect() 
	{ 
		if (!m_OnPlayerReconnect)
			m_OnPlayerReconnect = new ScriptInvoker();  
		
		return m_OnPlayerReconnect; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is reconnect list empty
	bool IsReconnectListEmpty()
	{
		return m_ReconnectPlayerList.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is reconnect functionality enabled
	bool IsReconnectEnabled() 
	{ 
		return m_bIsReconEnabled; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is subject playerID currently present in list of possible reconnects
	//! \param playerId is the subject
	//! \return state of the reconnecting subject
	SCR_EReconnectState IsInReconnectList(int playerId)
	{
		if (!m_bIsInit)
		{
			if (!Init())
				return SCR_EReconnectState.NOT_RECONNECT;
		}
		
		if (m_ReconnectPlayerList.IsEmpty())
			return SCR_EReconnectState.NOT_RECONNECT;
		
		int count = m_ReconnectPlayerList.Count();
		for (int i; i < count; i++)
		{
			if (m_ReconnectPlayerList[i].m_iPlayerId == playerId)
			{
				ChimeraCharacter char = ChimeraCharacter.Cast(m_ReconnectPlayerList[i].m_ReservedEntity);
				if (!char || char.GetCharacterController().IsDead())	// entity could have died meanwhile
				{
					m_ReconnectPlayerList.Remove(i);
					return SCR_EReconnectState.ENTITY_DISCARDED;
				}
				
				return SCR_EReconnectState.ENTITY_AVAILABLE;
			}
		}
		
		return SCR_EReconnectState.NOT_RECONNECT;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is subject entity is currently present in list of possible reconnects
	//! \param IEntity is the subject
	//! \return state of the reconnecting subject
	SCR_EReconnectState IsEntityReconnectList(IEntity entity)
	{
		if (!m_bIsInit)
		{
			if (!Init())
				return SCR_EReconnectState.NOT_RECONNECT;
		}
		
		if (m_ReconnectPlayerList.IsEmpty())
			return SCR_EReconnectState.NOT_RECONNECT;
		
		int count = m_ReconnectPlayerList.Count();
		for (int i; i < count; i++)
		{
			if (m_ReconnectPlayerList[i].m_ReservedEntity == entity)
			{
				ChimeraCharacter char = ChimeraCharacter.Cast(m_ReconnectPlayerList[i].m_ReservedEntity);
				if (!char || char.GetCharacterController().IsDead())	// entity could have died meanwhile
				{
					m_ReconnectPlayerList.Remove(i);
					return SCR_EReconnectState.ENTITY_DISCARDED;
				}
				
				return SCR_EReconnectState.ENTITY_AVAILABLE;
			}
		}
		
		return SCR_EReconnectState.NOT_RECONNECT;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return control of the entity subject controlled before disconnect
	//! \param playerId is the subject
	IEntity ReturnControlledEntity(int playerId)
	{		
		int count = m_ReconnectPlayerList.Count();
		for (int i; i < count; i++)
		{
			if (m_ReconnectPlayerList[i].m_iPlayerId == playerId)
			{
				IEntity ent = m_ReconnectPlayerList[i].m_ReservedEntity;
				PlayerManager playerManager = GetGame().GetPlayerManager();
				SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));		
				playerController.SetInitialMainEntity(ent);
	
				m_OnPlayerReconnect.Invoke(m_ReconnectPlayerList[i]);
				
				m_ReconnectPlayerList.Remove(i);
				return ent;
			}
		}
		
		return null;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Propagated from SCR_BaseGameMode OnPlayerDisconnected event
	//! \return true if this disconnect is saved as eligible for reconnect
	bool OnPlayerDC(int playerId, KickCauseCode cause)
	{
		KickCauseGroup2 groupInt = KickCauseCodeAPI.GetGroup(cause);
		int reasonInt = KickCauseCodeAPI.GetReason(cause);
				
		if (groupInt != RplKickCauseGroup.REPLICATION)
			return false;
		else if (reasonInt == RplError.SHUTDOWN)
			return false;
				
		bool addEntry = true;
		
		if (!m_ReconnectPlayerList.IsEmpty())
		{
			int count = m_ReconnectPlayerList.Count();
			for (int i; i < count; i++)
			{
				if (m_ReconnectPlayerList[i].m_iPlayerId == playerId)
				{
					addEntry = false;
					break;
				}
			}
		}
		
		if (addEntry)
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!ent)
				return false;
			
			SCR_ReconnectData newEntry = new SCR_ReconnectData(playerId, ent);
			m_ReconnectPlayerList.Insert(newEntry);
			m_OnAddedToReconnectList.Invoke(newEntry);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether theere is a proper connection to backend
	bool Init()
	{
		m_bIsInit = true;
			
		BackendApi backendApi = GetGame().GetBackendApi();
		if (!backendApi || !backendApi.IsActive() || (!backendApi.IsInitializing() && !backendApi.IsRunning()))
		{
			m_bIsReconEnabled = false;	// not connected to backend
			Deactivate(GetOwner());
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	override protected void OnPlayerAuditTimeouted(int playerId)
	{
		if (m_ReconnectPlayerList.IsEmpty())
			return;
		
		int count = m_ReconnectPlayerList.Count();
		for (int i; i < count; i++)
		{
			if (m_ReconnectPlayerList[i].m_iPlayerId == playerId)
			{
				RplComponent.DeleteRplEntity(m_ReconnectPlayerList[i].m_ReservedEntity, false);
				m_ReconnectPlayerList.Remove(i);
				return;
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{		
		RplComponent rplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rplComp.IsProxy() && m_bEnableReconnect)		// ends here if not authority
			SetEventMask(owner, EntityEvent.INIT);
		else 
			m_bIsReconEnabled = false;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		s_Instance = this;
		m_bIsReconEnabled = true;
		
		auto game = GetGame();
		if (game && !game.InPlayMode())
			return;
		
		SCR_BaseGameMode.Cast(game.GetGameMode()).GetOnPlayerAuditTimeouted().Insert(OnPlayerAuditTimeouted);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_ReconnectComponent()
	{
		if (SCR_BaseGameMode.Cast(GetGame().GetGameMode()))
		{
			SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetOnPlayerAuditTimeouted().Remove(OnPlayerAuditTimeouted);
		}
			
		s_Instance = null;
	}
};
