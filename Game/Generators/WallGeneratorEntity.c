//------------------------------------------------------------------------------------------------
class WallGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab", "et")]
	protected ResourceName MeshAtPoint;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	protected float PrePadding;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	protected float PostPadding;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	protected float m_fOffsetUp;

	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_bAlignWithNext;

	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_bAllowClipping;

	[Attribute("1", UIWidgets.CheckBox)]
	protected bool m_bGenerate;
};

//------------------------------------------------------------------------------------------------
//! Properties exposed in the wall generator property grid after adding a new wall length group
[BaseContainerProps()]
class WallWeightPair
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab", "et")]
	ResourceName m_sWallAsset;

	[Attribute("1", UIWidgets.EditBox)]
	float m_fWeight;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	float m_fPrePadding;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	float m_fPostPadding;
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class WallLengthGroup
{
	[Attribute("", UIWidgets.Object, "Prefab", "et")]
	ref array<ref WallWeightPair> m_aWallPrefabs;
};

//------------------------------------------------------------------------------------------------
//! Data container that holds the individudal wall asset related information
class WallGroupContainer
{
	bool m_bGenerated;
	float m_fMiddleObjectLength;
	float m_fSmallestWall = float.MAX;

	// for quick access, wallgroup lengths are in a seperate array (indices corresponding with those in wallgroup array)
	protected ref array<float> m_aLengths = {};

	protected ref array<ref WallGroup> m_aWallGroups = {};
	protected WallGeneratorEntity m_WallGenerator;

#ifdef WORKBENCH
	protected WorldEditorAPI m_Api;

