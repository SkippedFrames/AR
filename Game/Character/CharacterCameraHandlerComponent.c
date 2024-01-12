[ComponentEditorProps(category: "GameScripted/Character", description: "Scripted character camera handler (new)")]
class SCR_CharacterCameraHandlerComponentClass: CameraHandlerComponentClass
{
};

class SCR_CharacterCameraHandlerComponent : CameraHandlerComponent
{
	[Attribute(category: "Camera Shake")]
	ref SCR_RecoilCameraShakeParams m_pRecoilShakeParams;
	
	//! Progress of recoil based camera shake
	ref SCR_RecoilCameraShakeProgress m_pRecoilShake = new SCR_RecoilCameraShakeProgress();
	
	protected ref ScriptInvoker m_OnThirdPersonSwitch = new ScriptInvoker();
	static protected float s_fOverlayCameraFOV;
	
	protected int m_iHipsBoneIndex = 0;
	
	//------------------------------------------------------------------------------------------------
	void SCR_CharacterCameraHandlerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef ENABLE_DIAG 
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_CHARACTER_DEBUG_VIEW, "", "Cycle Debug View", "Character", "0, 8, 0, 1");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_DRAW_CAMERA_COLLISION_SOLVER, "", "Draw Camera Collision Solver", "Character");
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		m_OwnerCharacter = SCR_ChimeraCharacter.Cast(GetOwner());
		m_ControllerComponent = SCR_CharacterControllerComponent.Cast(m_OwnerCharacter.FindComponent(SCR_CharacterControllerComponent));
		m_AnimationComponent = CharacterAnimationComponent.Cast(m_OwnerCharacter.FindComponent(CharacterAnimationComponent));
		m_LoadoutStorage = EquipedLoadoutStorageComponent.Cast(m_OwnerCharacter.FindComponent(EquipedLoadoutStorageComponent));
		m_InputManager = GetGame().GetInputManager();
		m_IdentityComponent = CharacterIdentityComponent.Cast(m_OwnerCharacter.FindComponent(CharacterIdentityComponent));
		m_CmdHandler = CharacterCommandHandlerComponent.Cast(m_OwnerCharacter.FindComponent(CharacterCommandHandlerComponent));
			
		m_iHeadBoneIndex = m_OwnerCharacter.GetAnimation().GetBoneIndex("Head");
		m_iHipsBoneIndex = m_OwnerCharacter.GetAnimation().GetBoneIndex("Hips");
	}
	//------------------------------------------------------------------------------------------------
	override void OnCameraActivate()
	{
		OnCameraDeactivate();
		
		if (m_OwnerCharacter && m_AnimationComponent)
		{
			OnThirdPersonSwitch(IsInThirdPerson());
		}
		
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
		{
			if (playerController.m_bRetain3PV)
				SetThirdPerson(true);
			playerController.m_bRetain3PV = false;
		}
		if (!IsInThirdPerson() && m_OwnerCharacter)
		{
			OnAlphatestChange(m_OwnerCharacter.m_fFaceAlphaTest);
		}
	}
	//------------------------------------------------------------------------------------------------
	void OnAlphatestChange(int a)
	{
		m_OwnerCharacter.m_fFaceAlphaTest = a;
		
		if (m_IdentityComponent)
		{
			m_IdentityComponent.SetHeadAlpha(a);
		}
		
		if (m_LoadoutStorage)
		{
			auto ent = m_LoadoutStorage.GetClothFromArea(LoadoutHeadCoverArea);
			if (ent)
			{
				auto cloth = BaseLoadoutClothComponent.Cast(ent.FindComponent(BaseLoadoutClothComponent));
				if (cloth)
					cloth.SetAlpha(a);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCameraDeactivate()
	{
		if (m_OwnerCharacter)
			OnAlphatestChange(0);
		
		m_fFocusTargetValue = 0.0;
		m_fFocusValue = 0.0;
		SetFocusMode(m_fFocusValue);
	}
	
	bool IsDebugView()
	{
		#ifdef ENABLE_DIAG
		return DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_DEBUG_VIEW) != CharacterCameraSet.DEBUGVIEW_NONE;
		#else
		return false;
		#endif
	}

	//------------------------------------------------------------------------------------------------
	private bool m_bWasVehicleADS;
	bool CheckVehicleADS()
	{
		CheckIsInTurret(m_bWasVehicleADS);
		return m_bWasVehicleADS;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CheckIsInTurret(out bool isInAds)
	{
		isInAds = false;
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		// Fallback to default VehicleCamera behavior if getting out
		if (compartmentAccess && compartmentAccess.IsInCompartment() && !compartmentAccess.IsGettingOut())
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				BaseControllerComponent controller = compartment.GetController();
				if (controller)
				{
					TurretControllerComponent turretController = TurretControllerComponent.Cast(controller);
					if(turretController && turretController.GetTurretComponent())
					{
						isInAds = turretController.IsWeaponADS();
						return true;
					}
				}
			}
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override int CameraSelector()
	{
		m_AnimationComponent.GetMovementState(m_CharMovementState);
		
		#ifdef ENABLE_DIAG 
		if (IsDebugView())
		{
			return CharacterCameraSet.CHARACTERCAMERA_DEBUG;
		}
		#endif

		if (!IsInThirdPerson())
		{
			if (m_ControllerComponent.IsDead())
				return CharacterCameraSet.CHARACTERCAMERA_1ST_BONE_TRANSFORM;
		
			if (m_ControllerComponent.IsUnconscious())
				return CharacterCameraSet.CHARACTERCAMERA_1ST_UNCONSCIOUS;
		}

		if (SCR_BinocularsComponent.IsZoomedView())
		{
			return CharacterCameraSet.CHARACTERCAMERA_BINOCULARS;
		}
		
		//! game camera selection
		if (m_ControllerComponent.IsWeaponADS() || (m_ControllerComponent.GetInputContext().IsWeaponADS() && m_ControllerComponent.IsChangingStance()))
		{
			return CharacterCameraSet.CHARACTERCAMERA_ADS;
		}
		
		if (m_ControllerComponent.IsGadgetRaisedModeWanted() && !m_OwnerCharacter.IsInVehicle())
			return CharacterCameraSet.CHARACTERCAMERA_1ST;

		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		
		if (m_ControllerComponent.GetInspect() && !m_OwnerCharacter.IsInVehicle())
		{
			return CharacterCameraSet.CHARACTERCAMERA_1ST;
		}
		
		bool isRolling = false;
		if (m_ControllerComponent.IsRoll() && m_CmdHandler)
		{
			CharacterCommandMove moveCmd = m_CmdHandler.GetCommandMove();
			//! we need to stabilize camera before roll action is complete, so by that time player is ready to aim at target
			if (moveCmd && !moveCmd.IsBlendingOutRoll())
				isRolling = true;
		}
		
		if (IsInThirdPerson())
		{
			if( m_OwnerCharacter.IsInVehicle() && compartmentAccess && !compartmentAccess.IsGettingOut() )
			{
				bool isTurretAds = false;
				bool inTurret = CheckIsInTurret(isTurretAds);
				if (inTurret)
				{
					if (isTurretAds)
					{
						return CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE;
					}
					return CharacterCameraSet.CHARACTERCAMERA_3RD_TURRET;
				}
				
				if (m_ControllerComponent.GetInspect())
				{
					return CharacterCameraSet.CHARACTERCAMERA_1ST_VEHICLE;
				}

				return CharacterCameraSet.CHARACTERCAMERA_3RD_VEHICLE;
			}
			
			if (compartmentAccess && (compartmentAccess.IsGettingOut() || compartmentAccess.IsGettingIn()))
			{
				return CharacterCameraSet.CHARACTERCAMERA_3RD_ERC;
			}

			if( m_CharMovementState.m_CommandTypeId == ECharacterCommandIDs.CLIMB )
				return CharacterCameraSet.CHARACTERCAMERA_3RD_CLIMB;
			
			if (m_ControllerComponent.GetScrInputContext().m_iLoiteringType == 1)
			{
				SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(m_CmdHandler);
				if (scrCmdHandler && scrCmdHandler.IsLoitering())
					return CharacterCameraSet.CHARACTERCAMERA_3RD_SITTING;
			}
			
			if( m_CharMovementState.m_iStanceIdx == ECharacterStance.PRONE )
			{
				if (ShouldForceFirstPersonInThirdPerson(m_ControllerComponent))
				{
					if (isRolling)
						return CharacterCameraSet.CHARACTERCAMERA_1ST_BONE_TRANSFORM;
					
					return CharacterCameraSet.CHARACTERCAMERA_1ST;
				}
				
				return CharacterCameraSet.CHARACTERCAMERA_3RD_PRO;
			}
			else if( m_CharMovementState.m_iStanceIdx == ECharacterStance.CROUCH )
			{
				if (ShouldForceFirstPersonInThirdPerson(m_ControllerComponent))
					return CharacterCameraSet.CHARACTERCAMERA_1ST;
				
				return CharacterCameraSet.CHARACTERCAMERA_3RD_CRO;
			}
			
			if (ShouldForceFirstPersonInThirdPerson(m_ControllerComponent))
					return CharacterCameraSet.CHARACTERCAMERA_1ST;
			
			return CharacterCameraSet.CHARACTERCAMERA_3RD_ERC;
		}
		else if (compartmentAccess && (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut()))
		{
			return CharacterCameraSet.CHARACTERCAMERA_1ST_VEHICLE_TRANSITION;
		}
		else if( m_OwnerCharacter.IsInVehicle() )
		{
			bool isTurretAds = false;
			bool inTurret = CheckIsInTurret(isTurretAds);
			if (inTurret)
			{
				if (isTurretAds)
				{
					return CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE;
				}
				return CharacterCameraSet.CHARACTERCAMERA_1ST_TURRET;
			}
			else if( CheckVehicleADS() )
					return CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE;

			return CharacterCameraSet.CHARACTERCAMERA_1ST_VEHICLE;
		}
		//! Setup here all conditions where we want to use camera transformation from bone directly
		//! Roll case
		else if (isRolling)
		{
			return CharacterCameraSet.CHARACTERCAMERA_1ST_BONE_TRANSFORM;
		}
		return CharacterCameraSet.CHARACTERCAMERA_1ST;
	}
	
	protected bool ShouldForceFirstPersonInThirdPerson(CharacterControllerComponent controller)
	{
		if (!m_ControllerComponent.GetWeaponADSInput())
			return false;
		
		if (m_ControllerComponent.IsSprinting() || m_ControllerComponent.IsSwimming())
			return false;
		
		return true;
	}
	
	protected float m_fADSProgress;
	protected float m_fADSTime;
	
	override float GetCameraTransitionTime(int pFrom, int pTo)
	{
		float transTime = 0.4;
		
		if (pFrom == CharacterCameraSet.CHARACTERCAMERA_3RD_ERC && pTo == CharacterCameraSet.CHARACTERCAMERA_3RD_PRO
		 || pFrom == CharacterCameraSet.CHARACTERCAMERA_3RD_PRO && pTo == CharacterCameraSet.CHARACTERCAMERA_3RD_ERC)
			transTime = 1.4;
		else if (pFrom == CharacterCameraSet.CHARACTERCAMERA_3RD_CRO && pTo == CharacterCameraSet.CHARACTERCAMERA_3RD_PRO
			  || pFrom == CharacterCameraSet.CHARACTERCAMERA_3RD_PRO && pTo == CharacterCameraSet.CHARACTERCAMERA_3RD_CRO)
			transTime = 0.8;
		else if (pFrom == CharacterCameraSet.CHARACTERCAMERA_DEBUG || pTo == CharacterCameraSet.CHARACTERCAMERA_DEBUG)
			transTime = 0.0;
		else if (pTo == CharacterCameraSet.CHARACTERCAMERA_ADS || pTo == CharacterCameraSet.CHARACTERCAMERA_1ST_READY)
		{
			if (m_ControllerComponent)
			{
				m_fADSTime = m_ControllerComponent.GetADSTime();
				if (m_fADSProgress > 0)
					transTime = m_fADSTime * (1 - m_fADSProgress);
				else
					transTime = m_fADSTime;
			}
		}
		else if (pFrom == CharacterCameraSet.CHARACTERCAMERA_ADS)
		{
			transTime = m_fADSTime * m_fADSProgress;
		}
		else if (pTo == CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE)
		{
			CompartmentAccessComponent compAcc = m_OwnerCharacter.GetCompartmentAccessComponent();
			if (compAcc && compAcc.IsInCompartment())
			{
				BaseCompartmentSlot compartment = compAcc.GetCompartment();
				if (compartment)
				{
					BaseControllerComponent controller = compartment.GetController();
					if (controller)
					{
						TurretControllerComponent turretController = TurretControllerComponent.Cast(controller);
						if (turretController)
						{
							m_fADSTime = turretController.GetADSTime();
							if (m_fADSProgress > 0)
								transTime = m_fADSTime * (1 - m_fADSProgress);
							else
								transTime = m_fADSTime;
						}
					}
				}
			}
		}
		else if ((pFrom == CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE/* || pFrom == CharacterCameraSet.CHARACTERCAMERA_1ST_TURRET*/) && pTo == CharacterCameraSet.CHARACTERCAMERA_1ST_READY)
			transTime = 0.0;
		else if ((pFrom == CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE/* || pFrom == CharacterCameraSet.CHARACTERCAMERA_3RD_TURRET*/) && pTo == CharacterCameraSet.CHARACTERCAMERA_3RD_ERC)
			transTime = 0.0;
		else if (pFrom == CharacterCameraSet.CHARACTERCAMERA_1ST_VEHICLE_TRANSITION || pTo == CharacterCameraSet.CHARACTERCAMERA_1ST_VEHICLE_TRANSITION)
		{
			transTime = 0.8;
		}
		else
			transTime = GetCameraSet().GetTransitionTime(pFrom, pTo);

		return transTime;
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnThirdPersonSwitch(bool isInThirdPerson)
	{
		if (m_AnimationComponent)
		{
			if (isInThirdPerson)
				m_AnimationComponent.SetAnimationLayerTPP();
			else
				m_AnimationComponent.SetAnimationLayerFPP();
		}
		
		m_OnThirdPersonSwitch.Invoke();		
	}
	
	ScriptInvoker GetThirdPersonSwitchInvoker()
	{
		return m_OnThirdPersonSwitch;
	}
	
	// Empirical multiplier for focus deceleration
	protected const float FOCUS_DECELERATION_RATIO = 4.5;
	protected const float FOCUS_DEFAULT_ADS_TIME = 0.35;
	protected const float FOCUS_ADS_TIME_RATIO = 0.75;
	
	[Attribute("0.5", UIWidgets.Slider, "Focus interpolation time", "0 10 0.05")]
	protected float m_fFocusTime;
	
	[Attribute("0.9", UIWidgets.Slider, "Focus interpolation deceleration", "0 1 0.05")]
	protected float m_fFocusDeceleration;

	protected bool m_bDoInterpolateFocus;
	protected float m_fFocusTargetValue;
	protected float m_fFocusValue;
	protected float m_fAutoFocusProgress;
	
	//------------------------------------------------------------------------------------------------
	override void OnBeforeCameraUpdate(float pDt, bool pIsKeyframe)
	{
		m_fCameraDistanceFilter = Math.SmoothCD(m_fCameraDistanceFilter, m_fTargetDistance, m_fCameraDistanceFilterVel, 0.3, 1000, pDt);
		
		m_fCameraSlideFilter = Math.SmoothCD(m_fCameraSlideFilter, m_fTargetSlide, m_fCameraSlideFilterVelocity, m_fSlideTime, 1000, pDt);
		float currDiff = Math.AbsFloat(m_fTargetSlide - m_fCameraSlideFilter);
		if (currDiff <= sm_fDistanceEpsilon)
			m_fCameraSlideFilter = m_fTargetSlide;
		
		m_InputManager.ActivateContext("PlayerCameraContext");

		bool isADSAllowed = m_CmdHandler && m_CmdHandler.IsWeaponADSAllowed(false);
		bool isWeaponADS = isADSAllowed && m_ControllerComponent.GetWeaponADSInput() && !m_ControllerComponent.IsReloading();
		if (!isWeaponADS)
			CheckIsInTurret(isWeaponADS);

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());

		if (!isWeaponADS && playerController)
			isWeaponADS = playerController.GetGadgetFocus();

		// Gadgets do not have ADS time defined
		float adsTime = m_fADSTime;
		if (adsTime <= 0)
			adsTime = FOCUS_DEFAULT_ADS_TIME;

		float adsChange = pDt / adsTime;
		float autoFocusChange = adsChange / FOCUS_ADS_TIME_RATIO;

		if (isWeaponADS)
		{
			if (m_fADSProgress < 1)
				m_fADSProgress = Math.Min(1, m_fADSProgress + adsChange);

			if (m_fAutoFocusProgress < 1)
				m_fAutoFocusProgress = Math.Min(1, m_fAutoFocusProgress + autoFocusChange);
		}
		else
		{
			if (m_fADSProgress > 0)
				m_fADSProgress = Math.Max(0, m_fADSProgress - adsChange);

			if (m_fAutoFocusProgress > 0)
				m_fAutoFocusProgress = Math.Max(0, m_fAutoFocusProgress - autoFocusChange);
		}

		float cameraFocus;
		if (playerController && !m_ControllerComponent.GetDisableViewControls())
			cameraFocus = playerController.GetFocusValue(m_fAutoFocusProgress, pDt);

		if (!float.AlmostEqual(m_fFocusTargetValue, cameraFocus))
		{
			m_fFocusTargetValue = cameraFocus;
			m_bDoInterpolateFocus = true;
		}

		if (pIsKeyframe)
			UpdateViewBob(pDt);

		// When ADS in third person, the fpv view is not necesarilly toggled, but oddly enforced,
		// so the camera never activates/deactivates, thus bleeding values in
		if (IsInThirdPerson() && m_ControllerComponent && m_ControllerComponent.GetInputContext().GetDie())
			OnAlphatestChange(0);
		
		if (!m_bDoInterpolateFocus)
			return;
		
		float absDelta = 0;
		
		// Only care about transition if it takes time
		if (m_fFocusTime > 0)
		{
			absDelta = Math.AbsFloat(m_fFocusTargetValue - m_fFocusValue);
			
			// Scale the focusChange by absDelta to smooth out the transition
			float focusDeceleration = FOCUS_DECELERATION_RATIO * absDelta * m_fFocusDeceleration;
			
			// Linear component needs to be squared so that focus is 99% correct after m_fFocusTime
			float focusSpeed = (1 - m_fFocusDeceleration) * (1 - m_fFocusDeceleration) + focusDeceleration;
			float focusChange = pDt * focusSpeed / m_fFocusTime;
			
			if (m_fFocusTargetValue > m_fFocusValue)
				m_fFocusValue += focusChange;
			else
				m_fFocusValue -= focusChange;
			
			m_fFocusValue = Math.Clamp(m_fFocusValue, 0, 1);
		}
		
		if (float.AlmostEqual(absDelta, 0))
		{
			m_fFocusValue = m_fFocusTargetValue;
			m_bDoInterpolateFocus = false;
		}
		
		SetFocusMode(m_fFocusValue);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4], inout vector transformWS[4])
	{
		//! update head visibility
		vector headBoneMat[4];
		m_OwnerCharacter.GetAnimation().GetBoneMatrix(m_iHeadBoneIndex, headBoneMat);
		vector charMat[4];
		m_OwnerCharacter.GetWorldTransform(charMat);
		Math3D.MatrixMultiply4(charMat, headBoneMat, headBoneMat);
		OnAlphatestChange(255 - Math.Clamp((vector.Distance(transformWS[3], headBoneMat[3]) - 0.2) / 0.15, 0.0, 1.0)*255);
		
		if (m_ControllerComponent.IsDead())
			return;
		
		//! aiming update
		if( pIsKeyframe )
			UpdateAiming(transformMS);
		
		if (GetCurrentCamera())
			GetCurrentCamera().OnAfterCameraUpdate(pDt, pIsKeyframe, transformMS);
		
		//! update recoil and generic camera shake
		UpdateShake(pDt);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateShake(float pDt)
	{
		// Update and apply recoil based camera shake
		if (m_pRecoilShake && !m_pRecoilShake.IsFinished())
			m_pRecoilShake.Update(m_OwnerCharacter, pDt);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddShakeToToTransform(inout vector transform[4], inout float fieldOfView)
	{
		bool applyRecoilShake = true;
		#ifdef ENABLE_DIAG
		applyRecoilShake = !DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_RECOIL_CAMERASHAKE_DISABLE);
		#endif
		
		// Apply recoil shake
		if (applyRecoilShake && m_pRecoilShake)
			m_pRecoilShake.Apply(transform, fieldOfView);
		
		// Apply generic shakes from shake manager to out transform
		SCR_CameraShakeManagerComponent.ApplyCameraShake(transform, fieldOfView);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCameraSwitchPressed()
	{
		bool current = IsInThirdPerson();
		SetThirdPerson(!current);
	}
	
	//------------------------------------------------------------------------------------------------
	override void CollisionSolver(float pDt, inout ScriptedCameraItemResult pOutResult, inout vector resCamTM[4], bool isKeyframe)
	{
		// In case of changes, test:
		//  Freelook vs. collisions, heading vs. collisions
		//  Turrets - transitions between FPV, 3PV, ADS; collisions in ADS
		//  Jumping and collisions
		//  Collisions when close to a wall under a sharp angle, collisions when between two vehicles (<1m gap)
		//  Smoothness of transitions and slide/distance smoothing
		
		if (pOutResult.m_fDistance == 0.0)
		{
			m_fCameraDistanceFilter = 0.0;
			ResetSlide();
		}
		
		IEntity owner = pOutResult.m_pOwner;
		vector ownerTransformWS[4];
		if (owner)
			owner.GetWorldTransform(ownerTransformWS);
		else
			Math3D.MatrixIdentity4(ownerTransformWS);
		
		vector resultWorldTransform[4]; // If pOutResult.m_CameraTM is in someone's model space, this transforms it back to world space.
		if (pOutResult.m_pWSAttachmentReference)
			pOutResult.m_pWSAttachmentReference.GetOwner().GetWorldTransform(resultWorldTransform);
		else if (owner)
			owner.GetWorldTransform(resultWorldTransform);
		else
			Math3D.MatrixIdentity4(resultWorldTransform);
		
		vector hipBoneWS[4];
		m_OwnerCharacter.GetAnimation().GetBoneMatrix(m_iHipsBoneIndex, hipBoneWS); 
		Math3D.MatrixMultiply4(resultWorldTransform, hipBoneWS, hipBoneWS);

		#ifdef ENABLE_DIAG // These things are only useful for showing the debug spheres.
		vector camPos = pOutResult.m_CameraTM[3];
		vector inputCameraPosWS = camPos.Multiply4(resultWorldTransform);
		#endif
		
		vector camTransformLS[4];
		Math3D.MatrixCopy(pOutResult.m_CameraTM, camTransformLS);
		
		if (pOutResult.m_pWSAttachmentReference)
		{
			// If pOutResult.m_pWSAttachmentReference has value, the matrix was supplied in PointInfo space. Here we transform
			// it to model's space (and can then use resultWorldTransform to translate into world space).
			owner = pOutResult.m_pWSAttachmentReference.GetOwner();
			vector wsAttachmentWorldTransform[4];
			pOutResult.m_pWSAttachmentReference.GetModelTransform(wsAttachmentWorldTransform);
			Math3D.MatrixMultiply4(wsAttachmentWorldTransform, camTransformLS, camTransformLS);
		}
		
		vector basePos = camTransformLS[3]; // Camera position without heading applied.
		
		vector charRot[4];	// Matrix to transform from model space to heading space.
		Math3D.MatrixIdentity4(charRot);
		
		if (pOutResult.m_fPositionModelSpace == 2.0) // 2.0 is world space.
		{
			camTransformLS[3] = camTransformLS[3] - resultWorldTransform[3];
		}
		else
		{
			if (pOutResult.m_fUseHeading > 0.0)
			{
				vector headingMat[4];
				Math3D.AnglesToMatrix(Vector( Math.RAD2DEG * -pOutResult.m_fHeading, 0.0, 0.0), headingMat);
				
				if (pOutResult.m_pOwner)
					pOutResult.m_pOwner.GetLocalTransform(charRot);
				
				charRot[3] = vector.Zero;
				
				Math3D.MatrixInvMultiply4(charRot, headingMat, charRot);
				float charRotQ[4];
				Math3D.MatrixToQuat(charRot, charRotQ);
				float quatF[4] = {0.0, 0.0, 0.0, 1.0};
				Math3D.QuatLerp(charRotQ, quatF, charRotQ, pOutResult.m_fUseHeading);
				vector charRotQAngles = Math3D.QuatToAngles(charRotQ);
				Math3D.AnglesToMatrix(charRotQAngles, charRot);
				
				Math3D.MatrixMultiply4(charRot, camTransformLS, camTransformLS);
				
				if (pOutResult.m_fDistance == 0.0)
					camTransformLS[3] = basePos;
			}
			
			if (pOutResult.m_fPositionModelSpace > 0.0)
			{
				// We are blending between heading (0.0) and local space (1.0) - for instance between turret ADS and 3PV.
				vector msPos = camTransformLS[3];
				vector lsPos = basePos;
				camTransformLS[3] = vector.Lerp(lsPos, msPos, pOutResult.m_fPositionModelSpace);
			}
		}
		
		// Space and heading transformations are done. We transform them into WS to use with collision tracing.
		vector camTransformWS[4];
		if (pOutResult.m_fPositionModelSpace == 2.0)
			Math3D.MatrixCopy(camTransformLS, camTransformWS);
		else
			Math3D.MatrixMultiply4(resultWorldTransform, camTransformLS, camTransformWS);
		
		if (pOutResult.m_bAllowCollisionSolver && !isKeyframe && pOutResult.m_fDistance > 0.0)
		{
			// We always make the traces a bit longer than they need to be and "anticipate" if it is going to collide.
			// This way we can make the camera movement a bit smoother. This number says by how much they will be longer.
			float SIDE_TRACE_MARGIN_MULTIPLIER = 2.0;
			float BACK_TRACE_MARGIN_MULTIPLIER = 1.25;
			
			// Exclude all character-related entities from any tracing.
			array<IEntity> excludeArray = {m_OwnerCharacter};
			IEntity parent = m_OwnerCharacter.GetParent();
			IEntity sibling = m_OwnerCharacter.GetSibling();
			while (parent != null)
			{
				excludeArray.Insert(parent);
				while (sibling != null)
				{
					excludeArray.Insert(sibling);
					sibling = sibling.GetSibling();
				}
				sibling = parent.GetSibling();
				parent = parent.GetParent();
			}
			
			BaseWorld world = m_OwnerCharacter.GetWorld();
			autoptr TraceSphere param = new TraceSphere;
			param.Radius = 0.06;
			param.ExcludeArray = excludeArray;
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			param.LayerMask = TRACE_LAYER_CAMERA;
			
			// 1. Sideways collision (translating the camera to left/right from character).
			vector sideTraceStart = Vector(hipBoneWS[3][0], camTransformWS[3][1], hipBoneWS[3][2]);
			vector sideTraceEnd = camTransformWS[3];
			
			vector sideTraceDiff = sideTraceEnd - sideTraceStart;
			
			// Set up the side trace, we also limit the Y position of the trace end - otherwise, if the character jumps and
			// the margin is high, the trace will point downwards.
			param.Start = sideTraceStart;
			param.End = sideTraceStart + SIDE_TRACE_MARGIN_MULTIPLIER * sideTraceDiff; 
			param.End[1] = sideTraceStart[1] + sideTraceDiff[1];
	
			float rightTrace = world.TraceMove(param, null);
			
			// The slide we apply is not divided by the margin multiplier, since we want slide to move
			// in anticipation of hitting something. We still pass the multiplier here though,
			// so that we can change the speed of the smoothing based on how close the tracing hit.
			ApplySmoothedSlide(pDt, rightTrace, sideTraceStart, sideTraceDiff, SIDE_TRACE_MARGIN_MULTIPLIER, pOutResult.m_fShoulderDist, camTransformWS);
			#ifdef ENABLE_DIAG
			sideTraceEnd = vector.Lerp(param.Start, param.End, rightTrace);
			#endif
			
			// Slide is applied to the camera matrix, but distance is not.
			// So we apply it to the camTransformLS which we will be returning at the end.
			Math3D.MatrixInvMultiply4(resultWorldTransform, camTransformWS, camTransformLS);
			
			// 2. Backwards collision. Vars are defined outside the scope because of the debug spheres.
			float backTrace = 1;
			vector backTraceStart = camTransformWS[3];
			vector backTraceEnd = camTransformWS[3] - BACK_TRACE_MARGIN_MULTIPLIER * pOutResult.m_fDistance * camTransformWS[2];
			if (pOutResult.m_fPositionModelSpace <= 2.0 && pOutResult.m_fDistance > 0.0)
			{
				param.Start = backTraceStart;
				param.End = backTraceEnd;
				
				backTrace = world.TraceMove(param, null);
				// Unlike with slide, distance will not move "in anticipation", so we can directly multiply this value and compare it to 1.
				// We still want to have it a bit longer though, to cover the param.Radius margin.
				backTrace = BACK_TRACE_MARGIN_MULTIPLIER * backTrace;
				
				float newDistance = pOutResult.m_fDistance * Math.Min(1.0, backTrace);
				
				if (newDistance < sm_fDistanceEpsilon)
					newDistance = 0.0;
				
				if (Math.AbsFloat(m_fCameraDistanceFilter - newDistance) < sm_fDistanceEpsilon && m_fCameraDistanceFilterVel != 0.0)
					m_fCameraDistanceFilterVel = 0.0;
				
				// Set distance smoothing.
				m_fTargetDistance = newDistance;
				if(IsCameraBlending() || newDistance < m_fCameraDistanceFilter)
					m_fCameraDistanceFilter = newDistance;
				
				pOutResult.m_fDistance = m_fCameraDistanceFilter;
			}
				
			#ifdef ENABLE_DIAG
			if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_DRAW_CAMERA_COLLISION_SOLVER))
			{
				int baseColor = ARGB(255, 0, 110, 255);
				ShapeFlags shFlags = ShapeFlags.WIREFRAME|ShapeFlags.ONCE;
				
				Shape.CreateSphere(ARGB(255, 255, 255, 255), shFlags, inputCameraPosWS, param.Radius);								// WHITE
				
				Shape.CreateSphere(ARGB(255, 0, 255, 0), shFlags, sideTraceStart, param.Radius);									// GREEN
				Shape.CreateSphere(ARGB(255, 255, 255 * rightTrace, 0), shFlags, sideTraceEnd, param.Radius);						// YELLOW - RED
		
				Shape.CreateSphere(ARGB(255, 150, 0, 255), shFlags, backTraceStart, param.Radius * 0.9);							// PURPLE
				Shape.CreateSphere(ARGB(255, 0, 255, 255), shFlags, vector.Lerp(backTraceStart, backTraceEnd, 0.25), param.Radius * 0.35);	// TEAL
				if (backTrace < 1)
					Shape.CreateSphere(ARGB(255, 255, 255 * backTrace, 0), shFlags, backTraceStart + (backTraceEnd - backTraceStart).Normalized() * (m_fCameraDistanceFilter - param.Radius), param.Radius * (1 - backTrace));
			}
			#endif
		}
			
		// Return the camera position in local space with applied slide, distance is applied only if camera is in world space.
		Math3D.MatrixCopy(camTransformLS, resCamTM);
		
		if (pOutResult.m_fPositionModelSpace < 2.0 && pOutResult.m_fDistance > 0.0)
		{
			resCamTM[3] = resCamTM[3] - pOutResult.m_fDistance * resCamTM[2];
		}
	}

	//------------------------------------------------------------------------------------------------
	private void UpdateViewBob(float pDt)
	{
		int stance = m_ControllerComponent.GetStance();
		
		// Adjust bob from freelook
		float freeLookOffsetScale = 0;
		if (m_ControllerComponent.IsFreeLookEnabled() || m_ControllerComponent.IsTrackIREnabled())
			freeLookOffsetScale = 1;
		m_fBobFreelokFilter = Math.SmoothCD(m_fBobFreelokFilter, 1 - Math.Pow(1 - freeLookOffsetScale, 3), m_fBobFreelokFilterVel, 0.3, 1000, pDt);
		freeLookOffsetScale = m_fBobFreelokFilter;
		
		// Add bobbing based on speed
		m_fBob_ScaleFast = Math.Clamp(m_ControllerComponent.GetVelocity().Length() / Math.Max(m_AnimationComponent.GetTopSpeed(-1, false), 0.1), 0, 1);
		m_fBob_ScaleSlow += (m_fBob_ScaleFast - m_fBob_ScaleSlow) * (pDt * 4);
		
		m_fBob_ScaleFast = Math.Pow(m_fBob_ScaleSlow, 3);
		m_fBob_ScaleFast = Math.Clamp(m_fBob_ScaleFast + freeLookOffsetScale * m_fBob_ScaleFast, 0, 1);
		if (m_fBob_ScaleFast < 0.01)
		{
			m_fBob_Up *= 1 - pDt;
			m_fBob_Right *= 1 - pDt;
		}
		else
		{
			float stanceBobScale = 0.5;
			if (stance == ECharacterStance.PRONE)
				stanceBobScale = 0.25;
			else if (stance == ECharacterStance.CROUCH)
				stanceBobScale = 0.55;
			m_fBob_Up += Math.Clamp(pDt * 2 * stanceBobScale * m_fBob_ScaleFast, 0, 1);
			if (m_fBob_Up >= 1)
				m_fBob_Up -= 1;
			m_fBob_Right += Math.Clamp(pDt * 2.5 * stanceBobScale * m_fBob_ScaleFast, 0, 1);
			if (m_fBob_Right >= 1)
				m_fBob_Right -= 1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	float AddViewBobToTransform(inout vector pOutTransform[4], float pBobScale, bool pAllowTranslation)
	{
		// Add bob
		float bobMoveScale = (m_fBob_ScaleFast * 0.8 + 0.2) * pBobScale;
		if( pAllowTranslation )
		{
			float freeLookOffsetScale = m_fBobFreelokFilter;
			float bobFw = m_fBob_ScaleSlow * -0.5 * m_fBob_ScaleFast * (1 - freeLookOffsetScale);
			float bobUp = Math.Sin(m_fBob_Up * 360 * Math.DEG2RAD) * 0.02 * bobMoveScale;
			float bobRt = Math.Sin(m_fBob_Right * 360 * Math.DEG2RAD) * 0.015 * bobMoveScale;
			bobRt += m_fBob_ScaleSlow * -0.2 * m_fBob_ScaleFast;
			vector bobTranslation = pOutTransform[0] * bobRt + pOutTransform[3];
			bobTranslation = pOutTransform[1] * bobUp + bobTranslation;
			pOutTransform[3] = pOutTransform[2] * bobFw + bobTranslation;
		}
		vector angBob = vector.Zero;
		angBob[0] = Math.Sin(m_fBob_Up * 360 * Math.DEG2RAD) * 0.3 * bobMoveScale;		// YAW		< >
		angBob[1] = Math.Sin(m_fBob_Right * 360 * Math.DEG2RAD) * 0.3 * bobMoveScale;	// PITCH	^ v
		// ROLL REMOVED, AS ITS ONLY PERCEIVABLE EFFECT WAS THAT IT CAUSES STRONG HEAD BOB WHEN LOOKING DIRECTLY AT GROUND
		vector angBobMat[3], endBobmat[3];
		Math3D.AnglesToMatrix(angBob * pBobScale, angBobMat);
		Math3D.MatrixMultiply3(angBobMat, pOutTransform, endBobmat);
		pOutTransform[0] = endBobmat[0];
		pOutTransform[1] = endBobmat[1];
		pOutTransform[2] = endBobmat[2];
		
		return m_fBob_ScaleFast;
	}
	
	//------------------------------------------------------------------------------------------------
	private void UpdateAiming(vector transformMS[4])
	{
		return;
		/*
		vector characterMat[4];
		m_OwnerCharacter.GetWorldTransform(characterMat);
		
		if( Is3rdPersonView() )
		{
			vector dirLS = transformMS[2];
			vector dirWS = dirLS.Multiply3(characterMat);
			vector aimposLS = transformMS[3];
			vector aimposWS = aimposLS.Multiply4(characterMat);
			
			// Trace aiming for 3rd person
			autoptr TraceParam param = new TraceParam;
			param.Start = aimposWS + dirWS * 3;
			param.End = param.Start + dirWS * 100;
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			param.Exclude = m_OwnerCharacter;
			param.LayerMask = TRACE_LAYER_CAMERA;
			vector aimpos = param.End;
			
			float traced = m_OwnerCharacter.GetWorld().TraceMove(param, null);
			if (traced < 1)
				aimpos = (param.End - param.Start) * traced + param.Start;
			
			m_ControllerComponent.SetLookAtPosition(aimpos);
			if (m_ControllerComponent.IsFreeLookEnabled() || m_ControllerComponent.IsWeaponObstructed())
				m_ControllerComponent.SetAimPosition(vector.Zero);
			else
				m_ControllerComponent.SetAimPosition(aimpos);
		}
		else
		{
			m_ControllerComponent.SetAimPosition(vector.Zero);
			m_ControllerComponent.SetLookAtPosition(vector.Zero);
		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	bool Is3rdPersonView()
	{
		return !IsDebugView() && IsInThirdPerson() && !(m_ControllerComponent && m_ControllerComponent.IsWeaponADS());
	}
	

	//------------------------------------------------------------------------------------------------
	private SCR_ChimeraCharacter m_OwnerCharacter;
	private SCR_CharacterControllerComponent m_ControllerComponent;
	private CharacterAnimationComponent m_AnimationComponent;
	private InputManager m_InputManager;
	private EquipedLoadoutStorageComponent m_LoadoutStorage;	
	private CharacterIdentityComponent m_IdentityComponent;
	private CharacterCommandHandlerComponent m_CmdHandler;
	
	static private float sm_fDistanceEpsilon = 0.0001;
	
	private bool	m_bApplySmoothedSlideThisFrame = true;
	private float	m_fTargetSlide = 1;
	private float	m_fSlideTime = 0.4;	
	private float 	m_fCameraSlideFilter = 0;
	private float 	m_fCameraSlideFilterVelocity = 0;
	private float 	m_fChangedTimer = 0;
	private float 	m_fSlideTimeout = 0.4;
	private float	m_fBob_Up = 0;
	private float	m_fBob_Right = 0;
	private float	m_fBob_ScaleSlow = 0;
	private float	m_fBob_ScaleFast = 0;
	private int		m_iHeadBoneIndex = -1;
	private float	m_fTargetDistance = 0.0;	
	private float	m_fCameraDistanceFilter;	
	private float	m_fCameraDistanceFilterVel;
	private float	m_fBobFreelokFilter;	
	private float	m_fBobFreelokFilterVel;	
	private bool 	m_bPrevSlideDirectionInside = false;
	private bool	m_b3rdCollision_UseLeftShoulder = false;

	private ref CharacterMovementState m_CharMovementState = new CharacterMovementState();

	private void ResetSlide()
	{
		m_fCameraSlideFilter = 1;
		m_fTargetSlide = 1;
		m_fCameraSlideFilterVelocity = 0;
		m_fChangedTimer = 0;
	}
	
	private bool ApplySmoothedSlide(float pDt, float slide, vector sideTraceStart, vector sideTraceDiff, float sideTraceMarginMultiplier, float shoulderDist, inout vector inoutMat[4])
	{
		// Slide will only be enabled in one branch.
		m_bApplySmoothedSlideThisFrame = false;
		
		bool slideInside = Math.AbsFloat(slide) < Math.AbsFloat(m_fCameraSlideFilter);
		// Apply timer only when direction of slide is changed
		if (m_bPrevSlideDirectionInside && !slideInside)
		{
			m_fChangedTimer = m_fSlideTimeout;
		}
		m_bPrevSlideDirectionInside = slideInside;
		
		if (m_fChangedTimer > 0)
		{
			m_fChangedTimer = m_fChangedTimer - pDt;
		}
		// No smoothing while we are switching shoulders.
		if (Math.AbsFloat(shoulderDist) < 0.98 && m_fCameraSlideFilter > slide)
		{
			m_fCameraSlideFilter = slide;
			m_fCameraSlideFilterVelocity = 0;
			inoutMat[3] = sideTraceStart + sideTraceDiff * m_fCameraSlideFilter;
			return slideInside;
		}
		// Hard limit for the maximum slide to the side is the trace distance.
		if (slideInside && m_fCameraSlideFilter > slide * sideTraceMarginMultiplier)
		{
			m_fCameraSlideFilter = slide;
			m_fCameraSlideFilterVelocity = m_fSlideTimeout * (slide - m_fCameraSlideFilterVelocity);
		}
		else
		{
			if (IsCameraBlending())
			{
				m_fChangedTimer = 0;
			}
			if (m_fChangedTimer <= 0 && m_fCameraSlideFilter != slide)
			{
				float slideTime = m_fSlideTimeout;
				
				// The aim of this is to increase the speed of adjusting to the target if current slide puts
				// the camera too close to the wall (so that we don't reach the above hard limit as easily).
				if (slideInside)
				{
					float slideT = sideTraceMarginMultiplier * ((m_fCameraSlideFilter)/Math.Max(0.01, slide * sideTraceMarginMultiplier) - (1/sideTraceMarginMultiplier));
					slideTime = Math.Lerp(0.1, m_fSlideTimeout, Math.Clamp(slideT, 0, 1));
				}
				
				// Set up data for slide smoothing for next frame.
				m_bApplySmoothedSlideThisFrame = true;
				m_fTargetSlide = slide;
				m_fSlideTime = slideTime;
			}
		}
		
		inoutMat[3] = sideTraceStart + sideTraceDiff * m_fCameraSlideFilter;
		return slideInside;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CanUseOverlayCameraFOV()
	{
		if (m_ControllerComponent)
		{
			EWeaponObstructedState obstructedState = m_ControllerComponent.GetWeaponObstructedState();
			if (obstructedState >= EWeaponObstructedState.SLIGHTLY_OBSTRUCTED_CAN_FIRE)
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetOverlayCameraFOVScalarWeight()
	{
		if (!m_ControllerComponent)
			return 0.0;
		
		if (m_ControllerComponent.IsFreeLookEnabled())
			return 0.0;
		
		if (!CanUseOverlayCameraFOV())
			return 0.0;
		
		return 1.0;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_RecoilCameraShakeParams GetRecoilShakeParams()
	{
		return m_pRecoilShakeParams;
	}
	
	//------------------------------------------------------------------------------------------------
	static void SetOverlayCameraFOV(float fov)
	{
		s_fOverlayCameraFOV = fov;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override event float CalculateFovScalar(notnull CameraBase mainCamera, CameraBase overlayCamera)
	{
		float fov = mainCamera.GetVerticalFOV();
		if (overlayCamera && s_fOverlayCameraFOV > 0)
			fov = Math.Lerp(fov, s_fOverlayCameraFOV, GetOverlayCameraFOVScalarWeight());
		
		return fov / 90;
	}
};
