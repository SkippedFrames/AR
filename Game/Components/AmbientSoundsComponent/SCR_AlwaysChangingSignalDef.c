[BaseContainerProps(configRoot: true)]
class SCR_AlwaysChangingSignalDef
{
	[Attribute("", UIWidgets.EditBox, "[ms]")]
	float m_fInterpolationTimeMin;
	
	[Attribute("", UIWidgets.EditBox, "[ms]")]
	float m_fInterpolationTimeMax;

	[Attribute("", UIWidgets.EditBox, "")]
	float m_fSignalValueMin;
	
	[Attribute("", UIWidgets.EditBox, "")]
	float m_fSignalValueMax;

	[Attribute("", UIWidgets.EditBox, "")]
	string m_sSignalName;
	
	int m_iSignalIdx;
	float m_fInterpolationTime;
	float m_fSignalTarget;
	float m_fSignalTargetLast;
	float m_fTimer;
};
