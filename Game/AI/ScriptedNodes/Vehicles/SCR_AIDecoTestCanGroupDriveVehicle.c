class SCR_AIDecoTestCanGroupDriveVehicle: DecoratorTestScripted
{
	//------------------------------------------------------------------------------------------------
	//! Decorator test that checks vehicles registered on group for availability of compartments for group members
	//  "CanDrive" is true if there is available driver seat and entire group fits in the car(s) with available driver seats
	//  TODO: this node as decorator may check this potentially every frame and have big performance impact! Could be hidden 
	//  into some component that reacts on Events of compartments
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{	
		SCR_AIGroup group = SCR_AIGroup.Cast(agent);
		if (!group)
			return false;
		array<IEntity> vehicles = {};
		array<AIAgent> agents = {};
		group.GetUsableVehicles(vehicles);
		if (vehicles.IsEmpty())
			return false;
		int agentsCount = group.GetAgents(agents);		
		if (agents.IsEmpty())
			return false;
		IEntity vehicleToUse;
		foreach (AIAgent ag: agents)
		{
			IEntity agentEntity = ag.GetControlledEntity();
			if (!agentEntity)
				continue;
			IEntity vehicleOfAgent = CompartmentAccessComponent.GetVehicleIn(agentEntity);
			if (vehicles.Find(vehicleOfAgent) < 0)
				continue;
			agentsCount --;
			CompartmentAccessComponent compAcc = CompartmentAccessComponent.Cast(agentEntity.FindComponent(CompartmentAccessComponent));
			if (!compAcc)
				continue;
			if (!PilotCompartmentSlot.Cast(compAcc.GetCompartment()))
				continue;			
			vehicleToUse = vehicleOfAgent;	// group agent is driver inside some of groups registered vehicle	
		}
		
		if (agentsCount < 1)
			return true;
		// we will use vehicles array and search for available compartments only in vehicles that have available driver seat
		BaseCompartmentSlot compartment;
		ref array<BaseCompartmentSlot> compartments = {};
		BaseCompartmentManagerComponent compMan;
		if (!vehicleToUse && !SCR_AICompartmentHandling.FindAvailableCompartmentInVehicles(vehicles, ECompartmentType.Pilot, compartment, vehicleToUse))	
			return false; 			// no available driver
		
		while (!vehicles.IsEmpty() && agentsCount > 0 && vehicleToUse)
		{
			compMan = BaseCompartmentManagerComponent.Cast(vehicleToUse.FindComponent(BaseCompartmentManagerComponent));
			if (!compMan)
				break;
			compMan.GetCompartments(compartments);
			foreach (BaseCompartmentSlot comp: compartments)
			{
				if (!comp.GetOccupant() && comp.IsCompartmentAccessible())
					agentsCount --;
				if (agentsCount == 0)
					break;
			}
			vehicles.Remove(vehicles.Find(vehicleToUse));
			if (!SCR_AICompartmentHandling.FindAvailableCompartmentInVehicles(vehicles, ECompartmentType.Pilot, compartment, vehicleToUse))
				break; // no vehicle with available driver
		}
		return agentsCount == 0;			
	}
};
