class PrefabGeneratorPointData : ShapePointDataScriptBase // cannot be renamed SCR_ without changing terrain layers
{
	[Attribute(desc: "Generate assets on this segment")]
	bool m_bGenerate;
}

class SCR_PrefabGeneratorPointMeta
{
	bool m_bGenerate;
	vector m_vPos;
}

class SCR_PrefabGeneratorAssetPoint
{
	vector m_vPos;
	vector m_vForward;
	float m_fPerlinWeight;
	bool m_bDraw = true;
}

[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "PrefabGeneratorEntity", dynamicBox: true, visible: false)]
class PrefabGeneratorEntityClass : SCR_GeneratorBaseEntityClass
{
}

class PrefabGeneratorEntity : SCR_GeneratorBaseEntity // TODO: make it use SCR_ObstacleDetector
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab list", "et")]
	protected ref array<ResourceName> m_PrefabNames;

	[Attribute("", UIWidgets.EditBox, "Weights", "et")]
	protected ref array<float> m_Weights;

	[Attribute(desc: "If checked prefabs are placed only to vertices")]
	protected bool m_bOnlyToVertices;

	[Attribute("5", UIWidgets.EditBox, "Dinstance between spawned assets along the spline/polyline")]
	protected float m_fDistance;

	[Attribute("1", UIWidgets.CheckBox, "If checked prefabs are aligned with the shape")]
	protected bool m_bAlignWithShape;

	[Attribute(desc: "Only when aligning with shape")]
	protected bool m_bUseXAsForward;

	[Attribute(desc: "Flip forward")]
	protected bool m_bFlipForward;

	[Attribute("0", UIWidgets.EditBox, "How much we offset from center to the side")]
	protected float m_fOffsetRight;

	[Attribute("0", UIWidgets.EditBox, "How much offset variability we want in meters when using offset from the center")]
	protected float m_fOffsetVariance;

	[Attribute("0", UIWidgets.EditBox, "Gap from the centerline")]
	protected float m_fGap;

	[Attribute("0", UIWidgets.EditBox, "Random Spacing Offset")]
	protected bool m_fRandomSpacing;

	[Attribute("0", UIWidgets.EditBox, "How much we offset in world up vector")]
	protected float m_fOffsetUp;

	[Attribute("0", UIWidgets.EditBox, "Forward offset")]
	protected float m_fOffsetForward;

	[Attribute(desc: "Prefab spawn density uses Perlin distribution")]
	protected bool m_bPerlinDens;

	[Attribute("0", UIWidgets.EditBox, "Spawns prefabs if perlin value is above this threshold", "-1 1")]
	protected float m_fPerlinThreshold;

	[Attribute(desc: "Prefab spawn type uses Perlin distribution. Prefabs in the prefab array are stacked up from lower to higher indicies on the Y axis")]
	protected bool m_bPerlinAssetDistribution;

	[Attribute(desc: "Prefab size uses Perlin")]
	protected bool m_bPerlinSize;

	[Attribute("40", UIWidgets.EditBox, "Perlin frequency, higher values mean smoother transitions")]
	protected float m_fPerlinFrequency;

	[Attribute("0", UIWidgets.EditBox, "Perlin seed", "0 1000")]
	protected float m_fPerlinSeed;

	[Attribute("1", UIWidgets.EditBox, "Perlin Amplitude")]
	protected float m_fPerlinAmplitude;

	[Attribute("0", UIWidgets.EditBox, "Perlin Phase Offset")]
	protected float m_fPerlinOffset;

	[Attribute(desc: "Disables asset generation bellow perlin threshold")]
	protected bool m_fPerlinThrowAway;

	[Attribute("0", UIWidgets.EditBox, "Draw developer debug")]
	protected bool m_bDrawDebug;

