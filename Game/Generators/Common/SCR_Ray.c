class SCR_Ray
{
	vector m_vPosition;		//!< world coordinates
	vector m_vDirection;	//!< normalised vectorDir

	//------------------------------------------------------------------------------------------------
	static SCR_Ray Lerp(notnull SCR_Ray valueA, notnull SCR_Ray valueB, float t)
	{
		SCR_Ray result = new SCR_Ray();
		result.m_vPosition = vector.Lerp(valueA.m_vPosition, valueB.m_vPosition, t);
		result.m_vDirection = vector.Lerp(valueA.m_vDirection, valueB.m_vDirection, t).Normalized();
		return result;
	}
}