	//------------------------------------------------------------------------------------------------
	void WallGroupContainer(WorldEditorAPI api, array<ref WallLengthGroup> items, bool forward, string middleObj, WallGeneratorEntity ent)
	{
		m_Api = api;
		m_WallGenerator = ent;
		PrepareWallGroups(items,forward, middleObj, api);
	}

	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return m_aWallGroups.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	WallPair GetRandomWall(float biggestSmallerThan = -1)
	{
		int index = -1;
		for (int i = m_aLengths.Count() - 1; i >= 0; i--) // find the longest wall which is smaller than given amount
		{
			if (m_aLengths.Get(i) < biggestSmallerThan)
			{
				index = i;
				break;
			}
		}

		if (index != -1)
			return m_aWallGroups.Get(index).GetRandomWall();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Goes through walls and prepares a data structure that's used during wall generation
	void PrepareWallGroups(array<ref WallLengthGroup> groups, bool forward, string middleObj, WorldEditorAPI api)
	{
		m_aWallGroups.Clear();

		array<ref WallGroup> wallGroupsTmp = {};

		int forwardAxis = 2;

		if (forward)
			forwardAxis = 0;

		if (!middleObj.IsEmpty())
			m_fMiddleObjectLength = WallGeneratorEntity.MeasureEntity(middleObj, forwardAxis, api);

		WallGroup wallGroup;
		WallPair wallPair;
		float wallLength;
		foreach (WallLengthGroup group : groups)
		{
			wallGroup = new WallGroup();

			foreach (WallWeightPair pair : group.m_aWallPrefabs)
			{
				wallPair = new WallPair();

				wallPair.m_sWallAsset = pair.m_sWallAsset;
				wallPair.m_fPostPadding = pair.m_fPostPadding;
				wallPair.m_fPrePadding = pair.m_fPrePadding;
				wallGroup.m_aWeights.Insert(pair.m_fWeight);

				wallLength = WallGeneratorEntity.MeasureEntity(wallPair.m_sWallAsset, forwardAxis, api);
				// wallLength += wallPair.m_fPostPadding;
				wallPair.m_fWallLength = wallLength;

				if (wallLength > wallGroup.m_fWallLength)
					wallGroup.m_fWallLength = wallLength; // biggest length is the one being used for the whole group

				if (!wallPair.m_sWallAsset.IsEmpty())
					wallGroup.m_aWallPairs.Insert(wallPair);
			}

			if (wallGroup.m_aWallPairs.Count() != 0)
			{
				wallGroupsTmp.Insert(wallGroup);
				m_bGenerated = true;
			}
		}

		float smallestSize;
		int smallestIndex;

		// order groups by length
		while (wallGroupsTmp.Count() != 0)
		{
			smallestSize = float.MAX;
			smallestIndex = 0;
			foreach (int i, WallGroup g : wallGroupsTmp)
			{
				if (g.m_fWallLength < smallestSize)
				{
					smallestSize = g.m_fWallLength;
					if (smallestSize < m_fSmallestWall)
						m_fSmallestWall = smallestSize;
					smallestIndex = i;
				}
			}

			m_aWallGroups.Insert(wallGroupsTmp.Get(smallestIndex));
			m_aLengths.Insert(wallGroupsTmp.Get(smallestIndex).m_fWallLength);
			wallGroupsTmp.Remove(smallestIndex);
		}

		if (m_WallGenerator && m_WallGenerator.m_bDebug)
		{
			foreach (WallGroup group : m_aWallGroups)
			{
				Print(group.m_fWallLength);
				foreach (WallPair pair : group.m_aWallPairs)
				{
					Print(pair.m_sWallAsset);
					Print(pair.m_fWallWeight);
				}
			}
		}
	}
#endif
};

//------------------------------------------------------------------------------------------------
//! Wall pair data structure of wall asset + wall weight(extended with other params)
class WallPair
{
	string m_sWallAsset;
	float m_fWallWeight;
	float m_fWallLength;
	float m_fPostPadding;
	float m_fPrePadding;
};

//------------------------------------------------------------------------------------------------
//! Collection of wall pair data objects into a single wall group.
//! Wall group is meant to group walls of the same length. It's possible to add shorter wall(s) to a wall group as well(a feature required by map designers), but the wall group as a whole is considered to have only a single length(that of the longest wall in the wallgroup) and when considering which wall group is selected during generation,
//! it's the wall group length as a whole that's being considered, not individual walls within the group. When placing the walls during wall generation, after it's already been decided which wall group will satisfy the length requirenment,
//! and after a particular wall pair was selected based on the weights, the actual prefab is placed in the world and the exact length of this prefab is then deducted from the remaining segment(so no gap is created by possibly using a shorter variance).
class WallGroup
{
	ref array<ref WallPair> m_aWallPairs 	= {};
	ref array<float> 		m_aWeights 		= {};

	float m_fWallLength;

	//------------------------------------------------------------------------------------------------
	WallPair GetRandomWall()
	{
		int index = SCR_ArrayHelper.GetWeightedIndex(m_aWeights, Math.RandomFloat01());
		return m_aWallPairs.Get(index);
	}
};

//------------------------------------------------------------------------------------------------
// wall generator point metadata that's being operated on with the generator after the data gets extracted from raw polyline/spline data
class WallGeneratorPoint
{
	vector Pos;
	ResourceName CustomMesh;
	float m_fPrePadding;
	float m_fPostPadding;
	bool m_bGenerate;
	bool m_bAlignNext;
	bool m_bClip;
	float m_fOffsetUp;
};

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "WallGeneratorEntity", dynamicBox: true, visible: false)]
class WallGeneratorEntityClass : SCR_GeneratorBaseEntityClass
{
};

//------------------------------------------------------------------------------------------------
class WallGeneratorEntity : SCR_GeneratorBaseEntity
{
	//--- Category: Middle Object
	[Attribute("", UIWidgets.ResourceNamePicker, "Middle Object Prefab", "et", category: "Middle Object")]
	protected ResourceName MiddleObject;

	[Attribute(defvalue: "1", desc: "Place middle prefab at vertex", category: "Middle Object")]
	protected bool PlaceMiddleAtVertex;

	[Attribute(defvalue: "0", desc: "Place middle prefab at vertex only", category: "Middle Object")]
	protected bool PlaceMiddleAtVertexOnly;

