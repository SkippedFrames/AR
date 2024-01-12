//#define CURVE_APPLIED
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
/** @ingroup ManualCamera
*/

/*!
Camera speed based on height from terrain
*/
class SCR_SpeedByHeightManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("0.06", UIWidgets.Auto, "")]
	float m_fSpeedCoef;
	
	[Attribute(defvalue: "0.15")]
	float m_fMinSpeed;
	
	[Attribute(defvalue: "400")]
	float m_fMaxHeight;
	
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, params: "1 1 0 0")]
	private ref Curve m_fHeightCoef;

#ifdef CURVE_APPLIED
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		vector pos = CoordFromCamera(param.transform[3]);
		float surfaceY = param.world.GetSurfaceY(pos[0], pos[2]);
		if (pos[1] > 0) surfaceY = Math.Max(surfaceY, 0); //--- When above ground, use ASL, not ATL height
		float height = Math3D.Curve(ECurveType.CurveProperty2D, (pos[1] - surfaceY) / m_fMaxHeight, m_fHeightCoef)[1];
		param.multiplier *= Math.Max(height * m_fMaxHeight * m_fSpeedCoef, m_fMinSpeed);			
	}
#else
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		vector pos = CoordFromCamera(param.transform[3]);
		float surfaceY = param.world.GetSurfaceY(pos[0], pos[2]);
		if (pos[1] > 0) surfaceY = Math.Max(surfaceY, 0); //--- When above ground, use ASL, not ATL height
		float height = pos[1] - surfaceY;
		param.multiplier *= Math.Max(height * m_fSpeedCoef, m_fMinSpeed);
	}
#endif
	override bool EOnCameraInit()
	{
		return true;
	}
};

















