#ifdef WORKBENCH
class SCR_WorldEditorToolHelper
{
	protected static ref array<IEntity> s_aTempEntities;
	protected static ref array<ResourceName> s_aTempResourceNames;

	//------------------------------------------------------------------------------------------------
	//! Get the ResourceManager object
	//! \return available ResourceManager or null if unavailable
	static ResourceManager GetResourceManager()
	{
		return Workbench.GetModule(ResourceManager);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the WorldEditor object
	//! \return available WorldEditor or null if unavailable
	static WorldEditor GetWorldEditor()
	{
		return Workbench.GetModule(WorldEditor);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the World Editor API
	//! \return available WorldEditorAPI or null if unavailable
	static WorldEditorAPI GetWorldEditorAPI()
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		return worldEditor.GetApi();
	}

	//------------------------------------------------------------------------------------------------
	//! Return the currently opened Prefab's entity with Edit Prefab button in Resource Manager
	//! \return the entity's source, null if error
	static IEntitySource GetPrefabEditModeEntitySource()
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		if (!worldEditor.IsPrefabEditMode())
			return null;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (worldEditorAPI.GetEditorEntityCount() != 2) // world and Entity
			return null;

		return worldEditorAPI.GetEditorEntity(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the currently opened Prefab's entity with Edit Prefab button in Resource Manager
	//! \return the entity's source, null if error
	static ResourceName GetPrefabEditModeResourceName()
	{
		IEntitySource entitySource = GetPrefabEditModeEntitySource();
		if (!entitySource)
			return string.Empty;

		BaseContainer ancestor = entitySource.GetAncestor();
		if (!ancestor)
			return string.Empty;

		return ancestor.GetResourceName();
	}

	//------------------------------------------------------------------------------------------------
	//! Get a registered file's ResourceName (requires the .meta file)
	//! \return the registered file's ResourceName
	static ResourceName GetResourceNameFromFile(string absoluteFilePath)
	{
		MetaFile metaFile = GetResourceManager().GetMetaFile(absoluteFilePath);
		if (!metaFile)
			return string.Empty;

		return metaFile.GetResourceID();
	}

	//------------------------------------------------------------------------------------------------
	//! Get selected directories - does not get subdirectories, only selected ones (in their order of selection)
	//! \return resource names of directory that are selected in World Editor's Resource Browser
	static array<ResourceName> GetSelectedDirectories()
	{
		array<ResourceName> result = GetSelectedResources(true);
		if (!result)
			return null;

		for (int i = result.Count() - 1; i >= 0; i--)
		{
			ResourceName selectedResource = result[i];
			if (FilePath.StripExtension(selectedResource) != selectedResource)
				result.RemoveOrdered(i);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Wrapper for the method below
	//! \param wantedExtension (case-insensitive)
	//! \param keywords words that should be present in the file name (case-insensitive)
	//! \return an array of found ResourceNames - cannot be null
	static array<ResourceName> GetSelectedOrOpenedResources(string wantedExtension, array<string> keywords = null)
	{
		return GetSelectedOrOpenedResources({ wantedExtension }, keywords);
	}

	//------------------------------------------------------------------------------------------------
	//! Get selected or opened resources
	//! \param acceptedExtensions accepted extensions (case-insensitive)
	//! \param keywords words that should be present in the file name (case-insensitive)
	//! \return an array of found ResourceNames - cannot be null
	static array<ResourceName> GetSelectedOrOpenedResources(array<string> acceptedExtensions = null, array<string> keywords = null)
	{
		array<ResourceName> tempResult = GetSelectedResources(true);
		if (tempResult.IsEmpty())
		{
			ResourceName openedResource = GetPrefabEditModeResourceName();
			if (!openedResource.IsEmpty())
				tempResult.Insert(openedResource);
		}

		if (tempResult.IsEmpty())
			return tempResult;

		if (acceptedExtensions && acceptedExtensions.IsEmpty())
			acceptedExtensions = null;

		if (acceptedExtensions)
		{
			foreach (int i, string wantedExtension : acceptedExtensions)
			{
				wantedExtension.ToLower();
				acceptedExtensions[i] = wantedExtension;
			}
		}

		array<string> keywordsLC;
		if (keywords && !keywords.IsEmpty())
		{
			keywordsLC = {};
			foreach (string keyword : keywords)
			{
				if (keyword.IsEmpty())
					continue;

				keyword.ToLower();
				keywordsLC.Insert(keyword);
			}
		}

		array<ResourceName> result = {};

		foreach (ResourceName resourceName : tempResult)
		{
			string resourceNameLC = resourceName;
			resourceNameLC.ToLower();

			string extensionLC;
			resourceNameLC = FilePath.StripExtension(resourceName, extensionLC);
			extensionLC.ToLower();

			if (acceptedExtensions && !acceptedExtensions.Contains(extensionLC))
				continue;

			resourceNameLC = FilePath.StripPath(resourceNameLC);

			if (!keywordsLC || SCR_StringHelper.ContainsEvery(resourceNameLC, keywordsLC))
				result.Insert(resourceName);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all ResourceName that are selected in the Resource Browser
	//! \param recursive true to get a selected directory's files, false to stop at the directory
	//! \return array of ResourceName of selected resources
	static array<ResourceName> GetSelectedResources(bool recursive = true)
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		array<ResourceName> result = {};

		s_aTempResourceNames = {};
		worldEditor.GetResourceBrowserSelection(ResourceNameCallback, recursive);

		result.Copy(s_aTempResourceNames);
		s_aTempResourceNames = null;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Search Workbench-available files by extension and filters inside a provided directory
	//! \param fileExtensions
	//! \param searchStrArray
	//! \param rootPath
	//! \param recursive
	//! \return found resources
	static array<ResourceName> SearchWorkbenchResources(array<string> fileExtensions = null, array<string> searchStrArray = null, string rootPath = "", bool recursive = true)
	{
		array<ResourceName> result = {};

		s_aTempResourceNames = {};
		Workbench.SearchResources(ResourceNameCallback, fileExtensions, searchStrArray, rootPath, recursive);

		result.Copy(s_aTempResourceNames);
		s_aTempResourceNames = null;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Delete the entitySource's entity IF it exists
	//! \entitySource can be null
	static void DeleteEntityFromSource(IEntitySource entitySource)
	{
		if (!entitySource)
			return;

		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return;
		}

		IEntity entity = worldEditorAPI.SourceToEntity(entitySource);
		if (!entity)
			return;

		bool manageEditAction = BeginEntityAction();

		worldEditorAPI.DeleteEntity(entity);

		EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//! Begin an Entity Action in World Editor API if required
	//! \return true if a BeginEntityAction has been called and EndEntityAction has to be called by EndEntityAction(), false otherwise
	static bool BeginEntityAction()
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return false;
		}

		if (worldEditorAPI.IsDoingEditAction())
		{
			return false;
		}
		else
		{
			worldEditorAPI.BeginEntityAction();
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! End an Entity Action in World Editor API if required
	//! \param manageEditAction if World Editor Entity Action should be terminated, result of an earlier BeginEntityAction call
	static void EndEntityAction(bool manageEditAction)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return;
		}

		if (manageEditAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	// one day, func arguments will be supported in script methods - /perhaps/
//	static array<IEntity> QueryEntitiesByAABB(notnull World world, vector mins, vector maxs, QueryEntitiesCallback addEntity, QueryEntitiesCallback filterEntity = null, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL)
//	{
//		s_aTempEntities = {};
//		world.QueryEntitiesByAABB(mins, maxs, addEntity, filterEntity, queryFlags);
//		s_aTempEntities = null;
//	}

	//------------------------------------------------------------------------------------------------
	//! Return all entities found by moved sphere trace
	//! \param traceSphere
	//! \param world
	//! \return found entities or null on error
	static array<IEntity> TraceMoveEntitiesBySphere(notnull World world, notnull TraceSphere traceSphere)
	{
		array<IEntity> result = {};

		s_aTempEntities = {};
		world.TraceMove(traceSphere, TraceCallbackMethod);

		result.Copy(s_aTempEntities);
		s_aTempEntities = null;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Return all entities found by sphere trace
	//! \param traceSphere
	//! \param world
	//! \return found entities or null on error
	static array<IEntity> TracePositionEntitiesBySphere(notnull World world, notnull TraceSphere traceSphere)
	{
		array<IEntity> result = {};

		s_aTempEntities = {};
		world.TracePosition(traceSphere, TraceCallbackMethod);

		result.Copy(s_aTempEntities);
		s_aTempEntities = null;

		return result;
	}

	/*
		Callbacks
	*/

	//------------------------------------------------------------------------------------------------
	//! WorkbenchSearchResourcesCallback method used for Workbench searches and Resource Browser-selected files
	//! \param resName found ResourceName
	//! \param filePath absolute filepath of said ResourceName if available, empty string otherwise
	protected static void ResourceNameCallback(ResourceName resName, string filePath = "")
	{
		Print("DEBUG LINE | " + filePath + " " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.DEBUG);
		s_aTempResourceNames.Insert(resName);
	}

	//------------------------------------------------------------------------------------------------
	//! TraceEntitiesCallback method used for World tracing
	protected static bool TraceCallbackMethod(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		s_aTempEntities.Insert(e);
		return true;
	}
}
#endif
