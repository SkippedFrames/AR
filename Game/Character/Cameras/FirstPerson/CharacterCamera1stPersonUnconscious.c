// *************************************************************************************
// ! CharacterCamera1stPersonUnconscious - first person only unconscious
// *************************************************************************************
class CharacterCamera1stPersonUnconscious extends CharacterCamera1stPerson
{
	void CharacterCamera1stPersonUnconscious(CameraHandlerComponent pCameraHandler)
	{
	}
	
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		// Note: The transformation is discarded (overwritten) later on
		super.OnUpdate(pDt, pOutResult);

		pOutResult.m_vBaseAngles 		= "0 0 0";
		pOutResult.m_fUseHeading 		= 0.0;
		pOutResult.m_iDirectBoneMode 	= EDirectBoneMode.RelativeTransform;
		
		// TODO@AS: For now always use the camera bone. If transition is animated,
		// it is safe to revert to using GetCameraBoneIndex(), assuming sm_TagFPCamera is set.
		pOutResult.m_iDirectBone 		= sm_iCameraBoneIndex;
		
		// Head bone in model space is rotated 180 around its yaw axis,
		// so we simply undo this rotation
		vector lookAngles = m_CharacterHeadAimingComponent.GetLookAngles();
		float y = lookAngles[0] - 180.0;

		while (y < -180)
			y += 360;
		while (y > 180)
			y -= 360;

		lookAngles[0] = y;
		
		Math3D.AnglesToMatrix(lookAngles, pOutResult.m_CameraTM);	
		pOutResult.m_CameraTM[3] = m_OffsetLS;
		
		// Apply camera shake if there is shake to be applied
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
};
