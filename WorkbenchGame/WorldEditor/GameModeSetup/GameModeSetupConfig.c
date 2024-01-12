[BaseContainerProps(configRoot: true)]
class GameModeSetupConfig
{
	[Attribute(desc: "Prefabs that will be created automatically by the plugin.\nTheir classes, and classes of their children, will be used as required classes for the game mode.")]
	protected ref array<ref GameModeSetupConfigEntry> m_Prefabs;
	
	[Attribute(desc: "Template from which mission header will be created.")]
	protected ref MissionHeader m_MissionHeader;
	
	protected ref array<ref GameModeSetupConfigEntry> m_RequiredTypes = {};
	protected typename m_GameModeType;
	protected int m_LongestTypeLength;
	protected string m_WorldPath;
	protected string m_FileSystem;
	
	protected const string DESCRIPTION_ENTITY_PRESENT =					"        ✅ %1\n";
	protected const string DESCRIPTION_ENTITY_MISSING =					"        ❌ %1\n";

	protected const string DESCRIPTION_VALIDATION_SUCCESS =				"\n✅ The world is configured correctly.";
	protected const string DESCRIPTION_VALIDATION_NO_SUBSCENE =			"\n⛔ Missing Subscene\nYou are attempting to add the game mode to the root world.\nPlease create a subscene for it instead, while leaving the root world intact.\nThat will allow others to create their own subscenes with different game modes.\n";
	protected const string DESCRIPTION_VALIDATION_WRONG_GAME_MODE =		"\n⛔ Incompatible Game Mode\nThe scan found existing game mode of type %1.\nThat's incompatible with the expected type %2.\nThere can be only one game mode in the world, so if you wish to continue,\nyou have to manually remove the existing one. The plugin cannot automate this process.\n";
	protected const string DESCRIPTION_VALIDATION_PRESENT_ENTITIES =	"\nPresent entity types:\n (NOT USED)";
	protected const string DESCRIPTION_VALIDATION_MISSING_ENTITIES =	"\n⛔ Missing Entities\nOne or more required entity types are missing, listed below.\nThese can be generated automatically in the next step.\n";
	
	protected const string DESCRIPTION_GENERATION_ENTITY =				"        ❗ %1: %2\n";
	protected const string DESCRIPTION_GENERATION_MORE_STEPS_1 =		"\nSome entities need to be manually configured or replaced by more specialized prefabs:\n";
	protected const string DESCRIPTION_GENERATION_MORE_STEPS_2 =		"\nEach has a comment attached explaining necessary steps, so you can return to them later.\n";
	
	protected const string DESCRIPTION_MISSION_HEADER_EXISTS =			"✅ Mission header config at '%1' is configured correctly.\n";
	
	protected const string MISSIONS_PATH = "Missions";
	
