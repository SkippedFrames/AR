[BaseContainerProps()]
class SCR_SignalDefinition
{
	[Attribute("", UIWidgets.EditBox, "")]
	string m_sSignalName;
	
	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref SCR_SignalPointDefinition> m_aSignalPointDefinition;
	
	int m_iSignalIdx;
	
	float m_fTargetValue;
	float m_fCurrentValue;
	float m_fSpeed;
	int m_iCurrentPointIndex;
	
	//------------------------------------------------------------------------------------------------
	float GetSignalValue(float timeSlice)
	{
		m_fCurrentValue = m_fCurrentValue + m_fSpeed * timeSlice;
		
		if (m_fSpeed >= 0 && m_fCurrentValue >= m_fTargetValue || m_fSpeed < 0 && m_fCurrentValue <= m_fTargetValue)
		{
			// Saturate
			m_fCurrentValue = m_fTargetValue;
			
			// Get next index
			m_iCurrentPointIndex++;
			if (m_iCurrentPointIndex >= m_aSignalPointDefinition.Count())
				m_iCurrentPointIndex = 0;
						
			// Get new target value
			SCR_SignalPointDefinition signalPoindDefinition = m_aSignalPointDefinition[m_iCurrentPointIndex];
			m_fTargetValue = signalPoindDefinition.GetValue();
			
			// Get speed
			float time = signalPoindDefinition.GetTime() * 0.001;
			m_fSpeed = (m_fTargetValue - m_fCurrentValue) / time;	
		}
		
		return m_fCurrentValue;
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_SignalDefinition()
	{		
		int count = m_aSignalPointDefinition.Count();
		if (count == 0)
			return;
		
		// Get current value
		m_fCurrentValue = m_aSignalPointDefinition[0].GetValue();
		
		// Get next index
		if (count == 1)
			m_iCurrentPointIndex = 0;
		else
			m_iCurrentPointIndex = 1;
			
		// Get target value
		SCR_SignalPointDefinition signalPoindDefinition = m_aSignalPointDefinition[m_iCurrentPointIndex];
		m_fTargetValue = signalPoindDefinition.GetValue();
			
		// Get speed
		float time = signalPoindDefinition.GetTime() * 0.001;
		m_fSpeed = (m_fTargetValue - m_fCurrentValue) / time;		
	}
};
