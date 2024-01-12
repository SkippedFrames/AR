// *************************************************************************************
// ! CharacterCameraADS - Aim down sights camera
// *************************************************************************************
class CharacterCameraADS extends CharacterCameraBase
{
	//------------------------------------------------------------------------------------------------
	static const float 	CONST_UD_MIN	= -89.0;		//!< down limit
	static const float 	CONST_UD_MAX	= 89.0;			//!< up limit

	static const float 	CONST_LR_MIN	= -160.0;		//!< left limit
	static const float 	CONST_LR_MAX	= 160.0;		//!< right limit

	static const float CONST_TRANSLATIONZ_MIN = -0.1; //! how far back can camera move with weapon

	#ifdef ENABLE_DIAG
	private static bool s_bDebugRegistered;
	#endif
	
	//------------------------------------------------------------------------------------------------
	void CharacterCameraADS(CameraHandlerComponent pCameraHandler)
	{
		m_iHandBoneIndex = m_OwnerCharacter.GetAnimation().GetBoneIndex("righthandprop");
		m_fADSToFPSDeg = 45;
		m_fFreelookFOV = GetBaseFOV();

		m_WeaponManager = BaseWeaponManagerComponent.Cast(m_OwnerCharacter.FindComponent(BaseWeaponManagerComponent));
		m_AimingComponent = CharacterAimingComponent.Cast(m_OwnerCharacter.FindComponent(CharacterAimingComponent));
		m_OffsetLS = "0.0 0.03 -0.07";

		#ifdef ENABLE_DIAG
		if (!s_bDebugRegistered) {
			DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_CHARACTER_ADS_CAMERA, "", "ADS Camera", "Character", "0, 3, 0, 1");
			s_bDebugRegistered = true;
		}
		#endif

