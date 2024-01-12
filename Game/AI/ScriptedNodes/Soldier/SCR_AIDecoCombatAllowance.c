class SCR_AIDecoCombatAllowance : DecoratorScripted
{
	[Attribute("2", UIWidgets.ComboBox, "Combat action to test:", "", ParamEnumArray.FromEnum(EAICombatActions) )]
	private EAICombatActions m_eCombatActions;
		
	protected SCR_AICombatComponent m_CombatComponent;	

	override void OnInit(AIAgent owner)
	{
		IEntity entity = owner.GetControlledEntity();	
		if (!entity)
		{
			NodeError(this, owner, "Must be run on agent that has an entity!");	
			return;
		}
		m_CombatComponent = SCR_AICombatComponent.Cast(entity.FindComponent(SCR_AICombatComponent));				
	}
	
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_CombatComponent)
			return false;
		
		return m_CombatComponent.IsActionAllowed(m_eCombatActions);			
	}	
		
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	protected override string GetOnHoverDescription()
	{
		return "Decorator that tests allowed type of combat";
	}	
	
	override string GetNodeMiddleText() 
	{ 
		return "Test " + typename.EnumToString(EAICombatActions,m_eCombatActions);	
	};
};
