class AutoSpawnerToolCallbackContext
{
	protected array <ResourceName> m_aSelection;

	void AutoSpawnerToolCallbackContext(array <ResourceName> selection)
	{
		m_aSelection = selection;
	}

	void SearchResourcesCallback(ResourceName resName, string filePath)
	{
		m_aSelection.Insert(resName);
	}
};

[WorkbenchToolAttribute("Autospawner Tool", "Spawns selected prefabs/XOBs on a grid into an active layer.\n1. Select folders/files in Resource Browser\n2. Click on 'Scan Folders'\n3. Place objects into the scene", "", awesomeFontCode: 0xf468)]
class AutoSpawnerTool : WorldEditorTool
{
	[Attribute("1", UIWidgets.CheckBox, "Find *.et files in given folder")]
	private bool m_bFindEntities;

	[Attribute("1", UIWidgets.CheckBox, "Find *.xob files in given folder")]
	private bool m_bFindXOBs;

	[Attribute("1", UIWidgets.CheckBox)]
	private bool m_bShowPlacement;

	[Attribute("0", UIWidgets.CheckBox, "Attach a comment with entity path")]
	private bool m_bGenerateComment;
	
	[Attribute("0", UIWidgets.EditBox)]
	private float m_fCommentYOffset;

	[Attribute("10", UIWidgets.Slider, "Number of objects in a row.", "1 100 1")]
	private int m_iObjectsPerRow;

	[Attribute("1", UIWidgets.EditBox, "Spawn the selection this many times", "1 1000 1")]
	private int m_iSpawnMultiplier;

	[Attribute("1", UIWidgets.CheckBox, "Align by largest bounding box")]
	private bool m_bAlignByBoundingBox;

	[Attribute("0", UIWidgets.Slider, "Additional X axis offset", "0 150 1")]
	private float m_fXOffset;

	[Attribute("0", UIWidgets.Slider, "Additional Z axis offset", "0 150 1")]
	private float m_fZOffset;

	[Attribute("0", UIWidgets.Slider, "Y axis offset", "-50 50 1")]
	private float m_fYOffset;

	[Attribute("0", UIWidgets.Slider, "Cumulative object rotation.", "0 180 1")]
	private float m_fCumulativeRotation;

	[Attribute("", UIWidgets.FileNamePicker, "Select *.txt file with resource names to spawn")]
	private string m_PrefabList;

	private ref array <ResourceName> m_aSelection = {};
	private ref array <ResourceName> m_aSelectedEntities = {};
	private ref array <ResourceName> m_aSelectedXOBs = {};

	private ref array <IEntity> m_aSpawnedEntities = {};
	private ref array <IEntity> m_aPlacedEntityHistory = {};
	private ref array <int> m_aSpawnHistory = {};
	private ref array <int> m_aSpawnHistoryChunks = {};

	private int m_iEntityId;
	private int m_iEntityIdPrev;

	private ref Shape m_PlacementRect;
	private vector m_vPlacementRectSize = "1 0 1";

	private int m_iRowSizeX = 1;
	private int m_iRowSizeZ = 1;

	private float m_fLargestBBoxX;
	private float m_fLargestBBoxZ;

	WorldEditor we = Workbench.GetModule(WorldEditor);

	[ButtonAttribute("Spawn prefab list")]
	void SpawnFromPrefabList()
	{
		if (!FileIO.FileExists(m_PrefabList))
			return;

		FileHandle file = FileIO.OpenFile(m_PrefabList, FileMode.READ);
		if (!file)
			return;

		ClearSelection();

		string temp;
		while (file.ReadLine(temp) > -1)
		{
			m_aSelectedEntities.Insert(temp);
		}

		file.Close();

		Preload();
	}

	[ButtonAttribute("Undo")]
	private void Undo()
	{
		if (m_aSpawnHistoryChunks.IsEmpty())
			return;
		
		int chunkSize = m_aSpawnHistoryChunks[m_aSpawnHistoryChunks.Count()-1];
		int bottomIndex = m_aPlacedEntityHistory.Count() - chunkSize - 1;
		
		m_API.BeginEntityAction();
	

		for (int i = m_aPlacedEntityHistory.Count() - 1; i > bottomIndex; i--)
		{
			m_API.DeleteEntity(m_aPlacedEntityHistory[i]);
			m_aPlacedEntityHistory.Remove(i);
		}
				
		m_API.EndEntityAction();
		
		m_aSpawnHistoryChunks.Remove(m_aSpawnHistoryChunks.Count()-1);

		ClearSelection();
	}

