[WorkbenchPluginAttribute(name: "Edit Selected Prefab(s)", shortcut: "Ctrl+Shift+E", wbModules: {"WorldEditor"}, awesomeFontCode: 0xf1b2)]
class PrefabEditingPlugin: PrefabEditingPluginBase
{
	[Attribute(defvalue: "{F636AAB9EE015E5F}Configs/Workbench/PrefabEditingPlugin/PrefabEditingPluginConfig.conf", params: "conf", desc: "Config with rules defining which worlds will be used for which folders.")]
	private ResourceName m_Config;
	
	protected string GetFileName(ResourceName prefab)
	{
		string name = FilePath.StripPath(prefab);
		return FilePath.StripExtension(name);
	}
	protected IEntity CreateEntity(WorldEditorAPI api, ResourceName prefab, vector position, vector rotation)
	{
		string extension;
		FilePath.StripExtension(prefab, extension);
		if (extension == "ct")
		{
			IEntity entity = api.CreateEntity("GenericEntity", GetFileName(prefab), api.GetCurrentEntityLayerId(), null, position, rotation);
			api.CreateComponent(api.EntityToSource(entity), prefab);
			return entity;
		}
		else
		{
			return api.CreateEntity(prefab, GetFileName(prefab), api.GetCurrentEntityLayerId(), null, position, rotation);
		}
	}
	protected bool CreateWorldFiles(ResourceName worldPrefab, string targetPath)
	{
		//--- ToDo: Use more legit way to create the world?
		string worldPath = worldPrefab.GetPath();
		
		if (!FileIO.FileExists(worldPath))
		{
			Print(string.Format("Cannot load prefab, selected world '%2' doesn't exist!", worldPath), LogLevel.ERROR);
			return false;
		}
		
		FileIO.MakeDirectory("$profile:worlds");
		
		//--- World file
		FileHandle file = FileIO.OpenFile(targetPath + ".ent", FileMode.WRITE);
		file.WriteLine("SubScene {");
		file.WriteLine(string.Format(" Parent \"%1\"", worldPrefab));
		file.WriteLine("}");
		file.WriteLine("Layer default {");
		file.WriteLine(" Index 0");
		file.WriteLine("}");
		file.Close();
		
		//--- Layer file
		file = FileIO.OpenFile(targetPath + "_Layers/default.layer", FileMode.WRITE);
		file.Close();
		
		return true;
	}
	protected void SetCamera(WorldEditorAPI api, IEntity entity)
	{
		IEntitySource source = api.EntityToSource(entity);
		vector boundsMin = Vector(float.MAX, float.MAX, float.MAX);
		vector boundsMax = vector.Zero;
		GetEntitySourceBounds(api, source, boundsMin, boundsMax);

		vector size = boundsMax - boundsMin;
		float distance = Math.Max(size.Length() * 1.5, 1);
		if (distance > 100)
		{
			size = vector.Zero;
			distance = 100;
		}
		vector rotation = Vector(-135, -30, 0).AnglesToVector();
		vector pivot = entity.GetOrigin() + Vector(0, size[1] / 2, 0);
		api.SetCamera(pivot - rotation * distance, rotation);
	}
	protected void GetEntitySourceBounds(WorldEditorAPI api, IEntitySource source, out vector boundsMin, out vector boundsMax)
	{
		IEntity entity = api.SourceToEntity(source);
		if (!entity) return;
		
		if (entity.GetVObject())
		{
			vector entityMin, entityMax;
			entity.GetBounds(entityMin, entityMax);
			entityMin = entity.CoordToParent(entityMin);
			entityMax = entity.CoordToParent(entityMax);
			
			boundsMin[0] = Math.Min(boundsMin[0], entityMin[0]);
			boundsMin[1] = Math.Min(boundsMin[1], entityMin[1]);
			boundsMin[2] = Math.Min(boundsMin[2], entityMin[2]);
			
			boundsMax[0] = Math.Max(boundsMax[0], entityMax[0]);
			boundsMax[1] = Math.Max(boundsMax[1], entityMax[1]);
			boundsMax[2] = Math.Max(boundsMax[2], entityMax[2]);
		}
		
		int childrenCount = source.GetNumChildren();
		for (int i = 0; i < childrenCount; i++)
		{
			GetEntitySourceBounds(api, source.GetChild(i), boundsMin, boundsMax);
		}
	}
	protected PrefabEditingPluginConfig GetConfig(ResourceName configPath)
	{
		Resource configResource = Resource.Load(configPath);
		if (!configResource.IsValid())
		{
			Print(string.Format("Cannot load config '%1'!", configPath), LogLevel.ERROR);
			return null;
		}
		
		BaseResourceObject configContainer = configResource.GetResource();
		if (!configContainer) return null;
		
		BaseContainer configBase = configContainer.ToBaseContainer();
		if (!configBase) return null;
		
		if (configBase.GetClassName() != "PrefabEditingPluginConfig")
		{
			Print(string.Format("Config '%1' is of type '%2', must be 'PrefabEditingPluginConfig'!", configPath, configBase.GetClassName()), LogLevel.ERROR);
			return null;
		}
				
		return PrefabEditingPluginConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configBase));
	}
	
	override void Run()
	{
		if (!Workbench.OpenModule(WorldEditor) || !SCR_Global.IsEditMode()) return;
		
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor) return;
		
		WorldEditorAPI api = worldEditor.GetApi();
		if (!api) return;
		
		array<ResourceName> compatiblePrefabs = new array<ResourceName>;
		if (!GetPrefabs(compatiblePrefabs, true)) return;
		
		//--- Load config
		PrefabEditingPluginConfig config = GetConfig(m_Config);
		if (!config) return;
		
		PrefabEditingPluginConfigFolder folderSettings = config.GetFolderSettings(compatiblePrefabs[0]);
		if (!folderSettings) return;

		//--- Create world
		string targetPath = config.GetPath();
		if (!CreateWorldFiles(folderSettings.GetWorld(), targetPath)) return;
		
		if (!worldEditor.SetOpenedResource(targetPath + ".ent"))
		{
			Print(string.Format("Cannot load world '%1'!", folderSettings.GetWorld()), LogLevel.ERROR);
			return;
		}
		
		//--- Create entities
		api.BeginEntityAction();
		api.ClearEntitySelection();
		array<IEntity> entities = new array<IEntity>;
		
		vector pos, rot, size, boundsMin, boundsMax, boundsSize;
		pos = folderSettings.GetPosition();
		rot = folderSettings.GetRotation();
		size = vector.Zero;
		bool usePrefabPosition = folderSettings.UsePrefabPosition();
		
		IEntity entity;
		IEntitySource source;
		foreach (int i, ResourceName prefab: compatiblePrefabs)
		{
			if (usePrefabPosition)
			{
				source = SCR_BaseContainerTools.FindEntitySource(Resource.Load(prefab));
				source.Get("coords", pos);
				rot = vector.Zero;
			}
			
			entity = CreateEntity(api, prefab, pos, rot); //--- ToDo: Smarter placement of multiple entities?
			if (!entity)
			{
				Print(string.Format("Cannot create entity for prefab '%1'!", prefab), LogLevel.ERROR);
				api.EndEntityAction();
				return;
			}
			entities.Insert(entity);
			
			//--- Get bounding box
			source = api.EntityToSource(entity);
			boundsMin = Vector(float.MAX, float.MAX, float.MAX);
			boundsMax = -Vector(float.MAX, float.MAX, float.MAX);
			GetEntitySourceBounds(api, source, boundsMin, boundsMax);
			boundsSize = boundsMax - boundsMin;
			for (int n = 0; n < 3; n++)
				size[n] = Math.Max(size[n], boundsSize[n]);
		}
		float spacing = folderSettings.GetSpacing();
		size += Vector(spacing, spacing, spacing);
		
		//--- Select entities (start from the back, because the last one has transformation gizmo on it)
		int entitiesCount = entities.Count();
		int length = Math.Ceil(Math.Sqrt(entitiesCount));
		int row = 0;
		int column = 0;
		for (int i = entitiesCount; i--; i >= 0)
		{
			row = Math.Floor(i / length);
			column = i % length;
			api.AddToEntitySelection(entities[i]);
			if (!usePrefabPosition)
				api.ModifyEntityKey(entities[i], "coords", string.Format("%1 %2 %3", pos[0] - size[0] * column, pos[1], pos[2] - size[2] * row));
		}
		SetCamera(api, entities[0]);
		api.EndEntityAction();
		
		//--- Save to make sure that only user changes are marked
		worldEditor.Save();
	}
	
	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Edit Selected Prefab(s)' plugin", "", this);
	}
	[ButtonAttribute("Close")]
	bool ButtonClose()
	{
	}
};

