class SCR_AIFindTurrets: AITaskScripted
{
	static const string PORT_CENTER_OF_SEARCH		= "OriginIn";
	static const string PORT_RADIUS					= "RadiusIn";
	static const string PORT_TURRET_NUMBER			= "TurretNumber";
	static const string PORT_TURRET_FOUND			= "TurretsFound";
	
	private BaseWorld m_world;
	private int m_turretCount;
	private int m_agentsCount;
	private bool m_turretsFound;
	private SCR_AIGroup m_groupOwner;
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (!m_world && GetGame())
			m_world = GetGame().GetWorld();
		m_groupOwner = SCR_AIGroup.Cast(owner);
		if (!m_groupOwner)
		{
			m_groupOwner = SCR_AIGroup.Cast(owner.GetParentGroup());
			if (!m_groupOwner)
				NodeError(this, owner, "Node is not run on SCR_AIGroup agent or owner is not member of SCR_AIGroup!");
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_world)
		{
			ClearVariable(PORT_TURRET_NUMBER);
			ClearVariable(PORT_TURRET_FOUND);
			return ENodeResult.FAIL;			
		}	
		vector center;
		float radius;
		m_agentsCount = m_groupOwner.GetAgentsCount(); 
		
		GetVariableIn(PORT_CENTER_OF_SEARCH, center);
		GetVariableIn(PORT_RADIUS, radius);
		
		m_world.QueryEntitiesBySphere(center, radius, GetFirstEntity, FilterEntities, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT);
		if (m_turretsFound)
			SetVariableOut(PORT_TURRET_NUMBER, m_turretCount);
		else 
			ClearVariable(PORT_TURRET_NUMBER);
		SetVariableOut(PORT_TURRET_FOUND, m_turretsFound);
		return ENodeResult.SUCCESS;		
	}
	
	bool GetFirstEntity(IEntity ent) 
	{
		BaseCompartmentManagerComponent compComp = BaseCompartmentManagerComponent.Cast(ent.FindComponent(BaseCompartmentManagerComponent));
		if (!compComp)
			return true;
		
		array<BaseCompartmentSlot> compartmentSlots = {};
		compComp.GetCompartments(compartmentSlots);
		foreach (BaseCompartmentSlot slot : compartmentSlots)
		{
			TurretCompartmentSlot turretComp = TurretCompartmentSlot.Cast(slot);
			if (turretComp)
			{				
				if (!turretComp.AttachedOccupant() && turretComp.IsCompartmentAccessible())
				{
					//turretComp.GetCompartmentSlotID();
					m_turretCount += 1;
					m_turretsFound = true;
					if (m_turretCount <= m_agentsCount) // do not occupy more turrets that you have agents
						m_groupOwner.AllocateCompartment(turretComp);
					break;
				}
			}
		}
		return true; //continue search to next turret
	}
	
	bool FilterEntities(IEntity ent) 
	{
		return Turret.Cast(ent) != null;
	}
	
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_TURRET_NUMBER,
		PORT_TURRET_FOUND
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_CENTER_OF_SEARCH,
		PORT_RADIUS
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
};
