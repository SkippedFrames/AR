// *************************************************************************************
// ! CharacterCamera1stPersonVehicle - 1st person camera when character is in vehicle
// *************************************************************************************
class CharacterCamera1stPersonTurret extends CharacterCamera1stPerson
{

	protected TurretControllerComponent m_pTurretController;
	protected TurretComponent m_pControlledTurret;
	protected TurretCompartmentSlot m_pCompartment;
	protected vector m_vVertAimLimits;
	protected vector m_vHorAimLimits;
	protected float m_fLRAngleAdd;
	protected vector m_vPrevEyePosition;
	protected IEntity m_OwnerVehicle;

	protected vector m_vLastCameraAngles; //< Does not update in freelook

	//-----------------------------------------------------------------------------
	void CharacterCamera1stPersonTurret(CameraHandlerComponent pCameraHandler)
	{
		m_ApplyHeadBob = false;
		m_vPrevEyePosition = m_OwnerCharacter.EyePositionModel();
		m_OffsetLS = "0.0 0.1 0.0";
		m_bLRAngleNoLimit = true;
	}
		
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);

		if (m_pCompartmentAccess && m_pCompartmentAccess.IsInCompartment())
		{
			BaseCompartmentSlot compartment = m_pCompartmentAccess.GetCompartment();
			if (compartment)
			{
				m_OwnerVehicle = compartment.GetVehicle();
				
				SCR_VehicleCameraDataComponent vehicleCamData = SCR_VehicleCameraDataComponent.Cast(m_OwnerVehicle.FindComponent(SCR_VehicleCameraDataComponent));
				if (vehicleCamData)
				{
					m_fRollFactor = vehicleCamData.m_fRollFactor;
					m_fPitchFactor = vehicleCamData.m_fPitchFactor;
				}
				
				m_pTurretController = TurretControllerComponent.Cast(compartment.GetController());
				if (!m_pTurretController)
					return;

				// If we'll have multiple turrets, don't cache turret
				m_pControlledTurret = m_pTurretController.GetTurretComponent();
				if (!m_pControlledTurret)
					return;

				m_pControlledTurret.GetAimingLimits(m_vHorAimLimits, m_vVertAimLimits);
				m_pCompartment = TurretCompartmentSlot.Cast(compartment);
			}
		}
		
		m_fLRAngleAdd = 0;
	}
	//-----------------------------------------------------------------------------
	static float UpdateAngleWithTarget(out float pAngle, out float pAngleAdd, out float addVelocity, vector limits, float pDt, float target, float change, bool freeLook)
	{
		//! update it in degrees
		pAngle = target;
		pAngle = Math.Clamp(pAngle, limits[0], limits[1]);
		// remove freelook when switched back to controlled state
		pAngleAdd = Math.SmoothCD(pAngleAdd, 0, addVelocity, 0.14, 1000, pDt);
		
		return Math.Clamp(pAngle + pAngleAdd, limits[0], limits[1]);
	}
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		// no super for now, will calculate angle based on turret aiming (HeadAngles not supported when turret compartment attachment)
		if (!m_pControlledTurret)
			return;
		
		pOutResult.m_pWSAttachmentReference = null;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_fPositionModelSpace 	= 1.0;
		pOutResult.m_fFOV 					= m_fFOV;
		pOutResult.m_fNearPlane 			= 0.04;
		pOutResult.m_bAllowInterpolation 	= true;
		pOutResult.m_bUpdateWhenBlendOut 	= true;
		pOutResult.m_fUseHeading 			= 0.0;
		pOutResult.m_iDirectBoneMode 		= EDirectBoneMode.None;
		pOutResult.m_iDirectBone 			= -1;

		// character matrix
		vector charMat[4];
		m_OwnerCharacter.GetWorldTransform(charMat);

		vector offset = m_OffsetLS;
		if (m_pCompartment && m_pCompartment.IsDirectAimMode() && m_pTurretController.GetCanAimOnlyInADS())
		{
			offset = "0 0 0";
		}

		vector charAngles = Math3D.MatrixToAngles(charMat);
		vector lookAngles = m_CharacterHeadAimingComponent.GetLookAngles();

		CharacterControllerComponent charController = m_OwnerCharacter.GetCharacterController();
		
		//! apply to rotation matrix
		if (charController.IsFreeLookEnabled() || m_pTurretController.GetCanAimOnlyInADS())
		{
			m_vLastCameraAngles[0] = m_pControlledTurret.GetAimingDirectionWorld().VectorToAngles()[0];
			Math3D.AnglesToMatrix(m_vLastCameraAngles - charAngles + lookAngles, pOutResult.m_CameraTM);
		}
		else
		{
			m_vLastCameraAngles = m_pControlledTurret.GetAimingDirectionWorld().VectorToAngles();
			Math3D.AnglesToMatrix(m_vLastCameraAngles - charAngles, pOutResult.m_CameraTM);
		}
		
		//! lerp eye position to prevent nosiating shake when character walks around deployed turret
		m_vPrevEyePosition = vector.Lerp(m_vPrevEyePosition, m_OwnerCharacter.EyePositionModel(), 0.25);
		pOutResult.m_CameraTM[3] = m_vPrevEyePosition + offset;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
	
	//-----------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		AddVehiclePitchRoll(m_OwnerVehicle, pDt, transformMS);
	}
	
	//-----------------------------------------------------------------------------	
	override float GetBaseFOV()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return 0;
		
		return cameraManager.GetVehicleFOV();
	}
};
