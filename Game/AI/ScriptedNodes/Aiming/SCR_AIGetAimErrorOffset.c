enum EAimingPreference
{
	NONE,
	RANDOM,
	AUTOMATIC,
	FIXED,
};

class SCR_AIGetAimErrorOffset: AITaskScripted
{
	static const string PORT_ERROR_OFFSET = "ErrorOffset";
	static const string PORT_BASE_TARGET = "BaseTargetIn";
	static const string PORT_AIM_POINT = "AimPoint";
	static const string PORT_TOLERANCE = "AimingTolerance";
	static const float CLOSE_RANGE_THRESHOLD = 15.0;
	static const float LONG_RANGE_THRESHOLD = 200.0;
	static const float AIMING_ERROR_SCALE = 1.0; // TODO: game master and server option
	static const float AIMING_ERROR_FACTOR_MIN = 0.4; 
	static const float AIMING_ERROR_CLOSE_RANGE_FACTOR_MIN = 0.05;
	static const float AIMING_ERROR_FACTOR_MAX = 1.4;
	static const float MAXIMAL_TOLERANCE = 10.0;
	static const float MINIMAL_TOLERANCE = 0.003;
	
	protected SCR_AICombatComponent m_CombatComponent;
	// private SCR_AIInfoComponent m_InfoComponent;		temporary removed (adding threat effect later)
	
#ifdef AI_DEBUG
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif
	[Attribute("2", UIWidgets.ComboBox, "Hit zone selection", "", ParamEnumArray.FromEnum(EAimingPreference) )]
	protected EAimingPreference m_eAimingPreference;
	
	[Attribute("0", UIWidgets.ComboBox, "AimPoint type for fixed option", "", ParamEnumArray.FromEnum(EAimPointType) )]
	protected EAimPointType m_eAimPointType;
	
	[Attribute("0.03", UIWidgets.EditBox, "Default PrecisionXY")]
	protected float m_fDefaultPrecisionXY;

	private EAimPointType m_eCurrentAimPointType;

	private int m_iTorsoCount = 0;	
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity ent = owner.GetControlledEntity();
		if (ent)
			m_CombatComponent = SCR_AICombatComponent.Cast(ent.FindComponent(SCR_AICombatComponent));		
		//m_InfoComponent = SCR_AIInfoComponent.Cast(owner.FindComponent(SCR_AIInfoComponent));
		m_eCurrentAimPointType = m_eAimPointType;
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_ERROR_OFFSET,
		PORT_AIM_POINT,
		PORT_TOLERANCE
	};
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_BASE_TARGET
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
    //------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		//if (!m_CombatComponent || !m_InfoComponent)
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		
		IEntity entity = owner.GetControlledEntity();
		if (!entity)
			return ENodeResult.FAIL;
		
		BaseTarget target;
		AimPoint aimPoint;
		
#ifdef AI_DEBUG
		m_aDebugShapes.Clear();
		
#endif
		if (GetVariableIn(PORT_BASE_TARGET, target))
		{
			// get skill based on threat level
			// EAISkill currentSkill= GetSkillFromThreat(m_CombatComponent.GetAISkill(),m_InfoComponent.GetThreatState());
			
			EAISkill currentSkill = m_CombatComponent.GetAISkill();
			// choose a hitzone
			//m_eCurrentAimPointType = SelectAimPointType(m_eAimingPreference, m_eCurrentAimPointType);
			
			// Aim only at character head if enemy is in vehicle
			// Later this should be moved to SelectAimPointType when it is used again.
			IEntity targetEntity = target.GetTargetEntity();
			if (!targetEntity)
				return ENodeResult.FAIL;
			CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(targetEntity.FindComponent(CompartmentAccessComponent));
			if (compartmentAccess && compartmentAccess.GetCompartment())
				m_eCurrentAimPointType = EAimPointType.WEAK;
			
			aimPoint = GetAimPoint(target, m_eCurrentAimPointType);
			if (aimPoint)
			{
#ifdef AI_DEBUG
				if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_AIMPOINT))
					m_aDebugShapes.Insert(Shape.CreateSphere(COLOR_YELLOW_A, ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP, aimPoint.GetPosition(),aimPoint.GetDimension()));
#endif
				vector offsetX, offsetY;
				float angularSize, distance, distanceFactor, tolerance;
				
				GetTargetAngularBounds(entity, aimPoint, offsetX, offsetY, angularSize, distance);

				// correct aim point size based on distance to target
				distanceFactor = GetDistanceFactor(distance);
				
				offsetX = GetRandomFactor(currentSkill, 0) * offsetX * AIMING_ERROR_SCALE * distanceFactor;
				offsetY = GetRandomFactor(currentSkill, 0) * offsetY * AIMING_ERROR_SCALE * distanceFactor;
				
				tolerance = GetTolerance(entity, targetEntity, angularSize, distance);
				
				SetVariableOut(PORT_ERROR_OFFSET, offsetX + offsetY);
				SetVariableOut(PORT_AIM_POINT, aimPoint);
				SetVariableOut(PORT_TOLERANCE, tolerance);
