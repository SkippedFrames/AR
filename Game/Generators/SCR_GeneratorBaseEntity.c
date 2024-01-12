class SCR_GeneratorBaseEntityClass : GeneratorBaseEntityClass
{
	[Attribute()]
	ref Color m_Color;
}

//! SCR_GeneratorBaseEntity responsibilities:
//! - trigger a warning if the generator is not the child of a shape
//! - keep the generator at shape's {0,0,0}, at angles {0,0,0}, at scale 1
//! - (future) delete all child entities if no parent is set
//! - (future) set generator's and shape's "Editor Only" flag
class SCR_GeneratorBaseEntity : GeneratorBaseEntity
{

#ifdef WORKBENCH

	protected IEntitySource m_Source;
	protected IEntitySource m_ParentShapeSource;

	protected bool m_bIsChangingWorkbenchKey;

	protected static const ref Color BASE_GENERATOR_COLOR = Color.White;
	protected static const ref array<string> ACCEPTED_PARENT_CLASSNAMES = { "SplineShapeEntity", "PolylineShapeEntity" }; // hardcoded, ok for now

	//------------------------------------------------------------------------------------------------
	override void _WB_OnParentChange(IEntitySource src, IEntitySource prevParentSrc)
	{
		super._WB_OnParentChange(src, prevParentSrc);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		m_ParentShapeSource = src.GetParent();
		if (!m_ParentShapeSource || !ACCEPTED_PARENT_CLASSNAMES.Contains(m_ParentShapeSource.GetClassName()))
			DeleteAllChildren();
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (m_bIsChangingWorkbenchKey)
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return true;

		bool sameParentChange = parent && worldEditorAPI.EntityToSource(parent) == m_ParentShapeSource;

		// keep the scale at 1
		if (key == "scale")
		{
			if (sameParentChange)
				Print("Do not modify a generator entity itself! Change its parent shape instead", LogLevel.WARNING);

			m_bIsChangingWorkbenchKey = true;
			worldEditorAPI.SetVariableValue(src, null, "scale", "1");
			m_bIsChangingWorkbenchKey = false;
		}

		// keep the angles at 0
		array<string> angles = { "angleX", "angleY", "angleZ" };
		foreach (string angle : angles)
		{
			if (key == angle)
			{
				if (sameParentChange)
					Print("Do not modify a generator entity itself! Change its parent shape instead", LogLevel.WARNING);

				m_bIsChangingWorkbenchKey = true;
				worldEditorAPI.SetVariableValue(src, null, angle, "0");
				m_bIsChangingWorkbenchKey = false;
			}
		}

		if (!parent)		// if no parent, do not set to 0 0 0
			return true;	// do not warn about no parent here as the constructor does it

		// keep the generator at (relative) 0 0 0 as long as it has a parent
		if (key == "coords")
		{
			vector coords;
			src.Get("coords", coords);
			if (coords != vector.Zero)	// because this can trigger when changing parent shape's points
			{
				if (sameParentChange)
					Print("Do not modify a generator entity itself! Change its parent shape instead", LogLevel.WARNING);

				m_bIsChangingWorkbenchKey = true;
				worldEditorAPI.SetVariableValue(src, null, "coords", "0 0 0");
				m_bIsChangingWorkbenchKey = false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_CanSelect(IEntitySource src)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);
		m_ParentShapeSource = shapeEntitySrc;
		ResetGeneratorPosition(shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);
		m_ParentShapeSource = shapeEntitySrc;
		ResetGeneratorPosition(shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetGeneratorPosition(ShapeEntity shapeEntity = null)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		bool manageEditAction = !worldEditorAPI.IsDoingEditAction();
		if (manageEditAction)
			worldEditorAPI.BeginEntityAction();

		worldEditorAPI.SetVariableValue(m_Source, null, "coords", "0 0 0");

		if (manageEditAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	//! Delete all children without distinction, using WorldEditorAPI
	protected void DeleteAllChildren()
	{
		int count = m_Source.GetNumChildren();
		if (count < 1)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		array<IEntity> entities = {};
		for (int i = m_Source.GetNumChildren() - 1; i >= 0; --i)
		{
			entities.Insert(worldEditorAPI.SourceToEntity(m_Source.GetChild(i)));
		}
		worldEditorAPI.DeleteEntities(entities);
	}

	//------------------------------------------------------------------------------------------------
	//! \return 3D anchor points relative to the provided shape source
	protected static array<vector> GetPoints(notnull IEntitySource shapeEntitySrc)
	{
		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (!points)
			return {};

		array<vector> result = {};
		vector pos;
		for (int i, count = points.Count(); i < count; ++i)
		{
			points.Get(i).Get("Position", pos);
			result.Insert(pos);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return 3D points relative to the provided shape source or null if WorldEditorAPI is not available / source is not a shape
	protected array<vector> GetTesselatedShapePoints(notnull IEntitySource shapeEntitySrc)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return null;

		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeEntitySrc));
		if (!shapeEntity)
			return null;

		array<vector> result = {};
		shapeEntity.GenerateTesselatedShape(result);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected Color GetColor()
	{
		SCR_GeneratorBaseEntityClass prefabData = SCR_GeneratorBaseEntityClass.Cast(GetPrefabData());
		if (!prefabData)
			return BASE_GENERATOR_COLOR;

		return prefabData.m_Color;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntitySource src)
	{
		super._WB_OnCreate(src);

		ColorShape();

		// when generator entity gets created by any edit activity (given by _WB_OnCreate event) then re-generate its generated content
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		// re-generate content only if it's created with a parent
		IEntitySource shapeEntitySrc = src.GetParent();
		if (shapeEntitySrc)
		{
			ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeEntitySrc));
			if (shapeEntity)
				OnShapeInit(shapeEntitySrc, shapeEntity);
		}

		EntityFlags flags;
		src.Get("Flags", flags);
		bool manageEditAction = !worldEditorAPI.IsDoingEditAction();
		if (manageEditAction)
			worldEditorAPI.BeginEntityAction();

		worldEditorAPI.SetVariableValue(src, null, "Flags", (flags | EntityFlags.EDITOR_ONLY).ToString());

		if (manageEditAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	protected void ColorShape()
	{
		if (!m_Source)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		IEntitySource parentSource = m_Source.GetParent();
		if (!parentSource)
			return;

		array<ref ContainerIdPathEntry> containerPath = {};

		Color color = GetColor();
		string colorString = string.Format("%1 %2 %3 %4", color.R(), color.G(), color.B(), color.A());

		bool manageEditAction = !worldEditorAPI.IsDoingEditAction();
		if (manageEditAction)
			worldEditorAPI.BeginEntityAction();

		worldEditorAPI.SetVariableValue(parentSource, containerPath, "LineColor", colorString);

		if (manageEditAction)
			worldEditorAPI.EndEntityAction();
	}

#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	void SCR_GeneratorBaseEntity(IEntitySource src, IEntity parent)
	{

#ifdef WORKBENCH

		if (!_WB_GetEditorAPI()) // thumbnail generation
			return;

		m_Source = src;

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (worldEditor && !worldEditor.IsPrefabEditMode() && !ShapeEntity.Cast(parent))
		{
			Print("A Generator is not a direct child of a shape at " + GetOrigin(), LogLevel.WARNING);
			return;
		}

		m_ParentShapeSource = m_Source.GetParent();

#endif // WORKBENCH

	}
}
