[ComponentEditorProps(category: "GameScripted/Character", description: "Enables melee for character")]
class SCR_MeleeComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MeleeHitDataClass
{
	IEntity m_Entity = null;
	IEntity m_Weapon = null;
	SurfaceProperties m_SurfaceProps = null;
	int	m_iNodeIndex = -1;
	float m_fDamage = 0;
	int m_iColliderIndex = -1;
	vector m_vHitPosition = vector.Zero;
	vector m_vHitDirection = vector.Zero;
	vector m_vHitNormal = vector.Zero;
};

void OnMeleePerformedDelegate(IEntity owner);
typedef func OnMeleePerformedDelegate;
typedef ScriptInvokerBase<OnMeleePerformedDelegate> OnMeleePerformedInvoker;

//------------------------------------------------------------------------------------------------
class SCR_MeleeComponent : ScriptComponent
{
	//#define SCR_MELEE_DEBUG				//! uncomment to enable melee debug draws
	//#define SCR_MELEE_DEBUG_MSG			//! uncomment to enable melee debug prints into console
#ifdef SCR_MELEE_DEBUG
	ref array<ref Shape> m_aDbgSamplePositionsShapes;
	ref array<ref Shape> m_aDbgCollisionShapes;
#endif
	
	//------------------------------------------------------------------------------------------------
	private ref SCR_MeleeHitDataClass m_MeleeHitData;		//! holds melee hit data
	private bool m_bMeasurementDone;
	private bool m_bAttackAlreadyExecuted = false;			//! attack execution limiter
	private bool m_bMeleeAttackStarted = false;
	private float m_fMWPWeaponRange = 1;
	protected ref OnMeleePerformedInvoker m_OnMeleePerformed;
	
	//------------------------------------------------------------------------------------------------
	OnMeleePerformedInvoker GetOnMeleePerformed()
	{
		if (!m_OnMeleePerformed)
			m_OnMeleePerformed = new OnMeleePerformedInvoker;
		
		return m_OnMeleePerformed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called from character command handler, based on user input.
	void PerformAttack()
	{
		if (m_bAttackAlreadyExecuted)
			return;
		
#ifdef SCR_MELEE_DEBUG
		Do_ClearDbgShapes();
#endif
		
		//This can be attached to a script invoker, which tells us the player changed their weapon
		CollectMeleeWeaponProperties();
		m_bAttackAlreadyExecuted = true;
	}

	//------------------------------------------------------------------------------------------------
	/**
	 \brief Enable or disable measuring of hit detechtion probes
	 \param state if is/is not allowed to measure
	*/
	void SetMeleeAttackStarted(bool started)
	{
		m_bMeleeAttackStarted = started;
		
		if (!started)
			Do_OnFrameCleanup();
	}

	//------------------------------------------------------------------------------------------------
	/**
	 \brief Primary collision check that uses TraceMove and single TraceSphere fired from character eye pos along the aim fwd vector
	 \param pHitData SCR_MeleeHitDataClass holder that keeps information about actual hit
	 \return bool When hit is detected or it is miss
	*/
	private bool CheckCollisionsSimple(out SCR_MeleeHitDataClass pHitData)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		autoptr TraceSphere param = new TraceSphere;
		param.Exclude = character;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		param.Radius = 0.3;
		param.Start = character.EyePosition();
		//! trace to a range defined by MeleeWeaponProperties along the players direction
		param.End = param.Start + GetPlayersDirection() * m_fMWPWeaponRange;

		// Trace stops on the first hit entity if you want to change this, change the filter callback
		float hit = character.GetWorld().TraceMove(param, null);

		if (!param.TraceEnt)
			return false;
		
		vector dir = (param.End - param.Start);
		
		pHitData.m_Entity = param.TraceEnt;
		pHitData.m_iColliderIndex = param.ColliderIndex;
		pHitData.m_vHitPosition = dir * hit + param.Start;
		pHitData.m_vHitDirection = dir.Normalized();
		pHitData.m_vHitNormal = param.TraceNorm;
		pHitData.m_SurfaceProps = param.SurfaceProps;
		pHitData.m_iNodeIndex = param.NodeIndex;
		
#ifdef SCR_MELEE_DEBUG_MSG
		MCDbgPrint("Check collision from: "+param.Start);
		MCDbgPrint("Check collision to: "+param.End);
		MCDbgPrint("Check collision range: " +vector.Distance(param.End, param.Start));
		MCDbgPrint("Check collision entity: "+pHitData.m_Entity.ToString());
		MCDbgPrint("Check collision hitzone: "+pHitData.m_sHitZoneName);
		MCDbgPrint("Check collision hitPos: "+pHitData.m_vHitPosition);
#endif
		
		return true;
	}