	/*!
	Scan all World Editor entities and check if the required ones are present.
	\param[out] dialogMessage Message show in the dialog window
	\param[out] canAutogenerate True if the wizard can continue to auto-generation of entities
	\return True if all required entities are present
	*/
	bool ValidateWorld(out string dialogMessage, out bool canAutogenerate)
	{
		canAutogenerate = true;
		
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		
		string worldPath;
		api.GetWorldPath(worldPath);
		
		set<typename> worldTypes = GetWorldEntityTypes();
		
		//--- Check for present and missing types
		string presentTypes;
		string missingTypes;
		bool isSuccess = true;
		foreach (GameModeSetupConfigEntry entry: m_RequiredTypes)
		{
			//--- ToDo: Different format for log messages
			if (worldTypes.Contains(entry.m_Type))
			{
				presentTypes += string.Format(DESCRIPTION_ENTITY_PRESENT, entry.m_Type);
			}
			else
			{
				missingTypes += string.Format(DESCRIPTION_ENTITY_MISSING, entry.m_Type);
				isSuccess = false;
			}
		}
		
		//--- Check if incompatible game mode is present (there can only be one game mode per world)
		typename wrongGameMode;
		foreach (typename type: worldTypes)
		{
			if (type.IsInherited(BaseGameMode) && !type.IsInherited(m_GameModeType))
			{
				wrongGameMode = type;
				isSuccess = false;
				canAutogenerate = false;
				break;
			}
		}
		
		//--- Check if the world is a subscene
		bool noSubscene;
		if (api.GetNumSubScenes() == 1)
		{
			noSubscene = true;
			isSuccess = false;
			canAutogenerate = false;
		}
		
		if (isSuccess)
		{
			dialogMessage += DESCRIPTION_VALIDATION_SUCCESS;
		}
		else
		{
			if (noSubscene)
				dialogMessage += DESCRIPTION_VALIDATION_NO_SUBSCENE;
			
			if (wrongGameMode)
				dialogMessage += string.Format(DESCRIPTION_VALIDATION_WRONG_GAME_MODE, wrongGameMode, m_GameModeType);
			
			//if (presentTypes)
			//	dialogMessage += DESCRIPTION_VALIDATION_PRESENT_ENTITIES + presentTypes;
			
			if (missingTypes && !noSubscene && !wrongGameMode)
				dialogMessage += DESCRIPTION_VALIDATION_MISSING_ENTITIES + missingTypes;
		}
		
		return isSuccess;
	}
	/*!
	Create game mode prefabs in the world.
	\param[out] dialogMessage Message show in the dialog window
	\return True if all entities were created
	*/
	bool GenerateWorld(out string dialogMessage)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		
		BaseWorld world = api.GetWorld();
		vector min, max;
		world.GetBoundBox(min, max);
		vector worldCenter = vector.Lerp(min, max, 0.5);
		
		api.BeginEntityAction("GameModeSetupPlugin");
		int layerID = api.GetCurrentEntityLayerId();// api.CreateSubsceneLayer(api.GetCurrentSubScene(), "GameMode");
		
		int asciiLineBreak = 10;
		string lineBreak = asciiLineBreak.AsciiToString();
		
		IEntity entity;
		IEntitySource entitySource;
		foreach (int i, GameModeSetupConfigEntry entry: m_Prefabs)
		{
			string entityName = FilePath.StripExtension(FilePath.StripPath(entry.m_Prefab));
			
			worldCenter += vector.Forward;
			worldCenter[1] = world.GetSurfaceY(worldCenter[0], worldCenter[2]);
			
			//--- Entity
			entity = entry.CreateEntity(api, entityName, layerID, worldCenter, vector.Zero);
			entitySource = api.EntityToSource(entity);
			api.AddToEntitySelection(entity);
			
			//--- Comment entity
			if (entry.m_Comment)
			{
				entity = api.CreateEntity("CommentEntity", string.Empty, layerID, entitySource, vector.Up, vector.Zero);
				entitySource = api.EntityToSource(entity);
				api.SetVariableValue(entitySource, {}, "m_Comment", entityName + ": " + lineBreak + entry.m_Comment);
				api.SetVariableValue(entitySource, {}, "m_Color", "0 0 0 1");
				api.SetVariableValue(entitySource, {}, "m_FaceCamera", "1");
				api.SetVariableValue(entitySource, {}, "m_TextBackground", "1");
				api.SetVariableValue(entitySource, {}, "m_BackgroundColor", "1 0.6 0 1");
				api.SetVariableValue(entitySource, {}, "m_BackgroundTransparency", "0");
				
				dialogMessage += string.Format(DESCRIPTION_GENERATION_ENTITY, entityName, entry.m_Comment);
			}
		}
		api.UpdateSelectionGui();
		api.EndEntityAction("GameModeSetupPlugin");
		
		//--- Move camera to entities to give user clear feedback
		api.SetCamera(worldCenter + Vector(0, 4, 2), Vector(180, -60, 0).AnglesToVector());
		
