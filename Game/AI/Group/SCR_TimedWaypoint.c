class SCR_TimedWaypointClass: SCR_AIWaypointClass
{
};

class SCR_TimedWaypoint : SCR_AIWaypoint
{
	[Attribute("", UIWidgets.EditBox, "Minimal time to hold the waypoint before it completes")]
	float m_holdingTime;
	
	float GetHoldingTime()
	{
		return m_holdingTime;
	}
	
	void SetHoldingTime(float newTime)
	{
		 m_holdingTime = newTime;
	}
};
