class SCR_ChimeraCharacterClass : ChimeraCharacterClass
{
}

class SCR_ChimeraCharacter : ChimeraCharacter
{
	[NonSerialized()]
	float m_fFaceAlphaTest = 0; // Used to fade away the head when getting close with the 3rd person camera

	[NonSerialized()]
	FactionAffiliationComponent m_pFactionComponent;

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_pFactionComponent = FactionAffiliationComponent.Cast(FindComponent(FactionAffiliationComponent))
	}

	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		if (m_pFactionComponent)
			return m_pFactionComponent.GetAffiliatedFaction();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	string GetFactionKey()
	{
		Faction faction = GetFaction();
		if (!faction)
			return string.Empty;

		return faction.GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	//! Using RPC here because it is only for sound, so we don't care when weapon is streamed in.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetIllumination_BC(bool state, RplId rplId)
	{
		WeaponComponent wpnComp = WeaponComponent.Cast(Replication.FindItem(rplId));
		if (wpnComp)
		{
			IEntity wpnEntity = wpnComp.GetOwner();
			if (wpnEntity)
			{
				WeaponSoundComponent wpnSoundComp = WeaponSoundComponent.Cast(wpnEntity.FindComponent(WeaponSoundComponent));
				if (wpnSoundComp)
				{
					if (state)
						wpnSoundComp.SoundEvent(SCR_SoundEvent.SOUND_SCOPE_ILLUM_ON);
					else
						wpnSoundComp.SoundEvent(SCR_SoundEvent.SOUND_SCOPE_ILLUM_OFF);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_SetIllumination_S(bool state, RplId rplId)
	{
		RPC_SetIllumination_BC(state, rplId);
		//Broadcast to everybody
		Rpc(RPC_SetIllumination_BC, state, rplId);
	}

	//------------------------------------------------------------------------------------------------
	void SetIllumination(bool illuminated, RplId rplId)
	{
		//Ask the server to broadcast to everybody.
		Rpc(RPC_SetIllumination_S, illuminated, rplId);
	}

	//------------------------------------------------------------------------------------------------
	//! Using RPC here because it is only for sound, so we don't care when weapon is streamed in.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetNewZoomLevel_BC(int zoomLevel, bool increased, RplId rplId)
	{
		WeaponComponent wpnComp = WeaponComponent.Cast(Replication.FindItem(rplId));
		if (wpnComp)
		{
			IEntity wpnEntity = wpnComp.GetOwner();
			if (wpnEntity)
			{
				WeaponSoundComponent wpnSoundComp = WeaponSoundComponent.Cast(wpnEntity.FindComponent(WeaponSoundComponent));
				if (wpnSoundComp)
				{
					if (increased)
						wpnSoundComp.SoundEvent(SCR_SoundEvent.SOUND_SCOPE_ZOOM_IN);
					else
						wpnSoundComp.SoundEvent(SCR_SoundEvent.SOUND_SCOPE_ZOOM_OUT);
				}
			}
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_SetNewZoomLevel_S(int zoomLevel, bool increased, RplId rplId)
	{
		RPC_SetNewZoomLevel_BC(zoomLevel, increased, rplId);
		//Broadcast to everybody
		Rpc(RPC_SetNewZoomLevel_BC, zoomLevel, increased, rplId);
	}

	void SetNewZoomLevel(int zoomLevel, bool increased, RplId rplId)
	{
		Rpc(RPC_SetNewZoomLevel_S, zoomLevel, increased, rplId);
	}

	//------------------------------------------------------------------------------------------------	
	/*!
	Returns true if character is in driver compartment, optionally can check if vehicle is moving fast enough
	\param minSpeedSq Minimum speed squared [m/s]
	*/
	bool IsDriving(float minSpeedSq = -1)
	{
		if (!IsInVehicle())
			return false;

		CompartmentAccessComponent access = GetCompartmentAccessComponent();
		if (!access)
			return false;

		PilotCompartmentSlot pilotSlot = PilotCompartmentSlot.Cast(access.GetCompartment());
		if (!pilotSlot)
			return false;

		Vehicle vehicle = Vehicle.Cast(pilotSlot.GetOwner());
		if (!vehicle)
			return false;

		if (vehicle.GetPilotCompartmentSlot() != pilotSlot)
			return false;

		if (minSpeedSq <= 0)
			return true;

		Physics physics = vehicle.GetPhysics();
		if (!physics)
			return false;

		return physics.GetVelocity().LengthSq() >= minSpeedSq;
	}
}
