[EntityEditorProps(category: "GameScripted/Campaign", description: "This allows only vehicle requester enther the vehicle as driver for defined period of time.")]
class SCR_VehicleSpawnProtectionComponentClass : SCR_BaseLockComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_VehicleSpawnProtectionComponent : SCR_BaseLockComponent
{
	const int NO_OWNER = -1;
	const int NO_TIME_LIMIT = 0;
	
	[RplProp()]
	private int m_iVehicleOwnerID = NO_OWNER;
	
	protected int m_iTimeOfProtection;
	bool m_bOnlyDriverSeat = true;
	private SCR_CharacterControllerComponent m_CharControlComp;
	
	[Attribute(defvalue: "#AR-Campaign_Action_CannotEnterVehicle-UC", desc: "Text that will be displayed on actions telling reason why it cannot be used")];
	protected string m_sReasonText;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{			
		ClearEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ReleaseProtection()
	{
		RemoveEventHandlers();
		m_iVehicleOwnerID = NO_OWNER;
		Replication.BumpMe();
		
		GetGame().GetCallqueue().Remove(ReleaseProtection);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetProtectionTime(int protectionTime)
	{
		if (protectionTime != NO_TIME_LIMIT)
		{
			m_iTimeOfProtection = protectionTime;
			GetGame().GetCallqueue().CallLater(ReleaseProtection, m_iTimeOfProtection * 1000, false);
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVehicleOwner(int playerID)
	{
		// -2 == locked for everyone
		if (playerID == -2)
		{
			m_iVehicleOwnerID = playerID;		
			Replication.BumpMe();
			return;
		}
		
		IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!playerEnt)
			return;

		m_CharControlComp = SCR_CharacterControllerComponent.Cast(playerEnt.FindComponent(SCR_CharacterControllerComponent));
		if (!m_CharControlComp)
			return;
		
		// Add EH
		m_CharControlComp.GetOnPlayerDeath().Insert(ReleaseProtection);	
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconected);

		// Set owner of this vehicle
		m_iVehicleOwnerID = playerID;		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetProtectOnlyDriverSeat(bool onlyDriverSeat)
	{
		m_bOnlyDriverSeat = onlyDriverSeat;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetReasonText(string text)
	{
		m_sReasonText = text;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetVehicleOwner()
	{
		return m_iVehicleOwnerID;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsProtected(notnull IEntity playerEntering, notnull BaseCompartmentSlot compartmentSlot)
	{
		// Test if it is a driver seat or not.
		if (m_bOnlyDriverSeat && !PilotCompartmentSlot.Cast(compartmentSlot))
			return false;
				
		int testPlayerID = SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(playerEntering);
		// Check vehicle owenr ID
		if (m_iVehicleOwnerID == NO_OWNER || m_iVehicleOwnerID == testPlayerID /*m_Lobby.GetPlayerIdFromControlledEntity(playerEntering)*/)
		{
			#ifdef VEHICLE_LOCK
			PrintFormat("Player ID is %1, vehicle owner ID is %2. Vehicle is unlocked.",testPlayerID,m_iVehicleOwnerID);
			#endif
			return false;
		}
		#ifdef VEHICLE_LOCK
		PrintFormat("Player ID is %1, vehicle owner ID is %2. Vehicle is LOCKED.",testPlayerID,m_iVehicleOwnerID);
		#endif
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconected(int playerID)
	{
		if (playerID == m_iVehicleOwnerID)
			ReleaseProtection();
	}	
	//------------------------------------------------------------------------------------------------
	LocalizedString GetReasonText(IEntity user)
	{
		if (!user)
			return string.Empty;

		return m_sReasonText;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEventHandlers()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconected);
		
		if (m_CharControlComp)
			m_CharControlComp.GetOnPlayerDeath().Remove(ReleaseProtection);
	}
};