	[Attribute("0", UIWidgets.Slider, "Middle object pre-padding, essentially a gap between previous prefab and this", params: "-5 5 0.01", category: "Middle Object")]
	protected float MiddleObjectPrePadding;

	[Attribute("0", UIWidgets.Slider,"Middle object offset to the side", params: "-5 5 0.01", category: "Middle Object")]
	protected float MiddleObjectOffsetRight;

	[Attribute("0", UIWidgets.Slider, "Middle object post-padding, essentially a gap between this asset and the next one", params: "-5 5 0.01", category: "Middle Object")]
	protected float MiddleObjectPostPadding;

	[Attribute("0", UIWidgets.Slider, "Middle object offset up/down", params: "-10 10 0.01", category: "Middle Object")]
	protected float MiddleObjectOffsetUp;

	//--- Category: First Object

	[Attribute("", UIWidgets.ResourceNamePicker, "First Object Prefab", "et", category: "First Object")]
	protected ResourceName FirstObject;

	[Attribute("0", UIWidgets.Slider, "First object pre-padding, essentially a gap between previous vertex first object",params: "-5 5 0.01", category: "First Object")]
	protected float FirstObjectPrePadding;

	[Attribute("0", UIWidgets.Slider, "First object post-padding, essentially a gap between first object and the following one", params: "-5 5 0.01", category: "First Object")]
	protected float FirstObjectPostPadding;

	[Attribute("0", UIWidgets.Slider, "First object offset to the side", params: "-5 5 0.01", category: "First Object")]
	protected float FirstObjectOffsetRight;

	[Attribute("0", UIWidgets.Slider, "First object offset up/down", params: "-10 10 0.01", category: "First Object")]
	protected float FirstObjectOffsetUp;

	//--- Category: Last Object

	[Attribute("", UIWidgets.ResourceNamePicker, "Last Object Prefab", "et", category: "Last Object")]
	protected ResourceName LastObject;

	[Attribute("0", UIWidgets.Slider, "Last object pre-padding, essentially a gap between previous vertex first object",params: "-5 5 0.01", category: "Last Object")]
	protected float LastObjectPrePadding;

	[Attribute("0", UIWidgets.Slider, "Last object post-padding, essentially a gap between first object and the following one", params: "-5 5 0.01", category: "Last Object")]
	protected float LastObjectPostPadding;

	[Attribute("0", UIWidgets.Slider,"Last object offset to the side", params: "-5 5 0.01", category: "Last Object")]
	protected float LastObjectOffsetRight;

	[Attribute("0", UIWidgets.Slider, "Last object offset up/down", params: "-10 10 0.01", category: "Last Object")]
	protected float LastObjectOffsetUp;

	//--- Category: Global, Other and uncategorised

	[Attribute("", UIWidgets.None, "Prefab", "et")] // legacy
	protected ref array<ref ResourceName> WallPrefabs;

	[Attribute("0", UIWidgets.Slider, "Global object pre-padding, essentially a gap between previous prefab and this", params: "-2 2 0.01", category: "Global")]
	protected float PrePadding;

	[Attribute(defvalue: "1", desc: "Allow pre-padding on first wall asset in each line segment", category: "Other")]
	protected bool PrePadFirst;

	[Attribute("0", UIWidgets.Slider, "Allow pre-padding on first wall asset in each line segment",params: "-2 2 0.01", category: "Global")]
	protected float PostPadding;

	[Attribute(defvalue: "0", desc: "Copy the polyline precisely while sacrificing wall assets contact", category: "Other")]
	protected bool ExactPlacement;

	[Attribute(defvalue: "0", desc: "Start the wall from the other end of the polyline", category: "Other")]
	protected bool m_bStartFromTheEnd;

	[Attribute(defvalue: "1", desc: "Rotate object so that its X axis is facing in the direction of the polyline segment", category: "Other")]
	protected bool UseXAsForward;

