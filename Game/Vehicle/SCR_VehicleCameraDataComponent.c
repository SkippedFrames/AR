[ComponentEditorProps(category: "GameScripted/Vehicle", description:"Vehicle camera data")]
class SCR_VehicleCameraDataComponentClass: ScriptComponentClass
{
};


class SCR_VehicleCameraDataComponent : ScriptComponent
{
	[Attribute("false", UIWidgets.CheckBox, "Override the vehicle camera data component of the parent Vehicle\n")]
	bool m_bOverrideVehicleSettings;
	[Attribute("1", UIWidgets.EditBox, "Height the camera should stay above the vehicle's bounding box center\n[m]")]
	float m_fHeight;
	[Attribute("1", UIWidgets.EditBox, "Speed-based view bobbing multiplier\n[x * 100%]")]
	float m_fBobScale;
	[Attribute("1", UIWidgets.EditBox, "Acceleration-based view shaking multiplier\n[x * 100%]")]
	float m_fShakeScale;
	[Attribute("150", UIWidgets.EditBox, "Maximum speed at which camera distance and FOV adjustments are at their maximum\n[km/h]")]
	float m_fSpeedMax;
	[Attribute("8", UIWidgets.EditBox, "Maximum distance the camera should keep from the vehicle\n[m]")]
	float m_fDist_Max;
	[Attribute("3", UIWidgets.EditBox, "Minimum distance the camera should keep from the vehicle\n[m]")]
	float m_fDist_Min;
	[Attribute("5", UIWidgets.EditBox, "Desired distance the camera should keep from the vehicle\n[m]")]
	float m_fDist_Desired;
	[Attribute("70", UIWidgets.EditBox, "Field Of View of the camera\n[deg]")]
	float m_fFOV;
	[Attribute("10", UIWidgets.EditBox, "Maximum Field Of View adjustment based on speed\n[deg]")]
	float m_fFOV_SpeedAdjustMax;
	[Attribute("0.4", UIWidgets.Slider, "How much roll of the parent vehicle is applied to camera\n[x * 100%]", params: "-5 5 0.001")]
	float m_fRollFactor;
	[Attribute("0.2", UIWidgets.Slider, "How much pitch of the parent vehicle is applied to camera\n[x * 100%]", params: "-5 5 0.001")]
	float m_fPitchFactor;
	[Attribute("20", UIWidgets.EditBox, "Angle in first person added to the camera on the vehicle\n[deg]")]
	float m_fAngleFirstPerson;
	[Attribute("20", UIWidgets.EditBox, "Angle in third person added to the camera on the vehicle\n[deg]")]
	float m_fAngleThirdPerson;
	[Attribute(uiwidget: UIWidgets.Object)]
	ref SCR_VehicleCameraAimpoint m_pCameraAimpointData;
	[Attribute(uiwidget: UIWidgets.Object)]
	ref SCR_VehicleCameraAlignment m_pCameraAlignData;
};
