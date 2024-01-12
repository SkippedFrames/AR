[BaseContainerProps()]
class SCR_SignalPointDefinition
{
	[Attribute("1", UIWidgets.EditBox, "[ms]", params: "1 60000 0.001")]
	float m_fTimeMin;
	
	[Attribute("1", UIWidgets.EditBox, "[ms]", params: "1 60000 0.001")]
	float m_fTimeMax;
	
	[Attribute("", UIWidgets.EditBox, "")]
	float m_fValueMin;
	
	[Attribute("", UIWidgets.EditBox, "")]
	float m_fValueMax;
		
	//------------------------------------------------------------------------------------------------
	float GetValue()
	{
		return Math.RandomFloat(m_fValueMin, m_fValueMax);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetTime()
	{
		return Math.RandomFloat(m_fTimeMin, m_fTimeMax);
	}
	
};
