//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "Power Line Generator", dynamicBox: true, visible: false)]
class SCR_PowerlineGeneratorEntityClass : SCR_GeneratorBaseEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PowerlineGeneratorEntity : SCR_GeneratorBaseEntity
{
	[Attribute(defvalue: "1", desc: "How far should the poles be from each other.", category: "Pole Setup")]
	protected float m_fDistance;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_DefaultPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_StartPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_EndPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_DefaultJunctionPole;

	[Attribute(defvalue: "false", category: "Pole Setup")]
	protected bool m_bRotate180DegreeYawStartPole;

	[Attribute(defvalue: "false", category: "Pole Setup")]
	protected bool m_bRotate180DegreeYawEndPole;

	[Attribute(defvalue: "0", desc: "Expected empty space around the poles line by other generators", params: "0 100 1", category: "Pole Setup")]
	protected float Clearance; //!< Used by Obstacle Detector - TODO: rename to m_fClearance

	[Attribute(defvalue: "0", desc: "Maximum random pitch angle (from Y-axis)", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomPitchAngle;

	[Attribute(defvalue: "true", desc: "Make the random pitch possible on both sides (-180..+180°) or, if disabled, one side only (0..180°)", category: "Randomisation")]
	protected bool m_bRandomPitchOnBothSides;

	[Attribute(defvalue: "0", desc: "Maximum random roll angle (from Y-axis)", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomRollAngle;

	[Attribute(defvalue: "true", desc: "Make the random roll possible on both sides (-180..+180°) or, if disabled, one side only (0..180°)", category: "Randomisation")]
	protected bool m_bRandomRollOnBothSides;

	[Attribute(defvalue: "false", desc: "Apply randomisation to default poles", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollDefault;

	[Attribute(defvalue: "false", desc: "Apply randomisation to start pole", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollStart;

	[Attribute(defvalue: "false", desc: "Apply randomisation to end pole", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollEnd;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "emat", category: "Power Lines")]
	protected ResourceName m_PowerlineMaterial;

	[Attribute(defvalue: "0", category: "Debug")]
	protected bool m_bDrawDebugShapes;

	protected ref array<IEntity> m_aMyJunctionPoles = {};
	protected ref array<IEntity> m_aOtherJunctionPoles = {};

	protected ShapeEntity m_ParentShape;

	protected ref array<ref Shape> m_aDebugShapes = {};
	protected IEntity m_PreviousPowerPole;

	// QUERY DATA
	protected static ref array<IEntitySource> s_aGenerators = {};
	protected static IEntitySource s_CurrentQueryGenerator;

	protected bool m_bLastJunctionWasSameLine = true;

	protected IEntity m_StartPoleEntity;

#ifdef WORKBENCH
	protected static WorldEditorAPI s_Api;

	// #ifdef debug?
	// protected static ref array<ref Shape> s_aDebugShapes = {};

	protected IEntitySource m_ShapeSource; // Don't access directly! Always use GetShapeSource()! That guarantees you (hopefully) always get correct source
	protected IEntitySource m_PowerlineGeneratorSource;

	protected static const float TOLERANCE_SQUARED = 0.01; // 0.1 * 0.1
	protected static const string POINT_DATA_CLASS = "SCR_PowerlineGeneratorPointData"; // TODO: use .IsInherited(SCR_PowerlineGeneratorPointData)

	//------------------------------------------------------------------------------------------------
	protected static void GenerateJunctions(notnull IEntitySource generator, bool newQuery)
	{
		if (!s_Api || s_Api.UndoOrRedoIsRestoring())
			return;

		if (!s_Api.IsDoingEditAction())
		{
			s_Api.BeginEntityAction();
			GenerateJunctions(generator, newQuery);
			s_Api.EndEntityAction();
			return;
		}

		if (newQuery)
		{
			if (s_aGenerators)
				s_aGenerators.Clear();
			else
				s_aGenerators = {};
		}

		s_CurrentQueryGenerator = generator;
		s_aGenerators.Insert(generator);
		QueryGenerators(generator);

		SCR_PowerlineGeneratorEntity queriedGenerator;
		int generatorsCount = s_aGenerators.Count();
		for (int i = 0; i < generatorsCount; i++)
		{
			queriedGenerator = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(s_aGenerators[i]));
			if (!queriedGenerator)
				continue;

			queriedGenerator.DeletePoles();
			queriedGenerator.GenerateJunctions();
		}

		SCR_PowerlineGeneratorEntity otherQueriedGenerator;
		for (int i = 0; i < generatorsCount; i++)
		{
			queriedGenerator = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(s_aGenerators[i]));
			if (!queriedGenerator)
				continue;

			for (int j = generatorsCount - 1; j >= 0; j--)
			{
				if (i == j)
					continue;

				otherQueriedGenerator = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(s_aGenerators[j]));
				if (!otherQueriedGenerator)
					continue;

				queriedGenerator.FindCommonJunctionsPoints(otherQueriedGenerator.m_aMyJunctionPoles);
			}

			queriedGenerator.GeneratePoles();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsJunctionPoint(vector point)
	{
		for (int i = m_aMyJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (vector.DistanceSq(point, CoordToLocal(m_aMyJunctionPoles[i].GetOrigin())) < TOLERANCE_SQUARED)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasCommonPoint(SCR_PowerlineGeneratorEntity other)
	{
		IEntitySource sourceOther;
		sourceOther = s_Api.EntityToSource(other);
		if (!sourceOther)
			return false;

		IEntitySource shapeSourceOther;
		shapeSourceOther = sourceOther.GetParent();
		IEntitySource shapeSource = GetShapeSource();
		if (!shapeSource || ! shapeSourceOther)
			return false;

		array<vector> pointsThis, pointsOther;
		pointsThis = GetPoints(shapeSource);
		pointsOther = GetPoints(shapeSourceOther);
		if (!pointsThis || !pointsOther)
			return false;

		vector coordThis, coordOther;
		for (int i = pointsThis.Count() - 1; i >= 0; i--)
		{
			for (int j = pointsOther.Count() - 1; j >= 0; j--)
			{
				coordThis = CoordToParent(pointsThis[i]);
				coordOther = other.CoordToParent(pointsOther[j]);
				if (vector.DistanceSq(coordThis, coordOther) < TOLERANCE_SQUARED)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void FindCommonJunctionsPoints(array<IEntity> otherJunctionPoles)
	{
		array<vector> points = GetPoints(m_Source.GetParent());
		if (!points)
			return;

		for (int i = otherJunctionPoles.Count() - 1; i >= 0; i--)
		{
			for (int j = points.Count() - 1; j >= 0; j--)
			{
				if (vector.DistanceSq(CoordToParent(points[j]), otherJunctionPoles[i].GetOrigin()) < TOLERANCE_SQUARED)
				{
					// Isn't in the array yet
					if (!m_aOtherJunctionPoles.Contains(otherJunctionPoles[i]))
						m_aOtherJunctionPoles.Insert(otherJunctionPoles[i]);

					// We skip the rest of the points
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateJunctions()
	{
		m_aMyJunctionPoles.Clear();

		IEntitySource shapeSource = GetShapeSource();

		IEntity shapeEntity = s_Api.SourceToEntity(shapeSource);
		if (!shapeEntity)
			return;

		BaseContainerList points = shapeSource.GetObjectArray("Points");

		vector parentPositionXZ = shapeEntity.GetOrigin();
		float parentY = parentPositionXZ[1];
		BaseContainer point;
		vector lastPointPosition;
		vector currentPointPosition;
		vector nextPointPosition;
		float yaw;

		for (int i, count = points.Count(); i < count; i++)
		{
			point = points.Get(i);

			lastPointPosition = currentPointPosition;
			point.Get("Position", currentPointPosition);

			if (i > 0)
			{
				yaw = (currentPointPosition - lastPointPosition).Normalized().ToYaw();
			}
			else if (count > 1)
			{
				points.Get(1).Get("Position", nextPointPosition);
				yaw = (nextPointPosition - currentPointPosition).Normalized().ToYaw();
			}

			GenerateJunctionOnPoint(point, parentPositionXZ, parentY, yaw);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (key == "coords")
			return false;

		s_Api = _WB_GetEditorAPI();
		if (!s_Api)
			return false;

		IEntitySource thisSrc = s_Api.EntityToSource(this);
		BaseContainerTools.WriteToInstance(this, thisSrc);

		m_ParentShape = ShapeEntity.Cast(parent);
		if (m_ParentShape)
		{
			IEntitySource parentSrc = s_Api.EntityToSource(m_ParentShape);
			OnShapeInit(parentSrc, m_ParentShape);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateJunctionOnPoint(notnull BaseContainer point, vector parentPositionXZ, float parentY, float yaw)
	{
		SCR_PowerlineGeneratorJunctionData junctionData;

		BaseContainerList dataArr = point.GetObjectArray("Data");
		BaseContainer data;
		for (int j = 0, dataCount = dataArr.Count(); j < dataCount; ++j)
		{
			data = dataArr.Get(j);
			if (data.GetClassName() == POINT_DATA_CLASS)
			{
				data.Get("m_JunctionData", junctionData);
				break;
			}
		}

		if (!junctionData)
			return;

		ResourceName junctionResourceName = junctionData.m_sJunctionPrefab;
		if (junctionResourceName.IsEmpty())
			junctionResourceName = m_DefaultJunctionPole;

		if (junctionResourceName.IsEmpty())
			return;

		vector pointPosition;
		point.Get("Position", pointPosition);

		IEntity pole = s_Api.CreateEntity(junctionResourceName, "", s_Api.GetCurrentEntityLayerId(), s_Api.EntityToSource(this), pointPosition, vector.Zero);
		if (!pole)
			return;
		
		IEntitySource poleSrc = s_Api.EntityToSource(pole);

		yaw += junctionData.m_fYawOffset;
		if (yaw > 360)
			yaw -= 360;

		s_Api.ModifyEntityKey(pole, "angleY", yaw.ToString());

		SCR_PowerPole powerPole = GetPowerPoleFromEntity(pole);
		if (!powerPole)
			return;
		
		if (junctionData.m_bPowerSource)
		{
			IEntitySource powerPoleSrc = s_Api.EntityToSource(powerPole);
			if (powerPoleSrc)
			{
				s_Api.SetVariableValue(powerPoleSrc, null, "PowerSource", junctionData.m_bPowerSource.ToString(true));
				
				//Refresh powerpole pointer
				pole = s_Api.SourceToEntity(poleSrc);
				powerPole = GetPowerPoleFromEntity(pole);
			}
		}

		m_aMyJunctionPoles.Insert(powerPole);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeletePoles()
	{
		if (s_Api.IsDoingEditAction())
		{
			// Delete old poles
			IEntitySource entSrc = s_Api.EntityToSource(this);
			for (int i = entSrc.GetNumChildren() - 1; i >= 0; --i)
			{
				s_Api.DeleteEntity(s_Api.SourceToEntity(entSrc.GetChild(i)));
			}
		}
		else
		{
			s_Api.BeginEntityAction();
			DeletePoles();
			s_Api.EndEntityAction();
		}

		m_aMyJunctionPoles.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void GeneratePoles()
	{
		if (!m_ParentShape)
		{
			Print("Parent Shape is null, cannot generate power poles", LogLevel.ERROR);
			return;
		}

		if (!m_bDrawDebugShapes && m_aDebugShapes)
			m_aDebugShapes.Clear();

		s_Api = _WB_GetEditorAPI();
		if (!s_Api)
			return;

		if (!s_Api.IsDoingEditAction())
		{
			s_Api.BeginEntityAction();
			GeneratePoles();
			s_Api.EndEntityAction();
			return;
		}

		array<vector> anchorPoints = {};
		array<vector> shapePoints = {};
		m_ParentShape.GetPointsPositions(anchorPoints);
		m_ParentShape.GenerateTesselatedShape(shapePoints);
		BaseContainerList pointsSource = m_ParentShapeSource.GetObjectArray("Points");

		float pointDistance;
		float poleOffset;
		float currentDistance;
		vector currentPoint, prevPoint;
		bool spawnPerPoint, previousWasPerPoint;
		BaseContainerList dataArr;
		BaseContainer data;
		vector direction;
		float yaw;
		float nextPoleDistance;
		vector lastPolePosition, nextPolePosition;
		vector parentPositionXZ = m_ParentShape.GetOrigin();
		float parentY = parentPositionXZ[1];
		parentPositionXZ[1] = 0;

		for (int i, count = shapePoints.Count(); i < count; i++)
		{
			int anchorIndex = anchorPoints.Find(currentPoint); // TODO: make an index-tracked thing
			bool isAnchorPoint = anchorIndex > -1;

			poleOffset = 0;

			if (isAnchorPoint)
				AttachJunctionOnPoint(currentPoint);

			prevPoint = currentPoint;
			previousWasPerPoint = spawnPerPoint;
			currentPoint = shapePoints[i];

			//Get data of previous point
			if (i > 0 && isAnchorPoint)
			{
				dataArr = pointsSource.Get(anchorIndex).GetObjectArray("Data");
				for (int j = 0, dataCount = dataArr.Count(); j < dataCount; ++j)
				{
					data = dataArr.Get(j);
					if (data.GetClassName() == POINT_DATA_CLASS)
					{
						data.Get("m_fPoleOffset", poleOffset);
						if (m_fDistance + poleOffset < 1)
						{
							poleOffset = 1 - m_fDistance;
							Print("Pole offset is too small!", LogLevel.WARNING);
						}

						data.Get("m_bGeneratePerPoint", spawnPerPoint);
						break;
					}
				}
			}

			direction = (currentPoint - prevPoint).Normalized();
			yaw = direction.ToYaw();

			if (i == 1) //Generate start pole
				GenerateStartPole(prevPoint, parentY, parentPositionXZ, yaw);

			if (spawnPerPoint)
			{
				if (!isAnchorPoint)
					continue;

				if (anchorIndex == 1)
					continue;

				if (!IsJunctionPoint(prevPoint))
					GeneratePole(prevPoint, parentPositionXZ, parentY, yaw);

				if (i == count - 1) //Generate end pole
					GenerateEndPole(currentPoint, parentY, parentPositionXZ, yaw);
			}
			else
			{
				if (previousWasPerPoint)
				{
					GeneratePole(prevPoint, parentPositionXZ, parentY, yaw);
					currentDistance = 0;
				}

				if (i == 0)
					continue;

				pointDistance = vector.Distance(currentPoint, prevPoint);
				lastPolePosition = prevPoint;

				if (currentDistance + pointDistance > m_fDistance + poleOffset)
				{
					nextPoleDistance = (m_fDistance + poleOffset - currentDistance);
					pointDistance -= nextPoleDistance;

					nextPolePosition = prevPoint + direction * nextPoleDistance;

					lastPolePosition = nextPolePosition;
					GeneratePole(lastPolePosition, parentPositionXZ, parentY, yaw);
					currentDistance = 0;
				}
				else
				{
					currentDistance += pointDistance - poleOffset;
					if (i == count - 1) //Generate end pole
						GenerateEndPole(currentPoint, parentY, parentPositionXZ, yaw);

					continue;
				}

				while (pointDistance > m_fDistance + poleOffset)
				{
					if (m_fDistance == 0 && poleOffset == 0)
					{
						Print("m_fDistance in Powerline Generator and pole offset is 0. Not generating any poles.", LogLevel.WARNING);
						break;
					}

					nextPolePosition = lastPolePosition + direction * (poleOffset + m_fDistance);

					lastPolePosition = nextPolePosition;
					GeneratePole(lastPolePosition, parentPositionXZ, parentY, yaw);
					pointDistance -= m_fDistance + poleOffset;
				}

				if (i == count - 1) //Generate end pole
				{
					GenerateEndPole(currentPoint, parentY, parentPositionXZ, yaw);
					continue;
				}

				if (pointDistance > 0)
					currentDistance += pointDistance;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity FindJunctionOnPoint(vector point, out bool sameLine)
	{
		for (int i = m_aMyJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (m_aMyJunctionPoles[i] && vector.DistanceSq(point, CoordToLocal(m_aMyJunctionPoles[i].GetOrigin())) < TOLERANCE_SQUARED)
			{
				sameLine = true;
				return m_aMyJunctionPoles[i];
			}
		}

		for (int i = m_aOtherJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (m_aOtherJunctionPoles[i] && vector.DistanceSq(point, CoordToLocal(m_aOtherJunctionPoles[i].GetOrigin())) < TOLERANCE_SQUARED)
			{
				sameLine = false;
				return m_aOtherJunctionPoles[i];
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void AttachJunctionOnPoint(vector point)
	{
		for (int i = m_aMyJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (m_aMyJunctionPoles[i] && vector.DistanceSq(point, CoordToLocal(m_aMyJunctionPoles[i].GetOrigin())) < TOLERANCE_SQUARED)
				AttachJunction(m_aMyJunctionPoles[i], true);
		}

		for (int i = m_aOtherJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (m_aOtherJunctionPoles[i] && vector.DistanceSq(point, CoordToLocal(m_aOtherJunctionPoles[i].GetOrigin())) < TOLERANCE_SQUARED)
				AttachJunction(m_aOtherJunctionPoles[i], false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AttachJunction(IEntity junction, bool sameLine)
	{
		SCR_PowerPole junctionPole = GetPowerPoleFromEntity(junction);
		if (!junctionPole)
			return;

		m_bLastJunctionWasSameLine = sameLine;

		if (m_PreviousPowerPole)
			CreatePowerLine(junctionPole, GetPowerPoleFromEntity(m_PreviousPowerPole), sameLine);

		m_PreviousPowerPole = junctionPole;
	}

	//------------------------------------------------------------------------------------------------
	protected vector FindIntersection(
		float p0_x, float p0_y,
		float p1_x, float p1_y,
		float p2_x, float p2_y,
		float p3_x, float p3_y)
	{
		float s1_x, s1_y, s2_x, s2_y;
		s1_x = p1_x - p0_x;
		s1_y = p1_y - p0_y;
		s2_x = p3_x - p2_x;
		s2_y = p3_y - p2_y;

		float s, t;
		s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
		t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

		if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
			return Vector(p0_x + (t * s1_x), 0, p0_y + (t * s1_y)); // collision detected

		return vector.Zero; // no collision
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateEndPole(vector currentPoint, float parentY, vector parentPositionXZ, float yaw)
	{
		if (m_EndPole.IsEmpty())
			return;

		vector endPolePos = currentPoint;
		bool sameLine;
		IEntity pole = FindJunctionOnPoint(currentPoint, sameLine);

		if (pole)
		{
			m_bLastJunctionWasSameLine = sameLine;
		}
		else
		{
			pole = GeneratePoleAt(endPolePos, parentPositionXZ, parentY, m_EndPole);

			if (SCR_JunctionPowerPole.Cast(GetPowerPoleFromEntity(pole)))
				Print("End pole is of type SCR_JunctionPowerPole, make sure to do this using junction data on the last point instead.", LogLevel.WARNING);

			if (m_bRotate180DegreeYawEndPole)
			{
				yaw += 180;
				if (yaw > 360)
					yaw -= 360;
			}

			s_Api.ModifyEntityKey(pole, "angleY", yaw.ToString());
			if (m_bApplyPitchAndRollEnd)
				ApplyRandomPitchAndRoll(pole);
		}

		SCR_PowerPole endPole = GetPowerPoleFromEntity(pole);
		if (endPole)
			CreatePowerLine(GetPowerPoleFromEntity(m_PreviousPowerPole), endPole, sameLine, m_bRotate180DegreeYawEndPole);

		if (m_bDrawDebugShapes)
		{
			Shape shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, endPolePos + parentPositionXZ, 1);
			m_aDebugShapes.Insert(shape);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateStartPole(vector lastPoint, float parentY, vector parentPositionXZ, float yaw)
	{
		if (!s_Api)
		{
			Print("WorldEditorAPI not found in SCR_PowerlineGeneratorEntity.", LogLevel.WARNING);
			return;
		}

		if (m_StartPole.IsEmpty())
			return;

		vector startPolePos = lastPoint;
		bool sameLine;
		IEntity pole = FindJunctionOnPoint(lastPoint, sameLine);

		if (pole)
		{
			m_bLastJunctionWasSameLine = sameLine;
		}
		else
		{
			pole = GeneratePoleAt(startPolePos, parentPositionXZ, parentY, m_StartPole);
			if (!pole)
			{
				Print("Pole entity not created in SCR_PowerlineGeneratorEntity.", LogLevel.WARNING);
				return;
			}

			if (SCR_JunctionPowerPole.Cast(pole))
			{
				Print("Start pole is of type SCR_JunctionPowerPole, make sure to do this using junction data on the first point instead.", LogLevel.WARNING);
				m_bLastJunctionWasSameLine = true;
			}

			if (m_bRotate180DegreeYawStartPole)
			{
				yaw += 180;
				if (yaw > 360)
					yaw -= 360;
			}

			s_Api.ModifyEntityKey(pole, "angleY", yaw.ToString());
			if (m_bApplyPitchAndRollStart)
				ApplyRandomPitchAndRoll(pole);
		}

		m_PreviousPowerPole = pole;
		m_StartPoleEntity = pole;

		if (m_bDrawDebugShapes)
		{
			Shape shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, startPolePos + parentPositionXZ, 1);
			m_aDebugShapes.Insert(shape);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GeneratePoleAt(vector localPos, vector parentPositionXZ, float parentY, ResourceName poleResourceName)
	{
		float y;
		if (s_Api.TryGetTerrainSurfaceY(localPos[0] + parentPositionXZ[0], localPos[2] + parentPositionXZ[2], y))
			localPos[1] = y - parentY;

		return s_Api.CreateEntity(poleResourceName, "", s_Api.GetCurrentEntityLayerId(), s_Api.EntityToSource(this), localPos, vector.Zero);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyRandomPitchAndRoll(IEntity powerPole)
	{
		float pitch = 0, roll = 0;
		if (m_bRandomPitchOnBothSides)
			pitch = -m_fRandomPitchAngle + Math.RandomFloat(0, 2 * m_fRandomPitchAngle);
		else
			pitch = Math.RandomFloat(0, m_fRandomPitchAngle);

		if (m_bRandomRollOnBothSides)
			roll = -m_fRandomRollAngle + Math.RandomFloat(0, 2 * m_fRandomRollAngle);
		else
			roll = Math.RandomFloat(0, m_fRandomRollAngle);

		s_Api.ModifyEntityKey(powerPole, "angleX", pitch.ToString());
		s_Api.ModifyEntityKey(powerPole, "angleZ", roll.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GeneratePole(vector lastPolePosition, vector parentPositionXZ, float parentY, float yaw, ResourceName customPoleResourceName = string.Empty)
	{
		float y;
		IEntity pole;
		if (s_Api.TryGetTerrainSurfaceY(lastPolePosition[0] + parentPositionXZ[0], lastPolePosition[2] + parentPositionXZ[2], y))
			lastPolePosition[1] = y - parentY;

		ResourceName poleResourceName;
		if (customPoleResourceName.GetPath().IsEmpty())
			poleResourceName = m_DefaultPole; // already checked for empty string
		else
			poleResourceName = customPoleResourceName;

		pole = s_Api.CreateEntity(poleResourceName, "", s_Api.GetCurrentEntityLayerId(), s_Api.EntityToSource(this), lastPolePosition, vector.Zero);
		if (!pole)
			return null;

		s_Api.ModifyEntityKey(pole, "angleY", yaw.ToString());
		if (m_bApplyPitchAndRollDefault)
			ApplyRandomPitchAndRoll(pole);

		if (m_PreviousPowerPole && pole)
		{
			SCR_PowerPole thisPowerPole = GetPowerPoleFromEntity(pole);
			if (thisPowerPole)
			{
				bool inverse = m_StartPoleEntity == m_PreviousPowerPole && m_bRotate180DegreeYawStartPole;
				CreatePowerLine(thisPowerPole, GetPowerPoleFromEntity(m_PreviousPowerPole), inverse: inverse);
			}
		}

		m_PreviousPowerPole = pole;

		if (m_bDrawDebugShapes)
		{
			Shape shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, lastPolePosition + parentPositionXZ, 1);
			m_aDebugShapes.Insert(shape);
		}

		return pole;
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		// TODO handle this case better if needed, use the bbox arrays
		m_ParentShape = shapeEntity;
		m_Source = _WB_GetEditorAPI().EntityToSource(this);
		OnShapeInit(shapeEntitySrc, shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity CreatePowerLine(SCR_PowerPole startPole, SCR_PowerPole endPole, bool sameLine = true, bool inverse = false)
	{
		if (!startPole || !endPole || startPole == endPole)
			return null;

		if (SCR_JunctionPowerPole.Cast(startPole) || SCR_JunctionPowerPole.Cast(endPole))
			sameLine = m_bLastJunctionWasSameLine;

		int numCables = Math.Min(startPole.GetSlotsCount(sameLine), endPole.GetSlotsCount(sameLine));
		if (numCables <= 0)
		{
			Print("0 slots found on one of the power poles, please check the setup of your power poles.", LogLevel.WARNING);
			return null;
		}

		// Gather slot positions for both poles in world coordinates
		array<vector> startPoints = {};
		array<vector> endPoints = {};
		vector startSlotPos;
		SCR_JunctionPowerPole junctionPowerPole;
		for (int i = 0; i < numCables; ++i)
		{
			startSlotPos = startPole.CoordToParent(startPole.GetSlot(i, sameLine));
			startPoints.Insert(startSlotPos);
			endPoints.Insert(endPole.TryGetSlot(i, startSlotPos, sameLine));
		}

		// Create the PowerlineEntity
		IEntity powerline = s_Api.CreateEntity("PowerlineEntity", "", s_Api.GetCurrentEntityLayerId(), m_PowerlineGeneratorSource, CoordToLocal(startPoints[0]), vector.Zero);
		IEntitySource powerlineSrc = s_Api.EntityToSource(powerline);

		s_Api.SetVariableValue(powerlineSrc, null, "Material", m_PowerlineMaterial);

		array<ref ContainerIdPathEntry> containerPath;
		vector startPos, endPos;
		// Create cables for the entity
		for (int i = 0; i < numCables; ++i)
		{
			s_Api.CreateObjectArrayVariableMember(powerlineSrc, null, "Cables", "Cable", i);

			//Refresh powerline pointer
			powerline = s_Api.SourceToEntity(powerlineSrc);

			containerPath = {};
			containerPath.Insert(new ContainerIdPathEntry("Cables", i));

			// Add cable between each slot, convert to local coordinate space
			startPos = powerline.CoordToLocal(startPoints[i]);
			s_Api.SetVariableValue(powerlineSrc, containerPath, "StartPoint", startPos.ToString(false));

			//Refresh powerline pointer
			powerline = s_Api.SourceToEntity(powerlineSrc);

			endPos = powerline.CoordToLocal(endPoints[i]);
			if (inverse)
				endPos = powerline.CoordToLocal(endPoints[numCables - i - 1]);

			s_Api.SetVariableValue(powerlineSrc, containerPath, "EndPoint", endPos.ToString(false));
		}

		return powerline;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_PowerPole GetPowerPoleFromEntity(notnull IEntity powerPoleEntity)
	{
		SCR_PowerPole result = SCR_PowerPole.Cast(powerPoleEntity);
		if (result)
			return result;

		IEntitySource entitySource = s_Api.EntityToSource(powerPoleEntity);
		if (!entitySource)
			return null;

		for (int i = 0, childrenCount = entitySource.GetNumChildren(); i < childrenCount; i++)
		{
			result = SCR_PowerPole.Cast(s_Api.SourceToEntity(entitySource.GetChild(i)));
			if (result)
				return result;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected static void QueryGenerators(notnull IEntitySource generator, array<IEntitySource> checkedGenerators = null)
	{
		if (!s_Api || !s_aGenerators)
			return;

		IEntitySource shapeSource = generator.GetParent();
		if (!shapeSource)
			return;

		IEntity shapeEntity = s_Api.SourceToEntity(shapeSource);
		if (!shapeEntity)
			return;

		array<IEntitySource> checkedGeneratorsArray = checkedGenerators;
		if (!checkedGeneratorsArray)
			checkedGeneratorsArray = {};

		checkedGenerators = checkedGeneratorsArray;

		if (checkedGenerators.Contains(generator))
			return;

		checkedGenerators.Insert(generator);

		BaseContainerList points = shapeSource.GetObjectArray("Points");

		// Get bbox
		array<vector> vectorPoints = GetPoints(shapeSource);
		SCR_AABB bbox = new SCR_AABB(vectorPoints);

		// Query entities in bbox
		vector mat[4];
		shapeEntity.GetTransform(mat);

		IEntity generatorEntity = s_Api.SourceToEntity(generator);
		BaseWorld world = generatorEntity.GetWorld();

		bbox.m_vMin[1] = -50;
		bbox.m_vMax[1] = 50;

		// #ifdef debug?
		//s_aDebugShapes.Insert(Shape.Create(ShapeType.BBOX, ARGB(255, Math.RandomFloat(0, 255), Math.RandomFloat(0, 255), Math.RandomFloat(0, 255)), ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, generatorEntity.CoordToParent(bbox.m_vMin), generatorEntity.CoordToParent(bbox.m_vMax)));

		world.QueryEntitiesByAABB(generatorEntity.CoordToParent(bbox.m_vMin), generatorEntity.CoordToParent(bbox.m_vMax), QueryFilter);

		for (int i = s_aGenerators.Count() - 1; i >= 0; i--)
		{
			if (checkedGenerators.Contains(s_aGenerators[i]))
				continue;

			s_CurrentQueryGenerator = s_aGenerators[i];
			QueryGenerators(s_aGenerators[i], checkedGeneratorsArray);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Filter used by QueryGenerators' QueryEntitiesByAABB call
	protected static bool QueryFilter(IEntity entity)
	{
		ShapeEntity shape = ShapeEntity.Cast(entity);
		if (!shape)
			return true;

		IEntitySource shapeSource = s_Api.EntityToSource(shape);
		if (!shapeSource)
			return true;

		IEntitySource otherGeneratorSource;
		SCR_PowerlineGeneratorEntity otherGeneratorEntity;
		SCR_PowerlineGeneratorEntity currentGeneratorEntity;
		for (int i = shapeSource.GetNumChildren() - 1; i >= 0; --i)
		{
			otherGeneratorSource = shapeSource.GetChild(i);
			otherGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(otherGeneratorSource));
			if (!otherGeneratorEntity)
				continue;

			//Has to be refreshed here
			currentGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(s_CurrentQueryGenerator));
			if (!currentGeneratorEntity)
				continue;

			if (otherGeneratorSource && otherGeneratorSource != s_CurrentQueryGenerator && !s_aGenerators.Contains(otherGeneratorSource) && currentGeneratorEntity.HasCommonPoint(otherGeneratorEntity))
			{
				s_aGenerators.Insert(otherGeneratorSource);
				return true;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddPointData(BaseContainerList points)
	{
		if (!s_Api || s_Api.UndoOrRedoIsRestoring())
			return;

		BaseContainerList dataArr;
		BaseContainer data;
		for (int i = points.Count() - 1; i >= 0; i--)
		{
			dataArr = points[i].GetObjectArray("Data");
			int dataCount = dataArr.Count();
			bool hasPointData = false;
			for (int j = 0; j < dataCount; ++j)
			{
				data = dataArr.Get(j);
				if (data.GetClassName() == POINT_DATA_CLASS)
				{
					hasPointData = true;
					break;
				}
			}

			if (!hasPointData)
				s_Api.CreateObjectArrayVariableMember(points[i], null, "Data", POINT_DATA_CLASS, dataArr.Count());
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		m_ShapeSource = shapeEntitySrc;

		s_Api = _WB_GetEditorAPI();

		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (!points)
			return;

		AddPointData(points);

		GenerateJunctions(m_PowerlineGeneratorSource, true);
	}

	//------------------------------------------------------------------------------------------------
	//! To be used to obtain the shape's IEntitySource - do NOT refer to m_ShapeSource directly!
	//! \return shape's IEntitySource
	IEntitySource GetShapeSource()
	{
		if (!m_ShapeSource)
		{
			ShapeEntity shape = ShapeEntity.Cast(GetParent());
			if (shape)
			{
				m_ParentShape = shape;
				m_ShapeSource = _WB_GetEditorAPI().EntityToSource(shape);
			}
			else
			{
				IEntitySource source = s_Api.EntityToSource(this);
				m_ShapeSource = source.GetParent();
			}
		}

		return m_ShapeSource;
	}
#endif

	//------------------------------------------------------------------------------------------------
	void SCR_PowerlineGeneratorEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		if (GetGame().InPlayMode())
			return;

		m_PowerlineGeneratorSource = src;

		ShapeEntity shape = ShapeEntity.Cast(parent);
		if (shape)
		{
			m_ParentShape = shape;
			m_ShapeSource = _WB_GetEditorAPI().EntityToSource(shape);
		}

		SetEventMask(EntityEvent.INIT);
#endif
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_PowerlineGeneratorJunctionData
{
	[Attribute(desc: "Junction prefab to be used", params: "et")]
	ResourceName m_sJunctionPrefab;

	// not a slider for performance reason (type the value directly)
	[Attribute(desc: "Set the junction's yaw offset; can be used to setup the prefab properly", /* uiwidget: UIWidgets.Slider, */params: "0 360")]
	float m_fYawOffset;
	
	[Attribute("false")]
	bool m_bPowerSource;
}

//------------------------------------------------------------------------------------------------
class SCR_PowerlineGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute()]
	float m_fPoleOffset;

	[Attribute("0", "Starts generating poles on points, until a point with this attribute unchecked is reached.")]
	bool m_bGeneratePerPoint;

	[Attribute()]
	ref SCR_PowerlineGeneratorJunctionData m_JunctionData;
}
