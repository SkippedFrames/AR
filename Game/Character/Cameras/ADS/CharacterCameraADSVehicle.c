// *************************************************************************************
// ! CharacterCameraADSVehicle - ADS camera when character is in vehicle
// *************************************************************************************
class CharacterCameraADSVehicle extends CharacterCameraADS
{
	private TurretControllerComponent m_TurretController;
	private IEntity m_OwnerVehicle;
	private bool m_bUseCameraSlot;

	void CharacterCameraADSVehicle(CameraHandlerComponent pCameraHandler)
	{
		m_WeaponManager = null;

		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				m_OwnerVehicle = compartment.GetVehicle();

				SCR_VehicleCameraDataComponent vehicleCamData = SCR_VehicleCameraDataComponent.Cast(m_OwnerVehicle.FindComponent(SCR_VehicleCameraDataComponent));
				if (vehicleCamData)
				{
					m_fRollFactor = vehicleCamData.m_fRollFactor;
					m_fPitchFactor = vehicleCamData.m_fPitchFactor;
				}

				BaseControllerComponent controller = compartment.GetController();
				if (controller)
				{
					m_TurretController = TurretControllerComponent.Cast(controller);
					if (m_TurretController)
						m_WeaponManager = m_TurretController.GetWeaponManager();
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnBlendIn()
	{
		OnBlendingIn(1.0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnBlendOut()
	{
		OnBlendingOut(1.0);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnBlendingIn(float blendAlpha)
	{
		if (m_TurretController && blendAlpha >= GetSightsADSActivationPercentage())
		{
			if (!m_TurretController.GetCurrentSightsADS()) // Only enable if not enabled yet
				m_TurretController.SetCurrentSightsADS(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnBlendingOut(float blendAlpha)
	{
		if (m_TurretController && blendAlpha >= GetSightsADSDeactivationPercentage())
		{
			if (m_TurretController.GetCurrentSightsADS()) // Only disable if enabled
				m_TurretController.SetCurrentSightsADS(false);
		}
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

		TurretComponent turret = m_TurretController.GetTurretComponent();
		vector aimingTranslationWeaponLS = turret.GetRawAimingTranslation();

		//! sights transformation and FOV get
		vector sightWSMat[4];
		m_TurretController.GetCurrentSightsCameraTransform(sightWSMat, pOutResult.m_fFOV);

		PointInfo cameraSlot = turret.GetCameraAttachmentSlot();
		if (cameraSlot)
		{
			m_bUseCameraSlot = true;

			pOutResult.m_CameraTM[3] 			= aimingTranslationWeaponLS;
			pOutResult.m_iDirectBone 			= -1;
			pOutResult.m_iDirectBoneMode 		= EDirectBoneMode.None;
			pOutResult.m_bUpdateWhenBlendOut	= false;
			pOutResult.m_fDistance 				= 0;
			pOutResult.m_fUseHeading 			= 0;
			pOutResult.m_fNearPlane				= 0.025;
			pOutResult.m_bAllowInterpolation	= true;
			pOutResult.m_pWSAttachmentReference = cameraSlot;

			// Apply shake
			if (m_CharacterCameraHandler)
				m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
			return;
		}

		// character matrix
		vector charMat[4];
		m_OwnerCharacter.GetTransform(charMat);

		//Add translation
		vector aimingTranslationWeapon = aimingTranslationWeaponLS.Multiply3(sightWSMat);
		sightWSMat[3] = sightWSMat[3] - aimingTranslationWeapon;

		Math3D.MatrixInvMultiply4(charMat, sightWSMat, pOutResult.m_CameraTM);

		pOutResult.m_iDirectBone 			= -1;
		pOutResult.m_iDirectBoneMode 		= EDirectBoneMode.None;
		pOutResult.m_bUpdateWhenBlendOut	= false;
		pOutResult.m_fDistance 				= 0;
		pOutResult.m_fUseHeading 			= 0;
		pOutResult.m_fNearPlane				= 0.025;
		pOutResult.m_bAllowInterpolation	= true;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_pWSAttachmentReference = null;

		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}

	//------------------------------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		IEntity owner = m_OwnerVehicle;
		if (m_TurretController)
			owner = m_TurretController.GetOwner();

		if (m_bUseCameraSlot)
		{
			m_fPitchFactor = 0;
			AddVehiclePitchRoll(owner, pDt, transformMS);
		}
	}
};