		m_iJumpAnimTagId = GameAnimationUtils.RegisterAnimationTag("TagFall");
	}

	//------------------------------------------------------------------------------------------------
	override void OnDeactivate(ScriptedCameraItem pNextCamera)
	{
		OnBlendingOut(1);
	}

	//------------------------------------------------------------------------------------------------
	override void OnBlendIn()
	{
		if (m_CameraHandler.IsCameraBlending())
			return;

		OnBlendingIn(1);
	}

	//------------------------------------------------------------------------------------------------
	override void OnBlendOut()
	{
		if (m_CameraHandler.IsCameraBlending())
			return;

		OnBlendingOut(0);
	}

	//------------------------------------------------------------------------------------------------
	protected float GetSightsADSActivationPercentage() // percentage of blend <0,1>
	{
		if (m_LastWeaponComponent)
		{
			// Moved into BaseSightsComponent
			// TODO@HF Find out how much overhead switching between C++/Script has
			// Might want to check for ScriptedSightsComponent and use XXScript instead for performance
			BaseSightsComponent optics = m_LastWeaponComponent.GetSights();
			if (optics)
				return optics.GetADSActivationPercentage();
		}

		return 1.0;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetSightsADSDeactivationPercentage() // percentage of blend <0,1>
	{
		if (m_LastWeaponComponent)
		{
			// Moved into BaseSightsComponent
			// TODO@HF Find out how much overhead switching between C++/Script has
			// Might want to check for ScriptedSightsComponent and use XXScript instead for performance
			BaseSightsComponent optics = m_LastWeaponComponent.GetSights();
			if (optics)
				return optics.GetADSDeactivationPercentage();
		}

		return 0.0;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBlendingIn(float blendAlpha)
	{
		// The weapon may change at any moment
		BaseWeaponComponent currentWeapon = m_WeaponManager.GetCurrentWeapon();
		if (m_LastWeaponComponent && currentWeapon != m_LastWeaponComponent && m_LastWeaponComponent.IsSightADSActive())
			m_LastWeaponComponent.SightADSDeactivated();

		m_LastWeaponComponent = currentWeapon;

		if (blendAlpha < GetSightsADSActivationPercentage())
			return;

		if (currentWeapon && !currentWeapon.IsSightADSActive())
			currentWeapon.SightADSActivated();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBlendingOut(float blendAlpha)
	{
		// The weapon may change at any moment
		BaseWeaponComponent currentWeapon = m_WeaponManager.GetCurrentWeapon();
		if (m_LastWeaponComponent && currentWeapon != m_LastWeaponComponent && m_LastWeaponComponent.IsSightADSActive())
			m_LastWeaponComponent.SightADSDeactivated();

		m_LastWeaponComponent = currentWeapon;

		if (blendAlpha < GetSightsADSDeactivationPercentage())
			return;

		if (currentWeapon && currentWeapon.IsSightADSActive())
			currentWeapon.SightADSDeactivated();

		CameraManager cameraMgr = GetGame().GetCameraManager();
		if (cameraMgr)
			cameraMgr.SetOverlayCamera(null);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		if (pPrevCamera)
		{
			vector f = pPrevCamera.GetBaseAngles();
			m_fUpDownAngle = m_ControllerComponent.GetInputContext().GetAimingAngles()[1];
			m_fLeftRightAngle	= f[1];
		}

		m_fStabilizerAlpha = 0.0;
		m_fStabilizerAlphaVel = 0.0;
		m_lastStablePos = "0 0 0";
		
		// Reset entirety of sight blending data
		m_LastSightsComponent = null;
		m_bLastSightsBlend = false;
		m_fLastSightsBlendTime = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void SolveCameraHeadAttach(ADSCameraData cameraData, out ScriptedCameraItemResult pOutResult)
	{
		//! In some cases this compensation is needed (sloped surfaces)
		float parentPitch = m_OwnerCharacter.GetLocalYawPitchRoll()[1];
		cameraData.m_vLookAngles[1] = Math.Clamp(cameraData.m_vLookAngles[1] + parentPitch, CONST_UD_MIN, CONST_UD_MAX);

		vector headMatrix[4];
		m_OwnerCharacter.GetAnimation().GetBoneMatrix(GetCameraBoneIndex(), headMatrix);

		//! Sights transform relative to the head
		vector sightsRelativeMatrix[4];
		Math3D.MatrixInvMultiply4(headMatrix, cameraData.m_mSightsLocalMat, sightsRelativeMatrix);

		//! Fetch desired portion of recoil
		float recoilPortion;
		if (m_AimingComponent)
			recoilPortion = Math.Clamp(m_AimingComponent.GetRawAimingTranslation()[2] * cameraData.m_fCamRecoilAmount, -CAMERA_RECOIL_LIMIT, CAMERA_RECOIL_LIMIT);

		//! Local head plane offset by the recoil portion
		vector headPlane = cameraData.m_vSightsOffset;
		headPlane[2] = headPlane[2] - recoilPortion;

		//! Set matrix rotation
		Math3D.AnglesToMatrix(cameraData.m_vLookAngles, pOutResult.m_CameraTM);

		//! Project sights onto head plane
		vector resultPosition = SCR_Math3D.IntersectPlane(sightsRelativeMatrix[3], -sightsRelativeMatrix[2], headPlane, vector.Forward);

		//! Apply props
		pOutResult.m_CameraTM[3] = resultPosition;
		pOutResult.m_fFOV = cameraData.m_fFOV;
		pOutResult.m_fDistance = 0;
		pOutResult.m_fNearPlane = 0.0125;
		pOutResult.m_bAllowInterpolation = true;
		pOutResult.m_bUpdateWhenBlendOut = true;
		pOutResult.m_iDirectBone = GetCameraBoneIndex();
		pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativePosition;
	}

	/*!
		Stable ADS camera solver that used RightHandProp as camera root.
	*/
	protected void SolveNewMethod(ADSCameraData cameraData, out ScriptedCameraItemResult pOutResult, float pDt, bool allowInterpolation)
	{
		Animation anim = m_OwnerCharacter.GetAnimation();
		
		// Right hand prop is where weapon is attached to
		vector propBoneMS[4];
		anim.GetBoneMatrix(m_iHandBoneIndex, propBoneMS);

		// UPDATE:
		// This is quite silly considering what is done afterwards,
		// but the point is that we have to fight with zeroing, as it is
		// the only relevant aim modifier that we do not want to offset in
		// the camera like we do with recoil or sway
		vector zeroingLS[4];
		if (m_WeaponManager.GetCurrentWeapon() && m_WeaponManager.GetCurrentWeapon().GetCurrentSightsZeroingTransform(zeroingLS))
		{
			Math3D.MatrixMultiply4(cameraData.m_mSightsLocalMat, zeroingLS,cameraData.m_mSightsLocalMat);
		}

		// Get sights relative to right hand prop bone
		vector sightsMS[4];
		Math3D.MatrixInvMultiply4(propBoneMS, cameraData.m_mSightsLocalMat, sightsMS);
		
		
		float targetFOV = cameraData.m_fFOV;
		
		// Sights interpolation
		{
			// On sights change store stable last transform
			BaseSightsComponent currentSights = m_WeaponManager.GetCurrentSights();			
			
			// Just initialize to something in such case.
			if (!m_LastSightsComponent && currentSights)
			{
				Math3D.MatrixCopy(sightsMS, m_vLastSightMS);
				Math3D.MatrixCopy(sightsMS, m_vLastSightStMS);
				// And in such cases, we have no reasonable source, so just
				// disable all the blending
				m_LastSightsComponent = currentSights;
				m_bLastSightsBlend = false;
				m_fLastSightsBlendTime = 0;				
			}
			else if (m_LastSightsComponent != currentSights) // Use last stored transform if possible
			{
				Math3D.MatrixCopy(m_vLastSightMS, m_vLastSightStMS);
				m_fLastSightStFOV = m_fLastSightFOV;
				m_LastSightsComponent = currentSights;
				m_bLastSightsBlend = true;
				m_fLastSightsBlendTime = 0;
			}
			
			// Now during the blend, calculate alpha and apply some
			// arbitrary blend dark magic 
			if (m_bLastSightsBlend)
			{
				// No interpolation in such cases
				if (m_fLastSightsBlendDuration < 0)
				{
					m_fLastSightsBlendTime = 0;
					m_bLastSightsBlend = false;
				}
				else
				{
					float alpha = m_fLastSightsBlendTime / m_fLastSightsBlendDuration;
					// Terminate the blend, resetting the value.
					if (alpha >= 1.0)
					{
						m_fLastSightsBlendTime = 0;
						m_bLastSightsBlend = false;
					}
					else
					{
						vector interpBuffer[4]; // result transformation matrix
						
						// Last->target rotation blend
						// quaternion used for that nice quaternion interpolation that's not achievable with vectors
						float qb[4], qt[4];
						Math3D.MatrixToQuat(m_vLastSightStMS, qb); // start rotation+buffer
						Math3D.MatrixToQuat(sightsMS, qt); // target rotation
						Math3D.QuatLerp(qb, qb, qt, alpha);
						Math3D.QuatToMatrix(qb, interpBuffer);
						
						// Last->target position linear blend
						interpBuffer[3] = vector.Lerp(m_vLastSightStMS[3], sightsMS[3], alpha);
						
						// Apply the transformation
						Math3D.MatrixCopy(interpBuffer, sightsMS);
						targetFOV = Math.Lerp(m_fLastSightStFOV, cameraData.m_fFOV, alpha);
						
						// Update time
						m_fLastSightsBlendTime += pDt;
					}
				}
			}
		}
		
		// Store last transform (unstable)
		Math3D.MatrixCopy(sightsMS, m_vLastSightMS);
		// And last field of view
		m_fLastSightFOV = targetFOV;

		// Get sights to character MS
		Math3D.MatrixMultiply4(propBoneMS, sightsMS, sightsMS);

		// We have sights in right hand prop hand character MS..
		vector cameraBoneMS[4];
		anim.GetBoneMatrix(GetCameraBoneIndex(), cameraBoneMS);
		
		// If last stable position is uninitialized, there are few cases
		// where the interpolation will start at the character origin, creating
		// a very visually unpleasing jump/stutter. Using head bone transformation
		// should be at least somewhat relevant and should reduce this issue significantly.
		if (m_lastStablePos == vector.Zero)
		{
			m_lastStablePos = cameraBoneMS[3];
		}

		// Now let us project the position onto the head in MS
		// rayOrigin:
		//	sights --> sights back (towards head)
		// planeOrigin:
		// head --> forward ( head.z )

		vector pureSightsFwd = sightsMS[2];

		vector projectedPosMS = SCR_Math3D.IntersectPlane(sightsMS[3], -sightsMS[2], cameraBoneMS[3], cameraBoneMS[2]);
		vector projToSightMS = projectedPosMS - sightsMS[3];
		vector projToSightsDirMS = projToSightMS.Normalized();

		// Let's negate translation from modifiers - this will keep our camera
		// where it ought to be, but allow weapon to move independently of camera
		vector aimingTranslationWeaponLS = m_AimingComponent.GetRawAimingTranslation();
		//float zOffset = Math.Clamp(aimingTranslationWeaponLS[2], -CAMERA_RECOIL_LIMIT, CAMERA_RECOIL_LIMIT)

		vector aimingTranslationMS;
		for (int i = 0; i < 3; i++)
			aimingTranslationMS += aimingTranslationWeaponLS[i] * sightsMS[i];


		// And add additional translation, this time desired amount of Z translation
		//! Fetch desired portion of recoil
		float recoilPortion;
		if (m_AimingComponent)
			recoilPortion = Math.Clamp(aimingTranslationWeaponLS[2] * cameraData.m_fCamRecoilAmount, -CAMERA_RECOIL_LIMIT, CAMERA_RECOIL_LIMIT);
		vector extraTranslation = recoilPortion * sightsMS[2];

		// And add that to our result
		sightsMS[3] = sightsMS[3] - aimingTranslationMS - extraTranslation;

		// Now we will disregard any previous rotation... (LOL and use aiming or freelook angles directly)

		// Take look angles directly and correct those for character pitch
		vector aimingAnglesMS = cameraData.m_vLookAngles;
		aimingAnglesMS[1] = aimingAnglesMS[1] + m_OwnerCharacter.GetLocalYawPitchRoll()[1];

		// Use as intended
		Math3D.AnglesToMatrix(aimingAnglesMS, sightsMS);

		const float alphaEpsilon = 0.0005;
		// Stabilize camera in certain cases
		vector resultPosition = sightsMS[3];
		bool isUnstable = false;
		if (m_CmdHandler)
		{
			isUnstable = m_ControllerComponent.IsSprinting() ||
						m_CmdHandler.GetTargetLadder() != null ||
						m_ControllerComponent.IsMeleeAttack() ||
						m_CmdHandler.GetCommandModifier_ItemChange() && m_CmdHandler.GetCommandModifier_ItemChange().IsChangingItemTag() ||
						m_CmdHandler.IsProneStanceTransition() && m_ControllerComponent.GetMovementVelocity().LengthSq() > 0.0 ||
						(!m_WeaponManager || !m_WeaponManager.GetCurrentSights());
		}

		// Jump context is reset, so instead check for animation tag presence
		// and make camera unstable during this period as hand movement is unpredictable
		if (m_iJumpAnimTagId != -1)
		{
			CharacterAnimationComponent animComponent = m_ControllerComponent.GetAnimationComponent();
			if (animComponent && animComponent.IsPrimaryTag(m_iJumpAnimTagId))
				isUnstable = true;
		}
	
		// In transitions out of ADS override stable position and use that instead,
		// this will prevent some quirks as we expect weapon pose in ADS to be quite
		// straight forward, which might not be the case during transitions
		if (sm_TagADSTransitionOut != -1)
		{
			CharacterAnimationComponent animComponent = m_ControllerComponent.GetAnimationComponent();
			if (animComponent)
			{
				// In transitions use camera transform directly
				if (animComponent.IsPrimaryTag(sm_TagADSTransitionOut))
				{
					// Unless both tags are active, at which case we just try to somehow stabilize
					// the result, otherwise we would get jitter again
					if (sm_TagADSTransitionIn != -1 && animComponent.IsPrimaryTag(sm_TagADSTransitionIn))
						anim.GetBoneMatrix(sm_iCameraBoneIndex, cameraBoneMS);
					
					m_lastStablePos = cameraBoneMS[3];
					isUnstable = true;
				}
			}
		}


		if (isUnstable)
		{
			m_fStabilizerAlpha = Math.SmoothCD(m_fStabilizerAlpha, 1.0, m_fStabilizerAlphaVel, 0.1, 1000, pDt);
			resultPosition = m_lastStablePos;
		}
		else
		{
			m_fStabilizerAlpha = Math.SmoothCD(m_fStabilizerAlpha, 0.0, m_fStabilizerAlphaVel, 0.1, 1000, pDt);
		}

		float obstructedAlpha = m_ControllerComponent.GetObstructionAlpha();
		float lerpToCameraT = Math.Max(m_fStabilizerAlpha, obstructedAlpha);
		bool shouldStabilize = lerpToCameraT > alphaEpsilon;
		if (shouldStabilize)
		{
			resultPosition = vector.Lerp(resultPosition, cameraBoneMS[3], lerpToCameraT);
		}

		if (!isUnstable && obstructedAlpha < alphaEpsilon)
		{
			m_lastStablePos = resultPosition;
		}

		// Last but not least, blend FOV based on aiming vs. freelook angles
		// If in freelook or not fully blended out yet, update
		if (cameraData.m_bFreeLook || m_fFreelookBlendAlpha > alphaEpsilon)
		{
			vector weaponAimingDir = m_AimingComponent.GetAimingRotation().AnglesToVector();
			vector lookAimingDir = aimingAnglesMS.AnglesToVector();
			float blendAlpha = 1.0 - (vector.Dot(weaponAimingDir, lookAimingDir) + 1.0) * 0.5;
			const float blendOutSpeed = 3.0; // Higher values reduce the radius range of blend alpha, lower values extend further

			if (cameraData.m_bFreeLook)
				m_fFreelookBlendAlpha = Math.Clamp(Math.Sqrt(blendAlpha * blendOutSpeed), 0.0, 1.0);
			else m_fFreelookBlendAlpha -= pDt * 5.0; // if out of freelook, make sure to blend out, in certain cases the epsilon can never be logically hit

			if (m_fFreelookBlendAlpha <= alphaEpsilon)
				m_fFreelookBlendAlpha = 0.0;
			
			vector mat[4];
			vector offset = m_OffsetLS;
			vector additiveRotation = "0 0 0";
			m_CharacterHeadAimingComponent.GetLookTransformationLS(GetCameraBoneIndex(), EDirectBoneMode.RelativePosition, offset, additiveRotation, mat);
			Math3D.MatrixMultiply4(cameraBoneMS, mat, mat);
			vector goal = mat[3] + "0 0.035 -0.045"; // arbitrary offset to reduce the vnh/p (visible neck hole per pixel)
			resultPosition = vector.Lerp(resultPosition, goal, m_fFreelookBlendAlpha);
		}

		// If the result position is behind the camera bone, the camera might collide with the chest, so we move it forward.
		float resultNegativeZ = resultPosition[2] - cameraBoneMS[3][2];
		if (resultNegativeZ < 0)
		{
			sightsMS[3] = resultPosition + (-resultNegativeZ * pureSightsFwd[2] * pureSightsFwd);
		}
		else
		{
			sightsMS[3] = resultPosition;
		}

		// Get transformation to parent
		if (!shouldStabilize)
		{
			Math3D.MatrixInvMultiply4(propBoneMS, sightsMS, pOutResult.m_CameraTM);
			pOutResult.m_iDirectBone = m_iHandBoneIndex;
			pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativeTransform;
		}
		else
		{
			// Order of operations in hierarchical updates and the way camera interpolation
			// works has forced my hand to recompute all of this into the prop bone (hand) space
			// so we can ensure that we don't reparent the camera along the way, there still might
			// be a tiny bit of grain, but this section has already caused enough of pain
			pOutResult.m_iDirectBone = m_iHandBoneIndex;
			pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativeTransform;
			pOutResult.m_CameraTM[3] = sightsMS[3];
			Math3D.AnglesToMatrix(aimingAnglesMS, pOutResult.m_CameraTM);
			vector directTM[4];
			m_OwnerCharacter.GetAnimation().GetBoneMatrix(pOutResult.m_iDirectBone, directTM);
			Math3D.MatrixInvMultiply4(directTM, pOutResult.m_CameraTM, pOutResult.m_CameraTM);
		}

		pOutResult.m_fFOV = Math.Lerp(targetFOV, GetBaseFOV(), m_fFreelookBlendAlpha);
		pOutResult.m_fDistance = 0;
		pOutResult.m_fNearPlane = 0.0125;
		pOutResult.m_bAllowInterpolation = allowInterpolation;// && (shouldStabilize == m_bWasStabilizedLastFrame);
		pOutResult.m_fUseHeading = 1.0;
		pOutResult.m_bUpdateWhenBlendOut = true;
		pOutResult.m_fPositionModelSpace = 0.0;

		m_bWasStabilizedLastFrame = shouldStabilize;

		return;
	}

	//------------------------------------------------------------------------------------------------
	protected void SolveCameraHandAttach(ADSCameraData cameraData, out ScriptedCameraItemResult pOutResult, float pDt, bool allowInterpolation)
	{
		Animation anim = m_OwnerCharacter.GetAnimation();
		
		//! In some cases this compensation is needed (sloped surfaces)
		float parentPitch = m_OwnerCharacter.GetLocalYawPitchRoll()[1];
		cameraData.m_vLookAngles[1] = Math.Clamp(cameraData.m_vLookAngles[1] + parentPitch, CONST_UD_MIN, CONST_UD_MAX);

		vector headMatrix[4];
		anim.GetBoneMatrix(GetCameraBoneIndex(), headMatrix);

		//! Sights transform relative to the head
		vector sightsRelativeMatrix[4];
		Math3D.MatrixInvMultiply4(headMatrix, cameraData.m_mSightsLocalMat, sightsRelativeMatrix);

		//! Fetch desired portion of recoil
		float recoilPortion;
		if (m_AimingComponent)
			recoilPortion = Math.Clamp(m_AimingComponent.GetRawAimingTranslation()[2] * cameraData.m_fCamRecoilAmount, -CAMERA_RECOIL_LIMIT, CAMERA_RECOIL_LIMIT);

		//! Local head plane offset by the recoil portion
		vector headPlane = cameraData.m_vSightsOffset;
		headPlane[2] = headPlane[2] - recoilPortion;

		//! Project sights onto head plane
		vector resultPosition = SCR_Math3D.IntersectPlane(sightsRelativeMatrix[3], -sightsRelativeMatrix[2], headPlane, vector.Forward);

		// transform relative to hand
		vector handMatrix[4];
		anim.GetBoneMatrix(m_iHandBoneIndex, handMatrix);

		// Get sights relative to anchor
		vector relativeSightsMatrix[4];
		// Sight transform relative to head bone
		Math3D.MatrixInvMultiply4(handMatrix, cameraData.m_mSightsLocalMat, relativeSightsMatrix);

		// Prepare and apply rotation matrix
		Math3D.AnglesToMatrix(cameraData.m_vLookAngles, pOutResult.m_CameraTM);

		bool isUnstable = false;
		if (m_CmdHandler)
		{
			isUnstable = m_ControllerComponent.IsSprinting() ||
						m_CmdHandler.GetTargetLadder() != null ||
						m_ControllerComponent.IsMeleeAttack() ||
						m_CmdHandler.GetCommandModifier_ItemChange() && m_CmdHandler.GetCommandModifier_ItemChange().IsChangingItemTag();
		}

		if (isUnstable)
		{
			m_fStabilizerAlpha = Math.SmoothCD(m_fStabilizerAlpha, 1.0, m_fStabilizerAlphaVel, 0.14, 1000, pDt);
			resultPosition = m_lastStablePos;
		}
		else
		{
			m_fStabilizerAlpha = Math.SmoothCD(m_fStabilizerAlpha, 0.0, m_fStabilizerAlphaVel, 0.14, 1000, pDt);
		}

		float obstructedAlpha = m_ControllerComponent.GetObstructionAlpha();

		if (m_fStabilizerAlpha > 0 || obstructedAlpha > 0)
		{
			float t = Math.Max(m_fStabilizerAlpha, obstructedAlpha);
			vector headRelativePosition = "0 0 0";
			resultPosition = vector.Lerp(resultPosition, headRelativePosition, t);
		}

		if (!isUnstable && obstructedAlpha < 0.001)
		{
			m_lastStablePos = resultPosition;
		}

		//! Get end position rel. to hand
		resultPosition = resultPosition.Multiply4(headMatrix);
		resultPosition = resultPosition.InvMultiply4(handMatrix);


		//! Apply props
		pOutResult.m_CameraTM[3] = resultPosition;
		pOutResult.m_fFOV = cameraData.m_fFOV;
		pOutResult.m_fDistance = 0;
		pOutResult.m_fNearPlane = 0.0125;
		pOutResult.m_bAllowInterpolation = allowInterpolation;
		pOutResult.m_bUpdateWhenBlendOut = true;
		pOutResult.m_iDirectBone = m_iHandBoneIndex;
		pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativePosition;
		pOutResult.m_fPositionModelSpace = 0.0;
	}

	//------------------------------------------------------------------------------------------------
	protected void SolveCamera2DSight(ADSCameraData cameraData, out ScriptedCameraItemResult pOutResult)
	{
		// TODO: Please make camera position consistent with 2DPIPSights m_vCameraPoint
		float targetFOV;
		m_WeaponManager.GetCurrentSightsCameraTransform(cameraData.m_mSightsLocalMat, targetFOV);

		// TODO@AS: refactor
		SCR_2DOpticsComponent sights2D = SCR_2DOpticsComponent.Cast(m_WeaponManager.GetCurrentSights());
		BaseWeaponComponent weapon = m_WeaponManager.GetCurrentWeapon();

		// The camera position shall match the position and angle of camera with PIP scope
		vector cameraAngles;
		if (sights2D && weapon && weapon.IsSightADSActive())
		{
			vector sightsOffset = sights2D.GetSightsFrontPosition(true) + sights2D.GetCameraOffset() - sights2D.GetSightsOffset();
			vector cameraOffset = sightsOffset.Multiply3(cameraData.m_mSightsLocalMat);
			cameraData.m_mSightsLocalMat[3] = cameraData.m_mSightsLocalMat[3] + cameraOffset;

			// Get world orientation of sight
			// Turrets require getting owner transform to obtain the world transform of the sight reliably
			vector sightMat[4];
			sights2D.GetSightsTransform(sightMat, true);

			vector ownerMat[4];
			sights2D.GetOwner().GetWorldTransform(ownerMat);
			Math3D.MatrixMultiply3(ownerMat, sightMat, sightMat);

			// Get optic transformation in world coordinates
			vector opticMat[4];
			sights2D.GetCameraLocalTransform(opticMat);
			Math3D.MatrixMultiply3(ownerMat, opticMat, opticMat);

			// Substract optic transformation
			vector cameraMat[4];
			Math3D.MatrixInvMultiply3(opticMat, sightMat, cameraMat);

			cameraAngles = Math3D.MatrixToAngles(cameraMat);
		}

		SCR_2DPIPSightsComponent sightsPIP = SCR_2DPIPSightsComponent.Cast(sights2D);
		if (sightsPIP)
			targetFOV = sightsPIP.GetMainCameraFOV();

		//! clamp fov so sights FOV is never greater than current fov?
		pOutResult.m_fFOV = Math.Min(GetBaseFOV(), targetFOV);

		// Add recoil and sway, reduced by FOV for convenience
		vector adjustedLookAngles = cameraData.m_vLookAngles;

		AimingComponent aiming = m_OwnerCharacter.GetWeaponAimingComponent();
		if (aiming)
		{
			if (m_ControllerComponent.GetIsWeaponDeployed())
				adjustedLookAngles = aiming.GetAimingRotation();

			// Arbitrary conversion of aiming translation to recoil angle based on current FOV
			vector offset = aiming.GetModifiedAimingTranslation() * pOutResult.m_fFOV;
			adjustedLookAngles = adjustedLookAngles + Vector(offset[1], offset[2], 0);
		}

		// we start clean
		vector lookRot[4];

		adjustedLookAngles = adjustedLookAngles - cameraAngles;
		Math3D.AnglesToMatrix(adjustedLookAngles, lookRot);

		// snap to bone
		vector handBoneTM[4];
		m_OwnerCharacter.GetAnimation().GetBoneMatrix(m_iHandBoneIndex, handBoneTM);

		//! sights transform relative to hand bone
		Math3D.MatrixInvMultiply4(handBoneTM, cameraData.m_mSightsLocalMat, pOutResult.m_CameraTM);
		vector finalPos = pOutResult.m_CameraTM[3];

		//! sights in relation to hand
		vector viewMatHandRel[4];
		Math3D.MatrixInvMultiply4(handBoneTM, lookRot, pOutResult.m_CameraTM);

		//! apply position
		pOutResult.m_CameraTM[3] = finalPos;

		//! setup camera props
		pOutResult.m_iDirectBone 			= m_iHandBoneIndex;
		pOutResult.m_iDirectBoneMode 		= EDirectBoneMode.RelativeTransform;
		pOutResult.m_bUpdateWhenBlendOut	= true; // otherwise camera stops blending out properly
		pOutResult.m_fDistance 				= 0;
		pOutResult.m_fUseHeading 			= 0;
		pOutResult.m_fNearPlane				= 0.025;
		pOutResult.m_bBlendFOV 				= true; // otherwise FOV blend transitions awkwardly
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		if (m_CameraHandler && m_CameraHandler.IsCameraBlending())
		{
			if (m_CameraHandler.GetCurrentCamera() == this)
				OnBlendingIn(m_CameraHandler.GetBlendAlpha(this));
			else
				OnBlendingOut(m_CameraHandler.GetBlendAlpha(m_CameraHandler.GetCurrentCamera()));
		}

		//! update angles
		auto sights = m_WeaponManager.GetCurrentSights();
		bool canFreelook = sights && sights.CanFreelook();
		//! update fov
		m_fFreelookFOV = GetBaseFOV();

		pOutResult.m_vBaseAngles = GetBaseAngles();


		//! yaw pitch roll vector
		vector lookAngles = m_CharacterHeadAimingComponent.GetLookAngles();
		if (!canFreelook)
		{
			lookAngles[0] = m_CommandWeapons.GetAimAngleLR();
			lookAngles[1] = m_CommandWeapons.GetAimAngleUD();
		}
		//! Prepare data

		m_pCameraData.m_fDeltaTime = pDt;
		m_pCameraData.m_vLookAngles = lookAngles;
		m_pCameraData.m_fFOV = GetBaseFOV();

		//! Fetch sights transformation
		if (sights)
		{
			m_WeaponManager.GetCurrentSightsCameraTransform(m_pCameraData.m_mSightsLocalMat, m_pCameraData.m_fFOV);
			m_pCameraData.m_vSightsOffset = sights.GetSightsOffset();
			m_pCameraData.m_fCamRecoilAmount = sights.GetCameraRecoilAmount();
		}
		else
		{
			Math3D.MatrixIdentity4(m_pCameraData.m_mSightsLocalMat);
		}

		//! Recalculate FOV
		m_pCameraData.m_fFOV = Math.Min(GetBaseFOV(), m_pCameraData.m_fFOV);

		//! Fetch zeroing data
		// Apparently in rare cases like bandaging, weapon can be missing
		BaseWeaponComponent currentWeapon = m_WeaponManager.GetCurrent();
		if (currentWeapon)
		{
			vector zeroingMatrix[4];
			if (currentWeapon.GetCurrentSightsZeroingTransform(zeroingMatrix))
			{
				// add zeroing to our initial local sights
				zeroingMatrix[3] = -zeroingMatrix[3];
				Math3D.MatrixMultiply4(m_pCameraData.m_mSightsLocalMat, zeroingMatrix, m_pCameraData.m_mSightsLocalMat);
			}
		}

		// 2nd camera -> provided by sights if any
		CameraBase overlayCamera;

		//special handling for FOV blending for 2d sights
		//special handling for eye snapping too
		auto sights2d = SCR_2DOpticsComponent.Cast(sights);
		if (sights2d)
		{
			auto pip = SCR_2DPIPSightsComponent.Cast(sights2d);
			// 2D sights
			if (!pip || SCR_Global.IsScope2DEnabled())
			{
				SolveCamera2DSight(m_pCameraData, pOutResult);
				pOutResult.m_pOwner 				= m_OwnerCharacter;
				pOutResult.m_pWSAttachmentReference = null;
				return;
			}

			// Camera FOV to be used is different from the sights FOV
			m_pCameraData.m_fFOV = pip.GetMainCameraFOV();
			overlayCamera = pip.GetPIPCamera();
		}

		if (!canFreelook)
		{
			m_pCameraData.m_vLookAngles[0] = 0.0;
		}

		// Store freelook state
		m_pCameraData.m_bFreeLook = canFreelook && (m_ControllerComponent.IsFreeLookEnabled() || m_bForceFreeLook);

		#ifdef ENABLE_DIAG
			int solveMethod = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_ADS_CAMERA);

			if (solveMethod == 1)
				SolveCameraHandAttach(m_pCameraData, pOutResult, pDt, false);
			else if (solveMethod == 2)
				SolveCameraHeadAttach(m_pCameraData, pOutResult);
			else if (solveMethod == 3)
				SolveCameraHandAttach(m_pCameraData, pOutResult, pDt, true);
			else // :-)
				SolveNewMethod(m_pCameraData, pOutResult, pDt, true);
		#else
			SolveNewMethod(m_pCameraData, pOutResult, pDt, true);
		#endif 

		CameraManager cameraMgr = GetGame().GetCameraManager();
		if (cameraMgr != null)
		{
			// Update overlay camera
			if (m_pCameraData.m_bFreeLook)
			{
				// Supress overlay cam
				cameraMgr.SetOverlayCamera(null);
			}
			else if (overlayCamera)
			{
				// Override overlay comera if any is used
				cameraMgr.SetOverlayCamera(overlayCamera);
			}
		}

		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_pWSAttachmentReference = null;

		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}

	//------------------------------------------------------------------------------------------------
	protected BaseWeaponManagerComponent m_WeaponManager;
	protected BaseWeaponComponent m_LastWeaponComponent;
	protected CharacterAimingComponent m_AimingComponent;
	protected BaseSightsComponent m_BinocularSight;
	protected BaseSightsComponent m_LastSightsComponent;	// last used weapon sights or null if none
	protected vector m_vLastSightMS[4];						// last 'unstable' sight transform
	protected vector m_vLastSightStMS[4];					// last 'stable' sight transform
	protected float	 m_fLastSightFOV;						// last 'unstable' sight field of view
	protected float	 m_fLastSightStFOV;						// last 'stable' sight field of view
	protected bool m_bLastSightsBlend;						// whether sights are being blended, don't set manually
	protected float m_fLastSightsBlendTime;					// current value of blend, don't set manually
	protected float m_fLastSightsBlendDuration = 0.15;		// duration of blend in seconds, just a bit is good enough
	ref ADSCameraData m_pCameraData = new ADSCameraData();

	protected	int 	m_iHandBoneIndex;	//!< hand bone
	protected	int 	m_iHeadBoneIndex;	//!< head bone
	protected	AnimationTagID m_iJumpAnimTagId;
	protected	vector	m_OffsetLS;			//!< position offset
	protected	float	m_fADSToFPSDeg;		//!< freelook degrees for transitioning into fps pos
	protected	float	m_fFreelookFOV;
	protected	vector	m_lastStablePos;

	protected	float	m_fStabilizerAlpha = 0.0;
	protected	float	m_fStabilizerAlphaVel = 0.0;

	protected	bool	m_bWasStabilizedLastFrame = false;

	protected 	float	m_fFreelookBlendAlpha;

	private		vector	m_vLastEndPos;		//!< last position used for interpolation

	protected const float CAMERA_INTERP = 0.6;
	protected const float CAMERA_RECOIL_LIMIT = 0.25; //!< Maximum amount of recoil applied to camera from weapon in meters.
};
