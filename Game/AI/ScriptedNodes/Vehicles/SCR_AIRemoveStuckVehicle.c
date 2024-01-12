class SCR_AIRemoveStuckVehicle : AITaskScripted
{
	
	SCR_AIGroup m_rGroup;
	IEntity m_vehicleEnt;
	
	override void OnInit(AIAgent owner)
	{
		m_rGroup = SCR_AIGroup.Cast(owner);			
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (GetVariableIn("VehicleIn",m_vehicleEnt))
		{
			Vehicle vehicle = Vehicle.Cast(m_vehicleEnt);
			if (m_rGroup && vehicle)
			{
				
				m_rGroup.RemoveUsableVehicle(vehicle);
				return ENodeResult.SUCCESS;
			};
		}			
		return ENodeResult.FAIL;
	}	
	
	protected static ref TStringArray s_aVarsIn = {
		"VehicleIn"
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	override bool VisibleInPalette()
    {
        return true;
    }
	
	override string GetOnHoverDescription()
	{
		return "Remove stuck vehicle: when vehicle cannot be moved it removes it from group's list of owned vehicles";
	}	
};
