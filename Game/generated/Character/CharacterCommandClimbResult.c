/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

/*!
result from static test
*/
class CharacterCommandClimbResult
{
	bool		m_bIsClimb;
	bool		m_bIsClimbOver;
	bool		m_bFinishWithFall;

	float		m_fClimbHeight;

	vector 		m_ClimbGrabPoint;		//! grab point for climb && climb over (in local space of it's parent)
	vector 		m_ClimbGrabPointNormal;	//! normal to grabpoint position (used for character orientation)
	vector 		m_ClimbStandPoint;		//! where climb ends (in local space of it's parent)
	vector 		m_ClimbOverStandPoint;	//! where climb over ends (in local space of it's parent)

	IEntity		m_GrabPointParent;		//! parent of grabpoint
	IEntity		m_ClimbStandPointParent;
	IEntity		m_ClimbOverStandPointParent;

}

/*!
\}
*/
