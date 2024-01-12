#ifdef WORKBENCH
// temporary entity/Source solutions as container creation might come soon™ (BaseContainerTools.CreatePrefabEntityContainer)
class SCR_PrefabHelper
{
	protected static const string MESHOBJECT_CLASSNAME = "MeshObject";
	protected static const string DEFAULT_PARENT_PREFAB = "GenericEntity";
	protected static const string PREFAB_DOTTED_EXTENSION = ".et";
	static const string PREFAB_BASE_SUFFIX = "_base"; //<! used by SCR_PrefabManagementTool to strip it when creating a child

	//------------------------------------------------------------------------------------------------
	//! Create a clone of the provided prefab at the provided destination, overriding file if any
	//! Destination directory is created if it does not exist
	//! \param absoluteFilePath the directory is automatically created if needed and a trailing '.et' is automatically added if missing
	//! \return created clone's ResourceName
	static ResourceName ClonePrefab(ResourceName sourcePrefab, string absoluteFilePath)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(sourcePrefab))
		{
			Print("Source Prefab is empty", LogLevel.WARNING);
			return string.Empty;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteFilePath))
		{
			Print("Absolute File Path is empty", LogLevel.WARNING);
			return string.Empty;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return string.Empty;
		}

		if (!absoluteFilePath.EndsWith(PREFAB_DOTTED_EXTENSION))
			absoluteFilePath += PREFAB_DOTTED_EXTENSION;

		string absoluteDirPath = FilePath.StripFileName(absoluteFilePath);
		if (!FileIO.FileExists(absoluteDirPath))
		{
			if (!SCR_FileIOHelper.CreateDirectory(absoluteDirPath))
			{
				Print("Could not create the destination directory (" + absoluteDirPath + ")", LogLevel.ERROR);
				return string.Empty;
			}
		}

		IEntitySource entitySource = CreateEntitySourceWithoutEntity(sourcePrefab);
		if (!entitySource)
		{
			Print("Prefab's entity source could not be created", LogLevel.ERROR);
			return string.Empty;
		}

		IEntitySource actualPrefab = IEntitySource.Cast(entitySource.GetAncestor());
		if (!actualPrefab)
		{
			Print("Created entity's source does not have an ancestor", LogLevel.ERROR);
			return string.Empty;
		}

		if (!worldEditorAPI.CreateEntityTemplate(actualPrefab, absoluteFilePath))
		{
			Print("Could not save Prefab", LogLevel.ERROR);
			return string.Empty;
		}

		MetaFile metaFile = SCR_WorldEditorToolHelper.GetResourceManager().GetMetaFile(absoluteFilePath);
		if (!metaFile)
		{
			Print("Created Prefab's meta file could not be found", LogLevel.WARNING);
			return string.Empty;
		}

		return metaFile.GetResourceID();
	}

	//------------------------------------------------------------------------------------------------
	//! Create a child of the provided prefab at the provided destination, overriding file if any
	//! Destination directory is created if it does not exist
	//! \param absoluteFilePath the directory is automatically created if needed and a trailing '.et' is automatically added if missing
	//! \return created child's ResourceName
	static ResourceName CreateChildPrefab(ResourceName sourcePrefab, string absoluteFilePath)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(sourcePrefab))
		{
			Print("Source Prefab is empty", LogLevel.WARNING);
			return string.Empty;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteFilePath))
		{
			Print("Absolute File Path is empty", LogLevel.WARNING);
			return string.Empty;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return string.Empty;
		}

		if (!absoluteFilePath.EndsWith(PREFAB_DOTTED_EXTENSION))
			absoluteFilePath += PREFAB_DOTTED_EXTENSION;

		string absoluteDirPath = FilePath.StripFileName(absoluteFilePath);
		if (!FileIO.FileExists(absoluteDirPath))
		{
			if (!SCR_FileIOHelper.CreateDirectory(absoluteDirPath))
			{
				Print("Could not create directory \"" + absoluteDirPath + "\"", LogLevel.ERROR);
				return string.Empty;
			}
		}

		IEntitySource entitySource = CreateEntitySourceWithoutEntity(sourcePrefab);
		if (!entitySource)
		{
			Print("Prefab's entity source could not be created", LogLevel.ERROR);
			return string.Empty;
		}

		if (!worldEditorAPI.CreateEntityTemplate(entitySource, absoluteFilePath))
		{
			Print("Could not create Prefab", LogLevel.ERROR);
			return string.Empty;
		}

		MetaFile metaFile = SCR_WorldEditorToolHelper.GetResourceManager().GetMetaFile(absoluteFilePath);
		if (!metaFile)
		{
			Print("Created Prefab's meta file could not be found", LogLevel.WARNING);
			return string.Empty;
		}

		return metaFile.GetResourceID();
	}

	//------------------------------------------------------------------------------------------------
	//! Create Prefabs from the provided XOBs - only setting MeshObject to these Prefabs.
	//! Destination directory is created if it does not exist
	//! The difference with CreatePrefabFromXOB is that prefabs here cannot be named, their name will be based on XOB filename
	//! \param xobs list of XOBs to import
	//! \param absoluteDirPath a trailing '/' is automatically added if missing
	//! \param parentPrefab if not provided, GenericEntity is used
	//! \param createBasePrefab if a prefab_base.et should be created (from which prefab.et inherits)
	//! \return created prefabs, null on error
	static array<ResourceName> CreatePrefabsFromXOBs(notnull array<ResourceName> xobs, string absoluteDirPath, ResourceName parentPrefab = string.Empty, bool createBasePrefab = false)
	{
		if (xobs.IsEmpty())
			return {};

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return null;
		}

		if (!FileIO.FileExists(absoluteDirPath))
		{
			if (!SCR_FileIOHelper.CreateDirectory(absoluteDirPath))
			{
				Print("Could not create directory \"" + absoluteDirPath + "\"", LogLevel.ERROR);
				return null;
			}
		}

		bool manageEditAction = BeginEntityAction();

		IEntitySource entitySource = CreateEntitySourceWithoutEntity(parentPrefab);
		if (!entitySource)
		{
			Print("Prefab's entity source could not be created", LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		// if the MeshObject component doesn't exist, create it
		if (!CreateEntitySourceComponentIfNeeded(entitySource, MESHOBJECT_CLASSNAME))
		{
			Print("Could not add MeshObject component", LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		array<ResourceName> result = {};
		array<ref ContainerIdPathEntry> meshObjectPath = { new ContainerIdPathEntry(MESHOBJECT_CLASSNAME) };

		foreach (ResourceName xob : xobs)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(xob))
			{
				Print("Provided XOB is empty - skipping", LogLevel.WARNING);
				continue;
			}

			string prefabFileName = FilePath.StripExtension(FilePath.StripPath(xob.GetPath())) + PREFAB_DOTTED_EXTENSION;

			// apply XOB path to mesh
			if (!worldEditorAPI.SetVariableValue(entitySource, meshObjectPath, "Object", xob))
			{
				Print("Could not apply XOB model to " + prefabFileName, LogLevel.ERROR);
				continue;
			}

			ResourceName resourceName = SaveEntitySourceAsPrefab(entitySource, absoluteDirPath, prefabFileName, createBasePrefab);
			if (resourceName.IsEmpty())
			{
				Print("Could not save Prefab " + prefabFileName, LogLevel.WARNING);
				continue;
			}

			result.Insert(resourceName);
		}

		EndEntityAction(manageEditAction);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Create Prefab from the provided XOB - only setting MeshObject to the Prefab.
	//! Destination directory is created if it does not exist
	//! The difference with CreatePrefabsFromXOBs is that providing the .et path extracts the final file name
	//! the base prefab (if requested) will be named accordingly (name without extension + '_base.et')
	//! \param xob path of the XOB to import
	//! \param absoluteDestinationPath can be either a directory (a trailing '/' is automatically added if missing) or a file path (.et), the directory is automatically created if needed
	//! \param parentPrefab if not provided, GenericEntity is used
	//! \param createBasePrefab if a prefab_base.et should be created (from which prefab.et inherits)
	//! \return created prefabs, null on error
	static ResourceName CreatePrefabFromXOB(ResourceName xob, string absoluteDestinationPath, ResourceName parentPrefab = string.Empty, bool createBasePrefab = false)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(xob))
		{
			Print("Provided XOB is empty", LogLevel.ERROR);
			return string.Empty;
		}

		if (!absoluteDestinationPath.EndsWith(PREFAB_DOTTED_EXTENSION)) // use the already existing method
		{
			array<ResourceName> result = CreatePrefabsFromXOBs({ xob }, absoluteDestinationPath, parentPrefab, createBasePrefab);
			if (result.IsEmpty())
				return string.Empty;
			else
				return result[0];
		}

		string absoluteDirPath = FilePath.StripFileName(absoluteDestinationPath);
		if (absoluteDirPath.EndsWith("/"))
			absoluteDirPath = absoluteDirPath.Substring(0, absoluteDirPath.Length() - 1);

		if (!SCR_FileIOHelper.CreateDirectory(absoluteDirPath))
		{
			Print("Could not create directory \"" + absoluteDirPath + "\"", LogLevel.ERROR);
			return string.Empty;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return string.Empty;
		}

		bool manageEditAction = BeginEntityAction();

		IEntitySource entitySource = CreateEntitySourceFromXOB(xob, parentPrefab);
		SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);

		EndEntityAction(manageEditAction);

		if (!entitySource)
		{
			Print("Prefab's entity source could not be created", LogLevel.ERROR);
			return string.Empty;
		}

		string prefabFileName = FilePath.StripPath(absoluteDestinationPath);
		return SaveEntitySourceAsPrefab(entitySource, absoluteDirPath, prefabFileName, createBasePrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Create an entity and return an IEntitySource
	//! Keeps the entity in the world, does NOT delete the entity itself
	//! \param parentPrefab if not provided, GenericEntity is used
	//! \return created entity's IEntitySource or null on error
	static IEntitySource CreateEntitySource(ResourceName parentPrefab = string.Empty)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return null;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(parentPrefab))
			parentPrefab = DEFAULT_PARENT_PREFAB;

		bool manageEditAction = BeginEntityAction();

		IEntity entity = worldEditorAPI.CreateEntity(parentPrefab, "", worldEditorAPI.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		if (!entity)
		{
			Print("Entity could not be created (parent = " + parentPrefab + ")", LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		IEntitySource entitySource = worldEditorAPI.EntityToSource(entity);
		if (!entitySource)
		{
			Print("Entity source could not be found (parent = " + parentPrefab + ")", LogLevel.ERROR);
			worldEditorAPI.DeleteEntity(entity);
			EndEntityAction(manageEditAction);
			return null;
		}

		EndEntityAction(manageEditAction);

		return entitySource;
	}

	//------------------------------------------------------------------------------------------------
	//! A wrapper method for CreateEntitySource + DeleteEntityFromSource and World Editor action management
	//! \param parentPrefab if not provided, GenericEntity is used
	//! \return created entity's IEntitySource or null on error
	protected static IEntitySource CreateEntitySourceWithoutEntity(ResourceName parentPrefab = string.Empty)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		bool manageEditAction = BeginEntityAction();

		IEntitySource entitySource = CreateEntitySource(parentPrefab);
		SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);

		EndEntityAction(manageEditAction);

		return entitySource;
	}

	//------------------------------------------------------------------------------------------------
	//! Create an entity from an XOB and return an IEntitySource
	//! Keeps the entity in the world, does NOT delete the entity itself
	//  protected for now
	//! \param xob path of the XOB to import
	//! \param parentPrefab if not provided, GenericEntity is used
	//! \return created entity's IEntitySource
	protected static IEntitySource CreateEntitySourceFromXOB(ResourceName xob, ResourceName parentPrefab = string.Empty)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(xob))
		{
			Print("Provided XOB is empty", LogLevel.ERROR);
			return null;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return null;
		}

		string xobName = FilePath.StripPath(xob.GetPath());

		bool manageEditAction = BeginEntityAction();

		IEntitySource entitySource = CreateEntitySource(parentPrefab); // do NOT switch to CreateEntitySourceWithoutEntity!
		if (!entitySource)
		{
			Print("Prefab's entity source could not be created for xob " + xobName, LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		// if the MeshObject component doesn't exist, create it
		if (!CreateEntitySourceComponentIfNeeded(entitySource, MESHOBJECT_CLASSNAME))
		{
			Print("Could not add MeshObject component", LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		// apply XOB path to mesh
		if (!worldEditorAPI.SetVariableValue(entitySource, { new ContainerIdPathEntry(MESHOBJECT_CLASSNAME) }, "Object", xob))
		{
			Print("Could not apply XOB model to IEntitySource for xob " + xobName, LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		EndEntityAction(manageEditAction);

		return entitySource;
	}

	//------------------------------------------------------------------------------------------------
	//! Create and add a Component to the provided entitySource if not present
	//! \param entitySource
	//! \param componentClassname
	//! \return found or created component, null on error
	static IEntityComponentSource CreateEntitySourceComponentIfNeeded(notnull IEntitySource entitySource, string componentClassname)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(componentClassname))
		{
			Print("Provided componentClassname is empty", LogLevel.WARNING);
			return null;
		}

		IEntityComponentSource result = SCR_BaseContainerTools.FindComponentSource(entitySource, componentClassname);
		if (result)
			return result;

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		bool manageEditAction = BeginEntityAction();

		result = worldEditorAPI.CreateComponent(entitySource, componentClassname);
		if (!result)
			Print("Could not add the " + MESHOBJECT_CLASSNAME + " component to the Prefab", LogLevel.ERROR);

		EndEntityAction(manageEditAction);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the absolute file path of the provided resourceName
	//! \param resourceName the resourceName from which to obtain the absolute file path
	//! \param mustExist if true, the file MUST exist to return a valid value
	//! \return the provided resourceName's absolute file path or string.Empty on error / file not existing with mustExist true
	static string GetResourceNameAbsolutePath(ResourceName resourceName, bool mustExist = true)
	{
		string result;
		if (!Workbench.GetAbsolutePath(resourceName.GetPath(), result, mustExist))
			return string.Empty;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Gives the target's directory the source directory's hierarchy/path
	//! Relative paths are expected, so separator must be slash (/)
	//! @code
	//! string relativeSourceDirectory = "Assets/Rocks/Granite";
	//! string relativeTargetDirectory = "Prefabs/Miniatures";
	//! string result = GetRelativeParentDirectory(relativeSourceDirectory, relativeTargetDirectory);
	//! Print(result); // Prefabs/Miniatures/Rocks/Granite
	//! @endcode
	//! See SCR_PrefabHelper.FormatRelativePath
	//! \param relativeSourceDirectory the relative source directory, e.g Assets/Rocks/Granite
	//! \param relativeTargetDirectory the relative target directory, e.g Prefabs/Miniatures
	//! \return target relative directory without leading or trailing slash, e.g Prefabs/Miniatures/Rocks/Granite
	static string GetRelativeParentDirectory(string relativeSourceDirectory, string relativeTargetDirectory)
	{
		if (relativeSourceDirectory.IsEmpty())
			return FormatRelativePath(relativeTargetDirectory);

		if (relativeTargetDirectory.IsEmpty())
			return FormatRelativePath(relativeSourceDirectory);

		relativeSourceDirectory = FormatRelativePath(relativeSourceDirectory);
		relativeTargetDirectory = FormatRelativePath(relativeTargetDirectory);

		// both are root directories
		if (!relativeSourceDirectory.Contains("/") && !relativeTargetDirectory.Contains("/"))
			return relativeTargetDirectory;

		bool slashFound;
		string path;
		for (int i, length = relativeSourceDirectory.Length(); i < length; i++)
		{
			string character = relativeSourceDirectory[i];
			if (!slashFound)
			{
				if (character == "/")
					slashFound = true;
			}
			else
			{
				path += character;
			}
		}

		if (!slashFound)
			path = relativeSourceDirectory;

		return relativeTargetDirectory + "/" + path;
	}

	//------------------------------------------------------------------------------------------------
	//! Format provided relative path, removing leading and trailing slashes, replacing antislashes by slashes, etc.
	//! \param relativePath the relative path
	//! \return formatted relativePath
	protected static string FormatRelativePath(string relativePath)
	{
		if (relativePath.IsEmpty())
			return string.Empty;

		relativePath.Replace("\\", "/");
		relativePath.Replace("/" + "/", "/");

		if (relativePath.StartsWith("/"))
			relativePath = relativePath.Substring(1, relativePath.Length() - 1);

		if (relativePath.EndsWith("/"))
			relativePath = relativePath.Substring(0, relativePath.Length() - 1);

		return relativePath;
	}

	//------------------------------------------------------------------------------------------------
	//! Save a Prefab from the provided IEntitySource, with a _base Prefab or not
	//! Destination directory is created if it does not exist
	//! Keeps the entity in the world, does NOT delete the entity itself
	//! \param absoluteDirPath a trailing '/' is automatically added if missing
	//! \param prefabFileName a trailing '.et' is automatically added if missing
	//! \return ResourceName, empty on error
	static ResourceName SaveEntitySourceAsPrefab(notnull IEntitySource entitySource, string absoluteDirPath, string prefabFileName, bool createBasePrefab = false)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteDirPath))
		{
			Print("Provided absoluteDirPath is empty", LogLevel.ERROR);
			return string.Empty;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefabFileName))
		{
			Print("Provided prefabFileName is empty", LogLevel.ERROR);
			return string.Empty;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return string.Empty;
		}

		if (!absoluteDirPath.EndsWith("/"))
			absoluteDirPath += "/";

		if (!FileIO.FileExists(absoluteDirPath) && !SCR_FileIOHelper.CreateDirectory(absoluteDirPath))
		{
			Print("Could not create the destination directory (" + absoluteDirPath + ")", LogLevel.ERROR);
			return string.Empty;
		}

		if (!prefabFileName.EndsWith(PREFAB_DOTTED_EXTENSION))
			prefabFileName += PREFAB_DOTTED_EXTENSION;

		string absoluteFilePath = absoluteDirPath + prefabFileName;
		MetaFile metaFile;

		if (createBasePrefab)
		{
			string basePrefabFileName = FilePath.StripExtension(prefabFileName) + PREFAB_BASE_SUFFIX + PREFAB_DOTTED_EXTENSION;
			string absoluteBaseFilePath = absoluteDirPath + basePrefabFileName;

			// save base as Prefab
			if (!worldEditorAPI.CreateEntityTemplate(entitySource, absoluteBaseFilePath))
			{
				Print("Could not save base prefab " + basePrefabFileName + " at " + absoluteBaseFilePath, LogLevel.WARNING);
				return string.Empty;
			}

			metaFile = SCR_WorldEditorToolHelper.GetResourceManager().GetMetaFile(absoluteBaseFilePath);
			if (!metaFile)
			{
				Print("Created base Prefab's meta file could not be found", LogLevel.WARNING);
				return string.Empty;
			}

			// base prefab has been created, now let's create the inheriting child prefab

			entitySource = CreateEntitySourceWithoutEntity(metaFile.GetResourceID());
			if (!entitySource)
			{
				Print("Prefab's child entity source could not be created", LogLevel.ERROR);
				return string.Empty;
			}
		}

		// save as Prefab
		if (!worldEditorAPI.CreateEntityTemplate(entitySource, absoluteFilePath))
		{
			Print("Could not save prefab " + prefabFileName + " at " + absoluteFilePath, LogLevel.WARNING);
			return string.Empty;
		}

		metaFile = SCR_WorldEditorToolHelper.GetResourceManager().GetMetaFile(absoluteFilePath);
		if (!metaFile)
		{
			Print("Created Prefab's meta file could not be found", LogLevel.WARNING);
			return string.Empty;
		}

		return metaFile.GetResourceID();
	}

	//------------------------------------------------------------------------------------------------
	//! Update a Prefab
	//! \param actualPrefab the spawned Prefab's IEntitySource ancestor
	//! \return true on success, false on failure
	static bool UpdatePrefab(IEntitySource actualPrefab)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return false;
		}

		return worldEditorAPI.SaveEntityTemplate(actualPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Begin an Entity Action in World Editor API if required
	//! \return true if a BeginEntityAction has been called and EndEntityAction has to be called by EndEntityAction(), false otherwise
	protected static bool BeginEntityAction()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
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
	protected static void EndEntityAction(bool manageEditAction)
	{
		if (manageEditAction)
			SCR_WorldEditorToolHelper.GetWorldEditorAPI().EndEntityAction();
	}

};
#endif