[WorkbenchPluginAttribute(name: "Edit Selected Prefab(s)", shortcut: "Ctrl+Shift+E", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf1b2)]
class PrefabEditingPluginResourceManager: PrefabEditingPlugin
{
	override void GetSelected(out array<ResourceName> selection)
	{
		GetSelectedResourceBrowser(selection);
	}
};

[BaseContainerProps(configRoot: true)]
class PrefabEditingPluginConfig
{
	[Attribute(defvalue: "$profile:worlds/PrefabEditingPlugin")]
	private string m_Path;
	
	[Attribute()]
	private ref array<ref PrefabEditingPluginConfigFolder> m_Folders;
	
	string GetPath()
	{
		return m_Path;
	}
	PrefabEditingPluginConfigFolder GetFolderSettings(ResourceName prefab)
	{
		string prefabPath = prefab.GetPath();
		
		PrefabEditingPluginConfigFolder folderSettings = null;
		int pathLengthMax = -1;
		foreach (PrefabEditingPluginConfigFolder folderSettingsTemp: m_Folders)
		{
			string folderPath = folderSettingsTemp.GetFolder();
			int pathLength = folderPath.Length();
			if (prefabPath.StartsWith(folderPath) && pathLength > pathLengthMax)
			{
				folderSettings = folderSettingsTemp;
				pathLengthMax = pathLength;
			}
		}
		return folderSettings;
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Folder")]
class PrefabEditingPluginConfigFolder
{
	[Attribute(params: "folders")]
	private ResourceName m_Folder;
	
	[Attribute(defvalue: "{70ABDE49D02BA4F4}worlds/TestMaps/PrefabEditingPlugin/PrefabEditingPlugin.ent", uiwidget: UIWidgets.ResourceNamePicker, params: "ent")]
	private ResourceName m_World;
	
	[Attribute(desc: "When enabled, prefabs will be used on their own coordinates.")]
	private bool m_bUsePrefabPosition;
	
	[Attribute()]
	private vector m_vPosition;
	
	[Attribute(desc: "Rotation pitch, yaw, roll")]
	private vector m_vRotation;
	
	[Attribute(defvalue: "0.1", desc: "Minimal spacing [m] between spawned entities")]
	private float m_vSpacing;
	
	string GetFolder()
	{
		return m_Folder.GetPath();
	}
	ResourceName GetWorld()
	{
		return m_World;
	}
	bool UsePrefabPosition()
	{
		return m_bUsePrefabPosition;
	}
	vector GetPosition()
	{
		return m_vPosition;
	}
	vector GetRotation()
	{
		return m_vRotation;
	}
	float GetSpacing()
	{
		return m_vSpacing;
	}
};
