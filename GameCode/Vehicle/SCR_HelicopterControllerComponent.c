class SCR_HelicopterControllerComponentClass : HelicopterControllerComponentClass
{
}

/*!
	Class responsible for game helicopter.
	It connects all helicopter components together and handles all comunication between them.
*/
class SCR_HelicopterControllerComponent : HelicopterControllerComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		super.OnCompartmentEntered(vehicle, mgr, occupant, managerId, slotID);

		if (mgr.FindCompartment(slotID, managerId) == GetPilotCompartmentSlot())
		{
			GetGame().GetInputManager().AddActionListener("HelicopterLightsTaxiToggle", EActionTrigger.DOWN, ActionHelicopterLightsTaxiToggle);
			GetGame().GetInputManager().AddActionListener("HelicopterLightsLandingToggle", EActionTrigger.DOWN, ActionHelicopterLightsLandingToggle);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		super.OnCompartmentLeft(vehicle, mgr, occupant, managerId, slotID);

		if (mgr.FindCompartment(slotID, managerId) == GetPilotCompartmentSlot())
		{
			GetGame().GetInputManager().RemoveActionListener("HelicopterLightsTaxiToggle", EActionTrigger.DOWN, ActionHelicopterLightsTaxiToggle);
			GetGame().GetInputManager().RemoveActionListener("HelicopterLightsLandingToggle", EActionTrigger.DOWN, ActionHelicopterLightsLandingToggle);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionHelicopterLightsTaxiToggle(float value, EActionTrigger trigger)
	{
		PilotCompartmentSlot pilotSlot = GetPilotCompartmentSlot();
		if (!pilotSlot || pilotSlot.GetOccupant() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (lightManager)
			lightManager.SetLightsState(ELightType.SearchLight, !lightManager.GetLightsState(ELightType.SearchLight));
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionHelicopterLightsLandingToggle(float value, EActionTrigger trigger)
	{
		PilotCompartmentSlot pilotSlot = GetPilotCompartmentSlot();
		if (!pilotSlot || pilotSlot.GetOccupant() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		BaseLightManagerComponent lightManager = GetLightManager();
		if (lightManager)
			lightManager.SetLightsState(ELightType.HiBeam, !lightManager.GetLightsState(ELightType.HiBeam));
	}
}