		if (dialogMessage)
			dialogMessage = DESCRIPTION_GENERATION_MORE_STEPS_1 + dialogMessage + DESCRIPTION_GENERATION_MORE_STEPS_2;
		
		return true;
	}
	/*!
	Check if mission header for the world exists.
	\param[out] dialogMessage Message show in the dialog window
	\return True if the mission header exists
	*/
	bool ValidateMissionHeader(out string dialogMessage)
	{		
		ref array<ResourceName> resources = {};
		string missionsPath = m_FileSystem + MISSIONS_PATH;
		Workbench.SearchResources(resources.Insert, {"conf"}, null, missionsPath);
		Resource missionResource;
		BaseContainer missionContainer;
		ResourceName missionWorld;
		
		for (int i = resources.Count() - 1; i >= 0; i--)
		{
			missionResource = Resource.Load(resources[i]);
			if (!missionResource.IsValid())
				continue;
			
			missionContainer = missionResource.GetResource().ToBaseContainer();
			missionContainer.Get("World", missionWorld);
			if (m_FileSystem + missionWorld.GetPath() == m_WorldPath)
			{
				dialogMessage = string.Format(DESCRIPTION_MISSION_HEADER_EXISTS, resources[i].GetPath());
				return true;
			}
		}
		return false;
	}
	/*!
	Create mission header file for the world from a template.
	\param templatePath Config file of type GameModeSetupConfig where the template is configured
	\param[out] dialogMessage Message show in the dialog window
	\return True if the mission header file was created
	*/
	bool GenerateMissionHeader(string templatePath, out string dialogMessage)
	{
		//--- Get mission header from the template config (can't use m_MissionHeader directly, it's engine-controlled class that cannot have reference in script)
		Resource templateResource = Resource.Load(templatePath);
		BaseContainer templateContainer = templateResource.GetResource().ToBaseContainer();
		BaseContainer missionHeaderContainer = templateContainer.GetObject("m_MissionHeader");
		
		//--- Get world path with GUID and save it to the header
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		string absWorldPath;
		Workbench.GetAbsolutePath(m_WorldPath, absWorldPath);
		MetaFile worldMeta = resourceManager.GetMetaFile(absWorldPath);
		string fullWorldPath = worldMeta.GetResourceID();		
		missionHeaderContainer.Set("World", fullWorldPath);
		
		//--- Get target config path
		string worldName = FilePath.StripExtension(FilePath.StripPath(m_WorldPath));
		string missionHeaderPath = FilePath.Concat(m_FileSystem + MISSIONS_PATH, worldName);
		missionHeaderPath = FilePath.AppendExtension(missionHeaderPath, "conf");
		
		//--- Create the config
		if (!BaseContainerTools.SaveContainer(missionHeaderContainer, ResourceName.Empty, missionHeaderPath))
		{
			Print(string.Format("Unable to create mission header at %1!", missionHeaderPath), LogLevel.ERROR);
			return false;
		}
		
		//--- Open the config in Resource Manager, so the user can edit it straight away
		string missionHeaderAbsPath;
		Workbench.GetAbsolutePath(missionHeaderPath, missionHeaderAbsPath, false);
		resourceManager.RegisterResourceFile(missionHeaderAbsPath);
		resourceManager.SetOpenedResource(missionHeaderPath);
		
		dialogMessage = string.Format(DESCRIPTION_MISSION_HEADER_EXISTS, missionHeaderPath);
		
		return true;
	}
	protected set<typename> GetWorldEntityTypes()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		
		WBProgressDialog progress = new WBProgressDialog("Validating world entities...", worldEditor);
		
		set<typename> types = new set<typename>();
		IEntitySource entitySource;
		array<IEntitySource> queue = {};
		int containerCount = worldEditor.GetNumContainers();
		
		for (int i = 0; i < containerCount; i++)
		{
			entitySource = IEntitySource.Cast(worldEditor.GetContainer(i));
			
			if (entitySource.GetNumChildren() > 0)
				queue.Insert(entitySource);
			else
				types.Insert(entitySource.GetClassName().ToType());
			
			progress.SetProgress(i / containerCount);
		}
		
		while (!queue.IsEmpty())
		{
			entitySource = queue[0];
			queue.Remove(0);
			
			types.Insert(entitySource.GetClassName().ToType());
			
			for (int i = 0, count = entitySource.GetNumChildren(); i < count; i++)
			{
				queue.Insert(entitySource.GetChild(i));
			}
		}
		
		return types;
	}
	void Init()
	{		
		//--- Get file system
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		
		api.GetWorldPath(m_WorldPath);
		m_FileSystem = FilePath.FileSystemNameFromFileName(m_WorldPath);
		m_FileSystem = SCR_AddonTool.ToFileSystem(m_FileSystem);
	}
	void GameModeSetupConfig()
	{
		int r = 0;
		
		//--- Get types of generated prefabs and their children
		Resource prefabResource;
		IEntitySource prefabSource;
		array<IEntitySource> queue = {};
		array<ref GameModeSetupConfigEntry> queuePrefabs = {}; //--- Must be 'ref', otherwise entries will be null
		
		foreach (GameModeSetupConfigEntry entry: m_Prefabs)
		{
			prefabResource = Resource.Load(entry.m_Prefab);
			if (!prefabResource.IsValid())
				continue;
			
			prefabSource = prefabResource.GetResource().ToEntitySource();
			queue.Insert(prefabSource);
			queuePrefabs.Insert(entry);
		}
		
		map<string, ref GameModeSetupConfigEntry> entries = new map<string, ref GameModeSetupConfigEntry>();
		GameModeSetupConfigEntry entry;
		string requiredNamesSortedStatic[256];
		string className;
		typename type;
		while (!queue.IsEmpty())
		{
			prefabSource = queue[0];
			entry = queuePrefabs[0];
			
			queue.Remove(0);
			queuePrefabs.Remove(0);
			
			className = prefabSource.GetClassName();
			m_LongestTypeLength = Math.Max(m_LongestTypeLength, className.Length());
			requiredNamesSortedStatic[r] = className;
			r++;
			
			type = className.ToType();
			entry.m_Type = type;
			entries.Insert(className, entry);
			
			if (type.IsInherited(BaseGameMode))
				m_GameModeType = type;
			
			for (int i = 0, count = prefabSource.GetNumChildren(); i < count; i++)
			{
				queue.Insert(prefabSource.GetChild(i));
				queuePrefabs.Insert(new GameModeSetupConfigEntry());
			}
		}
		
		if (!m_GameModeType)
		{
			Print("No game mode entity found among prefabs to be auto-generated!", LogLevel.WARNING);
		}
		
		//--- Alphabetically sort the array of types
		StaticArray.Sort(requiredNamesSortedStatic);
		string sortedClassName;
		for (int i = 0; i < 256; i++)
		{
			sortedClassName = requiredNamesSortedStatic[i];
			if (!sortedClassName.IsEmpty())
				m_RequiredTypes.Insert(entries[sortedClassName]);
		}
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Prefab", true)]
class GameModeSetupConfigEntry
{
	[Attribute(desc: "Entity prefab to be spawned.", params: "et")]
	ResourceName m_Prefab;
	
	[Attribute(desc: "When defined, user will be notified that the entity requires additional configuration\nand a comment with this text will be created next to the entity.")]
	string m_Comment;
	
	typename m_Type;
	
	/*!
	Create entity from prefab.
	Can be overriden by inherited classes.
	\param api World Editor API
	\param entityName Name of created entity
	\param ID of World Editor layer in which the entity should be created
	\param pos Position of created entity
	\param dir Direction vector (not angles!) of created entity
	\return Created entity
	*/
	IEntity CreateEntity(WorldEditorAPI api, string entityName, int layerID, vector pos, vector dir)
	{
		return api.CreateEntity(m_Prefab, entityName, layerID, null, pos, dir);
	}
};
