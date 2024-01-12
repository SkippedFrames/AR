//------------------------------------------------------------------------------------------------
//! Set minmum time  to more than zero to track temporary turbo only, set it to -1 to track persistent turbo only.
//! Returns true if driver is using full throttle feature.
//! Set minimum time over zero to track temporary turbo only.
//! Set minimum time to zero to track both temporary and persistent turbo.
//! Set minimum time below zero to track persistent turbo only.
[BaseContainerProps()]
class SCR_VehicleTurboCondition : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.EditBox, "Minimum temporary turbo time. Set to below zero to track persistent turbo only.", "")]
	protected float m_fMinimumTime;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		if(GetGame().GetIsClientAuthority())
		{
			CarControllerComponent carController = CarControllerComponent.Cast(data.GetCurrentVehicleController());
			bool result = carController && carController.IsThrottleTurbo();
	
			if (m_fMinimumTime > 0)
				result = result && data.GetCurrentVehicleTurboTime() > m_fMinimumTime;
			else if (m_fMinimumTime < 0)
				result = result && data.GetCurrentVehicleTurboTime() == 0;
	
			return GetReturnResult(result);
		}
		else
		{
			CarControllerComponent_SA carController = CarControllerComponent_SA.Cast(data.GetCurrentVehicleController());
			bool result = carController && carController.IsThrottleTurbo();
	
			if (m_fMinimumTime > 0)
				result = result && data.GetCurrentVehicleTurboTime() > m_fMinimumTime;
			else if (m_fMinimumTime < 0)
				result = result && data.GetCurrentVehicleTurboTime() == 0;
	
			return GetReturnResult(result);
		}
	}
		
};