	[ButtonAttribute("Scan Folders")]
	private void FolderScan()
	{
		m_fLargestBBoxX = 0;
		m_fLargestBBoxZ = 0;

		ClearSelection();
		Debug.BeginTimeMeasure();

		AutoSpawnerToolCallbackContext ctx = new AutoSpawnerToolCallbackContext(m_aSelection);
		we.GetResourceBrowserSelection(ctx.SearchResourcesCallback, true);

		m_aSpawnHistory.Insert(m_iEntityId);
		m_iEntityIdPrev = m_aSpawnHistory[m_aSpawnHistory.Count()-1];

		foreach (string s : m_aSelection)
		{
			if (m_bFindEntities)
			{
				FindFilesByExtension(s, ".et", m_aSelectedEntities);
			}

			if (m_bFindXOBs)
			{
				FindFilesByExtension(s, ".xob", m_aSelectedXOBs);
			}
		}

		Debug.EndTimeMeasure("Folder scan done");

		Print("Number of *.et files: " + m_aSelectedEntities.Count());
		Print("Number of *.xob files: " + m_aSelectedXOBs.Count());

		for (int i = 1; i <= m_iSpawnMultiplier; ++i)
		{
			Preload();
		}
	}

	private void Preload()
	{
		if (m_bFindEntities)
			LoadEntities();

		if (m_bFindXOBs)
			LoadXOBs();

		vector min, max;
		foreach (IEntity ent : m_aSpawnedEntities)
		{
			if (!ent)
				continue;
			
			ent.GetBounds(min, max);

			float cur_bbox_x = Math.AbsFloat(min[0] - max[0]);
			float cur_bbox_z = Math.AbsFloat(min[2] - max[2]);

			if (cur_bbox_x > m_fLargestBBoxX)
			{
				m_fLargestBBoxX = cur_bbox_x;
			}

			if (cur_bbox_z > m_fLargestBBoxZ)
			{
				m_fLargestBBoxZ = cur_bbox_z;
			}
		}

		m_iRowSizeX = m_iObjectsPerRow;
		m_iRowSizeZ = m_aSpawnedEntities.Count() / m_iRowSizeX + 1;
	}

