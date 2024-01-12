class LightUserAction : BaseLightUserAction
{
	//! Type of action instead of 8 different classes
	[Attribute(defvalue: SCR_Enum.GetDefault(ELightType.Head), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ELightType))]
	protected ELightType m_eLightType;

	//! Side of turn signals
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Either", "-1"), ParamEnum("Left", "0"), ParamEnum("Right", "1")})]
	protected int m_iLightSide;

	//! Will only be shown if user is in vehicle?
	[Attribute()]
	protected bool m_bInteriorOnly;

	//! Will action be available for entities seated in pilot compartment only?
	[Attribute(defvalue: "1")]
	protected bool m_bPilotOnly;

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!GetLightManager())
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		// See if character is in vehicle
		if (character && character.IsInVehicle())
		{
			// See if character is in "this" (owner) vehicle
			CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
			if (!compartmentAccess)
				return false;

			// Character is in compartment
			// that belongs to owner of this action
			BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
			if (!slot)
				return false;

			// Check pilot only condition
			if (m_bPilotOnly && !PilotCompartmentSlot.Cast(slot))
				return false;

			// Check interior only condition
			if (m_bInteriorOnly && slot.GetOwner() != GetOwner())
				return false;

			return true;
		}

		// We cannot be pilot nor interior, if we are not seated in vehicle at all.
		if (m_bInteriorOnly || m_bPilotOnly)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (!lightManager)
			return;

		bool lightsState;
				

		lightsState = lightManager.GetLightsState(m_eLightType, m_iLightSide);

		if (RplSession.Mode() != RplMode.Client)
			lightsState = !lightsState;

		lightManager.SetLightsState(m_eLightType, lightsState, m_iLightSide);

		if (lightsState)
		{
			// Disable opposite turn signal
			if (m_iLightSide == 0)
				lightManager.SetLightsState(m_eLightType, false, 1);
			else if (m_iLightSide == 1)
				lightManager.SetLightsState(m_eLightType, false, 0);
		}

		// Sound
		PlaySound(pOwnerEntity, lightsState);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		BaseLightManagerComponent lightManager = GetLightManager();

		auto prefix = "";
		UIInfo actionInfo = GetUIInfo();
		if(actionInfo)
			prefix = actionInfo.GetName() + " ";

		if (lightManager && lightManager.GetLightsState(m_eLightType))
			outName = prefix + "#AR-UserAction_State_Off";
		else
			outName = prefix + "#AR-UserAction_State_On";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool GetLightAudioPos(out vector pos)
	{
		UserActionContext userActionContext = GetActiveContext();
		if (!userActionContext)
		{
			return false;
		}
		else
		{		
			pos = GetOwner().CoordToLocal(userActionContext.GetOrigin());
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	void PlaySound(IEntity ownerEntity, bool lightsState)
	{
		// Sound
		SoundComponent soundComponent = SoundComponent.Cast(ownerEntity.FindComponent(SoundComponent));
		if (!soundComponent)
			return;

		vector offset;
		if (GetLightAudioPos(offset))
		{
			if (lightsState)
				soundComponent.SoundEventOffset(SCR_SoundEvent.SOUND_VEHICLE_CLOSE_LIGHT_ON, offset);
			else
				soundComponent.SoundEventOffset(SCR_SoundEvent.SOUND_VEHICLE_CLOSE_LIGHT_OFF, offset);
		}
		else
		{
			if (lightsState)
				soundComponent.SoundEvent(SCR_SoundEvent.SOUND_VEHICLE_CLOSE_LIGHT_ON);
			else
				soundComponent.SoundEvent(SCR_SoundEvent.SOUND_VEHICLE_CLOSE_LIGHT_OFF);
		}
	}
}
