class SCR_AIGetAimDistanceCompensation : AITaskScripted
{
	protected static string TARGET_ENTITY_PORT = "TargetEntity";
	protected static string TARGET_POSITION_PORT = "TargetPosition";
	protected static string VECTOR_IN_PORT = "VectorIn";
	protected static string INITIAL_SPEED_COEFFICIENT = "InitialSpeedCoefficient";
	
	protected static string VECTOR_OUT_PORT = "VectorOut";
	
	protected BaseWeaponManagerComponent m_WeaponMgr;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity ent = owner.GetControlledEntity();
		if (!ent)
			return;
		m_WeaponMgr = BaseWeaponManagerComponent.Cast(ent.FindComponent(BaseWeaponManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_WeaponMgr)
			return ENodeResult.FAIL;
		
		// Read input variables
		vector vin;
		vector vout;
		float initSpeedCoef = 1.0;
		GetVariableIn(VECTOR_IN_PORT, vin);
		GetVariableIn(INITIAL_SPEED_COEFFICIENT, initSpeedCoef); // if not connected it will be still default 1.0
		
		// Resolve target position
		vector targetPos;
		IEntity targetEntity;
		
		if (GetVariableIn(TARGET_ENTITY_PORT, targetEntity))
		{
			if (!targetEntity)
			{
				Print("GetAimDistanceCompensation: provided null entity", LogLevel.WARNING);
				return ENodeResult.FAIL;
			}	
			targetPos = targetEntity.GetOrigin();
		}	
		else
			GetVariableIn(TARGET_POSITION_PORT, targetPos);
		
		// Resolve current weapon and muzzle
		BaseWeaponComponent currentWeapon;
		BaseMuzzleComponent currentMuzzle;
		
		currentWeapon = m_WeaponMgr.GetCurrentWeapon();
		if (currentWeapon)
			currentMuzzle = currentWeapon.GetCurrentMuzzle();
		
		// Resolve distance to target
		float distance;
		if (currentMuzzle)
		{
			vector muzzleMatrix[4];
			m_WeaponMgr.GetCurrentMuzzleTransform(muzzleMatrix);
			distance = vector.Distance(targetPos, muzzleMatrix[3]);
		}
		else
			distance = vector.Distance(targetPos, owner.GetControlledEntity().GetOrigin());
		
		// Read data from ballistic tables
		float heightOffset;
		float flightTime;
		if (currentWeapon && currentMuzzle)
		{
			// For normal weapons with muzzles
			heightOffset = BallisticTable.GetAimHeightOfNextProjectile(distance, flightTime, currentMuzzle);
		}
		else if (currentWeapon)
		{
			// For grenades
			heightOffset = BallisticTable.GetHeightFromProjectile(distance, flightTime, currentWeapon.GetOwner(), initSpeedCoef);
		}
		
		// Convert compensation vector to target space
		// The Aim Offset node receives the vector in target entity space
		vector vHeightCompensation;
		vHeightCompensation[1] = heightOffset;
		if (targetEntity)
			vHeightCompensation = targetEntity.VectorToLocal(vHeightCompensation);
		
		
		vout = vin + vHeightCompensation;
		// problem of calculation: if character is reloading magazine and no ammo is in barrel -> the heightOffset is zero
		// if (float.AlmostEqual(heightOffset, 0.0))
		//	Print("Error in testing mission: compensation is zero!", LogLevel.ERROR);
		SetVariableOut(VECTOR_OUT_PORT, vout);
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {VECTOR_OUT_PORT};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {TARGET_ENTITY_PORT, TARGET_POSITION_PORT, VECTOR_IN_PORT, INITIAL_SPEED_COEFFICIENT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription() {return "Returns vertical offset for ballistic compensation of range to target";}
}
