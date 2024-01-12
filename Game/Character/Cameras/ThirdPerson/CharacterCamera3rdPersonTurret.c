// *************************************************************************************
// ! CharacterCamera3rdPersonTurret - 3rd person camera when character is controlling turret
// *************************************************************************************
class CharacterCamera3rdPersonTurret extends CharacterCameraBase
{

	static const float 	CONST_UD_MIN = -45.0; //!< down limit
	static const float 	CONST_UD_MAX = 30.0; //!< up limit
	static const float 	CONST_UD_CAMERA_ANGLE_OFFSET = -10.0; //!< camera angle offset

	protected TurretControllerComponent m_pTurretController;
	protected TurretComponent m_pControlledTurret;
	protected vector m_vVertAimLimits;
	protected vector m_vHorAimLimits;
	protected vector m_vCameraCenter;
	protected float m_fLRAngleAdd;
	protected float m_fHeight;
	protected float m_fDist_Min;
	protected IEntity m_OwnerVehicle;

	protected vector m_vLastCameraAngles; //< Does not update in freelook
	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonTurret(CameraHandlerComponent pCameraHandler)
	{
		m_bLRAngleNoLimit = true;
	}
	
	//-----------------------------------------------------------------------------
	void InitCameraData()
	{
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			auto compartment = compartmentAccess.GetCompartment();
			m_OwnerVehicle = compartment.GetVehicle();
			IEntity turret = compartment.GetOwner();
			if (turret)
			{
				vector mins, maxs;
				turret.GetBounds(mins, maxs);
				m_vCameraCenter = (maxs - mins) * 0.5 + mins;
				
				SCR_VehicleCameraDataComponent vehicleCamData = SCR_VehicleCameraDataComponent.Cast(turret.FindComponent(SCR_VehicleCameraDataComponent));
				if (vehicleCamData)
				{
					m_fHeight = vehicleCamData.m_fHeight;
					m_fDist_Min = vehicleCamData.m_fDist_Min;
					m_fFOV = vehicleCamData.m_fFOV;
					m_vCameraCenter[1] = m_vCameraCenter[1] + m_fHeight;
					m_vCameraCenter[2] = m_vCameraCenter[2] - m_fDist_Min;
					m_fRollFactor = vehicleCamData.m_fRollFactor;
					m_fPitchFactor = vehicleCamData.m_fPitchFactor;
				}
				
				// If we'll have multiple turrets, don't cache turret
				m_pTurretController = TurretControllerComponent.Cast(compartment.GetController());
				if (!m_pTurretController)
					return;

				m_pControlledTurret = m_pTurretController.GetTurretComponent();
				if (!m_pControlledTurret)
					return;
				
				m_pControlledTurret.GetAimingLimits(m_vHorAimLimits, m_vVertAimLimits);
				m_vVertAimLimits[0] = CONST_UD_MIN;
				m_vVertAimLimits[1] = CONST_UD_MAX;
			}
		}
	}
	
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		
		InitCameraData();
	}
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();	
		if (compartmentAccess && compartmentAccess.IsInCompartment() )
		{
			auto compartment = compartmentAccess.GetCompartment();
			if (m_OwnerVehicle != compartment.GetVehicle()) 
			{
				InitCameraData();
			}
		}
		
		if (!m_pControlledTurret || !m_pTurretController)
			return;
		
		// character matrix
		vector charMat[4];
		m_OwnerCharacter.GetWorldTransform(charMat);

		// parent transform
		vector turretMat[4];
		IEntity parentEntity = m_OwnerCharacter.GetParent();
		if (!parentEntity)
			parentEntity = m_OwnerCharacter;

		parentEntity.GetWorldTransform(turretMat);
		
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

		// position
		vector cameraPositionLS = turretMat[3].InvMultiply4(turretMat) + Vector(0, m_vCameraCenter[1], 0);
		
		CompartmentAccessComponent cac = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (!(cac && (cac.IsGettingOut() || cac.IsGettingIn())))
		{
			// Offset by character height, height is not the same when entities are rotated, this is why we revert position without any rotation. Then we can do the height diff.
			float charHeight = (charMat[3] - turretMat[3]).InvMultiply3(turretMat)[1];
			cameraPositionLS[1] = cameraPositionLS[1] - charHeight;
		}
			
		// This is our pivot point
		pOutResult.m_CameraTM[3] = cameraPositionLS;
		
		const float camDist = 2.0;
		
		// other parameters
		pOutResult.m_fUseHeading 			= 0.0;
		pOutResult.m_fFOV 					= m_fFOV;
		pOutResult.m_fDistance 				= camDist;
		pOutResult.m_bAllowInterpolation 	= true;
		pOutResult.m_pWSAttachmentReference = null;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_bUpdateWhenBlendOut   = false;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
	
	//-----------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		AddVehiclePitchRoll(m_OwnerVehicle, pDt, transformMS);
	}
};
