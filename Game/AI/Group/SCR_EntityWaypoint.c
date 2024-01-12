class SCR_EntityWaypointClass: SCR_AIWaypointClass
{
};

class SCR_EntityWaypoint : SCR_AIWaypoint
{
	[Attribute("", UIWidgets.EditBox, "Related entity")]
	private string	m_sEntityName;
	private IEntity	m_Entity;
	
	//------------------------------------------------------------------------------------------------
	string GetEntityName()
	{
		return m_sEntityName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntityName(string entityName)
	{
		 m_sEntityName = entityName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntity(IEntity entity)
	{
		m_Entity = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetEntity()
	{
		if (m_Entity)
		{
			return m_Entity;
		}
		
		SetEntity(GetGame().GetWorld().FindEntityByName(m_sEntityName));
		return m_Entity;		
	}
};
