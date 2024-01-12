//------------------------------------------------------------------------------------------------
class SCR_DestructionMultiPhaseData
{
	// This integer is public as we don't want to have unnecessary getter & setter calls for it
	int m_iNextFreeIndex = -1;
	protected int m_iDamagePhase = 0;
	protected int m_iTargetDamagePhase = 0;
	protected ResourceName m_OriginalResourceName;
	
	//------------------------------------------------------------------------------------------------
	int GetTargetDamagePhase()
	{
		return m_iTargetDamagePhase;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTargetDamagePhase(int targetDamagePhase)
	{
		m_iTargetDamagePhase = targetDamagePhase;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetOriginalResourceName()
	{
		return m_OriginalResourceName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOriginalResourceName(ResourceName originalResourceName)
	{
		m_OriginalResourceName = originalResourceName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetDamagePhase()
	{
		return m_iDamagePhase;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDamagePhase(int damagePhase)
	{
		m_iDamagePhase = damagePhase;
	}
	
	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		m_iDamagePhase = 0;
		m_iTargetDamagePhase = 0;
		m_OriginalResourceName = "";
	}
};