#ifdef WORKBENCH
				// PrintFormat("Target size - used in tolerance: %1 target aimpointPosition: %2", distance * Math.Tan(tolerance * Math.DEG2RAD), aimPoint.GetPosition());
#endif
				return ENodeResult.SUCCESS;
			}
			else
			{
				ClearVariable(PORT_ERROR_OFFSET);
				ClearVariable(PORT_AIM_POINT);
				ClearVariable(PORT_TOLERANCE);
			}
		}
		return ENodeResult.FAIL;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	//! returns vectors of aimpoint volume projected onto shooter's BB center and the angular size of the target aimpoint
	void GetTargetAngularBounds(IEntity shooter, AimPoint targetAimpoint, out vector sizeVectorX, out vector sizeVectorY, out float angularSize, out float distance)
	{
		
		vector shooterCenter, joinNorm, join, min, max;
		float dimension = 2 * targetAimpoint.GetDimension();
		shooter.GetWorldBounds(min,max);
		
		// we wanted to use muzzle transform but that is unrelyable (when weapon switches, on init of fight, and so on)
		shooterCenter = (min + max) * 0.5;
		join = targetAimpoint.GetPosition() - shooterCenter;
		distance = join.Length();
		angularSize = Math.Atan2(dimension, distance) * Math.RAD2DEG;
		
		joinNorm = join.Normalized();
		sizeVectorX = (vector.Up * joinNorm).Normalized();
		sizeVectorY = (joinNorm * sizeVectorX).Normalized();
		
		sizeVectorX *= dimension;
		sizeVectorY *= dimension;
		
#ifdef AI_DEBUG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_PROJECTED_SIZE))
		{
			m_aDebugShapes.Insert(Shape.CreateArrow(shooterCenter, shooterCenter + sizeVectorX, 0.1, COLOR_BLUE, ShapeFlags.NOZBUFFER));
			m_aDebugShapes.Insert(Shape.CreateArrow(shooterCenter, shooterCenter + sizeVectorY, 0.1, COLOR_RED, ShapeFlags.NOZBUFFER));
			m_aDebugShapes.Insert(Shape.CreateArrow(shooterCenter, shooterCenter + joinNorm * dimension, 0.1, COLOR_YELLOW, ShapeFlags.NOZBUFFER));
		}	
#endif
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	//! returns factor of error scale based on distance to target 
	float GetDistanceFactor(float distance)
	{
		if (distance < CLOSE_RANGE_THRESHOLD)
			return Math.Map(distance, 0, CLOSE_RANGE_THRESHOLD, AIMING_ERROR_CLOSE_RANGE_FACTOR_MIN, AIMING_ERROR_FACTOR_MIN);

		float distanceCl = Math.Clamp((distance - CLOSE_RANGE_THRESHOLD) / LONG_RANGE_THRESHOLD, 0, 1);
		return Math.Lerp(AIMING_ERROR_FACTOR_MIN, AIMING_ERROR_FACTOR_MAX, distanceCl);
	}
	
	//------------------------------------------------------------------------------------------------
	// returns random factor based on AI skill
	float GetRandomFactor(EAISkill skill,float mu)
	{
		float sigma;
		switch (skill)
		{
			case EAISkill.ROOKIE :
			{
				sigma = 2;
				break;
			}
			case EAISkill.REGULAR :
			{
				sigma = 1;
				break;
			}
			case EAISkill.VETERAN :
			{
				sigma = 0.5;
				break;
			}
			case EAISkill.EXPERT :
			{
				sigma = 0.25;
				break;
			}
			case EAISkill.CYLON :
			{
				return 0;
			}
		}
		// PrintFormat("Gauss: %1, sigma: %2, skill: %3",result,sigma,typename.EnumToString(EAISkill,skill));
		return Math.RandomGaussFloat(sigma,mu);
	}
		
	//------------------------------------------------------------------------------------------------
	EAimPointType SelectAimPointType(EAimingPreference aimingPreference, EAimPointType currentSelection)
	{
		switch (aimingPreference)
		{
			case EAimingPreference.AUTOMATIC:
			{
				switch(currentSelection)
				{
					case EAimPointType.WEAK:
					{
						return EAimPointType.INCAPACITATE;
					}
					case EAimPointType.INCAPACITATE:
					{
						return EAimPointType.NORMAL;
					}
					case EAimPointType.NORMAL:
					{
						if (m_iTorsoCount < 2)
						{
							m_iTorsoCount += 1;
							return EAimPointType.NORMAL;
						}
						else 
						{
							m_iTorsoCount = 0;
							return EAimPointType.WEAK;
						}
					}
				};
				break;
			}
			case EAimingPreference.RANDOM: 
			{
				return Math.RandomInt(EAimPointType.NORMAL,EAimPointType.INCAPACITATE);
			}
			case EAimingPreference.FIXED:
			{
				return m_eAimPointType;
			}
		};
		return currentSelection;
	}
	
	//------------------------------------------------------------------------------------------------
	// returns skill corrected by current threat level and if AI can shoot under such suppression
	EAISkill GetSkillFromThreat(EAISkill inSkill, EAIThreatState threat)
	{
		switch (threat)
		{
			case EAIThreatState.THREATENED : 
			{		 
				switch (inSkill)
				{
					case EAISkill.ROOKIE :
					{
						return EAISkill.ROOKIE;
					}
					case EAISkill.REGULAR :
					{
						return EAISkill.ROOKIE;
					}
					case EAISkill.VETERAN :
					{
						return EAISkill.REGULAR;
					}
					case EAISkill.EXPERT :
					{
						return EAISkill.VETERAN;
					}
					case EAISkill.CYLON :
					{
						return EAISkill.CYLON;
					}
				};
				break;
			}
			case EAIThreatState.ALERTED :
			{
				switch (inSkill)
				{
					case EAISkill.ROOKIE :
					{
						return EAISkill.REGULAR;
					}
					case EAISkill.REGULAR :
					{
						return EAISkill.VETERAN;
					}
					case EAISkill.VETERAN :
					{
						return EAISkill.EXPERT;
					}
					case EAISkill.EXPERT :
					{
						return EAISkill.CYLON;
					}
					case EAISkill.CYLON :
					{
						return EAISkill.CYLON;
					}
				};
				break;
			}
			default :
			{
				return inSkill;
				break;
			}	
		}	
		return EAISkill.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! basic tolerance based on angular size of target in degrees
	float GetTolerance(IEntity observer, IEntity target, float angularSize, float distance)
	{
		float tolerance;
		bool setMaxTolerance;
	
		// Always use max tolerance in close range
		if (distance < CLOSE_RANGE_THRESHOLD)
			return MAXIMAL_TOLERANCE;
			
		tolerance = angularSize / 2; // half of the size
		// angular speed
		tolerance *= GetAngularSpeedFactor(observer, target, setMaxTolerance);
				
		if (setMaxTolerance)
			tolerance = MAXIMAL_TOLERANCE;
		else 
		{
			// weapon type tolerance modifier
			tolerance *= GetWeaponTypeFactor(m_CombatComponent.GetCurrentWeaponType());
		};
		return Math.Clamp(tolerance, MINIMAL_TOLERANCE, MAXIMAL_TOLERANCE);
	}	
	
	//------------------------------------------------------------------------------------------------	
	float GetAngularSpeedFactor(IEntity observer, IEntity enemy, out bool setBigTolerance)
	{
		IEntity parent = enemy.GetParent(); // getting the vehicle for character inside vehicle
		if (parent)
		{
			enemy = parent;					// case of driver
			parent = enemy.GetParent();
			if (parent)						// case of turret
				enemy = parent;
		}
		Physics ph = enemy.GetPhysics();
		if (ph)
		{
			vector positionVector = enemy.GetOrigin() - observer.GetOrigin();
			vector angularVelocity = positionVector * ph.GetVelocity() / positionVector.LengthSq();  // omega = (r x v) / ||r||^2 
			float angularSpeed = angularVelocity.Length();			
			
			if (angularSpeed < 0.07) // rougly 4 degs in radians
				return 1.0;
			else if (angularSpeed < 0.17) // roughly 10 degs in radians
				return 2;
		}	
		setBigTolerance = true;
		return 0;
	}

	//------------------------------------------------------------------------------------------------	
	float GetSuppressionFactor(EAIThreatState threat, out bool setBigTolerance)
	{
		switch (threat)
		{
			case EAIThreatState.THREATENED : 
			{		 
				setBigTolerance = true;
				return 0;
				break;
			}
			case EAIThreatState.ALERTED :
			{
				return 2.0;
				break;
			}
			default :
			{
				return 1.0;	
				break;
			}
		}
		return 0;
	}

	//------------------------------------------------------------------------------------------------		
	float GetWeaponTypeFactor(EWeaponType weaponType)
	{
		switch(weaponType)
		{
			case EWeaponType.WT_RIFLE:
			{
				return 1.0;
			}
			case EWeaponType.WT_MACHINEGUN:
			{
				return 2.0;
			}
			case EWeaponType.WT_HANDGUN:
			{
				return 1.5;
			}
			case EWeaponType.WT_FRAGGRENADE:
			{
				return 3.0;
			}
			case EWeaponType.WT_SMOKEGRENADE:
			{
				return 4.0;
			}
			case EWeaponType.WT_ROCKETLAUNCHER:
			{
				return 0.5;
			}
			case EWeaponType.WT_SNIPERRIFLE:
			{
				return 0.5;
			}
		}
		return 1.0;
	}
		
	//------------------------------------------------------------------------------------------------
	AimPoint GetAimPoint(BaseTarget target, EAimPointType aimPointType)
	{
		PerceivableComponent targetPerceivable = target.GetPerceivableComponent();
		if (!targetPerceivable)
			return null;
		
		array<ref AimPoint> aimPoints = {};
		targetPerceivable.GetAimpointsOfType(aimPoints, aimPointType);
		int index = aimPoints.GetRandomIndex(); // more aimpoints of same type -> pick one at random
		if (index > -1)
			return aimPoints[index];
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
    override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription() {return "Get aiming error position from the center of the target";}
};

