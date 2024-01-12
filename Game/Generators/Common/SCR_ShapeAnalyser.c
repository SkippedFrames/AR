//! The Shape Analyser provides "points" (Rays/Transforms) with position and vectorDir (vectorUp is not filled)
//! if the shape is closed, the first point is added as the last point FOR THE POLYLINE ONLY (the engine already does it for the spline)
class SCR_ShapeAnalyser
{
	protected bool m_bIsSpline;
	protected bool m_bIsClosed;

	protected ref array<ref SCR_Ray> m_aAnchorRays;			//!< points as shown by the Vector tool
	protected ref array<ref SCR_Ray> m_aMidPointRays;		//!< tesselated points between two "normal" points, regardless of distance (by num of tesselated points)
	protected ref array<ref SCR_Ray> m_aTesselatedPointRays;	//!< intermediate points

	// stats
	protected float m_fLength2D;
	protected float m_fLength3D;
	protected float m_fSurface;
	protected float m_fMinAltitude, m_fMaxAltitude;
	protected float m_fMinSlope, m_fMaxSlope; //!< in radians

	//------------------------------------------------------------------------------------------------
	bool IsClosed()
	{
		return m_bIsClosed;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSpline()
	{
		return m_bIsSpline;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_Ray> GetPoints()
	{
		array<ref SCR_Ray> result = {};
		foreach (SCR_Ray point : m_aAnchorRays)
		{
			result.Insert(point);
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_Ray> GetMiddlePoints()
	{
		array<ref SCR_Ray> result = {};
		foreach (SCR_Ray point : m_aMidPointRays)
		{
			result.Insert(point);
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_Ray> GetTesselatedPoints()
	{
		array<ref SCR_Ray> result = {};
		foreach (SCR_Ray point : m_aTesselatedPointRays)
		{
			result.Insert(point);
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return length2D, length3D, surface (-1 if shape is not closed), minAltitude, maxAltitude, minSlope, maxSlope
	array<float> GetStats()
	{
		return { m_fLength2D, m_fLength3D, m_fSurface, m_fMinAltitude, m_fMaxAltitude, m_fMinSlope, m_fMaxSlope };
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param isClosed -has- to be taken from EntitySource :-|
	void SCR_ShapeAnalyser(notnull ShapeEntity shapeEntity, bool isClosed)
	{
		m_bIsSpline = SplineShapeEntity.Cast(shapeEntity) != null;

		vector origin = shapeEntity.GetOrigin();

		array<vector> points = {};
		shapeEntity.GetPointsPositions(points);

		int pointsCount = points.Count();
		if (pointsCount < 2)
			return;

		array<vector> tesselatedPoints = {};
		shapeEntity.GenerateTesselatedShape(tesselatedPoints);

		m_aAnchorRays = {};
		m_aTesselatedPointRays = {};

		vector prevPoint, diff;
		vector prevTessPoint, currTessPoint;
		vector currPoint = points[0];
		int pointIndex;

		array<int> pointTesselatedIndices = {};

		// stats
		array<float> polygon2D = {};
		m_fMinAltitude = currPoint[1];
		m_fMaxAltitude = currPoint[1];
		m_fMinSlope = float.INFINITY;
		m_fMaxSlope = -float.INFINITY;

		for (int i, count = tesselatedPoints.Count(); i < count; i++)
		{
			currTessPoint = tesselatedPoints[i];

			if (i == 0)
				diff = tesselatedPoints[1] - currTessPoint;
			else
				diff = currTessPoint - prevTessPoint;

			SCR_Ray point = new SCR_Ray();
			point.m_vPosition = origin + currTessPoint;

			float slopeRad = Math.Atan2(diff[1], vector.DistanceXZ(diff, vector.Zero));

			if (i == 0)				// first
				point.m_vDirection = (tesselatedPoints[i + 1] - currTessPoint).Normalized();
			else if (i < count - 1)	// mid-curve - averages previous and next vector
				point.m_vDirection = (0.5 * ((currTessPoint - prevTessPoint) + (tesselatedPoints[i + 1] - currTessPoint))).Normalized();
				// (float.AlmostEqual) equivalent to
				// point.m_vDirection = vector.Lerp(currTessPoint - prevTessPoint, tesselatedPoints[i + 1] - currTessPoint, 0.5).Normalized();
			else					// last
				point.m_vDirection = (currTessPoint - prevTessPoint).Normalized();

			m_aTesselatedPointRays.Insert(point);

			if (currTessPoint == currPoint)
			{
				pointTesselatedIndices.Insert(i);
				m_aAnchorRays.Insert(point);

				pointIndex++;
				if (pointIndex >= pointsCount)
					pointIndex = 0;

				prevPoint = currPoint;
				currPoint = points[pointIndex];
			}

			// stats
			if (prevTessPoint)
			{
				m_fLength2D += vector.DistanceXZ(prevTessPoint, currTessPoint);
				m_fLength3D += vector.Distance(prevTessPoint, currTessPoint);
			}

			if (point.m_vPosition[1] < m_fMinAltitude)
				m_fMinAltitude = point.m_vPosition[1];

			if (point.m_vPosition[1] > m_fMaxAltitude)
				m_fMaxAltitude = point.m_vPosition[1];

			if (slopeRad < m_fMinSlope)
				m_fMinSlope = slopeRad;

			if (slopeRad > m_fMaxSlope)
				m_fMaxSlope = slopeRad;

			polygon2D.Insert(point.m_vPosition[0]);
			polygon2D.Insert(point.m_vPosition[2]);

			// loop
			prevTessPoint = currTessPoint;
		}

		m_aMidPointRays = {};
		if (m_bIsSpline) // take the middle point
		{
			int prevIndex = pointTesselatedIndices[0];
			int nextIndex;
			for (int i = 1, count = pointTesselatedIndices.Count(); i < count; i++)
			{
				nextIndex = pointTesselatedIndices[i];
				m_aMidPointRays.Insert(m_aTesselatedPointRays[prevIndex + 0.5 * (nextIndex - prevIndex)]);
				prevIndex = nextIndex;
			}
		}
		else
		{
			SCR_Ray currRay = m_aAnchorRays[0];
			SCR_Ray nextRay;
			for (int i = 1, count = m_aAnchorRays.Count(); i < count; i++) // start from 1
			{
				nextRay = m_aAnchorRays[i];
				m_aMidPointRays.Insert(SCR_Ray.Lerp(currRay, nextRay, 0.5));
				currRay = nextRay;
			}

			if (m_bIsClosed)
				m_aMidPointRays.Insert(SCR_Ray.Lerp(currRay, m_aAnchorRays[0], 0.5));
		}

		m_fSurface = SCR_Math2D.GetPolygonArea(polygon2D); // closed or not
	}
}
