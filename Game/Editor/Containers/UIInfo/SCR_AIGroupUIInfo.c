class SCR_AIGroupUIInfo : SCR_UIInfo
{
	protected ResourceName m_sGroupFlag;
	protected bool m_bFlagIsFromImageSet;
	
	//------------------------------------------------------------------------------------------------
	void SetGroupFlag(ResourceName value)
	{
		m_sGroupFlag = value;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetGroupFlag()
	{
		return m_sGroupFlag;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFlagIsFromImageSet(bool value)
	{
		m_bFlagIsFromImageSet = value;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetFlagIsFromImageSet()
	{
		return m_bFlagIsFromImageSet;
	}
}
