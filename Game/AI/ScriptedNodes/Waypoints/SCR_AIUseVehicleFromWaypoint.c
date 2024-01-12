class SCR_AIUseVehicleFromWaypoint : AITaskScripted
{
	static const string PORT_VEHICLE = "Vehicle";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_VEHICLE
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
			return NodeError(this,owner,"SCR_AIUseVehicleFromWaypoint must be run on SCR_AIGroup!");
		
		SCR_EntityWaypoint wp = SCR_EntityWaypoint.Cast(group.GetCurrentWaypoint());
		if (!wp)
		{
			return ENodeResult.FAIL;
		}
		
		IEntity vehicle = wp.GetEntity();
		if (!vehicle)
		{
			return ENodeResult.FAIL;
		}
		
		SetVariableOut(PORT_VEHICLE, vehicle);
		group.AddUsableVehicle(vehicle);

		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Returns vehicle from SCR_AIEntityWaypoint and adds it to UsableVehicles of the group";
	}		

};
