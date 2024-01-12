class SCR_DebugShapeManager
{
	protected ref array<ref Shape> m_aShapes = {};

	protected static const int DEFAULT_COLOUR = Color.RED;
	protected static const ShapeFlags DEFAULT_FLAGS = ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP;

	//------------------------------------------------------------------------------------------------
	Shape AddBBox(vector min, vector max, int colour = DEFAULT_COLOUR, ShapeFlags additionalFlags = 0)
	{
		Shape shape = Shape.Create(ShapeType.BBOX, colour, DEFAULT_FLAGS | additionalFlags, min, max);
		m_aShapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	Shape AddLine(vector from, vector to, int colour = DEFAULT_COLOUR)
	{
		vector points[2] = { from, to };
		Shape shape = Shape.CreateLines(colour, DEFAULT_FLAGS, points, 2);
		m_aShapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! max 50 points
	Shape AddPolyLine(notnull array<vector> points, int colour = DEFAULT_COLOUR)
	{
		int count = points.Count();
		if (count < 2)
			return null;

		if (count > 50)
			count = 50;

		vector pointsS[50];
		for (int i; i < count; i++)
		{
			pointsS[i] = points[i];
		}

		Shape shape = Shape.CreateLines(colour, DEFAULT_FLAGS, pointsS, count);
		m_aShapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	Shape AddArrow(vector from, vector to, int colour = DEFAULT_COLOUR)
	{
		Shape shape = Shape.CreateArrow(from, to, vector.Distance(from, to) * 0.333, colour, DEFAULT_FLAGS);
		m_aShapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	Shape AddCircleXZ(vector centre, float radius, int colour = DEFAULT_COLOUR)
	{
		Shape shape = CreateCircle(centre, vector.Up, radius, colour, radius * Math.PI, DEFAULT_FLAGS);
		m_aShapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! \param angleStartRad in counter-clockwise radians
	//! \param coveredAngleRad in counter-clockwise radians - can be negative
	Shape AddCircleArcXZ(vector centre, float angleStartRad, float coveredAngleRad, float radius, int colour = DEFAULT_COLOUR)
	{
		if (coveredAngleRad < 0)
		{
			angleStartRad += coveredAngleRad;
			coveredAngleRad = -coveredAngleRad;
		}

		if (angleStartRad < 0 || angleStartRad > Math.PI2)
			angleStartRad = Math.Repeat(angleStartRad, Math.PI2);

		if (/* coveredAngleRad < 0 || */ coveredAngleRad > Math.PI2)
			coveredAngleRad = Math.Repeat(coveredAngleRad, Math.PI2);

		vector forward = { Math.Cos(angleStartRad), 0, Math.Sin(angleStartRad) };

		Shape shape = CreateCircleArc(centre, vector.Up, forward, 0, coveredAngleRad * Math.RAD2DEG, radius, colour, radius * (Math.PI2 - coveredAngleRad), DEFAULT_FLAGS);
		m_aShapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! \param angleStartRad in counter-clockwise radians
	//! \param coveredAngleRad in counter-clockwise radians
	Shape AddCircleSliceXZ(vector centre, float angleStartRad, float coveredAngleRad, float radius, int colour = DEFAULT_COLOUR)
	{
		if (coveredAngleRad < 0)
		{
			angleStartRad -= coveredAngleRad;
			coveredAngleRad *= -1;
		}

		if (angleStartRad < 0 || angleStartRad > Math.PI2)
			angleStartRad = Math.Repeat(angleStartRad, Math.PI2);

		if (coveredAngleRad < 0 || coveredAngleRad > Math.PI2)
			coveredAngleRad = Math.Repeat(coveredAngleRad, Math.PI2);

		vector forward = { Math.Cos(angleStartRad), 0, Math.Sin(angleStartRad) };

		float angleDeg = coveredAngleRad * Math.RAD2DEG;
		Shape shape = CreateCircleSlice(centre, vector.Up, forward, 0, angleDeg, radius, colour, radius * (Math.PI2 - coveredAngleRad), DEFAULT_FLAGS);
		m_aShapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	Shape AddRectangle(vector origin, vector vectorDir, float length, float width, int colour = DEFAULT_COLOUR)
	{
		vector endPos = origin + vectorDir.Normalized() * length;

		width *= 0.5;

		float directionRad = Math.Atan2(vectorDir[2], vectorDir[0]);
		vector leftWidth = { Math.Cos(directionRad + Math.PI_HALF) * width, 0, Math.Sin(directionRad + Math.PI_HALF) * width };
		vector rightWidth = { Math.Cos(directionRad - Math.PI_HALF) * width, 0, Math.Sin(directionRad - Math.PI_HALF) * width };
		vector points[5] = {
			endPos + leftWidth,		// top-right
			endPos + rightWidth,	// bottom-right
			origin + rightWidth,	// bottom-left
			origin + leftWidth,		// top-left
			endPos + leftWidth,		// top-right
		};

		Shape shape = Shape.CreateLines(colour, DEFAULT_FLAGS, points, 5);
		m_aShapes.Insert(shape);
		return shape;
	}

	Shape AddAABBRectangleXZ(vector min, vector max, int colour = DEFAULT_COLOUR)
	{
		vector points[5];
		points[0] = Vector(min[0], min[1], max[2]);	// top-left
		points[1] = Vector(max[0], max[1], max[2]);	// top-right
		points[2] = Vector(max[0], max[1], min[2]);	// bottom-right
		points[3] = Vector(min[0], min[1], min[2]);	// bottom-left
		points[4] = Vector(min[0], min[1], max[2]);	// top-left
		Shape shape = Shape.CreateLines(colour, DEFAULT_FLAGS, points, 5);
		m_aShapes.Insert(shape);
		return shape;
	}

	//! \param directionRad is in counter-clockwise radians
	Shape AddRectangleXZ(vector origin, float directionRad, float length, float width, int colour = DEFAULT_COLOUR)
	{
		vector endPos = origin + { Math.Cos(directionRad) * length, 0, Math.Sin(directionRad) * length };

		width *= 0.5;
		vector leftWidth = { Math.Cos(directionRad + Math.PI_HALF) * width, 0, Math.Sin(directionRad + Math.PI_HALF) * width };
		vector rightWidth = { Math.Cos(directionRad - Math.PI_HALF) * width, 0, Math.Sin(directionRad - Math.PI_HALF) * width };
		vector points[5] = {
			endPos + leftWidth,		// top-right
			endPos + rightWidth,	// bottom-right
			origin + rightWidth,	// bottom-left
			origin + leftWidth,		// top-left
			endPos + leftWidth,		// top-right
		};

		Shape shape = Shape.CreateLines(colour, DEFAULT_FLAGS, points, 5);
		m_aShapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	array<ref Shape> AddParallelLines(vector origin, vector vectorDir, float length, float width, int colour = DEFAULT_COLOUR)
	{
		vector endPos = origin + vectorDir.Normalized() * length;

		float directionRad = Math.Atan2(vectorDir[2], vectorDir[0]);
		vector leftWidth = { Math.Cos(directionRad + Math.PI_HALF) * width, 0, Math.Sin(directionRad + Math.PI_HALF) * width };
		vector rightWidth = { Math.Cos(directionRad - Math.PI_HALF) * width, 0, Math.Sin(directionRad - Math.PI_HALF) * width };

		return {
			AddLine(origin + leftWidth, endPos + leftWidth, colour),
			AddLine(origin + rightWidth, endPos + rightWidth, colour),
		};
	}

	//------------------------------------------------------------------------------------------------
	Shape AddSphere(vector centre, float radius, int colour = DEFAULT_COLOUR, ShapeFlags additionalFlags = 0)
	{
		Shape shape = Shape.CreateSphere(colour, DEFAULT_FLAGS | additionalFlags, centre, radius);
		m_aShapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	void Remove(notnull Shape shape)
	{
		m_aShapes.RemoveItem(shape);
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_aShapes.Clear();
	}
}
