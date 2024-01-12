//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_InVehicleTurretADSCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true if character is in ADS of current controlled vehicle turret
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		// Current weapon
		BaseControllerComponent controller = data.GetCurrentVehicleController();
		if (!controller)
			return GetReturnResult(false);

		// Turret
		IEntity turretEntity = controller.GetOwner();
		if (!turretEntity)
			return GetReturnResult(false);

		TurretComponent turret = TurretComponent.Cast(turretEntity.FindComponent(TurretComponent));
		if (!turret)
			return GetReturnResult(false);

		// Sights
		ScriptedSightsComponent sights = ScriptedSightsComponent.Cast(turret.FindComponent(ScriptedSightsComponent));
		if (!sights)
			return GetReturnResult(false);

		// In sights ADS?
		bool result = sights.IsSightADSActive();

		return GetReturnResult(result);
	}
};
