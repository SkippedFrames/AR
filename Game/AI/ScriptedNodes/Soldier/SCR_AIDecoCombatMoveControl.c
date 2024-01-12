// Script File

class SCR_AIDecoCombatMoveControl : DecoratorScripted
{
	static const string PORT_NEXT_COVER = "NextCoverPos";
	static const string PORT_COMBAT_STANCE = "CombatStance";
	static const string PORT_ALLOW_STANCE_STAND = "AllowStanceStand";
	
	protected SCR_AICombatComponent m_CombatComponent;
	protected SCR_AIUtilityComponent m_Utility;
	
	IEntity m_entity;
	
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_NEXT_COVER,
		PORT_COMBAT_STANCE
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_ALLOW_STANCE_STAND
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	override void OnInit(AIAgent owner)
	{
		m_entity = owner.GetControlledEntity();					
	}
	
	protected override bool TestFunction(AIAgent owner)
	{
		if (m_entity && !m_CombatComponent)
		{
			m_CombatComponent = SCR_AICombatComponent.Cast(m_entity.FindComponent(SCR_AICombatComponent));			
		}
		
		if (!m_CombatComponent)
			return false;
		else if (!m_Utility)
		{
			m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));		
		}
		
		if (!m_CombatComponent.IsActionAllowed(EAICombatActions.MOVEMENT_WHEN_FIRE))
			return false;
		
		vector pos = m_CombatComponent.FindNextCoverPosition();
		if (pos == vector.Zero)
			return false;
		
		SetVariableOut(PORT_NEXT_COVER, pos);
		
		if (m_Utility)
		{
			bool allowStand = true;
			GetVariableIn(PORT_ALLOW_STANCE_STAND, allowStand);
			
			ECharacterStance threatStance = GetStanceFromThreat(m_Utility.m_ThreatSystem.GetState());
			if (threatStance == ECharacterStance.STAND && !allowStand)
				threatStance = ECharacterStance.CROUCH;
			SetVariableOut(PORT_COMBAT_STANCE, threatStance);
		}
		return true;			
	}	
		
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	protected override string GetOnHoverDescription()
	{
		return "Decorator that controls attack move";
	}	
};
