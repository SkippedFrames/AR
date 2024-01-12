class SCR_AIGetAllocatedCompartment: AITaskScripted
{
	static const string PORT_COMPARTMENT			= "CompartmentOut";
	static const string PORT_VEHICLE				= "VehicleOut";
	static const string ERROR_LOG					= "Node is not run on SCR_AIGroup agent or owner is not member of SCR_AIGroup!";
		
	protected SCR_AIGroup m_groupOwner;	
	protected int m_indexOfLastCompartment = -1;
	protected bool m_searchStarted = false;
	
	[Attribute("-1",uiwidget: UIWidgets.ComboBox, "Restrict to specific compartment?", "", ParamEnumArray.FromEnum(EAICompartmentType))]
	EAICompartmentType m_eCompartmentType;
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_groupOwner = SCR_AIGroup.Cast(owner);
		if (!m_groupOwner)
		{
			m_groupOwner = SCR_AIGroup.Cast(owner.GetParentGroup());
			if (!m_groupOwner)
				NodeError(this, owner, ERROR_LOG);
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_groupOwner)
			return NodeError(this, owner, ERROR_LOG);
		ref array<BaseCompartmentSlot> compartments = {};
		m_groupOwner.GetAllocatedCompartments(compartments);
		BaseCompartmentSlot compartmentOut;
		IEntity vehicleOut;		
		
		if (!m_searchStarted)
		{
			m_indexOfLastCompartment = compartments.Count() - 1;
			m_searchStarted = true;
		}	
		
		for (int index = m_indexOfLastCompartment; index >= 0; index--)
		{
			if (IsCompartmentOfSameType(compartments[index].Type(), m_eCompartmentType))
			{
				compartmentOut = compartments[index];
				vehicleOut = compartments[index].GetVehicle();
				m_indexOfLastCompartment = index - 1 ; // remember last index of compartment to search from
				break;
			}						
		}
		
		if (compartmentOut)
		{
			SetVariableOut(PORT_COMPARTMENT, compartmentOut);
			SetVariableOut(PORT_VEHICLE, vehicleOut);
			return ENodeResult.SUCCESS;
		}		
		return ENodeResult.FAIL;
	}
	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		m_searchStarted = false;
	}
			
	//------------------------------------------------------------------------------------------------
	bool IsCompartmentOfSameType(typename compartmentType, EAICompartmentType AICompartmentType)
	{
		switch (AICompartmentType)
		{
			case EAICompartmentType.Turret : 
			{
				return compartmentType == TurretCompartmentSlot;
			};
			case EAICompartmentType.Cargo : 
			{
				return compartmentType == CargoCompartmentSlot;
			};
			case EAICompartmentType.Pilot : 
			{
				return compartmentType == PilotCompartmentSlot;				
			};
			default: //unspecified means type is "ANY"
			{
				return true;
			};
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_COMPARTMENT,
		PORT_VEHICLE
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetOnHoverDescription() { return "Gets allocated compartments of group to distribute (and eventually occupy) along group members. \nOne can restrict which compartment types to allocate None means any"; };	
};
