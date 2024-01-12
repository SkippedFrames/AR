class SCR_AIGetFireTimes: AITaskScripted
{
	static const string PORT_FIRE_BURST = "FireBurstTime";
	static const string PORT_STABILIZATION = "StabilizationTime";
	static const string PORT_SUPPRESSION = "SuppressionTime";
	static const string PORT_REJECT_TIME = "RejectAimingTime";
	
	protected static string TARGET_ENTITY_PORT = "TargetEntity";
	protected static string TARGET_POSITION_PORT = "TargetPosition";
	
	static const float CLOSE_RANGE_THRESHOLD_SQ = 15 * 15;
		
	private SCR_AICombatComponent m_CombatComponent;
	private SCR_AIUtilityComponent m_Utility;
	
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity ent = owner.GetControlledEntity();
		if (ent)
			m_CombatComponent = SCR_AICombatComponent.Cast(ent.FindComponent(SCR_AICombatComponent));		
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
            PORT_FIRE_BURST,
            PORT_STABILIZATION,
            PORT_SUPPRESSION,
            PORT_REJECT_TIME
	};
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {TARGET_ENTITY_PORT, TARGET_POSITION_PORT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		
		EWeaponType weaponType = m_CombatComponent.GetCurrentWeaponType();
		float random = Math.RandomFloat(0.1,0.3); 
		
		// Factors are different than 1 only in close range
		float distStabilizationTimeFactor = 1;
		float distBurstTimeFactor = 1;
		
		vector targetPos;
		IEntity ownerEntity = owner.GetControlledEntity();
		
		if (!ownerEntity)
			return ENodeResult.FAIL;
		
		IEntity targetEntity;
			
		if (GetVariableIn(TARGET_ENTITY_PORT, targetEntity))
			targetPos = targetEntity.GetOrigin();
		else 
			GetVariableIn(TARGET_POSITION_PORT, targetPos);
		
		if (targetPos != vector.Zero)
		{
			float distToTargetSq = vector.DistanceSq(ownerEntity.GetOrigin(), targetPos);
			
			if (distToTargetSq < CLOSE_RANGE_THRESHOLD_SQ)
			{
				distStabilizationTimeFactor = Math.Map(distToTargetSq, 0, CLOSE_RANGE_THRESHOLD_SQ, 0, 1);
				distBurstTimeFactor = Math.Map(distToTargetSq, 0, CLOSE_RANGE_THRESHOLD_SQ, 0, 1);
			}
		}

		switch (weaponType)
		{
			case EWeaponType.WT_RIFLE:
			{
				SetVariableOut(PORT_FIRE_BURST, random * distBurstTimeFactor);
				SetVariableOut(PORT_STABILIZATION, 0.5 * distStabilizationTimeFactor);
				SetVariableOut(PORT_SUPPRESSION, 0.5 + random);
				SetVariableOut(PORT_REJECT_TIME, 1.0);
				break;
			}
			case EWeaponType.WT_MACHINEGUN:
			{
				SetVariableOut(PORT_FIRE_BURST, random * 2 * distBurstTimeFactor);
				SetVariableOut(PORT_STABILIZATION, 0.3 * distStabilizationTimeFactor);
				SetVariableOut(PORT_SUPPRESSION, 0.3 + random*2);
				SetVariableOut(PORT_REJECT_TIME, 1.0);
				break;
			}
			case EWeaponType.WT_SMOKEGRENADE:
			{
				ClearVariable(PORT_FIRE_BURST);
				SetVariableOut(PORT_STABILIZATION, 3.0 * distStabilizationTimeFactor);
				ClearVariable(PORT_SUPPRESSION);
				SetVariableOut(PORT_REJECT_TIME, 10.0);
				break;
			}
			case EWeaponType.WT_FRAGGRENADE:
			{
				ClearVariable(PORT_FIRE_BURST);
				SetVariableOut(PORT_STABILIZATION, 3.0 * distStabilizationTimeFactor);
				ClearVariable(PORT_SUPPRESSION);
				SetVariableOut(PORT_REJECT_TIME, 10.0);
				break;
			}
			case EWeaponType.WT_HANDGUN:
			{
				ClearVariable(PORT_FIRE_BURST);
				SetVariableOut(PORT_STABILIZATION, 1.0 * distStabilizationTimeFactor);
				ClearVariable(PORT_SUPPRESSION);
				SetVariableOut(PORT_REJECT_TIME, 5.0);
				break;
			}
			case EWeaponType.WT_ROCKETLAUNCHER:
			{
				ClearVariable(PORT_FIRE_BURST);
				SetVariableOut(PORT_STABILIZATION, 0.5 * distStabilizationTimeFactor);
				ClearVariable(PORT_SUPPRESSION);
				SetVariableOut(PORT_REJECT_TIME, 10.0);
				break;
			}
			case EWeaponType.WT_SNIPERRIFLE:
			{
				ClearVariable(PORT_FIRE_BURST);
				SetVariableOut(PORT_STABILIZATION, 1.0 * distStabilizationTimeFactor);
				ClearVariable(PORT_SUPPRESSION);
				SetVariableOut(PORT_REJECT_TIME, 20.0);
				break;
			}
			default:
			{
				SetVariableOut(PORT_FIRE_BURST, random * 3 * distBurstTimeFactor);
				ClearVariable(PORT_STABILIZATION);
				SetVariableOut(PORT_SUPPRESSION, random * 3);
				SetVariableOut(PORT_REJECT_TIME, 1.0);
				break;
			}
		}
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
    override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription() {return "Get all time constants for fire - it is weapon dependent";}
};

