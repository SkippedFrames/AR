// *************************************************************************************
// ! CharacterCamera3rdPersonProne - 3rd person camera in prone stance
// *************************************************************************************
class CharacterCamera3rdPersonProne extends CharacterCamera3rdPersonBase
{
	private float m_fCameraOffsetChangeVel;
	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonProne(CameraHandlerComponent pCameraHandler)
	{
		m_fDistance 		= 1.4;
		m_CameraOffsetMS	= "0.0 0.2 0.0";
		m_CameraOffsetLS	= "0.0 0.2 0.0";
		m_fShoulderWidth	= 0.4;
	}
	
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		pOutResult.m_fFOV = m_fFOV;
		float nextMSOffset = 0.2;
		bool sprinting = m_ControllerComponent.IsSprinting();
		if( sprinting )
			pOutResult.m_fFOV = m_fFOV + 2 * m_fBobScale;
		
		m_CameraOffsetMS[1] =  Math.SmoothCD(m_CameraOffsetMS[1], nextMSOffset, m_fCameraOffsetChangeVel, 0.2, 1000, pDt);
		super.OnUpdate(pDt, pOutResult);
	
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
};
