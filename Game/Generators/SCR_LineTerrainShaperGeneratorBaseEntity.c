// [EntityEditorProps(category: "GameLib/Scripted/Generator", description: "Line Terrain Shaper", dynamicBox: true, visible: false)] // not yet standalone
class SCR_LineTerrainShaperGeneratorBaseEntityClass : SCR_GeneratorBaseEntityClass
{
}

class SCR_LineTerrainShaperGeneratorBaseEntity : SCR_GeneratorBaseEntity
{
	/*
		Terrain Sculpting
	*/

	[Attribute(defvalue: "0", desc: "Adjust terrain's Y around the track to fit the spline's location and clearance", category: "Terrain Sculpting")]
	protected bool m_bSculptTerrain;

	[Attribute(defvalue: "0", desc: "Priority of terrain sculpting", category: "Terrain Sculpting")]
	protected int m_iTerrainSculptingPriority;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.EditBox, desc: "Width of the path before fall-off, the flat surface around the shape", category: "Terrain Sculpting", params: "0 100 0.1")]
	protected float m_fTerrainSculptingPathWidth;

	[Attribute(defvalue: "20", uiwidget: UIWidgets.EditBox, desc: "Width of the path fall-off", category: "Terrain Sculpting", params: "0 100 0.1")]
	protected float m_fTerrainSculptingFallOffWidth;

#ifdef WORKBENCH

	//! Read by WorldEditorAPI.AddTerrainFlatterEntity, CANNOT be renamed
	//! defined at the last moment in UpdateTerrainSimple from m_ParentShapeSource
	protected ShapeEntity m_ShapeEntity;

	protected static ref array<string> s_aTerrainUpdateKeys = {
		"m_bSculptTerrain", "m_iTerrainSculptingPriority",
		"m_fTerrainSculptingPathWidth", "m_fTerrainSculptingFallOffWidth",
	};

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		UpdateTerrainSimple();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeTransformInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeTransformInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		UpdateTerrain(shapeEntity, false, mins, maxes);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		UpdateTerrain(shapeEntity, false, mins, maxes);
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnInit(inout vector mat[4], IEntitySource src)
	{
		super._WB_OnInit(mat, src);

		UpdateTerrainSimple();
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI.AreGeneratorEventsEnabled())
			return false;

		BaseContainerTools.WriteToInstance(this, src);

		if (s_aTerrainUpdateKeys.Contains(key))
		{
			bool forceUpdate;
			if (key == "m_bSculptTerrain")
			{
				if (m_bSculptTerrain)
					forceUpdate = true;
				else
					worldEditorAPI.RemoveTerrainFlatterEntity(this, true);
			}

			UpdateTerrainSimple(forceUpdate);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntitySource src)
	{
		super._WB_OnCreate(src);

		UpdateTerrainSimple();
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnParentChange(IEntitySource src, IEntitySource prevParentSrc)
	{
		super._WB_OnParentChange(src, prevParentSrc);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		if (m_ParentShapeSource)
			UpdateTerrainSimple(true);
		else
			worldEditorAPI.RemoveTerrainFlatterEntity(this, true);
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnDelete(IEntitySource src)
	{
		super._WB_OnDelete(src);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		worldEditorAPI.RemoveTerrainFlatterEntity(this, true);
	}

	//------------------------------------------------------------------------------------------------
	// wrapper
	protected void UpdateTerrainSimple(bool forceUpdate = false)
	{
		if (!m_bSculptTerrain || !m_ParentShapeSource)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		m_ShapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
		if (!m_ShapeEntity)
			return;

		array<vector> updateMins = {};
		array<vector> updateMaxes = {};
		m_ShapeEntity.GetAllInfluenceBBoxes(m_ParentShapeSource, updateMins, updateMaxes);

		UpdateTerrain(m_ShapeEntity, forceUpdate, updateMins, updateMaxes);
	}

	//------------------------------------------------------------------------------------------------
	// where everything happens
	protected void UpdateTerrain(notnull ShapeEntity shapeEntity, bool forceUpdate, notnull array<vector> updateMins, notnull array<vector> updateMaxes)
	{
		if (!m_bSculptTerrain)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		vector mat[4];
		shapeEntity.GetWorldTransform(mat);

		array<vector> points = {};
		shapeEntity.GenerateTesselatedShape(points);

		if (points.Count() < 2)
		{
			Print("Terrain Flattener requires a shape with at least two points", LogLevel.WARNING);
			return;
		}

		vector firstPoint = points[0].Multiply4(mat);
		vector mins = firstPoint;
		vector maxs = firstPoint;

		vector pos;
		foreach (vector point : points)
		{
			pos = point.Multiply4(mat);
			for (int i = 0; i < 3; i++)
			{
				float val = pos[i];

				if (val < mins[i])
					mins[i] = val;

				if (val > maxs[i])
					maxs[i] = val;
			}
		}

		worldEditorAPI.AddTerrainFlatterEntity(this, mins, maxs, m_iTerrainSculptingPriority, m_fTerrainSculptingPathWidth * 0.5, m_fTerrainSculptingFallOffWidth, forceUpdate, updateMins, updateMaxes);
	}

#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_TerrainFlattenerGeneratorBaseEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		if (!_WB_GetEditorAPI())
			return;

		m_ShapeEntity = ShapeEntity.Cast(parent);
#endif // WORKBENCH
	}
}
