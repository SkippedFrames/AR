// *************************************************************************************
// ! CharacterCamera3rdPersonVehicle - 3rd person camera when character is in vehicle
// *************************************************************************************
class CharacterCamera3rdPersonVehicle extends CharacterCameraBase
{
	//-----------------------------------------------------------------------------
	static const float 	CONST_UD_MIN	= -89.0;		//!< down limit
	static const float 	CONST_UD_MAX	= 89.0;			//!< up limit

	static const float 	CONST_LR_MIN	= -180.0;		//!< left limit
	static const float 	CONST_LR_MAX	= 180.0;		//!< right limit

	static const float STEERING_DEGREES	= 5;
	static const float ANGULAR_INERTIA	= 5;
	protected float m_fSteeringAngle;
	protected float m_fInertiaAngle;

	//------------------------------------------------------------------------------------------------
	protected float m_fHeight;
	protected float m_fDist_Desired;
	protected float m_fDist_Min;
	protected float m_fDist_Max;
	protected float m_fFOV_SpeedAdjustMax;
	protected float m_fBobScale;
	protected float m_fShakeScale;
	protected float m_fSpeedMax;

	protected IEntity m_OwnerVehicle;
	protected BaseCompartmentSlot m_pCompartment;
	protected TurretControllerComponent m_TurretController;
	protected vector m_vCenter;

	protected vector m_vLastVel;
	protected vector m_vLastAngVel;
	protected vector m_vAcceleration;

	protected float m_f3rd_TraceClipPct;
	protected float m_fBob_FastUp;
	protected float m_fBob_FastRight;
	protected float m_fBob_SlowUp;
	protected float m_fBob_SlowRight;
	protected float m_fBob_ScaleFast;
	protected float m_fBob_ScaleSlow;
	protected float m_fBob_Acceleration;

	//rad
	protected float m_fAngleThirdPerson;

	protected SCR_VehicleCameraAimpoint m_pCameraAimpointData;
	protected SCR_VehicleCameraAlignment m_pCameraAlignData;

	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonVehicle(CameraHandlerComponent pCameraHandler)
	{
		m_fFOV = GetBaseFOV();
		m_bLRAngleNoLimit = true;
	}
	
