class SCR_BoardingWaypointClass: SCR_TimedWaypointClass
{
};

class SCR_AIBoardingWaypointParameters
{
	bool m_bIsDriverAllowed;
	bool m_bIsGunnerAllowed;
	bool m_bIsCargoAllowed;
};

class SCR_BoardingWaypoint : SCR_TimedWaypoint
{
	[Attribute("", UIWidgets.CheckBox, "Occupy driver")]
	private bool m_bDriverAllowed;
	
	[Attribute("", UIWidgets.CheckBox, "Occupy gunner")]
	private bool	m_bGunnerAllowed;
	
	[Attribute("", UIWidgets.CheckBox, "Occupy cargo")]
	private bool	m_bCargoAllowed;
	
	private ref SCR_AIBoardingWaypointParameters m_Parameters;
	//------------------------------------------------------------------------------------------------
	SCR_AIBoardingWaypointParameters GetAllowance()
	{
		return m_Parameters;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAllowance(bool driverAllowed, bool gunnerAllowed, bool cargoAllowed)
	{
		m_Parameters.m_bIsDriverAllowed = driverAllowed;
		m_Parameters.m_bIsGunnerAllowed = gunnerAllowed;
		m_Parameters.m_bIsCargoAllowed = cargoAllowed;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_Parameters = new SCR_AIBoardingWaypointParameters;
		m_Parameters.m_bIsDriverAllowed = m_bDriverAllowed;
		m_Parameters.m_bIsGunnerAllowed = m_bGunnerAllowed;
		m_Parameters.m_bIsCargoAllowed = m_bCargoAllowed;		
	}
};
