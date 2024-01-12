#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: "Object Brush",
	description: "Generate randomized compositions using a brush.\n" +
		"- click and drag to draw\n" +
		"- space to delete brush-created entities\n" +
		"- alt+click then click to create a (wide) line of entities (ESC to cancel)",
	// shortcut: "B", // unused
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF1FC)]
class SCR_ObjectBrushTool : WorldEditorTool
{
	protected static const float HECTARE_CONVERSION_FACTOR = 0.0001; // x/10000 (hectare is 100×100m)
	protected static const float MAX_SCALE_THRESHOLD = 1000;

	protected static const float RADIUS_STEP = 1;
	protected static const float RADIUS_MAX = 100;
	protected static const float RADIUS_MIN = 0.1;

	protected static const float STRENGTH_STEP = 1;
	protected static const float STRENGTH_MAX = 500;
	protected static const float STRENGTH_MIN = 0;

	protected static const float STRENGTH_RELATIVE_RADIUS_DISTANCE_TO_CREATE = 1 / 3;

	/*
		Category: OBJECT BRUSH
	*/

	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius of the brush", params: string.Format("%1 %2 %3", RADIUS_MIN, RADIUS_MAX, RADIUS_STEP), category: "Object Brush")]
	protected float m_fRadius;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Strength of the brush (objects per hectare (100×100m))", params: string.Format("%1 %2 %3", STRENGTH_MIN, STRENGTH_MAX, STRENGTH_STEP), category: "Object Brush")]
	protected float m_fStrength;

	[Attribute(uiwidget: UIWidgets.GraphDialog, desc: "Used to determine the scale fall off", category: "Object Brush")]
	protected ref Curve m_ScaleFallOffCurve;

	[Attribute(defvalue: "0", desc: "Apply density fall off to the brush", category: "Object Brush")]
	protected bool m_bDensityFallOffEnabled;

	[Attribute(uiwidget: UIWidgets.GraphDialog, desc: "Used to determine the density fall off", category: "Object Brush")]
	protected ref Curve m_DensityFallOffCurve;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, desc: "Sets how many sub areas will be made; higher amount of subareas will result in a more fluid density fall off", params: "1 100 1", category: "Object Brush")]
	protected int m_iDensityFallOffSubareaCount;

	[Attribute(defvalue: "0", desc: "Overwrite older brush strokes", category: "Object Brush")]
	protected bool m_bOverrideBrush;

	[Attribute(desc: "Define objects to paint - can take a SCR_ObjectBrushArrayConfig .conf", category: "Object Brush")]
	protected ref SCR_ObjectBrushArrayConfig m_ObjectsConfig;

	/*
		Category: Obstacles
	*/

	[Attribute(defvalue: "0", desc: "Objects generated by the brush will avoid static objects", category: "Obstacles")]
	protected bool m_bAvoidObjects;

	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, desc: "Object avoidance detection cylinder radius", params: "0 100 0.1", category: "Obstacles")]
	protected float m_fAvoidObjectsDetectionRadius;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, desc: "Object avoidance detection cylinder height", params: "0 1000 100", category: "Obstacles")]
	protected float m_fAvoidObjectsDetectionHeight;

	[Attribute(defvalue: "0", desc: "Objects generated by the brush will avoid roads", category: "Obstacles")]
	protected bool m_bAvoidRoads;

	[Attribute(defvalue: "0", desc: "Objects generated by the brush will avoid rivers", category: "Obstacles")]
	protected bool m_bAvoidRivers;

	[Attribute(defvalue: "0", desc: "Objects generated by the brush will avoid power lines", category: "Obstacles")]
	protected bool m_bAvoidPowerLines;

	[Attribute(defvalue: "0", desc: "Objects generated by the brush will avoid land", category: "Obstacles")]
	protected bool m_bAvoidLand;

	[Attribute(defvalue: "0", desc: "Objects generated by the brush will avoid ocean", category: "Obstacles")]
	protected bool m_bAvoidOcean;

	/*
		Category: Obstacles - Area
	*/

	[Attribute(defvalue: "0", desc: "Objects generated by the brush will avoid forests. Depends on Area Detection Radius", category: "Obstacles - Area")]
	protected bool m_bAvoidForests;

	[Attribute(defvalue: "0", desc: "Objects generated by the brush will avoid lakes. Depends on Area Detection Radius", category: "Obstacles - Area")]
	protected bool m_bAvoidLakes;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, "Radius to detect areas (forests and lakes) around the brush's starting point for avoidance - performance setting. Zero is world-wide detection", "0 1000 100", category: "Obstacles - Area")]
	protected int m_iAreaDetectionRadius;