	//-----------------------------------------------------------------------------
	void InitCameraData()
	{
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			m_pCompartment = compartmentAccess.GetCompartment();
			IEntity vehicle = m_pCompartment.GetOwner().GetRootParent();
			if (vehicle)
			{
				m_OwnerVehicle = vehicle;
				m_TurretController = TurretControllerComponent.Cast(m_pCompartment.GetController());

				SCR_VehicleCameraDataComponent vehicleCamData = SCR_VehicleCameraDataComponent.Cast(vehicle.FindComponent(SCR_VehicleCameraDataComponent));
				// Can owner replace Vehicle's camera data?
				SCR_VehicleCameraDataComponent ownerCamData = SCR_VehicleCameraDataComponent.Cast(m_pCompartment.GetOwner().FindComponent(SCR_VehicleCameraDataComponent));
				if (ownerCamData && ownerCamData.m_bOverrideVehicleSettings)
					vehicleCamData = ownerCamData;
				
				if( vehicleCamData )
				{
					m_fHeight = vehicleCamData.m_fHeight;
					m_fDist_Desired = vehicleCamData.m_fDist_Desired;
					m_fDist_Min = vehicleCamData.m_fDist_Min;
					m_fDist_Max = vehicleCamData.m_fDist_Max;
					//m_fFOV = vehicleCamData.m_fFOV;
					m_fFOV_SpeedAdjustMax = vehicleCamData.m_fFOV_SpeedAdjustMax;
					m_fBobScale = vehicleCamData.m_fBobScale;
					m_fShakeScale = vehicleCamData.m_fShakeScale;
					m_fSpeedMax = vehicleCamData.m_fSpeedMax;
					m_pCameraAimpointData = vehicleCamData.m_pCameraAimpointData;
					m_pCameraAlignData = vehicleCamData.m_pCameraAlignData;
					m_fRollFactor = vehicleCamData.m_fRollFactor;
					m_fPitchFactor = vehicleCamData.m_fPitchFactor;
					m_fAngleThirdPerson = vehicleCamData.m_fAngleThirdPerson * Math.DEG2RAD;
				}
				
				Physics physics = vehicle.GetPhysics();
				if (physics)
				{
					m_vCenter = physics.GetCenterOfMass();
				}
				else
				{
					vector mins, maxs;
					vehicle.GetBounds(mins, maxs);
					m_vCenter = (maxs - mins) * 0.5 + mins;
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		
		if (pPrevCamera)
		{
			vector f = pPrevCamera.GetBaseAngles();
			m_fUpDownAngle		= f[0]; 
			m_fLeftRightAngle	= f[1]; 
		}
		
		CharacterCamera1stPersonVehicle characterCamera1stPersonVehicle  = CharacterCamera1stPersonVehicle.Cast(pPrevCamera);
		if (characterCamera1stPersonVehicle)
		{
			m_fRollSmooth = characterCamera1stPersonVehicle.m_fRollSmooth;
			m_fRollSmoothVel = characterCamera1stPersonVehicle.m_fRollSmoothVel;
			m_fPitchSmooth = characterCamera1stPersonVehicle.m_fPitchSmooth;
			m_fPitchSmoothVel = characterCamera1stPersonVehicle.m_fPitchSmoothVel;
		}
		
		InitCameraData();
	}

	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			m_pCompartment = compartmentAccess.GetCompartment();
			IEntity vehicle = m_pCompartment.GetOwner().GetRootParent();
			if (vehicle && vehicle != m_OwnerVehicle)
			{
				InitCameraData();
			}
		}
		
		//! update angles
		float udAngle = UpdateUDAngle(m_fUpDownAngle, CONST_UD_MIN, CONST_UD_MAX, pDt);
		m_fLeftRightAngle = UpdateLRAngle(m_fLeftRightAngle, CONST_LR_MIN, CONST_LR_MAX, pDt);
		
		pOutResult.m_vBaseAngles = GetBaseAngles();
		
		//! update fov
		m_fFOV = GetBaseFOV();
		
		// phx speed
		vector characterOffset = vector.Zero;
		vector localVelocity = vector.Zero;
		vector localAngVelocity = vector.Zero;
		vector vehMat[4];
		
		float steeringAngle;
		if (m_OwnerVehicle)
		{
			vector charMat[4];
			m_OwnerVehicle.GetTransform(vehMat);
			m_OwnerCharacter.GetTransform(charMat);
			characterOffset = m_OwnerVehicle.VectorToLocal(vehMat[3] - charMat[3]);
			
			Physics physics = m_OwnerVehicle.GetPhysics();
			if (physics)
			{
				localVelocity = physics.GetVelocity().InvMultiply3(vehMat);
				localAngVelocity = physics.GetAngularVelocity().InvMultiply3(vehMat);
			}
			
			//CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
			if (compartmentAccess && PilotCompartmentSlot.Cast(compartmentAccess.GetCompartment()))
			{
				VehicleWheeledSimulation simulation = VehicleWheeledSimulation.Cast(m_OwnerVehicle.FindComponent(VehicleWheeledSimulation));
				if (simulation)
					steeringAngle = simulation.GetSteering();
			}
		}
		else
		{
			Math3D.MatrixIdentity4(vehMat);
		}
		
		// To smoothen out jittering a bit
		vector smoothVelocity = vector.Lerp(m_vLastVel, localVelocity, pDt);
		m_fInertiaAngle = Math.Lerp(m_fInertiaAngle, localAngVelocity[1] * ANGULAR_INERTIA, pDt);
		m_fSteeringAngle = Math.Lerp(m_fSteeringAngle, steeringAngle * STEERING_DEGREES, pDt);
		
		// store phx values
		m_vLastVel = localVelocity;
		m_vLastAngVel = localAngVelocity;
		
		vector yawPitchRoll = Math3D.MatrixToAngles(vehMat);
		
		//! yaw pitch roll vector
		vector lookAngles;
		lookAngles[0] = m_fLeftRightAngle + m_fInertiaAngle + m_fSteeringAngle;
		lookAngles[1] = udAngle - yawPitchRoll[1] * m_fPitchFactor;
		lookAngles[2] = 0.0;
		
		//! apply to rotation matrix
		Math3D.AnglesToMatrix(lookAngles, pOutResult.m_CameraTM);
		
		//! Roll
		{
			//! Remove roll from parent
			vector orientation[3]
			Math3D.AnglesToMatrix(Vector(0, 0, -yawPitchRoll[2]), orientation);
			Math3D.MatrixMultiply3(orientation, pOutResult.m_CameraTM, pOutResult.m_CameraTM);
			
			//! Dot product
			float angle = yawPitchRoll[2] * m_fRollFactor * Math.DEG2RAD * pOutResult.m_CameraTM[2][2];
			//! Apply roll factor
			float rollMask = Math.Max(0, vehMat[1][1]); // Do not apply roll factor when the vehicle is upside-down.
			SCR_Math3D.RotateAround(pOutResult.m_CameraTM, vector.Zero, pOutResult.m_CameraTM[2], -angle * rollMask, pOutResult.m_CameraTM);
		}
		// Camera offset
		float heightSign = vehMat[1][1]; // Adjust the sign of the height direction as the vehicle rolls.
		pOutResult.m_CameraTM[3] = m_vCenter + characterOffset + Vector(0, heightSign * m_fHeight, 0);
		
		// viewbob update
		UpdateViewBob(pDt, localVelocity, localAngVelocity);
		
		// offset based on speed
		float speed = localVelocity.Length();
		float speedScale = Math.Clamp(speed / (m_fSpeedMax * KILOMETERS_PER_HOUR_TO_METERS_PER_SEC), 0, 1);
		float camDist = Math.Clamp((m_fDist_Max - m_fDist_Min) * speedScale + m_fDist_Desired, m_fDist_Min, m_fDist_Max);
		
		// update auto align
		/*if (m_pCameraAlignData && m_OwnerVehicle)
		{
			bool isFocused = false;
			if (m_CharacterCameraHandler && m_CharacterCameraHandler.GetFocusMode() > 0)
				isFocused = true;
			
			vector aimChange = m_Input.GetAimChange();
			vector newAngles;
			if (m_pCameraAlignData.Update(aimChange, Vector(m_fLeftRightAngle, lookAngles[1], 0.0), smoothVelocity, isFocused, pDt, newAngles))
			{
				m_fLeftRightAngle = newAngles[0];
				m_fUpDownAngle = newAngles[1];
			}
		}*/
		
		// Translate camera aimpoint based on aimpoing curve data
		if (m_pCameraAimpointData) 
		{	
			vector translation = m_pCameraAimpointData.Sample(smoothVelocity[2] * METERS_PER_SEC_TO_KILOMETERS_PER_HOUR);
			translation[0] = m_pCameraAimpointData.SampleAside(localAngVelocity[1] * METERS_PER_SEC_TO_KILOMETERS_PER_HOUR); 
			pOutResult.m_CameraTM[3] = pOutResult.m_CameraTM[3] + translation;
		}
		
		if (m_OwnerVehicle)
		{
			bool applyCharAngle = true;
			if (sm_TagLyingCamera != -1)
			{
				CharacterAnimationComponent characterAnimationComponent = m_OwnerCharacter.GetAnimationComponent();
				if (characterAnimationComponent && characterAnimationComponent.IsPrimaryTag(sm_TagLyingCamera))
					applyCharAngle = false;
			}
			if (applyCharAngle)
				SCR_Math3D.RotateAround(pOutResult.m_CameraTM, pOutResult.m_CameraTM[3], pOutResult.m_CameraTM[0], m_fAngleThirdPerson, pOutResult.m_CameraTM);
			else
				SCR_Math3D.RotateAround(pOutResult.m_CameraTM, pOutResult.m_CameraTM[3], pOutResult.m_CameraTM[0], 0, pOutResult.m_CameraTM);
		}
		
		// other parameters
		pOutResult.m_fUseHeading 			= 0.0;
		pOutResult.m_fFOV 					= m_fFOV + speedScale * m_fFOV_SpeedAdjustMax;
		pOutResult.m_fDistance 				= camDist;
		pOutResult.m_pWSAttachmentReference = null;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
	
	//-----------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		//AddVehiclePitchRoll(m_OwnerVehicle, pDt, pOutResult.m_CameraTM);
	}
	
