[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
/** @ingroup ManualCamera
*/

/*!
Controls lense flare for camera
*/
class SCR_LensFlareManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("0", desc: "Lens flare set on camera. This is also the default lens flare set when camera is created. Note, types that require strings are not supported", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(CameraLensFlareSetType))]
	protected CameraLensFlareSetType m_iLensFlareType;
	
	CameraLensFlareSetType GetLensFlareType()
	{
		return m_iLensFlareType;
	}
	
	void SetLensFlareType(CameraLensFlareSetType newLensFlare)
	{
		m_iLensFlareType = newLensFlare;
		GetCameraEntity().SetLensFlareSet(newLensFlare, string.Empty);
	}
	
	override bool EOnCameraInit()
	{
		GetCameraEntity().SetLensFlareSet(m_iLensFlareType, string.Empty);
		return true;
	}
	
	override void EOnCameraSave(SCR_ManualCameraComponentSave data)
	{
		data.m_aValues = {m_iLensFlareType};
	}
	
	override void EOnCameraLoad(SCR_ManualCameraComponentSave data)
	{
		if (data.m_aValues && !data.m_aValues.IsEmpty())
			SetLensFlareType((int)data.m_aValues[0]);
	}
}