#ifdef WORKBENCH

	protected ref array<ref SCR_PrefabGeneratorAssetPoint> m_aPoints = {};

	protected static ref array<ref Shape> s_aDebugShapes = {};

	//------------------------------------------------------------------------------------------------
	protected override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (key == "coords")
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return false;

		IEntitySource entSrc = src.ToEntitySource();

		IEntitySource thisSrc = worldEditorAPI.EntityToSource(this);
		IEntitySource parentSrc = thisSrc.GetParent();
		BaseContainerTools.WriteToInstance(this, thisSrc);

		OnShapeChanged(parentSrc, ShapeEntity.Cast(parent), {}, {});
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetPos(BaseContainerList points, int i)
	{
		BaseContainer point = points.Get(i);
		vector pos;
		point.Get("Position", pos);
		return pos;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		if (!shapeEntity)
			shapeEntity = ShapeEntity.Cast(_WB_GetEditorAPI().SourceToEntity(shapeEntitySrc));

		OnShapeChanged(shapeEntitySrc, shapeEntity, {}, {});
	}

	//------------------------------------------------------------------------------------------------
	//! Offsets the points in the 'points' array
	protected void OffsetPointsMeta(notnull array<ref SCR_PrefabGeneratorPointMeta> metas, float offset)
	{
		array<ref SCR_PrefabGeneratorPointMeta> metasTemp = {};

		SCR_PrefabGeneratorPointMeta tmpMeta;
		foreach (SCR_PrefabGeneratorPointMeta meta : metas)
		{
			tmpMeta = new SCR_PrefabGeneratorPointMeta();
			tmpMeta.m_vPos = meta.m_vPos;
			tmpMeta.m_bGenerate = meta.m_bGenerate;
			metasTemp.Insert(tmpMeta);
		}

		vector forwardPrev = "1 1 1";
		for (int i, count = metas.Count(); i < count; i++)
		{
			vector forwardNext;

			if (i < count - 1)
				forwardNext = metas.Get(i + 1).m_vPos - metas.Get(i).m_vPos;
			else
				forwardNext = -forwardPrev;

			if (i == 0)
				forwardPrev = -forwardNext;

			forwardNext.Normalize();
			forwardPrev.Normalize();

			float dotProductPrevNext = vector.Dot(forwardPrev, forwardNext);
			bool almostLine = dotProductPrevNext < -0.95;
			vector diagonal = forwardNext + forwardPrev;

			vector normalRight = -forwardPrev * "0 1 0";
			normalRight.Normalize();
			float dotProductNormNext = vector.Dot(normalRight, forwardNext);
			bool isLeft = dotProductNormNext > 0;

			vector nextModified = dotProductPrevNext * forwardNext;
			float dist = vector.Distance(nextModified, forwardPrev);
			float diff;
			if (dist !=0)
				diff = offset / dist;

			diagonal *= diff;

			if (!isLeft)
				diagonal = diagonal * -1;

			if (almostLine)
			{
				vector vec = forwardNext - forwardPrev;
				vector right = vec * "0 1 0";
				right.Normalize();
				diagonal = right * offset;
			}

			metas.Get(i).m_vPos = metasTemp.Get(i).m_vPos + diagonal;
			forwardPrev = -forwardNext;

			// debug
			if (m_bDrawDebug)
			{
				vector matWrld[4];
				GetWorldTransform(matWrld);

				s_aDebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER, metas.Get(i).m_vPos.Multiply4(matWrld), metas.Get(i).m_vPos.Multiply4(matWrld) + forwardNext));
				s_aDebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, metas.Get(i).m_vPos.Multiply4(matWrld), metas.Get(i).m_vPos.Multiply4(matWrld) + forwardPrev));
				s_aDebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, metasTemp.Get(i).m_vPos.Multiply4(matWrld), metas.Get(i).m_vPos.Multiply4(matWrld)));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get a perlin value given specific parameters
	protected float SamplePerlin(float time, float frequency = 20, float seed = 1, float amplitude = 1, float phaseOffset = 0)
	{
		if (frequency == 0 || amplitude == 0) // division/multiplication by zero protection
			return 0;

		return Math.PerlinNoise(time / frequency, seed * 100, seed * 1000) * amplitude;
	}

	//------------------------------------------------------------------------------------------------
	//! generates array of positions(+other data) where each asset is supposed to spawn
	protected void GenerateAssetPoints(array<ref SCR_PrefabGeneratorPointMeta> metas)
	{
		float stepDistance = m_fDistance;

		if (m_bPerlinDens)
			stepDistance = 0.01; // TODO:make constant

		float distanceWalked = 0;
		if (m_fOffsetForward)
			stepDistance = m_fOffsetForward;

		float space = 0;
		float moveDist = 0;
		vector lastPos = "0 0 0";
		bool firstPoint = true;
		bool draw = true;

		for (int i = 0, countMinusOne = metas.Count() - 1; i < countMinusOne; i++)
		{
			vector posThis = metas[i].m_vPos;
			//Print(posThis);
			lastPos = posThis;
			vector forward = metas[i + 1].m_vPos - posThis;
			float dist = forward.Length();
			space += dist;

			// check for disabled generation on line segment
			draw = metas.Get(i).m_bGenerate;

			while (space >= stepDistance)
			{
				float leftOver = space - dist;
				if (leftOver < 0)
					leftOver = 0;

				if (firstPoint)
				{
					moveDist = 0;
					stepDistance = 0;
					firstPoint = false;
					if (m_fOffsetForward)
					{
						stepDistance = m_fOffsetForward;
						moveDist = dist - (space - stepDistance);
					}
				}
				else
				{
					stepDistance = m_fDistance;

					if (m_fRandomSpacing)
					{
						stepDistance = m_fDistance + Math.RandomFloat(-m_fDistance * 0.5, m_fDistance * 0.5);
						if (leftOver> stepDistance)
							leftOver = 0;
					}

					moveDist = stepDistance - leftOver;
				}

				space -= moveDist + leftOver;
				distanceWalked += stepDistance;
				vector newPos;
				float valuePerlin = SamplePerlin(distanceWalked + m_fPerlinOffset, m_fPerlinFrequency, m_fPerlinSeed, m_fPerlinAmplitude);

				newPos = lastPos + forward.Normalized() * moveDist;
				lastPos = newPos;
				SCR_PrefabGeneratorAssetPoint genPoint = new SCR_PrefabGeneratorAssetPoint();
				genPoint.m_vPos = newPos;
				genPoint.m_vForward = forward;
				genPoint.m_fPerlinWeight = valuePerlin;

				if (m_bPerlinDens)
				{
					float invValue = Math.InverseLerp(m_fPerlinThreshold, 1, valuePerlin);
					invValue = Math.Clamp(invValue, 0, 1);
					stepDistance = Math.Lerp(m_fDistance, 1, invValue); // TODO: expose values
				}

				bool throwAway = false;
				if (m_fPerlinThrowAway)
				{
					if (valuePerlin < m_fPerlinThreshold)
					{
						stepDistance = 0.01;
						throwAway = true;
					}
				}

				genPoint.m_bDraw = draw && !throwAway;
				m_aPoints.Insert(genPoint);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateMetaListLine(BaseContainerList points, array<ref SCR_PrefabGeneratorPointMeta> pointsMeta)
	{
		SCR_PrefabGeneratorPointMeta meta;
		for (int i = 0, count = points.Count(); i < count; i++)
		{
			BaseContainer point = points.Get(i);
			BaseContainerList dataArr = point.GetObjectArray("Data");
			bool generate = true;
			vector pos;
			point.Get("Position", pos);

			if (dataArr)
			{
				int dataCount = dataArr.Count();
				for (int j = 0; j < dataCount; j++)
				{
					BaseContainer data = dataArr.Get(j);
					if (data.GetClassName() == "PrefabGeneratorPointData")
					{
						data.Get("m_bGenerate", generate);
						break;
					}
				}
			}

			meta = new SCR_PrefabGeneratorPointMeta();
			meta.m_bGenerate = generate;
			meta.m_vPos = pos;
			pointsMeta.Insert(meta);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateMetaListSpline(array<ref SCR_PrefabGeneratorPointMeta> pointsMetaLine, array<vector> tesselatedPoints, array<ref SCR_PrefabGeneratorPointMeta> pointsMetaSpline)
	{
		if (!tesselatedPoints)
			return;

		bool generate = true;
		SCR_PrefabGeneratorPointMeta tmpMeta;
		foreach (vector tesselatedPoint : tesselatedPoints)
		{
			foreach (SCR_PrefabGeneratorPointMeta meta : pointsMetaLine)
			{
				if (tesselatedPoint == meta.m_vPos) // matching point on the spline with a point on the polyline to ascertain whether asset generation is disabled in point data or not for this segment
				{
					generate = meta.m_bGenerate;
					break;
				}
			}

			tmpMeta = new SCR_PrefabGeneratorPointMeta();
			tmpMeta.m_bGenerate = generate;
			tmpMeta.m_vPos = tesselatedPoint;
			pointsMetaSpline.Insert(tmpMeta);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		s_aDebugShapes.Clear();
		m_aPoints.Clear();

		if (!shapeEntitySrc)
			return;

		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (points != null)
		{
			array<ref SCR_PrefabGeneratorPointMeta> pointsMetaLine = {};
			GenerateMetaListLine(points, pointsMetaLine);

			int pointCount = points.Count();
			if (pointCount == 0)
				return;

			bool isShapeClosed;
			shapeEntitySrc.Get("IsClosed", isShapeClosed);

			if (pointCount > 2 && isShapeClosed)
				pointsMetaLine.Insert(pointsMetaLine[0]);

			if (m_fOffsetRight != 0)
				OffsetPointsMeta(pointsMetaLine, m_fOffsetRight);

			if (!m_bOnlyToVertices && m_fDistance > 0 && shapeEntitySrc.GetClassName() == "PolylineShapeEntity")
			{
				GenerateAssetPoints(pointsMetaLine);
				if (m_bDrawDebug)
					DrawCurveDebug(pointsMetaLine);
			}
			else if (!m_bOnlyToVertices && m_fDistance > 0 && shapeEntitySrc.GetClassName() == "SplineShapeEntity")
			{
				array<vector> pointsCurve = {};
				shapeEntity.GenerateTesselatedShape(pointsCurve);

				array<ref SCR_PrefabGeneratorPointMeta> pointsMetaSpline = {};

				GenerateMetaListSpline(pointsMetaLine, pointsCurve, pointsMetaSpline);

				if (pointCount > 2 && isShapeClosed)
					pointsMetaSpline.Insert(pointsMetaSpline[0]);

				if (m_fOffsetRight != 0)
					OffsetPointsMeta(pointsMetaSpline, m_fOffsetRight);

				GenerateAssetPoints(pointsMetaSpline);

				if (m_bDrawDebug)
					DrawCurveDebug(pointsMetaSpline);
			}
			else if (m_bOnlyToVertices)
			{
				if (m_bDrawDebug)
					DrawCurveDebug(pointsMetaLine);

				// First point special care
				SCR_PrefabGeneratorAssetPoint genPoint = new SCR_PrefabGeneratorAssetPoint();
				genPoint.m_vPos = pointsMetaLine[0].m_vPos;

				if (pointsMetaLine)
					genPoint.m_bDraw = pointsMetaLine.Get(0).m_bGenerate;

				if (isShapeClosed && pointCount > 2)
					genPoint.m_vForward = pointsMetaLine[1].m_vPos - pointsMetaLine[pointCount - 1].m_vPos.Normalized();
				else if (pointCount > 1)
					genPoint.m_vForward = (pointsMetaLine[1].m_vPos - pointsMetaLine[0].m_vPos).Normalized();

				m_aPoints.Insert(genPoint);

				// Middle points
				for (int i = 1, count = pointsMetaLine.Count(); i < count; i++)
				{
					genPoint = new SCR_PrefabGeneratorAssetPoint();
					genPoint.m_vPos = pointsMetaLine[i % pointCount].m_vPos;

					genPoint.m_bDraw = pointsMetaLine.Get(i).m_bGenerate;

					if (i < count - 1 || isShapeClosed)
						genPoint.m_vForward = (pointsMetaLine[(i + 1) % pointCount].m_vPos - pointsMetaLine[i - 1].m_vPos).Normalized();
					else if (i == count - 1)
						genPoint.m_vForward = (pointsMetaLine[i % pointCount].m_vPos - pointsMetaLine[i - 1].m_vPos).Normalized();

					m_aPoints.Insert(genPoint);
				}
			}
			else
			{
				return;
			}
		}

		Generate();
	}

	//------------------------------------------------------------------------------------------------
	//! Generate the assets
	protected void Generate()
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		array<string> prefabs = {};
		foreach (ResourceName prefabName : m_PrefabNames)
		{
			if (prefabName != string.Empty)
				prefabs.Insert(prefabName);
		}

		if (prefabs.IsEmpty()) // after that, prefabs is not used anymore - this because of m_Weights usage
			return;

		IEntitySource entSrc = worldEditorAPI.EntityToSource(this);

		IEntitySource childSrc;
		IEntity child;
		for (int i = entSrc.GetNumChildren() - 1; i >= 0; --i)
		{
			childSrc = entSrc.GetChild(i);
			child = worldEditorAPI.SourceToEntity(childSrc);
			worldEditorAPI.DeleteEntity(child);
		}

		bool isGeneratorVisible = worldEditorAPI.IsEntityVisible(this);
		vector worldMat[4];
		GetWorldTransform(worldMat);

		foreach (SCR_PrefabGeneratorAssetPoint localPoint : m_aPoints)
		{
			if (!localPoint.m_bDraw)
				continue;

			vector worldPos = localPoint.m_vPos.Multiply4(worldMat);

			vector rot = "0 0 0";
			vector mat[4];
			Math3D.DirectionAndUpMatrix(localPoint.m_vForward, vector.Up, mat);
			vector right = mat[0];
			vector angles = Math3D.MatrixToAngles(mat);

			if (m_fOffsetVariance != 0)
			{
				float offsetRandom = Math.RandomFloat(-m_fOffsetVariance * 0.5 + m_fGap * 0.5, m_fOffsetVariance * 0.5 - m_fGap * 0.5);
				if (offsetRandom < 0)
					offsetRandom -= m_fGap * 0.5;
				else
					offsetRandom += m_fGap * 0.5;

				worldPos = worldPos + offsetRandom * right;
			}

			if (m_bAlignWithShape)
			{
				if (m_bUseXAsForward)
					angles[0] = angles[0] - 90;

				if (m_bFlipForward)
					angles[0] = angles[0] + 180;

				rot = { angles[1], angles[0], angles[2] };
			}

			int index;

			if (m_bPerlinAssetDistribution)
			{
				float value = Math.Clamp(localPoint.m_fPerlinWeight, m_fPerlinThreshold, 1);
				float invValue = Math.InverseLerp(m_fPerlinThreshold, 1, value);

				index = Math.Clamp(SCR_ArrayHelper.GetWeightedIndex(m_Weights, invValue), 0, m_PrefabNames.Count() - 1);
			}
			else
			{
				index = Math.Clamp(SCR_ArrayHelper.GetWeightedIndex(m_Weights, Math.RandomFloat01()), 0, m_PrefabNames.Count() - 1);
			}

			string asset = m_PrefabNames[index];

			IEntity ent = worldEditorAPI.CreateEntityExt(asset, "", 0, null, worldPos, rot, TraceFlags.WORLD);
			IEntitySource src = worldEditorAPI.EntityToSource(ent);

			if (m_bPerlinSize)
			{
				BaseContainerList editorData = src.GetObjectArray("editorData");

				if (editorData && editorData.Count() > 0)
				{
					float valuePrln = Math.Clamp(localPoint.m_fPerlinWeight, m_fPerlinThreshold, 1);
					//Print("raw perlin:" +localPoint.m_fPerlinWeight);
					float invValue = Math.InverseLerp(m_fPerlinThreshold, 1, valuePrln);
					vector val;
					editorData.Get(0).Get("randomScale", val);

					string scale = (Math.Lerp(val[0], val[1], invValue)).ToString();
					worldEditorAPI.ModifyEntityKey(ent, "scale", scale);
				}
			}

			if (m_fOffsetUp != 0)
			{
				vector entPos;
				src.Get("coords", entPos);
				entPos[1] = entPos[1] + m_fOffsetUp;
				string coords = entPos[0].ToString() + " " + entPos[1].ToString() + " " + entPos[2].ToString();
				worldEditorAPI.ModifyEntityKey(ent, "coords", coords);
			}

			worldEditorAPI.SetEntityVisible(ent, isGeneratorVisible, false);
			worldEditorAPI.ParentEntity(this, ent, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Draws debug curve
	protected void DrawCurveDebug(array<ref SCR_PrefabGeneratorPointMeta> metas)
	{
		const int DEBUG_CURVE_LINE_SIZE = 8192;
		const float DEBUG_Y_MULTIPLIER = 5;
		static vector m_PerlinDebugLine[DEBUG_CURVE_LINE_SIZE];
		static vector m_ZeroDebugLine[DEBUG_CURVE_LINE_SIZE];
		static vector m_EdgeDebugLine[DEBUG_CURVE_LINE_SIZE];
		static vector m_ThresholdDebugLine[DEBUG_CURVE_LINE_SIZE];

		float stepDistance = 0.1;
		float distanceWalked = 0;
		vector lastPos = "0 0 0";
		float space = 0;
		float moveDist = 0;

		array<vector> line = {};
		int itemsNum = 0;

		vector matWrld[4];
		GetWorldTransform(matWrld);

		int i = 0;
		for (int countMinusOne = metas.Count() - 1; i < countMinusOne; i++)
		{
			vector posThis = metas[i].m_vPos;
			lastPos = posThis;
			vector forward = metas[i + 1].m_vPos - posThis;
			float dist = forward.Length();
			space += dist;

			m_ZeroDebugLine[i] = posThis.Multiply4(matWrld);

			if (i == countMinusOne - 1)
				m_ZeroDebugLine[i + 1] = metas[i + 1].m_vPos.Multiply4(matWrld);

			while (space >= stepDistance)
			{
				float leftOver = space - dist;
				if (leftOver < 0)
					leftOver = 0;

				moveDist = stepDistance - leftOver;

				space -= moveDist + leftOver;
				distanceWalked += stepDistance;
				vector stepPos;
				vector perlinPointPos;

				stepPos = lastPos + forward.Normalized() * moveDist;
				lastPos = stepPos;

				vector mat[4];
				vector up = {0, 1, 0};
				Math3D.DirectionAndUpMatrix(forward, up, mat);
				vector right = mat[0];

				float valuePerlin = SamplePerlin(distanceWalked + m_fPerlinOffset, m_fPerlinFrequency, m_fPerlinSeed, m_fPerlinAmplitude);
				//Print(distanceWalked);
				//Print(valuePerlin);
				perlinPointPos = stepPos + right * valuePerlin * DEBUG_Y_MULTIPLIER;

				if (itemsNum < DEBUG_CURVE_LINE_SIZE)
				{
					m_PerlinDebugLine[itemsNum] = perlinPointPos.Multiply4(matWrld);
					m_EdgeDebugLine[itemsNum] = (stepPos + right * DEBUG_Y_MULTIPLIER).Multiply4(matWrld);
					m_ThresholdDebugLine[itemsNum] = (stepPos + right * DEBUG_Y_MULTIPLIER * m_fPerlinThreshold).Multiply4(matWrld);
					itemsNum++;
				}
			}
		}

		s_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 255, 0), ShapeFlags.NOZBUFFER, m_PerlinDebugLine, itemsNum));
		s_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, m_ZeroDebugLine, i+ 1));
		s_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, m_EdgeDebugLine, itemsNum));
		s_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, m_ThresholdDebugLine, itemsNum));
	}

	//------------------------------------------------------------------------------------------------
	//! saves perlin in an image for debug purposes
	// unused
	protected void PerlinDebug()
	{
		string filePath = "d:\\test.dds";
		array<int> data = {};

		const int WIDTH = 1024;
		const int HEIGHT = 1024;

		for (int y = 0; y < HEIGHT; y++)
		{
			for (int x = 0; x < WIDTH; x++)
			{
				int count = x * y;

				float perlinVal = Math.PerlinNoise(x / m_fPerlinFrequency, y / m_fPerlinFrequency);
				//Print(perlinVal);

				int pixel = ARGB(255, perlinVal * 255, perlinVal * 255, perlinVal * 255);
				data.Insert(pixel);
			}
		}

		// save dds to file
		if (!TexTools.SaveImageData(filePath, WIDTH, HEIGHT, data))
		{
			//Print("Can't save image", LogLevel.ERROR);
			return;
		}
	}

#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void PrefabGeneratorEntity(IEntitySource src, IEntity parent)
	{
	}
}
