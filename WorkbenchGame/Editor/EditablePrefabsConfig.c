[BaseContainerProps(configRoot: true)]
class EditablePrefabsConfig
{
	const string META_EXTENSION = ".meta";
	const string COPY_EXTENSION = "_copy";
	
	[Attribute(defvalue: "Prefabs", desc: "Source directory where all prefabs are placed.", params: "unregFolders")]
	private ResourceName m_SourceDirectory;
	
	[Attribute(defvalue: "PrefabsEditable", desc: "Target directory where editable sub-prefabs will be crated.\nFolder structure inside will mimic the structure in the source directory.", params: "folders")]
	private ResourceName m_TargetDirectory;
	
	[Attribute(defvalue: "Auto", desc: "")]
	private string m_sAutoFolderName;
	
	[Attribute(defvalue: "E_", desc: "Prefix added before file name of each prefab (i.e., editable entity prefab of 'Car.et' will be 'E_Car.et')")]
	private string m_sFileNamePrefix;
	
	[Attribute(defvalue: "", desc: "Ignore file names containing this string. To define mutliple, separate them by a comma (without a space afterwards).")]
	private string m_sSourceBlacklist;
	
	[Attribute(desc: "Entities that will be created in temporary world to ensure correct functionality (e.g., AIWorld for AI groups)", params: "et")]
	protected ref array<ResourceName> m_SupportEntities;
	
	[Attribute()]
	private ref array<ref EditablePrefabsSetting> m_Settings;
	
