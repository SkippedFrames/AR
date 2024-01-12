class SCR_AIGetTarget: AITaskScripted
{	
	[Attribute("1", UIWidgets.EditBox, "Filter target by how long ago it was seen", "")]
	float m_fTimeSinceSeenMax;
	
	[Attribute("0", UIWidgets.ComboBox, "Wanted target type", "", ParamEnumArray.FromEnum(ETargetCategory) )]
	ETargetCategory m_eTargetType;
	
	protected static const string PORT_BASE_TARGET = "BaseTarget";

	protected PerceptionComponent m_PerceptionComp;
	protected SCR_AIConfigComponent m_ConfigComp;
	#ifdef WORKBENCH
	protected ref Shape m_Shape;
	#endif

	override bool VisibleInPalette()
    {
        return true;
    }
	
	protected override string GetNodeMiddleText()
	{
		return	"LastSeenMax: " + m_fTimeSinceSeenMax.ToString() + "\n" +
				"TargetCategory: " + typename.EnumToString(ETargetCategory, m_eTargetType);
	}
	
	override void OnInit(AIAgent owner)
	{
		GenericEntity ent = GenericEntity.Cast(owner.GetControlledEntity());
		if (!ent)
			return;
		
		m_PerceptionComp = PerceptionComponent.Cast(ent.FindComponent(PerceptionComponent));
		m_ConfigComp = SCR_AIConfigComponent.Cast(owner.FindComponent(SCR_AIConfigComponent));	
	}
		
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_ConfigComp || !m_PerceptionComp)
			return ENodeResult.FAIL;
		
		if (m_PerceptionComp && m_ConfigComp.m_EnablePerception)
		{
			BaseTarget baseTarget = m_PerceptionComp.GetClosestTarget(m_eTargetType, m_fTimeSinceSeenMax, m_fTimeSinceSeenMax);
			
			SetVariableOut(PORT_BASE_TARGET, baseTarget);
			
			if (baseTarget)
			{
#ifdef WORKBENCH
				if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_LASTSEEN))
					m_Shape = Shape.CreateSphere(COLOR_BLUE_A, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, baseTarget.GetLastSeenPosition(), 0.1);
#endif
				return ENodeResult.SUCCESS;
			}
			else
			{
				return ENodeResult.FAIL;
			}
		}

		return ENodeResult.FAIL;
	}
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_BASE_TARGET
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
};
