class SCR_AIGetEmptyCompartment : AITaskScripted
{
	static const string PORT_POSITION 	=	"Position";
	static const string PORT_AGENT 		=	"Agent";
	static const string PORT_VEHICLE 	=	"Vehicle";
	
	[Attribute("0", UIWidgets.CheckBox, "Occupy driver compartment with Group Leader Agent?" )]
	protected bool m_bAllowLeaderAsDriver;
	
	private IEntity m_VehicleEntity;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		GetVariableIn(PORT_VEHICLE, m_VehicleEntity);	
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (!m_VehicleEntity)
		{
			ClearVariable(PORT_POSITION);
			return ENodeResult.FAIL;
		}
		
		BaseCompartmentManagerComponent compartmentMan = BaseCompartmentManagerComponent.Cast(m_VehicleEntity.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentMan)
		{
			return NodeError(this, owner, "Missing compartment manager on IEntity" + m_VehicleEntity.ToString());
		}

		ref array<BaseCompartmentSlot>	compartments = {}, pilotComp = {}, turretComp = {}, cargoComp = {};
		int numOfComp = compartmentMan.GetCompartments(compartments);
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
		{
			return NodeError(this, owner, "GetEmptyCompartment not run on SCR_AIGroup agent!");
		}
		AIAgent groupMember;
		BaseCompartmentSlot compartmentToAlocate;
		bool foundEmptyCompartment;
		
		GetVariableIn(PORT_AGENT, groupMember);
		if (group.GetAgentsCount() == 1)
			m_bAllowLeaderAsDriver = true;
		
		foreach (BaseCompartmentSlot comp : compartments)
		{
			if (!comp.AttachedOccupant() && comp.IsCompartmentAccessible())
			{
				if (PilotCompartmentSlot.Cast(comp)) 
					pilotComp.Insert(comp);
				else if (TurretCompartmentSlot.Cast(comp))
					turretComp.Insert(comp);
				else
					cargoComp.Insert(comp);
				foundEmptyCompartment = true;
			}
		}
		
		if (!foundEmptyCompartment)
		{
			ClearVariable(PORT_POSITION);
			return ENodeResult.FAIL;
		}
		
		if (groupMember != group.GetLeaderAgent()) // this is not leader -> he has priority driver > turret > cargo
		{
			if (!pilotComp.IsEmpty())
				compartmentToAlocate = pilotComp[0];
			else if (!turretComp.IsEmpty())
				compartmentToAlocate = turretComp[0];
			else if (!cargoComp.IsEmpty())
				compartmentToAlocate = cargoComp[0];
		}
		else  // this is leader -> he has priority driver if m_bAllowLeaderAsDriver is true > cargo > turret > 			
		{
			if (m_bAllowLeaderAsDriver && !pilotComp.IsEmpty())
				compartmentToAlocate = pilotComp[0];
			else if (!cargoComp.IsEmpty())
				compartmentToAlocate = cargoComp[0];
			else if (!turretComp.IsEmpty())
				compartmentToAlocate = turretComp[0];
		}
		
		if (compartmentToAlocate)
		{
			group.AllocateCompartment(compartmentToAlocate);
			PrintFormat("Here %1 for %2", compartmentToAlocate, owner);
			SetVariableOut(PORT_POSITION, SCR_AICompartmentHandling.CompartmentClassToType(compartmentToAlocate.Type()));					
			return ENodeResult.SUCCESS;
		}
		
		ClearVariable(PORT_POSITION);
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_VEHICLE,
		PORT_AGENT
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_POSITION
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette()
    {
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
	override string GetOnHoverDescription()
	{
		return "Returns type of next usable compartment. Compartments are allocated in GetIn activity. The compartments are selected with priority pilot>turret>cargo unless AIAgent is leader";
	}
};
