// *************************************************************************************
// ! CharacterCamera1stPerson - 1st person camera
// *************************************************************************************
class CharacterCamera1stPerson extends CharacterCameraBase
{
	//-----------------------------------------------------------------------------
	static const float 	CONST_UD_MIN	= -89.0;		//!< down limit
	static const float 	CONST_UD_MAX	= 89.0;			//!< up limit

	static const float 	CONST_LR_MIN	= -160.0;		//!< left limit
	static const float 	CONST_LR_MAX	= 160.0;		//!< right limit
	
	//-----------------------------------------------------------------------------
	void CharacterCamera1stPerson(CameraHandlerComponent pCameraHandler)
	{
		m_pCompartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		// in head bone space
		m_OffsetLS = "0.0 0.03 -0.07";
		m_ApplyHeadBob = true;
	}
	
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		m_bCameraTransition = false;
	}
	
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		//Print("m_fLeftRightAngle : " + m_fLeftRightAngle);
		
		pOutResult.m_vBaseAngles 		= m_CharacterHeadAimingComponent.GetLookAngles();
		pOutResult.m_fUseHeading 		= 1.0;
		pOutResult.m_iDirectBoneMode 	= EDirectBoneMode.RelativePosition;
		pOutResult.m_iDirectBone 		= GetCameraBoneIndex();
		//! update fov
		m_fFOV = GetBaseFOV();
		if (m_bCameraTransition)
		{
			pOutResult.m_fUseHeading = 0.0;
			pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativeTransform;
			
			// Currently the 1pv camera bone in getin/getout needs to be investigated,
			// so as a workaround, we use head bone directly with position only
			if (sm_iHeadBoneIndex != -1)
			{
				pOutResult.m_iDirectBone = sm_iHeadBoneIndex;
				pOutResult.m_fUseHeading = 0.0;
				pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativePosition;
			}
		}
		
		vector additiveRotation = "0 0 0";
		m_CharacterHeadAimingComponent.GetLookTransformationLS(pOutResult.m_iDirectBone, pOutResult.m_iDirectBoneMode, m_OffsetLS, additiveRotation, pOutResult.m_CameraTM);

		if (m_UseLookPositionOverrideLS)
			pOutResult.m_CameraTM[3] = m_LookPositionOverrideLS;
		
		vector vYPR = "0 0 0";
		vYPR[1] = m_OwnerCharacter.GetLocalYawPitchRoll()[1];
		vector mat[4];
		Math3D.AnglesToMatrix(vYPR, mat);
		Math3D.MatrixMultiply4(mat, pOutResult.m_CameraTM, pOutResult.m_CameraTM);
		
		if( m_ApplyHeadBob )
			m_CharacterCameraHandler.AddViewBobToTransform(pOutResult.m_CameraTM, 1, false);

		pOutResult.m_fFOV		 			= m_fFOV;
		pOutResult.m_fNearPlane	 			= 0.0125;
		pOutResult.m_bUpdateWhenBlendOut 	= true;
		pOutResult.m_fPositionModelSpace 	= 0.0;
		pOutResult.m_bAllowInterpolation 	= true;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_pWSAttachmentReference = null;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}

	//-----------------------------------------------------------------------------
	override float GetBaseFOV()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return 0;
		
		return cameraManager.GetFirstPersonFOV();
	}
	
	//-----------------------------------------------------------------------------
	protected	vector	m_OffsetLS;			//!< position offset 

	protected	bool	m_ApplyHeadBob;

	protected bool m_bCameraTransition = false;
	protected CompartmentAccessComponent m_pCompartmentAccess;
	
	protected bool m_UseLookPositionOverrideLS = false;
	protected vector m_LookPositionOverrideLS;
};

class CharacterCamera1stPersonBoneTransform extends CharacterCamera1stPerson
{
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);
		pOutResult.m_fUseHeading = 0.0;
		pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativeTransform;
		vector additiveRotation = "0 0 0";
		m_CharacterHeadAimingComponent.GetLookTransformationLS(pOutResult.m_iDirectBone, pOutResult.m_iDirectBoneMode, m_OffsetLS, additiveRotation, pOutResult.m_CameraTM);
	}
}
