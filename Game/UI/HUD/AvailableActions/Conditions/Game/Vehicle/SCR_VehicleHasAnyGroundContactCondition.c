//------------------------------------------------------------------------------------------------
//! Returns true if vehicle is colliding with ground
[BaseContainerProps()]
class SCR_VehicleHasAnyGroundContactCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		VehicleBaseSimulation simulation;
		if (GetGame().GetIsClientAuthority())
		{
			VehicleControllerComponent controller = VehicleControllerComponent.Cast(data.GetCurrentVehicleController());
			if (controller)
				simulation = controller.GetBaseSimulation();
		}
		else
		{
			VehicleControllerComponent_SA controller = VehicleControllerComponent_SA.Cast(data.GetCurrentVehicleController());
			if (controller)
				simulation = controller.GetBaseSimulation();
		}

		if (!simulation)
			return false;

		return GetReturnResult(simulation.HasAnyGroundContact());
	}
}
