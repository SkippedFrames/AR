//! State of cluster which represents our processing of it.
//! ClusterState is persistent, while Cluster objects can be created
//! and deleted during their processing, while ClusterState should be passed
//! between Cluster objects which correspond to same actual cluster of targets.

enum EAITargetClusterState
{
	UNINITIALIZED,	// We have never processed it yet
	NONE,			// We don't know what to do with this yet
	INVESTIGATING,	// We are investigating the targets
	ATTACKING,		// We are attacking the targets
	DEFENDING,		// We defend ourselves against the targets
	LOST			// We haven't detected anything here for a long time, it's too old
}

class SCR_AITargetClusterState
{
	SCR_AIGroupTargetCluster m_Cluster;
	
	// Related to our processing of this cluster
	ref SCR_AIActivityBase m_Activity; // Activity assigned for this cluster
	float m_fTimer_ms = 0;
	float m_fMaxAge_s; // When age is above this, we switch to lost state
	EAITargetClusterState m_eState = EAITargetClusterState.NONE;				// Those two must be different at start to trigger a state switch event
	EAITargetClusterState m_ePrevState = EAITargetClusterState.UNINITIALIZED;
	
	// Values updated by ProcessTargets() call
	vector m_vBBMax;
	vector m_vBBMin;
	float m_fRadius; // Radius of cluster
	float m_fMaxTimestamp;
	int m_iCountDetected;
	int m_iCountIdentified;
	int m_iCountLost;
	int m_iCountDestroyed;
	
	
	void SCR_AITargetClusterState(SCR_AIGroupTargetCluster cluster)
	{
		m_Cluster = cluster;
	}
	
	//! Counts targets and saves them in member variables
	void ProcessTargets()
	{
		int nDetected = 0;
		int nIdentified = 0;
		int nLost = 0;
		int nDestroyed = 0;
		float maxTimestamp = 0;
		
		vector bbMax = vector.Zero;
		vector bbMin = vector.Zero;
		
		if (m_Cluster && !m_Cluster.m_aTargets.IsEmpty())
		{
			bbMax = Vector(-float.MAX, -float.MAX, -float.MAX);
			bbMin = Vector(float.MAX, float.MAX, float.MAX);
			
			foreach (SCR_AITargetInfo target : m_Cluster.m_aTargets)
			{
				EAITargetInfoCategory category = target.m_eCategory;
				switch (category)
				{
					case EAITargetInfoCategory.DETECTED:	nDetected++; break;
					case EAITargetInfoCategory.IDENTIFIED:	nIdentified++; break;
					case EAITargetInfoCategory.DESTROYED:	nDestroyed++; break;
					case EAITargetInfoCategory.LOST:		nLost++; break;
					case EAITargetInfoCategory.DISARMED:	nDestroyed++; break; // Disarmed counts as destroyed
				}
				
				if (target.m_fTimestamp > maxTimestamp)
					maxTimestamp = target.m_fTimestamp;
				
				vector tgtPos = target.m_vWorldPos;
				for (int i = 0; i < 3; i++)
				{
					float tgtPosI = tgtPos[i];
					if (tgtPosI > bbMax[i])
						bbMax[i] = tgtPosI;
					if (tgtPosI < bbMin[i])
						bbMin[i] = tgtPosI;
				}
			}
		}
		
		m_iCountLost = nLost;
		m_iCountDetected = nDetected;
		m_iCountIdentified = nIdentified;
		m_iCountDestroyed = nDestroyed;
		m_fMaxTimestamp = maxTimestamp;
		m_vBBMin = bbMin;
		m_vBBMax = bbMax;
	}
	
	//! Returns time passed since any new information was received from this cluster
	//! You must call ProcessTargets first
	float GetTimeSinceLastNewInformation()
	{
		PerceptionManager pm = GetGame().GetPerceptionManager();
		if (!pm)
			return 0;
		
		return pm.GetTime() - m_fMaxTimestamp;
	}
}