	protected bool m_bIsValid;
	protected string m_sSourcePath;
	protected string m_sTargetPath;
	protected string m_sTargetPathAuto;
	protected ref map<string, string> m_LinksFromTarget;
	protected ref map<string, string> m_LinksFromSource;
	protected ref array<string> m_aSourceBlacklist = new array<string>;
	protected string m_sCurrentAddon = "$ArmaReforger:";
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Prefab management
	/*!
	Create or update editable entity prefab
	\param api World Editor API
	\param prefab Source prefab
	*/
	void CreateEditablePrefab(WorldEditorAPI api, ResourceName prefab, bool onlyUpdate = false, map<string, EEditablePrefabResult> results = null)
	{
		if (prefab.IsEmpty()) return;
		
		if (IsBlacklisted(prefab))
		{
			if (results) results.Set(string.Format("@\"%1\"", prefab.GetPath()), EEditablePrefabResult.IGNORED);
			return;
		}
		
		if (!m_LinksFromSource) GetLinks();
		
		string addonName = SCR_AddonTool.GetResourceLastAddon(prefab);
		m_sCurrentAddon = SCR_AddonTool.ToFileSystem(addonName);
		
		bool exists = false;
		string prefabPath = prefab.GetPath();
		if (prefabPath.StartsWith(m_sTargetPath))
		{
			//--- From target
			if (prefabPath.StartsWith(m_sTargetPathAuto))
			{
				//--- Update prefab in target folder
				prefab = GetSourcePrefab(prefab);
				if (prefab.IsEmpty()) return;
				exists = true;
			}
			else
			{
				///--- Not in 'Auto' folder
				Print(string.Format("Cannot update editable prefab @\"%1\", it's not auto-generated (i.e., not in '%2' folder)!", prefabPath, m_sTargetPathAuto), LogLevel.WARNING);
				return;
			}
		}
		else
		{
			//--- From source
			Resource prefabResource = Resource.Load(prefab);
			if (prefabResource && SCR_EditableEntityComponentClass.GetEditableEntitySource(prefabResource))
			{
				//--- Is already configured as editable entity - proceed, but only update components
				UpdateEditablePrefab(api, prefab);
				return;
			}
			else
			{
				//--- Not an editable entity, find its editable variant
				ResourceName existingPrefab;
				exists = GetLinkFromSource(GetGUID(prefab), existingPrefab);
				
				if (existingPrefab.IsEmpty())
				{
					//--- Create
					if (onlyUpdate) return;
					string prefabName = FilePath.StripPath(prefabPath);
					prefabPath.Replace(prefabName, m_sFileNamePrefix + prefabName);
					prefabPath.Replace(m_sSourcePath, m_sTargetPathAuto);
				}
				else
				{
					//--- Update
					prefabPath = existingPrefab.GetPath();
				}
			}
		}
		
		//--- Create prefab directory
		string directoryPath = FilePath.StripFileName(prefabPath);
		if (!CreateDirectoryFor(directoryPath)) return;
		
		IEntity entity = api.CreateEntity(prefab, "", api.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		if (!entity)
		{
			Print(string.Format("Unable to create entity %1!", prefab), LogLevel.ERROR);
			return;
		}
		IEntitySource entitySource = api.EntityToSource(entity);
		
		//--- Remove coordinates which were added when the entity was spawned
		entitySource.ClearVariable("coords");
		
		//--- Reset placement type, e.g., "slopelandcintact". Would clash with editor positioning on clients
		api.SetVariableValue(entitySource, {}, "placement", "none");
		
		//--- Update source composition
		bool needSave;
		bool editableChildren = true;
		IEntityComponentSource compositionComponentSource = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_SlotCompositionComponent);
		if (compositionComponentSource)
		{
			entity = api.SourceToEntity(entitySource);
			compositionComponentSource.Get("m_bEditableChildren", editableChildren);
			SCR_SlotCompositionComponent compositionComponent = SCR_SlotCompositionComponent.Cast(entity.FindComponent(SCR_SlotCompositionComponent));
			if (compositionComponent)
			{
#ifdef WORKBENCH
				needSave = compositionComponent._WB_ConfigureComposition(api, entitySource.GetAncestor());
#endif
			}
		}
		
		//--- Get existing target prefab
		IEntitySource currentEntitySource;
		Resource currentResource = Resource.Load(prefabPath);
		if (currentResource && currentResource.IsValid())
			currentEntitySource = currentResource.GetResource().ToEntitySource();
		
		//--- Update children
		array<ref SCR_EditorLinkEntry> links = new array<ref SCR_EditorLinkEntry>();
		if (editableChildren && !UpdateChildPrefabs(api, entitySource, links, results: results))
		{
			if (results) results.Set(string.Format("@\"%1\"", prefab.GetPath()), EEditablePrefabResult.FAILED);
			//Print(string.Format("Editable entity creation FAILED: from @\"%1\"", prefab.GetPath()), LogLevel.WARNING);
			api.DeleteEntity(entity);
			entitySource.Release(true);
			return;
		}
		
		//--- Add link component
		if (links.Count() != 0)
		{
			IEntityComponentSource linkComponent = api.CreateComponent(entitySource, "SCR_EditorLinkComponent");
			foreach (int i, SCR_EditorLinkEntry entry: links)
			{
				if (api.CreateObjectArrayVariableMember(entitySource, {ContainerIdPathEntry("SCR_EditorLinkComponent")}, "m_aEntries", "SCR_EditorLinkEntry", i))
				{
					array<ref ContainerIdPathEntry> entryPath = {ContainerIdPathEntry("SCR_EditorLinkComponent"), ContainerIdPathEntry("m_aEntries", i)};
					api.SetVariableValue(entitySource, entryPath, "m_Prefab", entry.m_Prefab);
					api.SetVariableValue(entitySource, entryPath, "m_vPosition", entry.m_vPosition.ToString(false));
					api.SetVariableValue(entitySource, entryPath, "m_vAngles", entry.m_vAngles.ToString(false));
					api.SetVariableValue(entitySource, entryPath, "m_fScale", entry.m_fScale.ToString());
				}
			}
			//api.SetVariableValue(entitySource, {ContainerIdPathEntry("SCR_EditorLinkComponent"), ContainerIdPathEntry("m_aEntries")}, "Name", string.Format(m_sNameFormat, prefabName));
		}
		
		//--- Add components
		EditablePrefabsSetting settings = GetSettings(prefabPath);
		if (settings) settings.AddComponents(this, api, prefab, prefabPath, entitySource, currentEntitySource, entitySource);
		
		//--- Create prefab file
		string absolutePath;
		Workbench.GetAbsolutePath(m_sCurrentAddon + prefabPath, absolutePath, false);
		
		//--- Back up prefab and meta files
		string metaPath = prefabPath + META_EXTENSION;
		string metaPathCopy = metaPath + COPY_EXTENSION;
		array<string> backup = new array<string>;
		if (exists)
		{
			BackupFile(prefabPath, backup);
			FileIO.CopyFile(metaPath, metaPathCopy);
		}
		
		//--- Create new prefab
		api.CreateEntityTemplate(entitySource, absolutePath);
		
		//--- Restore meta file (prefab creation overwrites it) and supress 'dummy' changes of component prefabs GUIDs
		if (exists)
		{
			CompareBackup(prefabPath, backup);
			FileIO.CopyFile(metaPathCopy, metaPath);
			FileIO.DeleteFile(metaPathCopy);
		}
		
		//--- Save changes in the original entity
		if (needSave)
			Workbench.GetModule(WorldEditor).Save();
		
		api.DeleteEntity(api.SourceToEntity(entitySource));
		entitySource.Release(true);
		
		if (exists)
		{
			if (results) results.Set(string.Format("@\"%1\" from @\"%2\"", prefabPath, prefab.GetPath()), EEditablePrefabResult.UPDATED);
			//Print(string.Format("Editable entity UPDATED: @\"%1\" from @\"%2\"", prefabPath, prefab.GetPath()), LogLevel.DEBUG);
		}
		else
		{
			if (results) results.Set(string.Format("@\"%1\" from @\"%2\"", prefabPath, prefab.GetPath()), EEditablePrefabResult.CREATED);
			//Print(string.Format("Editable entity ADDED: @\"%1\" from @\"%1\"", prefabPath, prefab.GetPath()), LogLevel.DEBUG);
		}
	}
	void UpdateEditablePrefab(WorldEditorAPI api, ResourceName prefab)
	{
		IEntity entity = api.CreateEntity(prefab, "", api.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		IEntitySource entitySource = api.EntityToSource(entity);
		IEntitySource prefabSource = entitySource.GetAncestor();
		if (!prefabSource) return;
		
		string prefabPath = prefab.GetPath();
		EditablePrefabsSetting settings = GetSettings(prefabPath);
		if (settings) settings.AddComponents(this, api, prefab, prefabPath, prefabSource, prefabSource, entitySource);
		
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		worldEditor.Save();
		
		api.DeleteEntity(api.SourceToEntity(entitySource));
		entitySource.Release(true);
		
		Print(string.Format("Editable entity UPDATED: @\"%1\"", prefabPath), LogLevel.DEBUG);
	}
	string VerifyEditablePrefab(WorldEditorAPI api, ResourceName prefab, bool onlyFileChanges = false)
	{
		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource) return string.Empty;
		
		BaseResourceObject prefabBase = prefabResource.GetResource();
		if (!prefabBase) return string.Empty;
		
		IEntitySource prefabEntity = prefabBase.ToEntitySource();
		if (!prefabEntity) return string.Empty;
		
		string prefabPath = prefab.GetPath();
		IEntitySource ancestorEntity = prefabEntity.GetAncestor();
		if (ancestorEntity)
		{			
			string correctPath = ancestorEntity.GetResourceName().GetPath();
			correctPath.Replace(m_sSourcePath, m_sTargetPathAuto);
			
			string correctName = FilePath.StripPath(correctPath);
			correctPath.Replace(correctName, m_sFileNamePrefix + correctName);
			
			if (prefabPath != correctPath)
			{
				//--- Source name changed, rename the prefab as well
				prefab = MoveEditablePrefab(api, prefabPath, correctPath);
			}
			
			//--- Update prefab from the source
			if (!onlyFileChanges) CreateEditablePrefab(api, prefab);
			return correctPath;
		}
		else
		{
			//--- Source deleted, delete the prefab as well
			DeleteEditablePrefab(api, prefabPath);
		}
		return string.Empty;
	}
	/*!
	Move existing editable prefab to a new path (includes also renaming the prefab)
	\param prevPath Current path
	\param newPath New path
	\return New path with GUID
	*/
	protected ResourceName MoveEditablePrefab(WorldEditorAPI api, string prevPath, string newPath)
	{
		if (!MoveFile(prevPath, newPath)) return ResourceName.Empty;
		
		EditablePrefabsSetting settings = GetSettings(newPath);
		if (settings) settings.Move(this, api, prevPath, newPath);
		
		GetLinks(true);
		
		string absPath;
		Workbench.GetAbsolutePath(newPath, absPath);
		if (absPath.IsEmpty())
		{
			Print(string.Format("Cannot MOVE '%1', unable to find absolute path for '%2'!", prevPath, newPath), LogLevel.ERROR);
			return ResourceName.Empty;
		}
		
		Print(string.Format("Editable entity MOVED:\n      Old: '%1'\n      New: '%2'", prevPath, newPath), LogLevel.WARNING);
		
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.RegisterResourceFile(absPath);
		return meta.GetResourceID();
	}
	/*!
	Delete existing editable prefab
	*/
	protected void DeleteEditablePrefab(WorldEditorAPI api, string prefabPath)
	{
		FileIO.DeleteFile(prefabPath);
		FileIO.DeleteFile(prefabPath + META_EXTENSION);
		
		EditablePrefabsSetting settings = GetSettings(prefabPath);
		if (settings) settings.Delete(this, api, prefabPath);
		
		Print(string.Format("Editable entity DELETED '%1' (its source no longer exists)!", prefabPath), LogLevel.WARNING);
	}
	/*!
	Change all child prefabs to their editable variants.
	\param prefab Editable entity prefab
	\param toEditable True to convert sourc entities to editable, false to do it the other way around
	* /
	void UpdateEditableChildPrefabs(ResourceName prefab, bool toEditable = true)
	{
		string prefabPath = prefab.GetPath();
		if (!FileIO.FileExist(prefabPath)) return;
	
		Print(string.Format("--- Processing prefab '%1'", prefab), LogLevel.DEBUG);
		
		//--- Replace prefab paths directly in the file (there is no API to do it 'cleanly')
		FileHandle file = FileIO.OpenFile(prefabPath, FileMode.READ);
		
		string text, line, substring, replacement;
		int n = 0;
		int index, length;
		bool canReplace;
		while (file.ReadLine(line) > 0)
		{
			if (line.Contains(".et\" {"))
			{
				index = line.IndexOf("{") + 1;
				length = line.LastIndexOf("}") - index;
				substring = line.Substring(index, length);
				
				if (toEditable)
				{
					canReplace = GetLinkFromSource(substring, replacement);
				}
				else
				{
					canReplace = GetLinkFromTarget(substring, replacement);
				}
					
				if (canReplace)
				{
					index = line.IndexOf("\"") + 1;
					length = line.LastIndexOf("\"") - index;
					substring = line.Substring(index, length);
					line.Replace(substring, replacement);
					Print(string.Format("'%1' converted to '%2'", substring, replacement), LogLevel.DEBUG);
				}
				else
				{
					if (toEditable)
					{
						canReplace = GetLinkFromTarget(substring, replacement);
					}
					else
					{
						canReplace = GetLinkFromSource(substring, replacement);
					}
					
					if (!canReplace && n != 0)
					{
						index = line.IndexOf("\"") + 1;
						length = line.LastIndexOf("\"") - index;
						substring = line.Substring(index, length);
						Print(string.Format("Cannot convert '%1', matching prefab not found!", substring), LogLevel.WARNING);
					}
				}
			}
			
			if (n != 0) text += "\n";
			text += line;
			n++;
		}
		file.Close();
		
		file = FileIO.OpenFile(prefabPath, FileMode.WRITE);
		FPrint(file, text);
		file.Close();
	}
	/*!
	Move a file and its meta file from one path to another
	\param prevPath Current path (without GUIDs)
	\param newPath New path (without GUIDs)
	*/
	bool MoveFile(string prevPath, string newPath)
	{
		if (prevPath == newPath)
		{
			Print(string.Format("Cannot MOVE '%1', target path is the same as current path!", prevPath), LogLevel.ERROR);
			return false;
		}
		if (newPath.IsEmpty())
		{
			Print(string.Format("Cannot MOVE '%1', target path is empty!", prevPath), LogLevel.ERROR);
			return false;
		}
			
		string newDirectory = FilePath.StripFileName(newPath);
		if (!CreateDirectoryFor(newDirectory)) return false;
		
		//--- Copy prefab to new directory and delete the old one
		FileIO.CopyFile(prevPath, newPath);
		FileIO.DeleteFile(prevPath);
		
		//--- Copy meta file (use intermediary file to prevent warning about duplicate GUID)
		string prevMetaPath = prevPath + META_EXTENSION;
		string newMetaPath = newPath + META_EXTENSION;
		if (FileIO.FileExists(prevMetaPath))
		{
			FileIO.CopyFile(prevMetaPath, newMetaPath + COPY_EXTENSION);
			FileIO.DeleteFile(prevMetaPath);
			FileIO.CopyFile(newMetaPath + COPY_EXTENSION, newMetaPath);
			FileIO.DeleteFile(newMetaPath + COPY_EXTENSION);
		}
		
		return true;
	}
	/*!
	Print auto-configuration results to log.
	\param results map of confired prefabs with their results
	*/
	void LogResults(map<string, EEditablePrefabResult> results)
	{
		map<EEditablePrefabResult, LogLevel> logLevels = new map<EEditablePrefabResult, LogLevel>;
		logLevels.Insert(EEditablePrefabResult.FAILED, LogLevel.ERROR);
		logLevels.Insert(EEditablePrefabResult.CREATED, LogLevel.DEBUG);
		logLevels.Insert(EEditablePrefabResult.UPDATED, LogLevel.DEBUG);
		logLevels.Insert(EEditablePrefabResult.MOVED, LogLevel.DEBUG);
		logLevels.Insert(EEditablePrefabResult.DELETED, LogLevel.WARNING);
		logLevels.Insert(EEditablePrefabResult.NON_EDITABLE, LogLevel.VERBOSE);
		logLevels.Insert(EEditablePrefabResult.IGNORED, LogLevel.VERBOSE);
			
		typename resultTypeName = EEditablePrefabResult;
		EEditablePrefabResult resultQueried;
		string resultName;
		LogLevel resultLogLevel;
		for (int i = 0, count = resultTypeName.GetVariableCount(); i < count; i++)
		{
			resultTypeName.GetVariableValue(NULL, i, resultQueried);
			resultName = resultTypeName.GetVariableName(i);
			resultLogLevel = logLevels.Get(resultQueried);
			
			foreach (string resourceName, EEditablePrefabResult result: results)
			{
				if (result == resultQueried) Print(string.Format("Editable Entity %1: %2", resultName, resourceName), resultLogLevel);
			}
		}
	}
		
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Support functions
	protected bool UpdateChildPrefabs(WorldEditorAPI api, IEntitySource entitySource, array<ref SCR_EditorLinkEntry> links, bool forceDisable = false, map<string, EEditablePrefabResult> results = null, int depth = 0)
	{
		ResourceName ancestor, prefabEditable;
		IEntitySource child;
		BaseContainer ancestorSource;
		bool prefabFound, sourcePrefabFound;
		bool isValid = true;
		for (int e = 0, childrenCount = entitySource.GetNumChildren(); e < childrenCount; e++)
		{
			child = entitySource.GetChild(e);
			prefabFound = false;
			sourcePrefabFound = false;
			bool forceDisableChildren = forceDisable;
			
			if (forceDisable)
			{
				//--- Disable, one of the parents is a link
				api.SetVariableValue(child, {}, "Flags", EntityFlags.EDITOR_ONLY.ToString());
			}
			else
			{
				//--- Find editable prefab variant
				ancestorSource = child.GetAncestor();
				while (ancestorSource)
				{
					ancestor = ancestorSource.GetResourceName();
					if (ancestor.Contains("/"))
					{
						ancestor = Workbench.GetResourceName(ancestor);
							
						if (SCR_BaseContainerTools.FindComponentSource(child, SCR_EditableEntityComponent))
						{
							prefabFound = true;
							prefabEditable = ancestor;
						}
						else
						{
							prefabFound = GetLinkFromSource(GetGUID(ancestor), prefabEditable);
							sourcePrefabFound = GetLinkFromTarget(GetGUID(ancestor));
						}
						break;
					}	
					ancestorSource = ancestorSource.GetAncestor();
				}
				
				if (prefabFound)
				{
					//--- Editable prefab - add a source link	
					vector position;
					child.Get("coords", position);
						
					float angleX, angleY, angleZ;
					child.Get("angleX", angleX);
					child.Get("angleY", angleY);
					child.Get("angleZ", angleZ);
					
					float scale;
					child.Get("scale", scale);
					
					//--- Add to the list of links
					links.Insert(new SCR_EditorLinkEntry(prefabEditable, position, Vector(angleX, angleY, angleZ), scale));
					
					//--- Mark the original entity as not-editable (override existing flags)
					api.SetVariableValue(child, {}, "Flags", EntityFlags.EDITOR_ONLY.ToString());
					forceDisableChildren = true; //--- Disable all children
				}
				else if (sourcePrefabFound)
				{
					//--- ERROR: Is editable entity
					Print(string.Format("Child entity of type '%1' is already auto-generated! Source compositions must not contain any editable prefabs from '%2/%3'!", GetResourceNameLink(child), m_TargetDirectory, m_sAutoFolderName), LogLevel.WARNING);
					isValid = false;
				}
				/*else if (SCR_BaseContainerTools.FindComponentSource(child, RplComponent))
				{
					//--- ERROR: Contains RplComponent
					Print(string.Format("Child entity '%1' contains RplComponent, but does not have editable variant! Children must not be replicated!", GetResourceNameLink(child)), LogLevel.WARNING);
					isValid = false;
				}*/
				else
				{
					if (results) results.Set(GetResourceNameLink(child), EEditablePrefabResult.NON_EDITABLE);
					if (!SCR_BaseContainerTools.FindComponentSource(child, "Hierarchy"))
					{
						//--- Non-editable prefab or no prefab at all - add to hierarchy
						api.CreateComponent(child, "Hierarchy");
					}
				}
			}
				
			//--- We need to go deeper
			isValid &= UpdateChildPrefabs(api, child, links, forceDisableChildren, results, depth + 1);
		}
		return isValid;
	}
	protected void DisableComponent(WorldEditorAPI api, IEntitySource entitySource, BaseContainer componentSource, array<ref ContainerIdPathEntry> componentsPath)
	{
		if (!componentSource) return;
			
		array<ref ContainerIdPathEntry> componentsPathNew = new array<ref ContainerIdPathEntry>;
		foreach (ContainerIdPathEntry entry: componentsPath) componentsPathNew.Insert(entry);
		componentsPathNew.Insert(new ContainerIdPathEntry(componentSource.GetClassName()));
			
		if (componentSource.GetClassName() != "Hierarchy") //--- Don't disable hierarchy, dummy entities should still be tied to their parent
			api.SetVariableValue(entitySource, componentsPathNew, "Enabled", "0");
		
		BaseContainerList components = componentSource.GetObjectArray("components");
		if (components)
		{
			int componentsCount = components.Count();
			for (int i = 0; i < componentsCount; i++)
			{
				DisableComponent(api, entitySource, components.Get(i), componentsPathNew);
			}
		}
	}
	protected void BackupFile(string filePath, out notnull array<string> backup)
	{
		FileHandle file = FileIO.OpenFile(filePath, FileMode.READ);
		string line;
		while (file.ReadLine(line) > 0)
		{
			backup.Insert(line);
		}
		file.Close();
	}
	protected void CompareBackup(string filePath, notnull array<string> backup)
	{
		FileHandle file = FileIO.OpenFile(filePath, FileMode.READ);
		string line, text;
		int i = 0;
		int backupCount = backup.Count();
		bool apply = false;
		while (file.ReadLine(line) > 0)
		{
			if (i >= backupCount)
			{
				apply = false;
				break;
			}
			
			if (line != backup[i])
			{
				if (line.Contains(".ct"))
				{
					//--- Component GUID updated, restore the original to prevent false-flag SVN diffs
					line = backup[i];
					apply = true;
				}
				else
				{
					//--- Major file change, don't restore the backup
					apply = false;
					break;
				}
			}
			if (i != 0) text += "\n";
			text += line;
			i++;
		}
		file.Close();
		
		if (apply)
		{
			file = FileIO.OpenFile(filePath, FileMode.WRITE);
			file.Write(text);
			file.Close();
		}
	}
	protected ResourceName GetResourceNameLink(BaseContainer container)
	{
		string resourceName = container.GetClassName();
		while (container)
		{
			if (container.GetResourceName().Contains("/"))
			{
				resourceName = string.Format("@\"%1\"", container.GetResourceName().GetPath());
				break;
			}
			container = container.GetAncestor();
		}
		return resourceName;
	}
	protected string GetGUID(ResourceName prefab)
	{
		int index = prefab.IndexOf("}");
		if (index == -1) return ResourceName.Empty;
		return prefab.Substring(1, index - 1);
	}
	protected EditablePrefabsSetting GetSettings(string prefabPath)
	{
		EditablePrefabsSetting settings = null;
		int pathLengthMax = -1;
		foreach (EditablePrefabsSetting settingsTemp: m_Settings)
		{
			string targetPath = settingsTemp.GetTarget();
			int pathLength = targetPath.Length();
			if (prefabPath.StartsWith(targetPath) && pathLength > pathLengthMax)
			{
				settings = settingsTemp;
				pathLengthMax = pathLength;
			}
		}
		return settings;
	}
	protected bool IsBlacklisted(string path)
	{
		foreach (string entry: m_aSourceBlacklist)
		{
			if (path.Contains(entry))
				return true;
		}
		return false;
	}
		
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Attribute getters
	/*!
	Get source path
	\return Path without GUID
	*/
	string GetSourcePath()
	{
		return m_sSourcePath;
	}
	/*!
	Get raw target path
	\return Path without GUID
	*/
	string GetTargetPath()
	{
		return m_sTargetPath;
	}
	/*!
	Get path of auto folder in target path
	\return Path without GUID
	*/
	string GetTargetPathAuto()
	{
		return m_sTargetPathAuto;
	}
	/*!
	Get prefix added to editable entity prefabs
	\return Prefix
	*/
	string GetPrefix()
	{
		return m_sFileNamePrefix;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- General getters
	/*!
	Get config from a prefab
	\param configPath Prefab
	\return Config
	*/
	static EditablePrefabsConfig GetConfig(ResourceName configPath)
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
		
		if (configBase.GetClassName() != "EditablePrefabsConfig")
		{
			Print(string.Format("Config '%1' is of type '%2', must be 'EditablePrefabsConfig'!", configPath, configBase.GetClassName()), LogLevel.ERROR);
			return null;
		}
				
		return EditablePrefabsConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configBase));
	}
	/*!
	Check if the config is configured correctly
	\return True if valid
	*/
	bool IsValid()
	{
		return m_bIsValid;
	}
	/*!
	Create a directory for editable entity prefab on given path
	\patam filePath Prefab path
	\return True if created
	*/
	bool CreateDirectoryFor(out string filePath, string addon = "")
	{	
		array<string> directories = new array<string>;
		filePath.Split("/", directories, true);
	
		int directoriesCount = directories.Count();
		if (directoriesCount == 0) return false;
	
		if(addon.IsEmpty())
			addon = m_sCurrentAddon;
		filePath = addon;
		for (int i = 0; i < directoriesCount; i++)
		{
			filePath = FilePath.Concat(filePath, directories[i]);
		
			if (!FileIO.MakeDirectory(filePath))
			{
				Print(string.Format("Unable to create directory '%1'!", filePath), LogLevel.ERROR);
				return false;
			}
		}
		return true;
	}
	/*!
	Create empty world which can be safely edited.
	*/
	bool CreateWorld()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor) return false;
		
		WorldEditorAPI api = worldEditor.GetApi();
		if (!api) return false;
		
		string targetPath = ".worlds/EditablePrefabsWorld";
		
		//--- World already opened, skip
		string currentWorld;
		api.GetWorldPath(currentWorld);
		if (currentWorld.Contains(targetPath))return true;
		
		string worldPath = "$profile:" + targetPath + ".ent";
		string layerPath = "$profile:" + targetPath + "_default.layer";
			
		//--- World file
		if (!FileIO.FileExists(worldPath))
		{
			FileIO.MakeDirectory("$profile:.worlds");
			FileHandle file = FileIO.OpenFile(worldPath, FileMode.WRITE);
			file.WriteLine("Layer default {");
			file.WriteLine(" Index 0");
			file.WriteLine("}");
			file.Close();
		}
		
		//--- Layer file
		//if (!FileIO.FileExist(layerPath)) //--- Always rewrite it
		//{
			FileIO.MakeDirectory("$profile:.worlds");
			FileHandle file = FileIO.OpenFile(layerPath, FileMode.WRITE);
			file.WriteLine("GenericWorldEntity world {");
			file.WriteLine("}");
		
			foreach (ResourceName prefab: m_SupportEntities)
			{
				file.WriteLine(string.Format("%1 : \"%2\" {", SCR_BaseContainerTools.GetContainerClassName(prefab), prefab));
				file.WriteLine("}");
			}
		
			file.Close();
		//}
			
		return Workbench.GetModule(WorldEditor).SetOpenedResource(worldPath);
	}
	/*!
	Initialize links to and from editable prefabs
	*/
	void GetLinks(bool forced = false)
	{
		//--- Already defined
		if (!forced && m_LinksFromTarget && m_LinksFromSource) return;
		
		m_LinksFromTarget = new map<string, string>;
		m_LinksFromSource = new map<string, string>;
		
		ref array<ResourceName> resources = new array<ResourceName>;
		Workbench.SearchResources(resources.Insert, {"et"});
		
		string prefabPath, prefabGUID, ancestorGUID;
		ResourceName ancestor;
		Resource prefabResource;
		IEntitySource entitySource, ancestorSource;
		foreach (ResourceName prefab: resources)
		{
			prefabPath = prefab.GetPath();
			if (!prefabPath.StartsWith(m_sTargetPathAuto)) continue;
			
			prefabResource = Resource.Load(prefab);
			if (!prefabResource || !prefabResource.IsValid()) continue;
			
			entitySource = SCR_BaseContainerTools.FindEntitySource(prefabResource);
			if (!entitySource) continue;
			
			ancestorSource = entitySource.GetAncestor();
			if (!ancestorSource) continue;
			
			ancestor = ancestorSource.GetResourceName();
		
			prefabGUID = GetGUID(prefab);
			ancestorGUID = GetGUID(ancestor);
			
			m_LinksFromTarget.Insert(prefabGUID, ancestor);
			m_LinksFromSource.Insert(ancestorGUID, prefab);
		}
	}
	/*!
	Get a link from a prefab to its editable prefab
	\param guid Prefab GUID
	\param[out] linkedPrefab Found prefab
	\return True if found
	*/
	bool GetLinkFromSource(string guid, out ResourceName linkedPrefab = ResourceName.Empty)
	{
		return m_LinksFromSource.Find(guid, linkedPrefab);
	}
	/*!
	Get a link from editable prefab to its source prefab
	\param guid Prefab GUID
	\param[out] linkedPrefab Found prefab
	\return True if found
	*/
	bool GetLinkFromTarget(string guid, out ResourceName linkedPrefab = ResourceName.Empty)
	{
		return m_LinksFromTarget.Find(guid, linkedPrefab);
	}
	/*!
	Check if the prefab is an editable entity
	\return True if it's an editable entity
	*/
	bool IsEditableEntity(ResourceName prefab)
	{
		return prefab.GetPath().StartsWith(m_sTargetPath);
	}
	/*!
	Get source prefab of editable entity prefab
	\param prefab Queried prefab
	\return Source prefab
	*/
	ResourceName GetSourcePrefab(ResourceName prefab)
	{
		if (!IsEditableEntity(prefab)) return ResourceName.Empty;
		
		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource) return ResourceName.Empty;
		
		IEntitySource prefabEntity = SCR_BaseContainerTools.FindEntitySource(prefabResource);
		if (!prefabEntity) return ResourceName.Empty;
		
		IEntitySource ancestorEntity = prefabEntity.GetAncestor();
		if (!ancestorEntity) return ResourceName.Empty;
		
		return ancestorEntity.GetResourceName();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	void EditablePrefabsConfig()
	{
		if (m_SourceDirectory.IsEmpty())
		{
			Print("Cannot use config EditablePrefabsConfig, m_SourceDirectory attribute is empty!", LogLevel.ERROR);
		}
		if (m_TargetDirectory.IsEmpty())
		{
			Print("Cannot use config EditablePrefabsConfig, m_TargetDirectory attribute is empty!", LogLevel.ERROR);
		}
	
		m_bIsValid = true;
		m_sSourcePath = m_SourceDirectory.GetPath();
		m_sTargetPath = m_TargetDirectory.GetPath();
		m_sTargetPathAuto = m_sTargetPath + "/" + m_sAutoFolderName;
		m_sSourceBlacklist.Split(",", m_aSourceBlacklist, true);
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Target")]
class EditablePrefabsSetting
{
	[Attribute(params: "unregFolders et")]
	private ResourceName m_Target;

	[Attribute()]
	private ref array<ref EditablePrefabsComponent_Base> m_Components;

	string GetTarget()
	{
		return m_Target.GetPath();
	}
	void AddComponents(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource, IEntitySource currentEntitySource, IEntitySource instanceEntitySource)
	{
		//--- Get existing components on source file (to prevent duplicates)
		BaseContainer componentContainer, componentCurrent;
		map<string, BaseContainer> sourceComponentClasses = new map<string, BaseContainer>;
		int componentsCount = entitySource.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			componentContainer = entitySource.GetComponent(i);
			sourceComponentClasses.Insert(componentContainer.GetClassName(), componentContainer);
		}
	
		//--- Get existing components on target file (to avoid overriding certain values)
		map<string, BaseContainer> currentComponentClasses = new map<string, BaseContainer>;
		if (currentEntitySource)
		{
			componentsCount = currentEntitySource.GetComponentCount();
			for (int i = 0; i < componentsCount; i++)
			{
				componentContainer = currentEntitySource.GetComponent(i);
				currentComponentClasses.Insert(componentContainer.GetClassName(), componentContainer);
			}
		}
		
		//--- Add components from prefabs
		Resource componentResource;
		BaseResourceObject componentBase;
		foreach (EditablePrefabsComponent_Base component: m_Components)
		{
			//--- Get component on current target prefab		
			if (!sourceComponentClasses.Find(component.GetClassName(), componentContainer))
			{
				componentContainer = component.AddComponent(config, api, prefab, targetPath, entitySource);
			}
			if (componentContainer)
			{
				//--- If the component exists in source prefab, but is disabled, make sure to enable it here
				bool enabled;
				componentContainer.Get("Enabled", enabled);
				if (!enabled)
					api.SetVariableValue(entitySource, { new ContainerIdPathEntry(componentContainer.GetClassName()) }, "Enabled", "1");
			
				currentComponentClasses.Find(component.GetClassName(), componentCurrent);
				component.EOnCreate(config, api, prefab, targetPath, entitySource, instanceEntitySource, componentContainer, componentCurrent);
				sourceComponentClasses.Insert(componentContainer.GetClassName(), componentContainer);
			}
		}
	}
	void Delete(EditablePrefabsConfig config, WorldEditorAPI api, string prefabPath)
	{
		foreach (EditablePrefabsComponent_Base component: m_Components)
		{
			component.EOnDelete(config, api, prefabPath);
		}
	}
	void Move(EditablePrefabsConfig config, WorldEditorAPI api, string currentPath, string newPath)
	{
		foreach (EditablePrefabsComponent_Base component: m_Components)
		{
			component.EOnMove(config, api, currentPath, newPath);
		}
	}
};

enum EEditablePrefabResult
{
	FAILED,
	CREATED,
	UPDATED,
	MOVED,
	DELETED,
	NON_EDITABLE,
	IGNORED
};