	private void LoadXOBs()
	{
		m_API.BeginEntityAction();
		foreach (ResourceName xob_path : m_aSelectedXOBs)
		{
			IEntity ent = m_API.CreateEntity("GenericEntity", string.Empty, m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
			IEntitySource src = m_API.EntityToSource(ent);

			m_API.CreateComponent(src, "MeshObject");
			m_API.ModifyComponentKey(ent, "MeshObject", "Object", xob_path);
			GenerateComment(ent, xob_path);

			m_aSpawnedEntities.Insert(ent);
			m_iEntityId++;
		}
		m_API.EndEntityAction();
	}

	private void LoadEntities()
	{
		m_API.BeginEntityAction();
		foreach (ResourceName entity_path : m_aSelectedEntities)
		{
			IEntity ent = m_API.CreateEntity(entity_path, string.Empty, m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
			GenerateComment(ent, entity_path);

			m_aSpawnedEntities.Insert(ent);
			m_iEntityId++;
		}
		m_API.EndEntityAction();
	}

	private void GenerateComment(IEntity ent, ResourceName res_name)
	{
		if (!m_bGenerateComment)
			return;
		IEntitySource src = m_API.EntityToSource(ent);
		IEntity comment = m_API.CreateEntity("CommentEntity", string.Empty, m_API.GetCurrentEntityLayerId(), src, vector.Zero, vector.Zero);

		vector min, max;
		ent.GetBounds(min, max);

		vector pos;
		pos[1] = max[1] - min[1] + m_fCommentYOffset;

		m_API.ModifyEntityKey(comment, "coords", pos.ToString(false));
		m_API.ModifyEntityKey(comment, "m_Comment", res_name.GetPath());
		// m_API.ModifyEntityKey(comment, "m_FaceCamera", "1");
	}

	private int PlaceSelection(vector trace_end, bool isOnClick)
	{
		int j = 0;
		int k = 0;
		int chunkSize = 0;

		m_API.BeginEntityAction();
		foreach (IEntity ent : m_aSpawnedEntities)
		{
			if (ent)
			{
				if (j%m_iRowSizeX == 0)
				{
					j = 0;
					k++;
				}
				vector pos = trace_end;

				float offset_x = m_fXOffset;
				float offset_z = m_fZOffset;

				if (m_bAlignByBoundingBox == true)
				{
					offset_x += m_fLargestBBoxX;
					offset_z += m_fLargestBBoxZ;
				}

				pos[0] = pos[0] + j * offset_x + offset_x;
				pos[2] = pos[2] + k * offset_z - offset_z / 2;
				pos[1] = m_API.GetTerrainSurfaceY(pos[0], pos[2]) + m_fYOffset;

				EntityFlags flags = ent.GetFlags();
				if ((flags & EntityFlags.RELATIVE_Y) != 0)
				{
					float height = m_API.GetWorld().GetSurfaceY(pos[0], pos[2]);
					pos[1] = pos[1] - height;
				}

				m_API.ModifyEntityKey(ent, "coords", pos.ToString(false));
				m_API.ModifyEntityKey(ent, "angleY", (m_fCumulativeRotation*(m_iRowSizeX*(k-1) + j)).ToString(false));
				if (isOnClick)
					m_aPlacedEntityHistory.Insert(ent);
				
				chunkSize++;
				j++;
				}
		}
		m_API.EndEntityAction();
		
		return chunkSize;
	}

	override void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		vector trace_start;
		vector trace_end;
		vector trace_dir;
		int historyChunkSize = 0;

		if (m_API.TraceWorldPos(x, y, TraceFlags.WORLD | TraceFlags.ENTS, trace_start, trace_end, trace_dir))
		{
			historyChunkSize = PlaceSelection(trace_end, true);
		}
		if (historyChunkSize > 0)
			m_aSpawnHistoryChunks.Insert(historyChunkSize);

		ClearSelection();
	}

	override void OnMouseMoveEvent(float x, float y)
	{
		vector trace_start;
		vector trace_end;
		vector trace_dir;

		float rect_size_x = m_iRowSizeX;
		float rect_size_z = m_iRowSizeZ;

		if (m_bAlignByBoundingBox)
		{
			rect_size_x = m_iRowSizeX * (m_fLargestBBoxX + m_fXOffset);
			rect_size_z = m_iRowSizeZ * (m_fLargestBBoxZ + m_fZOffset);
		}

		m_vPlacementRectSize[0] = rect_size_x;
		m_vPlacementRectSize[2] = rect_size_z;

		if (m_aSelectedXOBs.Count() > 0 || m_aSelectedEntities.Count() > 0)
		{
			if (m_API.TraceWorldPos(x, y, TraceFlags.WORLD | TraceFlags.ENTS, trace_start, trace_end, trace_dir))
			{
				m_PlacementRect = Shape.Create(ShapeType.BBOX, COLOR_YELLOW, ShapeFlags.NOOUTLINE, trace_end, trace_end + m_vPlacementRectSize);

				if (m_bShowPlacement)
				{
					PlaceSelection(trace_end, false);
				}
				else
				{
					m_API.BeginEntityAction();
					foreach (IEntity ent : m_aSpawnedEntities)
					{
						m_API.ModifyEntityKey(ent, "coords", "0 0 0");
					}
					m_API.EndEntityAction();
				}
			}
		}
		else
		{
			delete m_PlacementRect;
		}
	}

	private void FindFilesByExtension(ResourceName path, string extension, out array <ResourceName> list)
	{
		if (path.EndsWith(extension))
		{
			list.Insert(path);
		}
	}

	private void ClearSelection()
	{
		m_aSelection.Clear();
		m_aSelectedEntities.Clear();
		m_aSelectedXOBs.Clear();
		m_aSpawnedEntities.Clear();
	}

	private string FormatEntityIndex()
	{
		string id = m_iEntityId.ToString();

		if (m_iEntityId < 10)
			id = string.Format("000%1", m_iEntityId);
		else if (m_iEntityId < 100)
			id = string.Format("00%1", m_iEntityId);
		else if (m_iEntityId < 1000)
			id = string.Format("0%1", m_iEntityId);

		return id;
	}
};
