class SCR_AIGroupFireteam : Managed
{
	protected ref array<AIAgent> m_aAgents = {};
	protected ref array<SCR_AIInfoComponent> m_aInfoComponents = {};
	
	protected bool m_bLocked = false;
	
	//--------------------------------------------------------------------------
	void AddMember(AIAgent agent, SCR_AIInfoComponent infoComponent)
	{
		m_aAgents.Insert(agent);
		m_aInfoComponents.Insert(infoComponent);
	}
	
	//--------------------------------------------------------------------------
	void RemoveMember(AIAgent agent)
	{
		int id = m_aAgents.Find(agent);
		if (id == -1)
			return;
		m_aAgents.Remove(id);
		m_aInfoComponents.Remove(id);
	}
	
	//--------------------------------------------------------------------------
	protected void RemoveMember(int id, out AIAgent outAgent, out SCR_AIInfoComponent outInfoComp)
	{
		outAgent = m_aAgents[id];
		outInfoComp = m_aInfoComponents[id];
		
		m_aAgents.Remove(id);
		m_aInfoComponents.Remove(id);
	}
	
	//--------------------------------------------------------------------------
	void GetMember(int id, out AIAgent outAgent, out SCR_AIInfoComponent outInfoComp)
	{
		if (!m_aAgents.IsIndexValid(id))
			return;
		
		outAgent = m_aAgents[id];
		outInfoComp = m_aInfoComponents[id];
	}
	
	//--------------------------------------------------------------------------
	AIAgent GetMember(int id)
	{
		if (!m_aAgents.IsIndexValid(id))
			return null;
		
		return m_aAgents[id];
	}
	
	//--------------------------------------------------------------------------
	IEntity GetFirstMemberEntity()
	{
		if (m_aAgents.IsEmpty())
			return null;
		foreach (AIAgent agent : m_aAgents)
		{
			if (!agent)
				continue;
			IEntity controlledEntity = agent.GetControlledEntity();
			return controlledEntity;
		}
		return null;
	}
	
	//--------------------------------------------------------------------------
	void GetMembers(notnull array<AIAgent> outAgents)
	{
		outAgents.Clear();
		foreach (auto a : m_aAgents)
			outAgents.Insert(a);
	}
	
	//--------------------------------------------------------------------------
	//! Moves 'count' members from other fireteam to this one
	void MoveMembersFrom(notnull SCR_AIGroupFireteam otherFt, int count)
	{
		// Bail if wrong count
		if (count <= 0)
			return;
		
		// Clamp count
		count = Math.ClampInt(count, 0, otherFt.m_aAgents.Count());
		
		// Remove the required amount of members
		for (int i = 0; i < count; i++)
		{
			// Remove last member from other fireteam
			int lastId = otherFt.m_aAgents.Count() - 1;
			AIAgent agent;
			SCR_AIInfoComponent infoComp;
			otherFt.RemoveMember(lastId, agent, infoComp);
			
			// Add the member to our fireteam
			AddMember(agent, infoComp);
		}
	}
	
	//--------------------------------------------------------------------------
	bool HasMember(AIAgent agent)
	{
		return m_aAgents.Find(agent) != -1;
	}
	
	//--------------------------------------------------------------------------
	int GetMemberCount()
	{
		return m_aAgents.Count();
	}
	
	//--------------------------------------------------------------------------
	//! Don't ever try to free a fireteam yourself, use SCR_AIGroupFireteamLock instead
	void Internal_OnLockDestroyed()
	{
		m_bLocked = false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Acquiring fireteams
	
	//--------------------------------------------------------------------------
	bool IsLocked()
	{
		return m_bLocked;
	}
	
	//--------------------------------------------------------------------------
	//! Tries to lock the fireteam, on success returns the Lock object.
	//! Store the returned Lock object as strong reference.
	SCR_AIGroupFireteamLock TryLock()
	{
		if (m_bLocked)
			return null;
		
		m_bLocked = true;
		
		SCR_AIGroupFireteamLock lock = new SCR_AIGroupFireteamLock(this);
		return lock;
	}
};