	//-----------------------------------------------------------------------------
	private void UpdateViewBob(float pDt, vector localVelocity, vector localAngularVelocity)
	{
		if (pDt <= 0)
			return;
		
		vector velDiff = localVelocity - m_vLastVel;
		vector angVelDiff = localAngularVelocity - m_vLastAngVel;
		float speed = localVelocity.Length();
		float accel = velDiff.Length() * (1 / pDt) + angVelDiff.Length() * (1 / pDt);
		float timeScale;
		if (accel > m_fBob_Acceleration)
			m_fBob_Acceleration += (accel - m_fBob_Acceleration) * Math.Clamp(pDt * 12, 0, 1);
		else
			m_fBob_Acceleration += (accel - m_fBob_Acceleration) * Math.Clamp(pDt * 7, 0, 1);
		
		// Add bobbing based on speed
		m_fBob_ScaleSlow += (Math.Clamp(speed / (m_fSpeedMax * KILOMETERS_PER_HOUR_TO_METERS_PER_SEC), 0, 1) - m_fBob_ScaleSlow) * (pDt * 4);
		float slowTimeSlice = (m_fBob_ScaleSlow * m_fBob_ScaleSlow * 1.5 + 1) * pDt;
		m_fBob_SlowUp += Math.Clamp(slowTimeSlice, 0, 1);
		if (m_fBob_SlowUp >= 1)
			m_fBob_SlowUp -= 1;
		m_fBob_SlowRight += Math.Clamp(slowTimeSlice * 0.75, 0, 1);
		if (m_fBob_SlowRight >= 1)
			m_fBob_SlowRight -= 1;
		
		// Add bobbing based on acceleration
		m_fBob_ScaleFast = Math.Clamp(m_fBob_Acceleration * 0.03, 0, 1);
		float fastTimeSlice = m_fBob_ScaleFast * 5 * pDt;
		if (m_fBob_ScaleFast < 0.01)
		{
			m_fBob_FastUp *= 1 - pDt;
			m_fBob_FastRight *= 1 - pDt;
		}
		else
		{
			m_fBob_FastUp += Math.Clamp(fastTimeSlice, 0, 1);
			if (m_fBob_FastUp >= 1)
				m_fBob_FastUp -= 1;
			m_fBob_FastRight += Math.Clamp(fastTimeSlice * 1.4, 0, 1);
			if (m_fBob_FastRight >= 1)
				m_fBob_FastRight -= 1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void AddViewBobToTransform(inout vector outTransform[4])
	{
		float bobFastScale = m_fBob_ScaleFast * m_fShakeScale;
		float bobSlowScale = m_fBob_ScaleSlow * m_fBob_ScaleSlow * m_fBobScale;
		
		float bobFw = Math.Sin(m_fBob_FastUp * 360 * Math.DEG2RAD) * -0.05 * bobFastScale;
		float bobUp = Math.Sin(m_fBob_FastUp * 360 * Math.DEG2RAD) * 0.05 * bobFastScale;
		float bobRt = Math.Sin(m_fBob_FastRight * 360 * Math.DEG2RAD) * 0.05 * bobFastScale;
		float bobYaw = Math.Sin(m_fBob_FastRight * 360 * Math.DEG2RAD) * 0.3 * bobFastScale;
		float bobPitch = Math.Sin(m_fBob_FastUp * 360 * Math.DEG2RAD) * 0.3 * bobFastScale;
		float bobRoll = Math.Sin(m_fBob_FastRight * 360 * Math.DEG2RAD) * 0.4 * bobFastScale;
		bobFw += Math.Sin(m_fBob_SlowUp * 360 * Math.DEG2RAD) * -0.025 * bobSlowScale;
		bobUp += Math.Sin(m_fBob_SlowUp * 360 * Math.DEG2RAD) * 0.05 * bobSlowScale;
		bobRt += Math.Sin(m_fBob_SlowRight * 360 * Math.DEG2RAD) * 0.05 * bobSlowScale;
		bobYaw += Math.Sin(m_fBob_SlowRight * 360 * Math.DEG2RAD) * 0.2 * bobSlowScale;
		bobPitch += Math.Sin(m_fBob_SlowUp * 360 * Math.DEG2RAD) * 0.2 * bobSlowScale;
		bobRoll += Math.Sin(m_fBob_SlowRight * 360 * Math.DEG2RAD) * 0.45 * bobSlowScale;
		
		vector angBobMat[3], endBobmat[3];
		Math3D.AnglesToMatrix(Vector(bobPitch, bobYaw, bobRoll), angBobMat);
		Math3D.MatrixMultiply3(outTransform, angBobMat, endBobmat);
		outTransform[0] = endBobmat[0];
		outTransform[1] = endBobmat[1];
		outTransform[2] = endBobmat[2];
		outTransform[3] = Vector(bobRt, bobUp, bobFw).Multiply3(outTransform) + outTransform[3];
	}
	
	//-----------------------------------------------------------------------------
	override float GetBaseFOV()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return 0;
		
		return cameraManager.GetVehicleFOV();
	}
}
