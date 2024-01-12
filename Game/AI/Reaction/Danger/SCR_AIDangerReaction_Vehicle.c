[BaseContainerProps()]
class SCR_AIDangerReaction_Vehicle : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		IEntity vehicleObject = dangerEvent.GetVictim();
		if (!vehicleObject)
			return false;
		
		// Ignore if we already have an action to move from that vehicle
		if (SCR_AIMoveFromDangerBehavior.ExistsBehaviorForEntity(utility, vehicleObject))
			return false;
		
		SCR_BaseCompartmentManagerComponent compManager = SCR_BaseCompartmentManagerComponent.Cast(vehicleObject.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compManager)
			return false;
		array<IEntity> occupants = {};
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		compManager.GetOccupantsOfType(occupants, ECompartmentType.Pilot);
		if (occupants.Count() > 0 && agent.IsEnemy(occupants[0]))	
		{
			SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromIncomingVehicleBehavior(utility, null, vector.Zero, dangerEntity: vehicleObject);
			utility.AddAction(behavior);
			return true;
		}
		return false;
	}	
};
