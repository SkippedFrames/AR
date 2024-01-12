//------------------------------------------------------------------------------------------------
class SCR_ScriptedDamageManagerData
{
	int m_iNextFreeIndex = -1;
	protected ref ScriptInvoker m_OnDamage;
	protected ref ScriptInvoker m_OnDamageOverTimeAdded;
	protected ref ScriptInvoker m_OnDamageOverTimeRemoved;
	protected ref ScriptInvoker m_OnDamageStateChanged;

	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		delete m_OnDamage;
		delete m_OnDamageStateChanged;
		delete m_OnDamageOverTimeAdded;
		delete m_OnDamageOverTimeRemoved;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamage(bool createNew = true)
	{
		if (!m_OnDamage && createNew)
			m_OnDamage = new ScriptInvoker();
		return m_OnDamage;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageOverTimeAdded(bool createNew = true)
	{
		if (!m_OnDamageOverTimeAdded && createNew)
			m_OnDamageOverTimeAdded = new ScriptInvoker();
		return m_OnDamageOverTimeAdded;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageOverTimeRemoved(bool createNew = true)
	{
		if (!m_OnDamageOverTimeRemoved && createNew)
			m_OnDamageOverTimeRemoved = new ScriptInvoker();
		return m_OnDamageOverTimeRemoved;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageStateChanged(bool createNew = true)
	{
		if (!m_OnDamageStateChanged && createNew)
			m_OnDamageStateChanged = new ScriptInvoker();
		return m_OnDamageStateChanged;
	}
};