	//! TODO: call this from SCR_CharacterControllerComponent.OnWeaponSelected() when possible and remove from PerformAttack()
	//------------------------------------------------------------------------------------------------
	/**
	 \brief Prepare information from MeleeWeaponProperties component on weapon
	 \param weapon Equipped melee weapon we are getting the data from
	*/
	private void CollectMeleeWeaponProperties()
	{
		BaseWeaponManagerComponent wpnManager = BaseWeaponManagerComponent.Cast(GetOwner().FindComponent(BaseWeaponManagerComponent));
		if (!wpnManager)
			return;
		
		WeaponSlotComponent currentSlot = WeaponSlotComponent.Cast(wpnManager.GetCurrent());
		if (!currentSlot)
			return;
		
		//! get current weapon and store it into SCR_MeleeHitDataClass instance
		m_MeleeHitData.m_Weapon = currentSlot.GetWeaponEntity();

		IEntity weaponEntity = currentSlot.GetWeaponEntity();
		if (!weaponEntity)
			return;
		
		SCR_MeleeWeaponProperties meleeWeaponProperties = SCR_MeleeWeaponProperties.Cast(weaponEntity.FindComponent(SCR_MeleeWeaponProperties));
		if (!meleeWeaponProperties)
			return;
		
		m_MeleeHitData.m_fDamage = meleeWeaponProperties.GetWeaponDamage();
		m_fMWPWeaponRange = meleeWeaponProperties.GetWeaponRange();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the direction the player is looking
	private vector GetPlayersDirection()
	{
		vector aimMat[4];
		Math3D.AnglesToMatrix(CharacterControllerComponent.Cast(GetOwner().FindComponent(CharacterControllerComponent)).GetInputContext().GetAimingAngles() * Math.RAD2DEG, aimMat);
		return aimMat[2];
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles playing of sound & setting of signals (AUDIO)
	private void HandleMeleeSound()
	{		
		SoundComponent soundComp = SoundComponent.Cast(m_MeleeHitData.m_Weapon.FindComponent(SoundComponent));
		if (!soundComp)
			return;
	
		GameMaterial material = m_MeleeHitData.m_SurfaceProps;
		
		if (!material)	
			return;
		
		int surfaceInt = material.GetSoundInfo().GetSignalValue();
		
		soundComp.SetSignalValueStr("HitZone", m_MeleeHitData.m_iColliderIndex);
		soundComp.SetSignalValueStr("Surface", surfaceInt);
		
		#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_PRINT_MELEESURFACE))
		{
			Print(string.Format("HitZone: %1", m_MeleeHitData.m_iColliderIndex));
			Print(string.Format("Surface: %1", surfaceInt));
			Print(string.Format("-------------------------------------"))
		}
		#endif
		
		vector mat[4];
		Math3D.MatrixIdentity3(mat);
		mat[3] = m_MeleeHitData.m_vHitPosition;
		soundComp.SoundEventTransform(SCR_SoundEvent.SOUND_MELEE_IMPACT, mat);
	}
	
	
	//------------------------------------------------------------------------------------------------
	// Goes upwards the hierarchy of startEntity searching for DamageManagerComponent to deal damage to
	protected DamageManagerComponent SearchHierarchyForDamageManager(IEntity startEntity, out HitZone hitzone)
	{
		if (!startEntity)
			return null;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(startEntity.FindComponent(DamageManagerComponent));
		if (damageManager)
		{
			hitzone = damageManager.GetDefaultHitZone();
			return damageManager;
		}
			
		DamageManagerComponent tempManager;
		
		while (startEntity)
		{
			//if the parent entity has a multiphase destruction we don't want to pass the damage
			//Because multi phase destructibles can be quite big and be separated by multiple entities on the hierarchy
			tempManager = DamageManagerComponent.Cast(startEntity.FindComponent(DamageManagerComponent));
			if (tempManager)
			{
				//if its a multiphase destructible, we dont pass dmg.
				if (!SCR_DestructionMultiPhaseComponent.Cast(tempManager))
				{	
					damageManager = tempManager;
					break;
				}
			}
			
			startEntity = startEntity.GetParent();
		}
		
		if (damageManager)
			hitzone = damageManager.GetDefaultHitZone();
		
		return damageManager;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Processes the melee attack
	\Applies the damage if applicable
	*/
	private void ProcessMeleeAttack()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComponent && rplComponent.IsProxy())
			return; // Don't call on clients
		
		if (!CheckCollisionsSimple(m_MeleeHitData))
			return;
		
		if (m_MeleeHitData.m_Weapon)
			HandleMeleeSound();
		
#ifdef SCR_MELEE_DEBUG
		m_aDbgSamplePositionsShapes.Clear();
		Debug_DrawSphereAtPos(m_MeleeHitData.m_vHitPosition, m_aDbgSamplePositionsShapes, 0xff00ff00, 0.03, ShapeFlags.NOZBUFFER);
#endif
		
		vector hitPosDirNorm[3];
		hitPosDirNorm[0] = m_MeleeHitData.m_vHitPosition;
		hitPosDirNorm[1] = m_MeleeHitData.m_vHitDirection;
		hitPosDirNorm[2] = m_MeleeHitData.m_vHitNormal;
		
		if (m_OnMeleePerformed)
			m_OnMeleePerformed.Invoke(GetOwner());
		
		//! check if the entity has the damage manager component
		HitZone hitzone;
		DamageManagerComponent damageManager = SearchHierarchyForDamageManager(m_MeleeHitData.m_Entity, hitzone);
		if (damageManager)
		{
			damageManager.HandleDamage(EDamageType.MELEE,
			m_MeleeHitData.m_fDamage, 
			hitPosDirNorm,
			m_MeleeHitData.m_Entity, 
			hitzone,
			Instigator.CreateInstigator(GetOwner()),
			m_MeleeHitData.m_SurfaceProps,
			m_MeleeHitData.m_iColliderIndex, 
			m_MeleeHitData.m_iNodeIndex);
			return;
		}
		
		SCR_DestructibleEntity destructible = SCR_DestructibleEntity.Cast(m_MeleeHitData.m_Entity);
		if (destructible)
		{
			destructible.HandleDamage(EDamageType.MELEE, m_MeleeHitData.m_fDamage, hitPosDirNorm);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! cleanup
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	/**
	 \brief Cleanup of several variables called on frame
	*/
	private void Do_OnFrameCleanup()
	{
		m_bAttackAlreadyExecuted = false;
		m_bMeasurementDone = false;
	}
	
#ifdef SCR_MELEE_DEBUG
	//! dbg
	//------------------------------------------------------------------------------------------------
	/**
	 \brief Cleanup of several variables called on frame
	*/
	private void Do_ClearDbgShapes()
	{
		m_aDbgSamplePositionsShapes.Clear();
		m_aDbgCollisionShapes.Clear();
	}
#endif


	//------------------------------------------------------------------------------------------------
	//! DEBUG methods
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Drawing of sphere if specified attributes on given vector pos
	private void Debug_DrawSphereAtPos(vector v, array<ref Shape> dbgShapes, int color = COLOR_BLUE, float size = 0.03, ShapeFlags shapeFlags = ShapeFlags.VISIBLE)
	{
		shapeFlags = ShapeFlags.NOOUTLINE | shapeFlags;
		
		vector matx[4];
		Math3D.MatrixIdentity4(matx);
		matx[3] = v;
		Shape s = Shape.CreateSphere(color, shapeFlags, GetOwner().GetOrigin(), size);
		s.SetMatrix(matx);
		dbgShapes.Insert(s);
	}
	
	//------------------------------------------------------------------------------------------------
	private void Debug_DrawSphereAtPos(vector pos, int color, array<ref Shape> dbgShapes)
	{
		vector matx[4];
		Math3D.MatrixIdentity4(matx);
		matx[3] = pos;
		int shapeFlags = ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP;
		Shape s = Shape.CreateSphere(color, shapeFlags, pos, 0.05);
		s.SetMatrix(matx);
		dbgShapes.Insert(s);
	}
	
	//------------------------------------------------------------------------------------------------
	private void Debug_DrawLineSimple(vector start, vector end, array<ref Shape> dbgShapes)
	{
		vector p[2];
		p[0] = start;
		p[1] = end;

		int shapeFlags = ShapeFlags.NOOUTLINE;
		Shape s = Shape.CreateLines(ARGBF(1, 1, 1, 1), shapeFlags, p, 2);
		dbgShapes.Insert(s);	
	}
	
#ifdef SCR_MELEE_DEBUG_MSG
	//------------------------------------------------------------------------------------------------
	//! Melee specific prints only
	private void MCDbgPrint(string msg = "")
	{
		Print("\\_ SCR_MELEE_DEBUG] " + GetGame().GetTickCount().ToString() + " ** " + msg);
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	//! Event methods
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		if (!m_bAttackAlreadyExecuted || !m_bMeleeAttackStarted || m_bMeasurementDone)
			return;
		
		//! Simplified hit detection
#ifdef SCR_MELEE_DEBUG
		//! draws a line with start and end point higlighted	
		vector start = ChimeraCharacter.Cast(GetOwner()).EyePosition();
		vector end = start + GetPlayersDirection() * m_fMWPWeaponRange;
		
		Debug_DrawLineSimple(start, end, m_aDbgCollisionShapes);
		Debug_DrawSphereAtPos(start, ARGBF(0.75, 0, 0, 1), m_aDbgCollisionShapes);
		Debug_DrawSphereAtPos(end, ARGBF(0.75, 0, 0, 1), m_aDbgCollisionShapes);
#endif
		
		ProcessMeleeAttack();
		m_bMeasurementDone = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_MeleeHitData = new SCR_MeleeHitDataClass;

		if (!GetGame().GetWorldEntity())
			return;
		
	#ifdef ENABLE_DIAG 
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_PRINT_MELEESURFACE, "", "Print Melee Surface", "Sounds"); 
	#endif
		
#ifdef SCR_MELEE_DEBUG
		m_aDbgSamplePositionsShapes = new array<ref Shape>;
		m_aDbgCollisionShapes = new array<ref Shape>;
#endif
	}
};
