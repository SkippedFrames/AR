class SCR_AIRemoveVehicleFromGroup : AITaskScripted
{	
	static const string PORT_VEHICLE = "VehicleIn";
	
	[Attribute("1", UIWidgets.CheckBox, "Remove all vehicles used by any group member?")]
    bool m_bRemoveAllUsedVehicles;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
		{
			return NodeError(this,owner, "Node must be run on SCR_AIGroup agent!");			
		}
		
		IEntity agentEntity, vehicleEntity;
		
		if (GetVariableIn(PORT_VEHICLE, vehicleEntity))
		{
			group.RemoveUsableVehicle(vehicleEntity);
			return ENodeResult.SUCCESS;
		};
		
		if (!m_bRemoveAllUsedVehicles)
		{
			return ENodeResult.FAIL;
		};
				
		ref array<AIAgent> agents = {};
		group.GetAgents(agents);
		CompartmentAccessComponent compartmentAccess;
		BaseCompartmentSlot compartment;
		ENodeResult nodeResult = ENodeResult.FAIL;
		
		
		foreach (AIAgent agent : agents)
		{
			agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;
			compartmentAccess = CompartmentAccessComponent.Cast(agentEntity.FindComponent(CompartmentAccessComponent));
			if (!compartmentAccess)
				continue;
			compartment = compartmentAccess.GetCompartment();
			if (!compartment)
				continue;
			vehicleEntity = compartment.GetOwner();
			if (vehicleEntity)
			{
				group.RemoveUsableVehicle(vehicleEntity);
				nodeResult = ENodeResult.SUCCESS;
			}	
		}
		// if noone of the group is inside a vehicle and no vehicle provided => wrong use of the node? -> fail
		return nodeResult;
		
	}
		
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_VEHICLE
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Removes vehicle from the list of usable vehicles of the group. Provide either a vehicle entity or removes all vehicles used by any group member";
	}		

};