	[Attribute(defvalue: "0", desc: "Rotate object 180° around the Yaw axis", category: "Other")]
	protected bool Rotate180;

	[Attribute("0.5", UIWidgets.Slider, "Allow overshooting the segment line by this amount when placing assets", params: "-5 5 0.01", category: "Global")]
	protected float m_fOvershoot;

	[Attribute(defvalue: "0", desc: "If you want to generate objects smaller than 10 centimetres", category: "Other")]
	protected bool UseForVerySmallObjects;

	[Attribute("0", UIWidgets.Slider, "Objects offset to the side", params: "-5 5 0.01", category: "Global")]
	protected float m_fOffsetRight;

	[Attribute("0", UIWidgets.Slider, "Object offset up/down", params: "-5 5 0.01", category: "Global")]
	protected float m_fOffsetUp;

	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, "Contains wall groups which group Wall/Weight pairs by length", category: "Global")]
	protected ref array<ref WallLengthGroup> m_aWallGroups;

	[Attribute(defvalue: "0", desc: "Draw developer debug", category: "Other")]
	bool m_bDebug;

	[Attribute(defvalue: "0", desc: "Whether or not walls should be snapped to the terrain", category: "Other")]
	protected bool m_bSnapToTerrain;

	protected float m_fFirstObjectLength; // unused?
	protected float m_fLastObjectLength;

	protected IEntitySource m_ParentSource;

	protected ref WallGroupContainer wallGroupContainer;

	protected ref array<ref WallGeneratorPoint> m_Points = {};
	protected static ref array<ref Shape> m_DebugShapes = {};

	//------------------------------------------------------------------------------------------------
	void WallGeneratorEntity(IEntitySource src, IEntity parent)
	{
		#ifdef WORKBENCH
		SetEventMask(EntityEvent.INIT);
		#endif
	}

	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! \return entity side on provided axis, 1 on close-to-zero (< 0.00001m) measurement, float.MAX on failure
	static float MeasureEntity(ResourceName entityName, int measureAxis, WorldEditorAPI api)
	{
		float result = float.MAX;

		if (entityName.IsEmpty())
			return result;

		GenericEntity wallEntity = null;

		Resource resource = Resource.Load(entityName);
		if (resource.IsValid())
			wallEntity = GenericEntity.Cast(GetGame().SpawnEntityPrefab(resource, api.GetWorld()));

		if (!wallEntity)
			return result;

		vector minBB;
		vector maxBB;
		wallEntity.GetBounds(minBB, maxBB);
		delete wallEntity;

		result = (maxBB - minBB)[measureAxis];
		if (result < 0.000001)
		{
			Print("Wall asset " + entityName + " is too small, does it have a valid mesh?", LogLevel.ERROR);
			return 1; // generating is invalid, just avoid infinite loop
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (key == "coords")
			return false;

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api == null || api.UndoOrRedoIsRestoring())
			return false;

		IEntitySource entSrc = src.ToEntitySource();

		IEntitySource parentSrc;

		IEntitySource thisSrc = api.EntityToSource(this);
		parentSrc = thisSrc.GetParent();
		BaseContainerTools.WriteToInstance(this, thisSrc);
		OnShapeChanged(parentSrc, ShapeEntity.Cast(parent), {}, {});
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Offsets the points in the 'points' array, used with m_fOffsetRight
	void OffsetPoints(array<vector> points, float offset, bool debugAllowed = false)
	{
		array<vector> pointsTemp = {};
		pointsTemp.Copy(points);
		points.Clear();

		vector forwardPrev = "1 1 1";
		for (int i = 0, cnt = pointsTemp.Count(); i < cnt; i++)
		{
			vector forwardNext;

			if (i < cnt-1)
				forwardNext = pointsTemp.Get(i+1) - pointsTemp.Get(i);
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
			if (dist != 0)
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

			points.Insert(pointsTemp[i] + diagonal);
			forwardPrev = -forwardNext;

			// debug
			if (m_bDebug && debugAllowed)
			{
				vector matWrld[4];
				GetWorldTransform(matWrld);

				m_DebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER, pointsTemp.Get(i).Multiply4(matWrld), pointsTemp.Get(i).Multiply4(matWrld) + forwardNext));
				m_DebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, pointsTemp.Get(i).Multiply4(matWrld), pointsTemp.Get(i).Multiply4(matWrld) + forwardPrev));
				m_DebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, pointsTemp.Get(i).Multiply4(matWrld), pointsTemp.Get(i).Multiply4(matWrld) + diagonal));
			}
		}
	};

	//------------------------------------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		// already in an ifdef WORKBENCH
		// #ifdef WORKBENCH
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		// TODO: auto-trigger generation here as well
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api || api.UndoOrRedoIsRestoring())
			return;

		Preprocess(shapeEntitySrc);
		Generate(shapeEntity);
		// #endif
	}

	//------------------------------------------------------------------------------------------------
	protected void Preprocess(IEntitySource shapeEntitySrc)
	{
		m_Points.Clear();
		WorldEditorAPI api = _WB_GetEditorAPI();

		wallGroupContainer = new WallGroupContainer(api, m_aWallGroups, UseXAsForward, MiddleObject, this);
		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (points != null && points.Count() >= 2 && wallGroupContainer.m_bGenerated != 0)
		{
			bool isShapeClosed = false;
			shapeEntitySrc.Get("IsClosed", isShapeClosed);
			array<vector> pointsVec = {};
			int pointCount = points.Count();
			bool addFirstAsLast = false;

			if (isShapeClosed && pointCount > 2) // no reason to "loop" for 2 points
				addFirstAsLast = true;

			// offset start
			BaseContainer point;
			vector pos;
			for (int i = 0; i < pointCount; i++)
			{
				point = points.Get(i);
				point.Get("Position", pos);
				pointsVec.Insert(pos);
			}

			if (addFirstAsLast)
				pointsVec.Insert(pointsVec[0]);

			if (m_fOffsetRight != 0)
				OffsetPoints(pointsVec,m_fOffsetRight, true);
			// offset end

			ResourceName customMesh;
			float prePadding, postPadding, offsetUp;
			bool generate, clipping, align;
			BaseContainerList dataArr;
			BaseContainer data;
			int dataCount, lastPointIndex;
			WallGeneratorPoint genPoint;

			for (int i = 0; i < pointCount; i++)
			{
				point = points.Get(i);
				pos = pointsVec[i];
				customMesh = string.Empty;
				prePadding = 0;
				postPadding = 0;
				generate = true;
				clipping = false;
				offsetUp = 0;
				align = false;
				dataArr = point.GetObjectArray("Data");
				dataCount = dataArr.Count();

				for (int j = 0; j < dataCount; ++j)
				{
					data = dataArr.Get(j);
					if (data.GetClassName() == "WallGeneratorPointData")
					{
						data.Get("MeshAtPoint", customMesh);
						data.Get("PrePadding", prePadding);
						data.Get("PostPadding", postPadding);
						data.Get("m_bGenerate", generate);
						data.Get("m_bAllowClipping", clipping);
						data.Get("m_fOffsetUp", offsetUp);
						data.Get("m_bAlignWithNext", align);
						break;
					}
				}

				genPoint = new WallGeneratorPoint();
				genPoint.Pos = pos;
				genPoint.CustomMesh = customMesh;
				genPoint.m_fPrePadding = prePadding;
				genPoint.m_fPostPadding = postPadding;
				genPoint.m_bGenerate = generate;
				genPoint.m_bClip = clipping;
				genPoint.m_fOffsetUp = offsetUp;
				genPoint.m_bAlignNext = align;
				m_Points.Insert(genPoint);
			}

			if (addFirstAsLast)
			{
				m_Points.Insert(m_Points[0]);
				lastPointIndex = m_Points.Count() - 1;
				m_Points.Get(lastPointIndex).Pos = pointsVec[lastPointIndex];
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		if (!shapeEntitySrc)
			return;

		Preprocess(shapeEntitySrc);
		Generate(shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	IEntity PlacePrefab(
		bool generate,
		ResourceName name,
		out vector pos,
		vector dir,
		vector prevDir,
		float rotationAdjustment,
		bool isGeneratorVisible,
		float length,
		float prePadding,
		float postPadding,
		float offsetUp,
		bool alignNext,
		bool prepadNext,
		bool snapToGround = false,
		bool allowClipping = false,
		vector offsetRight = "0 0 0")
	{
		if (name.IsEmpty())
			return null;

		vector prepadDirection;
		vector orientation;

		if (alignNext)
			orientation = dir;
		else
			orientation = prevDir;

		if (prepadNext)
			prepadDirection = dir;
		else
			prepadDirection = prevDir;

		pos += prepadDirection * prePadding;
		IEntity ent;
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource thisSource = api.EntityToSource(this);
		int layerID = api.GetCurrentEntityLayerId();

		if (generate)
		{
			vector rotMat[4];
			Math3D.DirectionAndUpMatrix(orientation, "0 1 0", rotMat);
			vector rot = Math3D.MatrixToAngles(rotMat);
			rot[1] = rot[0] + rotationAdjustment;
			rot[0] = 0;
			rot[2] = 0;

			if (m_bSnapToTerrain)
			{
				vector world = CoordToParent(pos);
				pos[1] = api.GetTerrainSurfaceY(world[0], world[2]);
				if (m_ParentSource)
					pos[1] = pos[1] - api.SourceToEntity(m_ParentSource).GetOrigin()[1];
			}

			if (snapToGround)
			{
				vector matWrld[4];
				GetWorldTransform(matWrld);

				ent = api.CreateEntityExt(name, "", layerID, thisSource, (pos + offsetRight), rot, TraceFlags.WORLD);
				api.ParentEntity(this, ent, true);
			}
			else
			{
				ent = api.CreateEntity(name, "", layerID, thisSource, pos + offsetRight, rot);
			}

			if (offsetUp != 0)
			{
				vector entPos;
				api.EntityToSource(ent).Get("coords", entPos);
				entPos[1] = entPos[1] + offsetUp;
				string coords = entPos[0].ToString() + " " + entPos[1].ToString() + " " + entPos[2].ToString();
				api.ModifyEntityKey(ent, "coords", coords);
			}

			api.SetEntityVisible(ent, isGeneratorVisible, false);
		}

		if (allowClipping)
			length = 0;

		if (UseForVerySmallObjects)
			pos += dir * (length + postPadding);
		else
			// if objects are big, advancement lower than 0.1 (10cm) is probably a bug
			// and we may end up freezing by generating outrageous amout of entities
			pos += dir * Math.Max(0.1, length + postPadding);

		return ent;
	}

	//------------------------------------------------------------------------------------------------
	void Generate(ShapeEntity shapeEntity)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api == null || api.UndoOrRedoIsRestoring() || wallGroupContainer.IsEmpty())
			return;

		IEntitySource entSrc = api.EntityToSource(this);
		m_ParentSource = entSrc.GetParent();
		int childCount = entSrc.GetNumChildren();

		IEntitySource childSrc;
		IEntity child;
		for (int i = childCount - 1; i >= 0; --i)
		{
			childSrc = entSrc.GetChild(i);
			child = api.SourceToEntity(childSrc);
			api.DeleteEntity(child);
		}

		if (m_Points.Count() < 2)
			return;

		int forwardAxis = 2;
		if (UseXAsForward)
			forwardAxis = 0;

		m_fFirstObjectLength = 0;
		m_fLastObjectLength = 0;

		if (FirstObject)
			m_fFirstObjectLength = MeasureEntity(FirstObject, forwardAxis, api);

		if (LastObject)
			m_fLastObjectLength = MeasureEntity(LastObject, forwardAxis, api);

		bool isGeneratorVisible = api.IsEntityVisible(this);

		// copy points so we have them after the entity is reinitialised
		array<ref WallGeneratorPoint> localPoints = m_Points;
		if (m_bStartFromTheEnd)
			InvertWallGeneratorPointArray(localPoints);

		float rotationAdjustment = 0;

		if (UseXAsForward)
			rotationAdjustment = -90;

		if (Rotate180)
			rotationAdjustment += 180;

		if (m_bStartFromTheEnd)
			rotationAdjustment += 180;

		vector from = localPoints[0].Pos;
		vector to, dir, prevDir, rightVec;

		bool firstUsed, exhausted, lastPoint, lastSegment, generate, firstPass;
		ResourceName customMesh;
		float remaining;

		// while-loop variables
		string bestWall;
		float bestLen, prePaddingToUse, postPaddingToUse, offsetUp, lengthRequirement;
		bool allowClipping, alignNext, prepadNext, custom;
		bool firstPlaced, placeMiddle, placeLast, lastPlaced, lastInSegmentDoNotPlace, middleOfSegmentDoNotPlace;
		vector offsetRight;
		WallPair wall;

		for (int i, count = localPoints.Count(); i < count; i++)
		{
			exhausted = false;
			lastPoint = i == count - 1;
			lastSegment = i == count - 2;

			prevDir = dir;

			if (ExactPlacement)
			{
				if (i == 1)
					from = localPoints[i].Pos;
				else
					from = localPoints[i].Pos + (dir * PrePadding);
			}

			if (lastPoint)
			{
				to = localPoints[i].Pos;
				dir = (localPoints[i].Pos - localPoints[i-1].Pos).Normalized();
			}
			else
			{
				to = localPoints[i+1].Pos;
				dir = (to - from).Normalized();
				if (i == 0)
					prevDir = dir;
			}

			customMesh = localPoints[i].CustomMesh;
			generate = localPoints[i].m_bGenerate;
			remaining = (to - from).Length() + m_fOvershoot;

			firstPass = true; // first segment on the polyline

			rightVec = dir * "0 1 0";

			// walking the polyline line segment and populating it with assets
			// as long as there is enough room for at least the smallest wall available
			while (!exhausted)
			{
				bestWall = string.Empty;
				bestLen = 0;
				prePaddingToUse = PrePadding;
				postPaddingToUse = PostPadding;
				allowClipping = false;
				alignNext = true;
				prepadNext = true;
				offsetUp = m_fOffsetUp;
				custom = false;
				offsetRight = vector.Zero;
				firstPlaced = false;
				placeMiddle = false;
				placeLast = false;
				lengthRequirement = 0;
				lastPlaced = false;
				lastInSegmentDoNotPlace = false;
				middleOfSegmentDoNotPlace = false;

				// first object
				if (i == 0 && firstPass && !FirstObject.IsEmpty())
				{
					bestWall = FirstObject;
					bestLen = MeasureEntity(FirstObject, forwardAxis, api);
					prePaddingToUse = FirstObjectPrePadding;
					postPaddingToUse = FirstObjectPostPadding;
					firstPlaced = true;
					offsetRight = rightVec * FirstObjectOffsetRight;
					offsetUp = FirstObjectOffsetUp;
				}
				// custom mesh from the vertex data
				else if (!customMesh.IsEmpty())
				{
					bestWall = customMesh;
					prePaddingToUse = localPoints[i].m_fPrePadding;
					postPaddingToUse = localPoints[i].m_fPostPadding;
					allowClipping = localPoints[i].m_bClip;
					offsetUp = localPoints[i].m_fOffsetUp;
					alignNext = localPoints[i].m_bAlignNext;
					prepadNext = false;
					bestLen = MeasureEntity(customMesh, forwardAxis, api);
					customMesh = string.Empty;
					custom = true;
				}
				else if (!lastPoint)
				{
					wall = wallGroupContainer.GetRandomWall(remaining);
					if (wall)
					{
						bestLen = wall.m_fWallLength;
						bestWall = wall.m_sWallAsset;
						prePaddingToUse += wall.m_fPrePadding;
						postPaddingToUse += wall.m_fPostPadding;
					}
				}
				else
				{
					// get here when the vertex has wall data with a mesh object and it's a last point on the polyline
					break;
				}

				if (bestLen + PrePadding <= 0)
				{
					prePaddingToUse = 0;
					Print("PrePadding is set too low, not using it in this instance", LogLevel.WARNING);
				}

				if (!PrePadFirst && firstPass) // do not prepad the first asset in the segment
					prePaddingToUse = 0;

				PlacePrefab(generate, bestWall, from, dir, prevDir, rotationAdjustment, isGeneratorVisible, bestLen, prePaddingToUse, postPaddingToUse, offsetUp, alignNext, prepadNext, false, allowClipping, offsetRight);

				remaining -= (bestLen * !allowClipping) + prePaddingToUse + postPaddingToUse; // TODO: fix bool multiplier

				placeMiddle = MiddleObject && !custom;
				placeLast = !LastObject.IsEmpty() && lastSegment;

				lengthRequirement = wallGroupContainer.m_fSmallestWall; // the minimal space required to place the next wall asset

				if (placeMiddle)
					lengthRequirement += wallGroupContainer.m_fMiddleObjectLength;

				if (placeLast)
					lengthRequirement += m_fLastObjectLength;

				if (remaining < lengthRequirement)
					exhausted = true;

				lastPlaced = false;

				// no more room for 'normal' assets + placing last object
				if (exhausted && placeLast)
				{
					bestWall = LastObject;
					bestLen = m_fLastObjectLength;
					prePaddingToUse = LastObjectPrePadding;
					postPaddingToUse = LastObjectPostPadding;
					offsetRight = rightVec * LastObjectOffsetRight;
					offsetUp = LastObjectOffsetUp;

					PlacePrefab(generate, bestWall, from, dir, prevDir, rotationAdjustment, isGeneratorVisible, bestLen, prePaddingToUse, postPaddingToUse, offsetUp, alignNext, prepadNext, false, allowClipping, offsetRight);
					lastPlaced = true;
					remaining -= (bestLen * !allowClipping) + prePaddingToUse + postPaddingToUse;
				}

				// placing middle object
				if (placeMiddle)
				{
					lastInSegmentDoNotPlace = exhausted && !(PlaceMiddleAtVertex || PlaceMiddleAtVertexOnly);

					// do not place middle object after first/last assets and any other assets which do not close a line segment
					middleOfSegmentDoNotPlace = (!exhausted && PlaceMiddleAtVertexOnly) || (PlaceMiddleAtVertexOnly && (lastPlaced || firstPlaced));

					if (!lastInSegmentDoNotPlace && !middleOfSegmentDoNotPlace)
					{
						from += dir * MiddleObjectPrePadding;
						offsetRight = rightVec * MiddleObjectOffsetRight;
						PlacePrefab(generate, MiddleObject, from, dir, prevDir, rotationAdjustment, isGeneratorVisible, wallGroupContainer.m_fMiddleObjectLength, MiddleObjectPrePadding, MiddleObjectPostPadding, MiddleObjectOffsetUp,true, prepadNext, true, false, offsetRight);
						remaining -= wallGroupContainer.m_fMiddleObjectLength + MiddleObjectPrePadding + MiddleObjectPostPadding;
					}
				}

				firstPass = false;
			}
		}

		// get the array back to normal
		if (m_bStartFromTheEnd)
			InvertWallGeneratorPointArray(localPoints);
	}

	protected void InvertWallGeneratorPointArray(notnull inout array<ref WallGeneratorPoint> items)
	{
		int itemsCount = items.Count();
		if (itemsCount < 2)
			return;

		int flooredMiddle = itemsCount / 2;
		itemsCount--;

		for (int i; i < flooredMiddle; i++)
		{
			items.SwapItems(i, itemsCount - i);
		}
	}
	#endif
};