#ifdef DEBUG
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif

	protected ref SCR_ObstacleDetector m_ObstacleDetector;

	protected ref map<ref SCR_ObjectBrushObjectBase, ref EntityID> m_mCreatedObjects = new map<ref SCR_ObjectBrushObjectBase, ref EntityID>();
	protected ref map<ref SCR_ObjectBrushObjectBase, ref EntityID> m_mActiveBrushObjects = new map<ref SCR_ObjectBrushObjectBase, ref EntityID>();

	protected bool m_bIsMouseHeldDown;

	protected ref RandomGenerator m_RandomGenerator;
	// the parameter determines the granularity of the grid. This results in the granularity being world size / 10.
	protected ref ForestGeneratorGrid m_Grid = new ForestGeneratorGrid(10);

	protected ref Shape m_BrushShape;
	protected ref array<ref Shape> m_aLineShapes = {};

	protected bool m_bDeleteMode;	//<! is Space pressed
	protected bool m_bLineMode;	//<! is Alt   pressed
	protected bool m_bAreasDetectedByWorld;	//<! were the area detected by world's BBox
	protected bool m_bManageEditAction;		//<! is doing an action

	protected vector m_vFirstLinePoint;
	protected vector m_vSecondLinePoint;

	protected vector m_vLastMousePosition;
	protected vector m_vLastObjectCreationCentrePosition;

	protected int m_iBrushShapeColor = ARGB(255, 0, 255, 0);

	//------------------------------------------------------------------------------------------------
	//! Create objects around the provided position, otherwise x/y screen to terrain pos
	//! World Editor must be doing an Edit action
	//! \param mouseX
	//! \param mouseY
	//! \param position
	protected void CreateObjects(float mouseX, float mouseY, vector position)
	{
		if (!m_API)
		{
			Print("m_API is null", LogLevel.ERROR);
			return;
		}

		// safety that should never trigger
		if (!m_API.IsDoingEditAction())
		{
			Print("Workbench isn't performing edit action!", LogLevel.WARNING);
			return;
		}

		if (!m_ObjectsConfig || !m_ObjectsConfig.m_aObjectArray || m_ObjectsConfig.m_aObjectArray.IsEmpty())
			return; // silent stop - config warning managed in OnMousePressEvent

		float oneDividedByFallOffCount = 1 / m_iDensityFallOffSubareaCount;
		float radiusDividedByFallOffCount = m_fRadius * oneDividedByFallOffCount;

		array<ref SCR_ObjectBrushObjectBase> objectsCreationData = {};

		if (m_bDensityFallOffEnabled)
		{
			float subareaRadius = radiusDividedByFallOffCount;
			float previousArea = 0;
			int objectGenerationAttemptCurrentAmount = 0;
			float curvePoint = 0;
			float totalArea = Math.PI * m_fRadius * m_fRadius * HECTARE_CONVERSION_FACTOR;
			int objectGenerationAttemptMaximumAmount = Math.Ceil(totalArea * m_fStrength);

			SCR_ObjectBrushObjectBase obj;
			for (int i = 0; i < m_iDensityFallOffSubareaCount; i++)
			{
				float area = Math.PI * subareaRadius * subareaRadius * HECTARE_CONVERSION_FACTOR;
				area -= previousArea;
				previousArea += area;
				subareaRadius += radiusDividedByFallOffCount;

				float densityFallOffMultiplier = Math3D.Curve(ECurveType.CurveProperty2D, Math.Clamp(curvePoint, 0, 1), m_DensityFallOffCurve)[1];

				int objectGenerationAttemptsForSubarea = Math.Ceil(area * m_fStrength * densityFallOffMultiplier);
				if (objectGenerationAttemptMaximumAmount < objectGenerationAttemptsForSubarea)
					objectGenerationAttemptsForSubarea = objectGenerationAttemptMaximumAmount;

				for (int x = 0; x < objectGenerationAttemptsForSubarea; x++)
				{
					obj = GetRandomBrushObjectData();
					if (obj)
					{
						obj.m_iSubareaIndex = i + 1;
						objectsCreationData.Insert(obj);
					}
				}

				curvePoint += oneDividedByFallOffCount;
			}
		}
		else
		{
			float area = Math.PI * m_fRadius * m_fRadius * HECTARE_CONVERSION_FACTOR;
			int objectGenerationAttempts = Math.Ceil(area * m_fStrength);

			SCR_ObjectBrushObjectBase obj;
			for (int x = 0; x < objectGenerationAttempts; x++)
			{
				obj = GetRandomBrushObjectData();
				if (obj)
				{
					obj.m_iSubareaIndex = 1;
					objectsCreationData.Insert(obj);
				}
			}
		}

		if (m_bOverrideBrush)
			DeleteObjects(position);

		IEntity entity;
		IEntitySource entitySource;
		EntityID entityID;

		vector angles;

		vector traceStart;
		vector traceEndManual;
		vector traceDir;
		vector point;
		vector transformTemp[4];

		if (position == vector.Zero)
			m_API.TraceWorldPos(mouseX, mouseY, TraceFlags.WORLD, traceStart, traceEndManual, traceDir);
		else
			traceEndManual = position;

		m_vLastObjectCreationCentrePosition = traceEndManual;

		// area splines detected in OnMousePressEvent
		// non-area splines sphere detection here
		m_ObstacleDetector.RefreshRoadObstaclesBySphere(traceEndManual, m_fRadius);

		// if not world bbox detection (that is eventually done in OnMousePressEvent), refresh by sphere
		if (m_iAreaDetectionRadius > 0 && (m_bAvoidForests || m_bAvoidLakes))
		{
			m_ObstacleDetector.RefreshAreaObstaclesBySphere(traceEndManual, m_iAreaDetectionRadius);
			m_bAreasDetectedByWorld = false;
		}

		BaseWorld world = m_API.GetWorld();

		foreach (SCR_ObjectBrushObjectBase obj : objectsCreationData)
		{
			if (!obj.m_Prefab) // .IsEmpty() is slower (-:
				continue;

			if (m_bDensityFallOffEnabled)
			{
				float minDistanceFromCenter = radiusDividedByFallOffCount * (obj.m_iSubareaIndex - 1);
				float maxDistanceFromCenter = radiusDividedByFallOffCount * obj.m_iSubareaIndex;
				point = m_RandomGenerator.GenerateRandomPointInRadius(minDistanceFromCenter, maxDistanceFromCenter, traceEndManual);
			}
			else
			{
				point = m_RandomGenerator.GenerateRandomPointInRadius(0, m_fRadius, traceEndManual);
			}

#ifdef DEBUG
			m_aDebugShapes.Insert(CreateCircle(point, vector.Up, 0.5, ARGB(255, 0, 0, 255), 4, ShapeFlags.NOZBUFFER));
#endif

			if (m_Grid.IsColliding(point, obj))
				continue;

			float terrainY = world.GetSurfaceY(point[0], point[2]);
			point[1] = terrainY;

			if (m_ObstacleDetector.HasObstacle(point))
				continue;

			float scale;

			if (!obj.m_bOverrideRandomization)
			{
				entity = m_API.CreateEntityExt(obj.m_Prefab, "", m_API.GetCurrentEntityLayerId(), null, point, vector.Zero, TraceFlags.WORLD);
				if (!entity)
				{
					Print("Could not create entity from prefab", LogLevel.WARNING);
					continue;
				}

				scale = entity.GetScale();
			}
			else
			{
				entity = m_API.CreateEntity(obj.m_Prefab, "", m_API.GetCurrentEntityLayerId(), null, point, vector.Zero);
				if (!entity)
				{
					Print("Could not create entity from prefab", LogLevel.WARNING);
					continue;
				}

				if (obj.m_fMinScale == obj.m_fMaxScale)
				{
					scale = obj.m_fMinScale;
				}
				else
				{
					if (obj.m_fMinScale < obj.m_fMaxScale)
						scale = m_RandomGenerator.RandFloatXY(obj.m_fMinScale, obj.m_fMaxScale);
					else
						scale = m_RandomGenerator.RandFloatXY(obj.m_fMaxScale, obj.m_fMinScale);
				}
			}

			if (obj.m_bScaleFalloff)
			{
				if (m_fRadius <= 0)
				{
					Print("Please set radius to a value higher than 0", LogLevel.WARNING);
					return;
				}

				float distanceFromCenter = vector.DistanceXZ(traceEndManual, point);
				float distanceFromCenterInPercent = distanceFromCenter / (m_fRadius * 0.01);
				float scaleFallOffMultiplier = Math3D.Curve(ECurveType.CurveProperty2D, Math.Clamp(distanceFromCenterInPercent * 0.01, 0, 1), m_ScaleFallOffCurve)[1];

				scale *= scaleFallOffMultiplier;

				if (obj.m_fLowestScaleFalloffValue > obj.m_fMaxScale)
					Print("Lowest scale fall off value for " + obj.m_Prefab + " is higher than max scale value! Check parameters!", LogLevel.WARNING);

				if (scale < obj.m_fLowestScaleFalloffValue)
					scale = obj.m_fLowestScaleFalloffValue;

				if (scale > MAX_SCALE_THRESHOLD)
					scale = MAX_SCALE_THRESHOLD;
			}

			entitySource = m_API.EntityToSource(entity);

			int flags;
			if (entitySource.Get("Flags", flags))		// set point's Y value depending on entitySource's RELATIVE_Y flag
			{										// 0 if relative, otherwise terrain surface
				if (flags & EntityFlags.RELATIVE_Y)
					point[1] = 0;
				else
					point[1] = terrainY;
			}

			point[1] = point[1] + obj.m_fPrefabOffsetY;
			if (obj.m_fMinRandomVerticalOffset != 0 || obj.m_fMaxRandomVerticalOffset != 0)
				point[1] = point[1] + m_RandomGenerator.RandFloatXY(obj.m_fMinRandomVerticalOffset, obj.m_fMaxRandomVerticalOffset);

			m_API.ModifyEntityKey(entity, "coords", point.ToString(false));

			angles = vector.Zero;

			if (obj.m_bAlignToNormal)
			{
				// allow for random Yaw
				if (obj.m_bOverrideRandomization && obj.m_fRandomYawAngle > 0)
					m_API.ModifyEntityKey(entity, "angleY", m_RandomGenerator.RandFloatXY(-obj.m_fRandomYawAngle, obj.m_fRandomYawAngle).ToString());

				vector mat[4];
				entity.GetWorldTransform(mat);
				SCR_TerrainHelper.OrientToTerrain(mat, world);
				angles = Math3D.MatrixToAngles(mat);
			}
			else if (obj.m_bOverrideRandomization)
			{
				if (obj.m_fRandomPitchAngle > 0)
					angles[1] = m_RandomGenerator.RandFloatXY(-obj.m_fRandomPitchAngle, obj.m_fRandomPitchAngle);
				if (obj.m_fRandomYawAngle > 0)
					angles[0] = m_RandomGenerator.RandFloatXY(-obj.m_fRandomYawAngle, obj.m_fRandomYawAngle);
				if (obj.m_fRandomRollAngle > 0)
					angles[2] = m_RandomGenerator.RandFloatXY(-obj.m_fRandomRollAngle, obj.m_fRandomRollAngle);
			}

			m_Grid.AddEntry(obj, point);

			if (scale != 1)
				m_API.ModifyEntityKey(entity, "scale", scale.ToString());

			if (angles != vector.Zero)
			{
				m_API.ModifyEntityKey(entity, "angleX", angles[1].ToString());
				m_API.ModifyEntityKey(entity, "angleY", angles[0].ToString());
				m_API.ModifyEntityKey(entity, "angleZ", angles[2].ToString());
			}

			entityID = entity.GetID();
			m_mCreatedObjects.Insert(obj, entityID);
			m_mActiveBrushObjects.Insert(obj, entityID);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Delete all Brush-created entities around the provided position
	//! \param position
	protected void DeleteObjects(vector position)
	{
		if (m_mCreatedObjects.IsEmpty() || position == vector.Zero)
			return;

		map<SCR_ObjectBrushObjectBase, EntityID> objectsToDelete = new map<SCR_ObjectBrushObjectBase, EntityID>();
		BaseWorld world = m_API.GetWorld();

		IEntity entity;
		vector entOrigin;
		foreach (SCR_ObjectBrushObjectBase obj, EntityID entityID : m_mCreatedObjects)
		{
			entity = world.FindEntityByID(entityID);
			if (!entity)
			{
				objectsToDelete.Insert(obj, entityID);
				continue;
			}

			entOrigin = entity.GetOrigin();
			float diff = vector.DistanceXZ(entOrigin, position);

			if (diff <= m_fRadius)
			{
				if (m_bOverrideBrush)
				{
					if (!m_mActiveBrushObjects.Contains(obj))
						objectsToDelete.Insert(obj, entityID);
				}
				else
				{
					objectsToDelete.Insert(obj, entityID);
				}
			}
		}

		if (objectsToDelete.IsEmpty())
			return;

		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();

		foreach (SCR_ObjectBrushObjectBase obj, EntityID entID : objectsToDelete)
		{
			entity = world.FindEntityByID(entID);

			m_Grid.RemoveEntry(obj);
			if (entity)
				m_API.DeleteEntity(entity);

			m_mCreatedObjects.Remove(obj);
		}

		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete all Brush-created entities at once
	[ButtonAttribute("Delete created entities")]
	protected void DeleteEntities()
	{
		if (m_mCreatedObjects.IsEmpty())
			return;

		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();

		BaseWorld world = m_API.GetWorld();
		if (!world)
		{
			Print("World is null", LogLevel.ERROR);
			return;
		}

		IEntity entity;
		foreach (EntityID entityID : m_mCreatedObjects)
		{
			entity = world.FindEntityByID(entityID);
			if (entity)
				m_API.DeleteEntity(entity);
		}

		OnActivate();

		m_mCreatedObjects.Clear();

#ifdef DEBUG
		m_aDebugShapes.Clear();
#endif

		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on Mouse click - see WorldEditorTool.OnMousePressEvent()
	//! Filtered to work only for left-click here
	//! Creates/Refreshes the Obstacles Detector and deals with Click / Alt+Click
	//! \param x
	//! \param y
	//! \param buttons
	override void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		if (buttons != WETMouseButtonFlag.LEFT)
			return;

		string worldPath;
		m_API.GetWorldPath(worldPath);
		if (worldPath.IsEmpty())
		{
			Print("A proper world is not loaded", LogLevel.WARNING);
			return;
		}

		if (!m_ObjectsConfig || !m_ObjectsConfig.m_aObjectArray || m_ObjectsConfig.m_aObjectArray.IsEmpty())
		{
			Print("Object Brush's Objects Config is not filled", LogLevel.WARNING);
			return;
		}

		vector traceStart, traceEnd, traceDir;
		m_API.TraceWorldPos(x, y, TraceFlags.WORLD, traceStart, traceEnd, traceDir);

		m_bIsMouseHeldDown = true;
		m_RandomGenerator = new RandomGenerator();

		BaseWorld world = m_API.GetWorld();

		// this very operation loop takes some time (~500ms for ~2000 entities) when Ctrl+Z a big amount of entities
		foreach (SCR_ObjectBrushObjectBase objBase, EntityID entityID : m_mCreatedObjects)
		{
			if (world.FindEntityByID(entityID))
				continue;

			m_Grid.RemoveEntry(objBase);
			m_mCreatedObjects.Remove(objBase);
		}

		// create/check the Obstacle Detector on each click to consider any settings change
		CreateAndInitialiseObstacleDetector();

		// non-area splines sphere detected in CreateObjects
		// area splines detected here
		if (!m_bAreasDetectedByWorld && (m_bAvoidForests || m_bAvoidLakes) && m_iAreaDetectionRadius < 1)
		{
			m_ObstacleDetector.RefreshAreaObstaclesByWorld();
			m_bAreasDetectedByWorld = true;
		}

		// normal click
		if (!m_bLineMode && !GetModifierKeyState(ModifierKey.ALT))
		{
			m_bManageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction(); // ended in OnMouseRelease

			m_mActiveBrushObjects.Clear();

			if (m_bDeleteMode)
				DeleteObjects(traceEnd);
			else
				CreateObjects(x, y, traceEnd);
		}
		else // in lineMode or Alt is pressed, draw the entity line
		{
			if (m_vFirstLinePoint == vector.Zero)
			{
				m_bLineMode = true;
				m_vFirstLinePoint = traceEnd;
				return;
			}

			m_bLineMode = false;

			bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();

			m_mActiveBrushObjects.Clear();

			m_vSecondLinePoint = traceEnd;

			float dist = vector.Distance(m_vFirstLinePoint, m_vSecondLinePoint);
			vector dir = vector.Direction(m_vFirstLinePoint, m_vSecondLinePoint);
			dir.Normalize();

			vector point = m_vFirstLinePoint;

#ifdef DEBUG
			vector p[2];
			p[0] = m_vFirstLinePoint;
			p[1] = m_vSecondLinePoint;
			m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));
			m_aDebugShapes.Insert(CreateCircle(m_vFirstLinePoint, vector.Up, 2, ARGB(255, 0, 255, 0), 4, ShapeFlags.NOZBUFFER));
			m_aDebugShapes.Insert(CreateCircle(m_vSecondLinePoint, vector.Up, 2, ARGB(255, 0, 255, 0), 4, ShapeFlags.NOZBUFFER));
#endif

			if (m_bDeleteMode)
				DeleteObjects(point);
			else
				CreateObjects(x, y, point);

			float halfRadius = m_fRadius * 0.5;
			while (dist - halfRadius > 0)
			{
				point = point + dir * halfRadius;
				dist -= halfRadius;

#ifdef DEBUG
				m_aDebugShapes.Insert(CreateCircle(point, vector.Up, 1, ARGB(255, 0, 255, 0), 12, ShapeFlags.NOZBUFFER));
#endif

				if (m_bDeleteMode)
					DeleteObjects(point);
				else
					CreateObjects(x, y, point);
			}

			if (dist > m_fRadius * 0.25)
			{
#ifdef DEBUG
				m_aDebugShapes.Insert(CreateCircle(m_vSecondLinePoint, vector.Up, 1, ARGB(255, 0, 255, 0), 12, ShapeFlags.NOZBUFFER));
#endif

				if (m_bDeleteMode)
					DeleteObjects(m_vSecondLinePoint);
				else
					CreateObjects(x, y, m_vSecondLinePoint);
			}

			m_vFirstLinePoint = vector.Zero;
			m_vSecondLinePoint = vector.Zero;

			SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on Mouse click release - see WorldEditorTool.OnMouseReleaseEvent()
	//! Filtered to work only for left-click here
	//! Clears "held mouse click" flag and stops Edit action
	//! \param x
	//! \param y
	//! \param buttons
	override void OnMouseReleaseEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		if (buttons != WETMouseButtonFlag.LEFT)
			return;

		m_bIsMouseHeldDown = false;
		m_vLastObjectCreationCentrePosition = vector.Zero;

		SCR_WorldEditorToolHelper.EndEntityAction(m_bManageEditAction); // ending OnMousePressEvent's normal click
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on Mouse movement - see WorldEditorTool.OnMouseMoveEvent()
	//! Create/Delete entities and draw Alt+Click additional shapes
	//! \param x
	//! \param y
	//! \param buttons
	override void OnMouseMoveEvent(float x, float y)
	{
		m_aLineShapes.Clear();

		vector traceStart, traceEnd, traceDir;
		m_API.TraceWorldPos(x, y, TraceFlags.WORLD, traceStart, traceEnd, traceDir);

		m_vLastMousePosition = traceEnd;

		if (m_bDeleteMode)
			m_iBrushShapeColor = ARGB(255, 255, 0, 0);
		else
			m_iBrushShapeColor = ARGB(255, 0, 255, 0);

		m_BrushShape = CreateCircle(traceEnd, vector.Up, m_fRadius, m_iBrushShapeColor, 50, ShapeFlags.NOZBUFFER);

		// in Alt+Click mode
		if (m_vFirstLinePoint != vector.Zero)
		{
			m_aLineShapes.Insert(CreateCircle(m_vFirstLinePoint, vector.Up, m_fRadius, m_iBrushShapeColor, 50, ShapeFlags.NOZBUFFER));

			vector fromTo = (traceEnd - m_vFirstLinePoint).Normalized();
			fromTo = fromTo * m_fRadius;

			vector points[2];
			points[0] = m_vFirstLinePoint + fromTo;
			points[1] = traceEnd - fromTo;

			// middle line
			if (vector.DistanceXZ(m_vFirstLinePoint, traceEnd) > m_fRadius * 2)
				m_aLineShapes.Insert(Shape.CreateLines(m_iBrushShapeColor, ShapeFlags.NOZBUFFER, points, 2));

			// left border
			vector offset = { -fromTo[2], 0, fromTo[0] };
			points[0] = m_vFirstLinePoint + offset;
			points[1] = traceEnd + offset;
			m_aLineShapes.Insert(Shape.CreateLines(m_iBrushShapeColor, ShapeFlags.NOZBUFFER, points, 2));

			// right border
			offset = { fromTo[2], 0, -fromTo[0] };
			points[0] = m_vFirstLinePoint + offset;
			points[1] = traceEnd + offset;
			m_aLineShapes.Insert(Shape.CreateLines(m_iBrushShapeColor, ShapeFlags.NOZBUFFER, points, 2));
		}

		if (!m_bIsMouseHeldDown)
			return;

		if (m_bDeleteMode)
			DeleteObjects(traceEnd);
		else if (m_vLastObjectCreationCentrePosition != vector.Zero && vector.DistanceXZ(m_vLastMousePosition, m_vLastObjectCreationCentrePosition) >= STRENGTH_RELATIVE_RADIUS_DISTANCE_TO_CREATE * m_fRadius)
			CreateObjects(x, y, traceEnd);
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on Mouse scroll wheel - see WorldEditorTool.OnWheelEvent()
	//! Used to set Brush's radius (Ctrl) or strength (Shift)
	//! \param delta the scroll wheel difference value
	override void OnWheelEvent(int delta)
	{
		// adjusts m_fRadius value using a CTRL + Scrollwheel keybind
		if (GetModifierKeyState(ModifierKey.CONTROL))
		{
			m_fRadius = AdjustValueUsingScrollwheel(delta, m_fRadius, RADIUS_MIN, RADIUS_MAX, RADIUS_STEP);
			m_BrushShape = CreateCircle(m_vLastMousePosition, vector.Up, m_fRadius, m_iBrushShapeColor, 50, ShapeFlags.NOZBUFFER);
			UpdatePropertyPanel();
		}

		// adjusts m_fStrength value using a SHIFT + Scrollwheel keybind
		if (GetModifierKeyState(ModifierKey.SHIFT))
		{
			m_fStrength = AdjustValueUsingScrollwheel(delta, m_fStrength, STRENGTH_MIN, STRENGTH_MAX, STRENGTH_STEP);
			UpdatePropertyPanel();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on keyboard (normal) key press - see WorldEditorTool.OnKeyPressEvent()
	//! Used to switch to Delete mode (Space) or cancel Alt+Click (Esc)
	//! \param key
	//! \param isAutoRepeat
	override void OnKeyPressEvent(KeyCode key, bool isAutoRepeat)
	{
		if (key == KeyCode.KC_SPACE)
		{
			m_bDeleteMode = true;
			m_iBrushShapeColor = ARGB(255, 255, 0, 0);
			m_BrushShape = CreateCircle(m_vLastMousePosition, vector.Up, m_fRadius, m_iBrushShapeColor, 50, ShapeFlags.NOZBUFFER);
		}
		else if (key == KeyCode.KC_ESCAPE)
		{
			if (m_bLineMode) // Alt+click line-drawing mode
			{
				m_bLineMode = false;
				m_vFirstLinePoint = vector.Zero;
				m_aLineShapes.Clear();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on keyboard (normal) key release - see WorldEditorTool.OnKeyReleaseEvent()
	//! Used to switch from Delete mode (Space)
	//! \param key
	//! \param isAutoRepeat
	override void OnKeyReleaseEvent(KeyCode key, bool isAutoRepeat)
	{
		if (key == KeyCode.KC_SPACE)
		{
			m_bDeleteMode = false;
			m_iBrushShapeColor = ARGB(255, 0, 255, 0);
			m_BrushShape = CreateCircle(m_vLastMousePosition, vector.Up, m_fRadius, m_iBrushShapeColor, 50, ShapeFlags.NOZBUFFER);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when another WE Tool is selected - see WorldEditorTool.OnDeActivate()
	//! Delete Brush's shape and resets Obstacle Detector's Areas flag
	override void OnDeActivate()
	{
		m_BrushShape = null;
		m_bAreasDetectedByWorld = false; // one could go create a Forest or a Lake, let's re-check
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when this WE Tool is selected - see WorldEditorTool.OnActivate()
	//! Sets up the grid size
	override void OnActivate()
	{
		m_Grid.Clear();

		if (!m_API.GetWorld())
			return;

		vector terrainMin, terrainMax;
		m_API.GetWorld().GetBoundBox(terrainMin, terrainMax);

		float x = terrainMax[0] - terrainMin[0];
		float z = terrainMax[2] - terrainMin[2];

		m_Grid.Resize(x, z);
	}

	//------------------------------------------------------------------------------------------------
	//! Pick an object to create's information, from probabilities/weight (a weight of zero is ignored)
	//! It is assumed m_ObjectsConfig.m_aObjectArray is not null nor empty as this is checked in CreateObjects
	//! \return a selection from m_ObjectsConfig.m_aObjectArray - CAN return null if weights are zero
	protected SCR_ObjectBrushObjectBase GetRandomBrushObjectData()
	{
		if (m_ObjectsConfig.m_aObjectArray.Count() == 1)
		{
			if (m_ObjectsConfig.m_aObjectArray[0].m_fWeight <= 0)
				return null;

			return SCR_ObjectBrushObjectBase.Cast(m_ObjectsConfig.m_aObjectArray[0].Clone());
		}

		array<float> weights = {};
		foreach (SCR_ObjectBrushObjectBase obj : m_ObjectsConfig.m_aObjectArray)
		{
			weights.Insert(obj.m_fWeight);
		}

		int index = SCR_ArrayHelper.GetWeightedIndex(weights, m_RandomGenerator.RandFloat01());
		if (!m_ObjectsConfig.m_aObjectArray.IsIndexValid(index))
			return null;

		SCR_ObjectBrushObjectBase result = m_ObjectsConfig.m_aObjectArray[index];
		if (!result)
			return null;

		return SCR_ObjectBrushObjectBase.Cast(result.Clone());
	}

	//------------------------------------------------------------------------------------------------
	//! Helps getting proper new value for a property
	//! \param delta the scrollwheel value (obtained in OnWheelEvent) that is a multiple of 120
	//! \param currentValue the value from which to start
	//! \param min the min value
	//! \param max the max value
	//! \param step the step by which delta's converted value will be multiplied
	//! \return the min-max clamped new value
	protected float AdjustValueUsingScrollwheel(float delta, float currentValue, float min, float max, float step)
	{
		// delta returns multiples of 120 - converting it into a more useable value of multiples of 1
		float value = currentValue + (delta / 120) * step;

		if (value < min)
			return min;

		if (value > max)
			return max;

		return value;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates an SCR_ObstacleDetector if it is null or invalid
	//! Refreshes that Obstacle Detector with current UI settings (Avoid Objects, etc)
	protected void CreateAndInitialiseObstacleDetector()
	{
		if (!m_ObstacleDetector || !m_ObstacleDetector.IsValid())
		{
			m_ObstacleDetector = new SCR_ObstacleDetector(m_API);
			m_bAreasDetectedByWorld = false;
		}

		m_ObstacleDetector.SetAvoidObjects(m_bAvoidObjects);
		m_ObstacleDetector.SetAvoidObjectsDetectionRadius(m_fAvoidObjectsDetectionRadius);
		m_ObstacleDetector.SetAvoidObjectsDetectionHeight(m_fAvoidObjectsDetectionHeight);
		m_ObstacleDetector.SetAvoidRoads(m_bAvoidRoads);
		m_ObstacleDetector.SetAvoidRivers(m_bAvoidRivers);
		m_ObstacleDetector.SetAvoidPowerLines(m_bAvoidPowerLines);
		m_ObstacleDetector.SetAvoidForests(m_bAvoidForests);
		m_ObstacleDetector.SetAvoidLakes(m_bAvoidLakes);
		m_ObstacleDetector.SetAvoidLand(m_bAvoidLand);
		m_ObstacleDetector.SetAvoidOcean(m_bAvoidOcean);
	}
}
#endif