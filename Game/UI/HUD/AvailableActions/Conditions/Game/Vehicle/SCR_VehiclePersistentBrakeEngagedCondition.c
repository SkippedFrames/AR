//------------------------------------------------------------------------------------------------
//! Returns true if character is in a vehicle
[BaseContainerProps()]
class SCR_VehiclePersistentBrakeEngagedCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in vehicle
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		if(GetGame().GetIsClientAuthority())
		{
			CarControllerComponent carController = CarControllerComponent.Cast(data.GetCurrentVehicleController());
			if (!carController)
				return false;
	
			bool result = carController.GetPersistentHandBrake();
			return GetReturnResult(result);
		}
		else
		{
			CarControllerComponent_SA carController = CarControllerComponent_SA.Cast(data.GetCurrentVehicleController());
			if (!carController)
				return false;
	
			bool result = carController.GetPersistentHandBrake();
			return GetReturnResult(result);
		}
	}
};
