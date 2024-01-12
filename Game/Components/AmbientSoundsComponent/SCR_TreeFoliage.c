[BaseContainerProps()]
class SCR_TreeFoliage
{
	bool m_bUpdatePosition;
	// Foliage width
	float m_fWidth;
	// Foliage center 2D
	vector m_vCenter;
	// Foliage min height
	float m_fMinY;
	// Foliage max height
	float m_fMaxY;
	// Foliage sound audio handle
	AudioHandle m_AudioHandle;
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TreeFoliage()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
	}
};

