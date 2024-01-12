//! Axis-Aligned Bounding Box
//! Used by generators
class SCR_AABB : Managed
{
	vector m_vMin;
	vector m_vMax;
	vector m_vDimensions; // width, height, depth

	//------------------------------------------------------------------------------------------------
	bool DetectCollision2D(SCR_AABB other)
	{
		return
			m_vMin[0] < other.m_vMin[0] + other.m_vDimensions[0] &&
			m_vMin[0] + m_vDimensions[0] > other.m_vMin[0] &&
			m_vMin[2] < other.m_vMin[2] + other.m_vDimensions[2] &&
			m_vMin[2] + m_vDimensions[2] > other.m_vMin[2];
	}

	//------------------------------------------------------------------------------------------------
	void SCR_AABB(notnull array<vector> points)
	{
		if (points.IsEmpty())
			return;

		m_vMin = { float.MAX, float.MAX, float.MAX };
		m_vMax = { -float.MAX, -float.MAX, -float.MAX };

		foreach (vector point : points)
		{
			for (int i = 0; i < 3; i++)
			{
				if (point[i] < m_vMin[i])
					m_vMin[i] = point[i];

				if (point[i] > m_vMax[i])
					m_vMax[i] = point[i];
			}
		}

		m_vDimensions = m_vMax - m_vMin;
	}
}
