void SCR_AIOnFireteamRemoved(SCR_AIGroupFireteam fireteam);
typedef func SCR_AIOnFireteamRemoved;

class SCR_AIGroupFireteamManager : Managed
{
	protected const int FIRETEAM_MIN_SIZE = 2;
	
	protected SCR_AIGroup m_Group;
	protected ref array<ref SCR_AIGroupFireteam> m_aFireteams = {};
	bool m_bRebalanceFireteams = false; // True when fireteams become unbalanced
	
	// Fireteam events
	protected ref ScriptInvokerBase<SCR_AIOnFireteamRemoved> Event_OnFireteamRemoved = new ScriptInvokerBase<SCR_AIOnFireteamRemoved>();
	
	//---------------------------------------------------------------------------------------------------
	void SCR_AIGroupFireteamManager(SCR_AIGroup group)
	{
		m_Group = group;
	}
	
	//---------------------------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIOnFireteamRemoved> GetOnFireteamRemoved()
	{
		return Event_OnFireteamRemoved;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Called from SCR_AIGroupUtilityComponent
	void OnAgentAdded(AIAgent agent, SCR_AIInfoComponent infoComp)
	{	
		// Add to fireteam
		// We find smallest fireteam and rebalance them later
		SCR_AIGroupFireteam destFt = FindSmallestFireteam();
		if (!destFt)
			destFt = CreateFireteam();
		destFt.AddMember(agent, infoComp);
		
		m_bRebalanceFireteams = true;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Called from SCR_AIGroupUtilityComponent
	void OnAgentRemoved(AIAgent agent)
	{
		// Remove from fireteam
		SCR_AIGroupFireteam ft = FindFireteam(agent);
		if (ft)
		{
			ft.RemoveMember(agent); // bye
			
			int ftSize = ft.GetMemberCount();
			if (ftSize == 0)
				RemoveFireteam(ft);
			
			// Rebalance fireteams later
			m_bRebalanceFireteams = true;
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected SCR_AIGroupFireteam CreateFireteam()
	{
		SCR_AIGroupFireteam ft = new SCR_AIGroupFireteam();
		m_aFireteams.Insert(ft);
		return ft;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void RemoveFireteam(SCR_AIGroupFireteam ft)
	{
		int id = m_aFireteams.Find(ft);
		if (id == -1)
			return;
		
		if (ft.GetMemberCount() != 0)
		{
			Print("SCR_AIGroupUtilityComponent: removing a non-empty fireteam", LogLevel.ERROR);
			return;
		}
		
		Event_OnFireteamRemoved.Invoke(ft);
		
		m_aFireteams.Remove(id);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected SCR_AIGroupFireteam FindSmallestFireteam(array<SCR_AIGroupFireteam> fireteamsExclude = null)
	{
		if (m_aFireteams.IsEmpty())
			return null;
		
		int minSize = int.MAX;
		SCR_AIGroupFireteam outFt;
		for (int i = 0; i < m_aFireteams.Count(); i++)
		{
			SCR_AIGroupFireteam ft = m_aFireteams[i];
			int size = ft.GetMemberCount();
			if (size < minSize)
			{
				if (!fireteamsExclude || (fireteamsExclude && fireteamsExclude.Find(ft) == -1))
				{
					minSize = size;
					outFt = ft;
				}
			}
		}
		
		return outFt;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected SCR_AIGroupFireteam FindBiggestFireteam(array<SCR_AIGroupFireteam> fireteamsExclude = null)
	{
		if (m_aFireteams.IsEmpty())
			return null;
		
		int maxSize = int.MIN;
		SCR_AIGroupFireteam outFt;
		for (int i = 0; i < m_aFireteams.Count(); i++)
		{
			SCR_AIGroupFireteam ft = m_aFireteams[i];
			int size = ft.GetMemberCount();
			if (size > maxSize)
			{
				if (!fireteamsExclude || (fireteamsExclude && fireteamsExclude.Find(ft) == -1))
				{
					maxSize = size;
					outFt = ft;
				}
			}
		}
		
		return outFt;
	}
	
	//---------------------------------------------------------------------------------------------------
	// Tries to find at least 'count' free fireteams. Returns true if it was able to achieve this amount of fireteams.
	bool FindFreeFireteams(notnull array<SCR_AIGroupFireteam> outFireteams, int count, array<SCR_AIGroupFireteam> fireteamsExclude = null)
	{
		outFireteams.Clear();
		for (int i = 0; i < m_aFireteams.Count(); i++)
		{
			SCR_AIGroupFireteam ft = m_aFireteams[i];
			if (!ft.IsLocked())
			{
				if (!fireteamsExclude || (fireteamsExclude && fireteamsExclude.Find(ft) == -1))
				{
					outFireteams.Insert(ft);
					if (outFireteams.Count() == count)
						return true;
				}
			}
		}
		
		return false;
	}
	
	//---------------------------------------------------------------------------------------------------
	// Returns all free fireteams
	void GetFreeFireteams(notnull array<SCR_AIGroupFireteam> outFireteams, array<SCR_AIGroupFireteam> fireteamsExclude = null)
	{
		outFireteams.Clear();
		for (int i = 0; i < m_aFireteams.Count(); i++)
		{
			SCR_AIGroupFireteam ft = m_aFireteams[i];
			if (!ft.IsLocked())
			{
				if (!fireteamsExclude || (fireteamsExclude && fireteamsExclude.Find(ft) == -1))
				{
					outFireteams.Insert(ft);
				}
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Finds fireteam which has the provided agent
	SCR_AIGroupFireteam FindFireteam(AIAgent agent)
	{
		foreach (SCR_AIGroupFireteam ft : m_aFireteams)
		{
			if (ft.HasMember(agent))
				return ft;
		}
		return null;
	}
	
	//---------------------------------------------------------------------------------------------------
	int GetFireteamCount()
	{
		return m_aFireteams.Count();
	}
	
	//---------------------------------------------------------------------------------------------------
	int GetFireteamId(notnull SCR_AIGroupFireteam ft)
	{
		foreach (int id, auto _ft : m_aFireteams)
		{
			if (_ft == ft)
				return id;
		}
		return -1;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected static int GetMaxFireteamSize(int groupSize)
	{
		if (groupSize >= 12)
			return 4;
		else if (groupSize > 4)
			return 3;
		else if (groupSize == 4)
			return 2; // When exactly 4 members, we want two fireteams of 2 members
		else
			return groupSize; // When below 4 members, one fireteam or 1-2-3
	}
	
	
	//---------------------------------------------------------------------------------------------------
	void RebalanceFireteams()
	{
		int groupSize = m_Group.GetAgentsCount();
		
		// Bail if group size is 0
		if (groupSize == 0)
		{
			m_bRebalanceFireteams = false;
			return;
		}
				
		int maxFtSize = GetMaxFireteamSize(groupSize);
		float fMaxFtSize = maxFtSize;
		int agentsCount = m_Group.GetAgentsCount();
		float fAgentsCount = agentsCount;
		
		int desiredFtCount = Math.Ceil(fAgentsCount / fMaxFtSize);
		
		if (m_aFireteams.Count() < desiredFtCount)
		{
			// Create new fireteams
			int newFtCount = desiredFtCount - m_aFireteams.Count();
			for (int i = 0; i < newFtCount; i++)
				CreateFireteam();
		}
		else if (m_aFireteams.Count() > desiredFtCount)
		{
			// Delete fireteams ...
			
			int deleteFtCount = m_aFireteams.Count() - desiredFtCount;
			array<SCR_AIGroupFireteam> fireteamsDelete = {};
			
			// Find smallelst fireteams for deletion
			for (int i = 0; i < deleteFtCount; i++)
			{
				SCR_AIGroupFireteam smallestFt = FindSmallestFireteam(fireteamsDelete);
				fireteamsDelete.Insert(smallestFt);
			}
			
			// Move members from those selected fireteams, and delete them
			foreach (SCR_AIGroupFireteam fireteamDelete : fireteamsDelete)
			{
				SCR_AIGroupFireteam destinationFt = FindSmallestFireteam(fireteamsDelete);
				destinationFt.MoveMembersFrom(fireteamDelete, fireteamDelete.GetMemberCount());
				RemoveFireteam(fireteamDelete);
			}
		}
		
		array<SCR_AIGroupFireteam> fireteamsTooBig = {};
		array<SCR_AIGroupFireteam> fireteamsTooSmall = {};
		CountUnbalancedFireteams(fireteamsTooBig, fireteamsTooSmall);
		
		//array<SCR_AIGroupFireteam> fireteamsExclude = {};
		bool failed = false;
		int nIterations = 0;
		const int maxIterations = 128;
		while ((!fireteamsTooBig.IsEmpty() || !fireteamsTooSmall.IsEmpty()) && !failed && nIterations < maxIterations)
		{
			if (!fireteamsTooBig.IsEmpty())
			{
				// First split big fireteams
				SCR_AIGroupFireteam srcFt = fireteamsTooBig[0];
				
				// Take one big fireteam and move some members to smaller fireteams
				int nExcessMembers = srcFt.GetMemberCount() - maxFtSize;
				for (int i = 0; i < nExcessMembers; i++)
				{
					SCR_AIGroupFireteam dstFt = FindSmallestFireteam(fireteamsTooBig);
					if (!dstFt)
					{
						// It shouldn't be possible
						Print(string.Format("SCR_AIGroupUtilityComponent: failed to reorganize fireteams, all other fireteams are full. %1",
							DiagGetFireteamsData()), LogLevel.ERROR);
						failed = true;
						break;
					}
					else
					{
						dstFt.MoveMembersFrom(srcFt, 1);
					}
				}
			}
			else if (!fireteamsTooSmall.IsEmpty())
			{
				// Second fill up the smallest fireteams
				SCR_AIGroupFireteam dstFt = fireteamsTooSmall[0];
				
				int nLackMembers = FIRETEAM_MIN_SIZE - dstFt.GetMemberCount(); // How many more members we need
				
				for (int i = 0; i < nLackMembers; i++)
				{
					SCR_AIGroupFireteam srcFt = FindBiggestFireteam(fireteamsTooSmall);
					
					if (!srcFt)
					{
						// It shouldn't be possible in general case
						// It could only happen when group size is very small
						//if (agentsCount > maxFtSize)
							Print(string.Format("SCR_AIGroupUtilityComponent: failed to reorganize fireteams, all other fireteams are too small. %1",
								DiagGetFireteamsData()), LogLevel.ERROR);
						failed = true; // For very small group size (1) it might be impossible to make 'balanced' fireteams at all, due to lack of group members
						break;
					}
					else
					{
						dstFt.MoveMembersFrom(srcFt, 1);
					}
				}
				
			}
			
			CountUnbalancedFireteams(fireteamsTooBig, fireteamsTooSmall);
			nIterations++;
		}
		
		if (nIterations == maxIterations)
		{
			Print(string.Format("SCR_AIGroupUtilityComponent: RebalanceFireteams: max amount of iterations has been reached. %1", DiagGetFireteamsData()), LogLevel.ERROR);
		}
		
		m_bRebalanceFireteams = false;
	}
	protected void CountUnbalancedFireteams(notnull array<SCR_AIGroupFireteam> fireteamsTooBig, notnull array<SCR_AIGroupFireteam> fireteamsTooSmall)
	{
		int maxFtSize = GetMaxFireteamSize(m_Group.GetAgentsCount());
		fireteamsTooBig.Clear();
		fireteamsTooSmall.Clear();
		foreach (SCR_AIGroupFireteam ft : m_aFireteams)
		{
			int size = ft.GetMemberCount();
			if (size == maxFtSize) // Perfect
				continue;
			
			if (size > maxFtSize)
				fireteamsTooBig.Insert(ft);
			else if (size < FIRETEAM_MIN_SIZE)
				fireteamsTooSmall.Insert(ft); // Includes empty or with one member
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Returns string with data about fireteams
	string DiagGetFireteamsData()
	{
		string s = string.Format("Fireteams: %1: ", m_aFireteams.Count());
		foreach (SCR_AIGroupFireteam ft : m_aFireteams)
		{
			string strLocked = string.Empty;
			if (ft.IsLocked())
				strLocked = "L";
			
			s = s + string.Format("%1%2, ", ft.GetMemberCount(), strLocked);
		}
		return s;
	}
	
	//---------------------------------------------------------------------------------------------------
	void DiagDrawFireteams()
	{
		array<AIAgent> members = {};
		foreach (int fireteamId, SCR_AIGroupFireteam ft : m_aFireteams)
		{
			ft.GetMembers(members);
			foreach (AIAgent agent : members)
			{
				IEntity e = agent.GetControlledEntity();
				if (!e)
					continue;
				vector textPos = e.GetOrigin() + Vector (0, 0.5, 0);
				string text = string.Format("FT: %1", fireteamId);
				DebugTextWorldSpace.Create(GetGame().GetWorld(), text, DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
					textPos[0], textPos[1], textPos[2], color: Color.GREEN, bgColor: Color.BLACK,
					size: 13.0); 
			}
		}
	}
}
