[BaseContainerProps(configRoot: true)]
class LocParserManager
{
	[Attribute("{C014582791ECBF24}Language/localization.st", desc: "String table where new strings will be added.", params: "st")]
	protected ResourceName m_StringTablePath;
	
	[Attribute("AR-", desc: "Project prefix added at the beginning of every string ID.")]
	protected string m_sPrefix;
	
	[Attribute("!", desc: "Ignore string when it starts with this expression.\nUsed to mark placeholder texts which are replaced by script.")]
	protected string m_sPlaceholderPrefix;
	
	[Attribute()]
	protected ref array<ref LocParserRule> m_aRules;
	
	protected ResourceName m_File;
	protected string m_sFileName, m_sFileExt, m_sFileLink, m_sContainerName, m_sContainerPath;
	protected bool m_bLogOnly;
	protected BaseContainer m_StringTable;
	protected ref map<string, string> m_IDs = new map<string, string>;
	protected LocalizationEditor m_LocEditor;
	protected WorldEditorAPI m_API;
	protected ref array<BaseContainer> m_aRuleSources = new array<BaseContainer>;
	protected int m_iCurrentRule;
	
	static const string LOCALIZED_PREFIX = "#";
	
	BaseContainer UpdateItem(out string id, string text)
	{
		//--- Add project prefix
		if (!m_sPrefix.IsEmpty() && !id.StartsWith(m_sPrefix))
			id = m_sPrefix + id;
		
		//--- Check if the item already exist
		string existingText;
		bool exists = m_IDs.Find(id, existingText);
		
		BaseContainer item;
		if (!m_bLogOnly && !exists)
		{
			//--- Create item
			int index = m_IDs.Count();
			if (!m_API.CreateObjectArrayVariableMember(m_StringTable, null, "Items", "CustomStringTableItem", index))
				return null;
			
			//--- Retrieve the item
			BaseContainerList items = m_StringTable.GetObjectArray("Items");
			item = items.Get(index);
			
			//--- Apply template
			UpdateFromTemplate(item);
			
			//--- Save mandatory information
			string userName;
			Workbench.GetUserName(userName);
			
			m_LocEditor.ModifyProperty(item, item.GetVarIndex("Id"), id);
			m_LocEditor.ModifyProperty(item, item.GetVarIndex("Target_en_us"), text);
			m_LocEditor.ModifyProperty(item, item.GetVarIndex("SourceFile"), m_File);
			m_LocEditor.ModifyProperty(item, item.GetVarIndex("Modified"), Workbench.GetPackedUtcTime().ToString());
			m_LocEditor.ModifyProperty(item, item.GetVarIndex("Author"), userName);
			m_LocEditor.ModifyProperty(item, item.GetVarIndex("LastChanged"), userName);
		}
		
		//--- Log results
		Print(string.Format("%1", m_sContainerPath), LogLevel.VERBOSE);
		if (exists)
		{
			Print(string.Format("%1%2 = \"%3\"", LOCALIZED_PREFIX, id, text), LogLevel.NORMAL);
			if (text == existingText)
			{
				Print(string.Format("Item already localized, but its text is the same.\n ", id, existingText), LogLevel.WARNING);
			}
			else
			{
				Print(string.Format("%1%2 = \"%3\"\n               Item already localized, and its text differs! Localized text will be used!\n ", LOCALIZED_PREFIX, id, existingText), LogLevel.ERROR);
			}
		}
		else
		{
			Print(string.Format("%1%2 = \"%3\"\n ", LOCALIZED_PREFIX, id, text), LogLevel.NORMAL);
			m_IDs.Set(id, text);
		}
		
		return item;
	}
	protected void UpdateFromTemplate(BaseContainer item)
	{
		BaseContainer itemTemplate = m_aRuleSources[m_iCurrentRule];
		if (!itemTemplate)
			return;
		
		string varName;
		for (int v = 0, varCount = item.GetNumVars(); v < varCount; v++)
		{
			varName = item.GetVarName(v);
			if (!itemTemplate.IsVariableSet(varName))
				continue;
			
			switch (item.GetDataVarType(v))
			{
				case DataVarType.STRING:
				{
					string value;
					itemTemplate.Get(varName, value);
					m_LocEditor.ModifyProperty(item, v, value);
					break;
				}
				case DataVarType.INTEGER:
				{
					int value;
					itemTemplate.Get(varName, value);
					m_LocEditor.ModifyProperty(item, v, value.ToString());
					break;
				}
				case DataVarType.BOOLEAN:
				{
					bool value;
					itemTemplate.Get(varName, value);
					m_LocEditor.ModifyProperty(item, v, value.ToString());
					break;
				}
				case DataVarType.STRING_ARRAY:
				{
					array<string> valueInput;
					itemTemplate.Get(varName, valueInput);
					string valueOutput;
					foreach (int i, string entry: valueInput)
					{
						if (i > 0) valueOutput += ",";
						valueOutput += string.ToString(entry);
					}
					m_LocEditor.ModifyProperty(item, v, valueOutput);
					break;
				}
				case DataVarType.INTEGER_ARRAY:
				{
					array<int> valueInput;
					itemTemplate.Get(varName, valueInput);
					string valueOutput;
					foreach (int i, int entry: valueInput)
					{
						if (i > 0) valueOutput += ",";
						valueOutput += entry.ToString();
					}
					m_LocEditor.ModifyProperty(item, v, valueOutput);
					break;
				}
				case DataVarType.BOOLEAN_ARRAY:
				{
					array<bool> valueInput;
					itemTemplate.Get(varName, valueInput);
					string valueOutput;
					foreach (int i, bool entry: valueInput)
					{
						if (i > 0) valueOutput += ",";
						valueOutput += entry.ToString();
					}
					m_LocEditor.ModifyProperty(item, v, valueOutput);
					break;
				}
				default:
				{
					Print(string.Format("Cannot copy variable '%1' from template, it has unsupported type %2!", varName, typename.EnumToString(DataVarType, item.GetDataVarType(v))), LogLevel.ERROR);
				}
			}
		}
	}
	protected void ProcessContainer(BaseContainer container, out int foundCount, array<BaseContainer> objects, array<int> indexes, string path, out int index = 0)
	{
		if (!objects.IsEmpty())
		{
			string containerName;
			if (m_sFileExt == "layout")
			{
				container.Get("Name", containerName);
			}
			else
			{
				containerName = container.GetName();
				if (containerName.IsEmpty()) containerName = container.GetClassName();
			}
			path += " > " + containerName;
		}
		
		array<BaseContainer> objectsCopy = new array<BaseContainer>;
		objectsCopy.Copy(objects);
		objectsCopy.InsertAt(container, 0);
		
		array<int> indexesCopy = new array<int>;
		indexesCopy.Copy(indexes);
		indexesCopy.InsertAt(index, 0);
		
		//--- Process variables
		string varName, id, text;
		bool found;
		for (int v = 0, varCount = container.GetNumVars(); v < varCount; v++)
		{
			varName = container.GetVarName(v);
			
			//--- Skip if inherited from parent
			if (!container.IsVariableSetDirectly(varName))
				continue;
			
			if (container.IsType(varName, string) && container.Get(varName, text))
			{
				//--- String, try to localize it
				if (text.IsEmpty())
					continue;
				
				//--- Already localized
				if (text.Contains(LOCALIZED_PREFIX) || text.StartsWith(m_sPlaceholderPrefix))
					continue;
				
				//--- Localize
				found = false;
				foreach (int r, LocParserRule rule: m_aRules)
				{
					if (rule.Evaluate(m_sFileName, varName, objectsCopy))
					{
						indexesCopy.Set(indexesCopy.Count() - 1, index);
						id = rule.GetID(m_sFileName, varName, objectsCopy, indexesCopy);
						index++;
						if (!id.IsEmpty())
						{
							found = true;
							foundCount++;
							id = m_sPrefix + id;
							m_sContainerPath = path;
							m_iCurrentRule = r;
							rule.Localize(m_sFileName, varName, objectsCopy, id, text);
							
							if (!m_bLogOnly) container.Set(varName, LOCALIZED_PREFIX + id);
						}
						break;
					}
				}
				
				//--- No rule caught it, but the string is still marked for localization
				string uiWidget = container.GetUIWidget(v);
				if (m_bLogOnly && !found && (uiWidget == "localeeditbox" || uiWidget == "editboxWithButton"))
				{
					foundCount++;
					m_sContainerPath = path;
					Print(string.Format("%1", m_sContainerPath), LogLevel.VERBOSE);
					Print(string.Format("No rule found for: %1 = '%2'\n\n", varName, text), LogLevel.WARNING);
				}
				
				continue;
			}
			
			BaseContainer object = container.GetObject(varName);
			if (object)
			{
				//--- Object, process its variables
				ProcessContainer(object, foundCount, objectsCopy, indexesCopy, path + " > " + varName);
			}
			else
			{
				//--- Array of objects, process every element
				BaseContainerList list = container.GetObjectArray(varName);
				if (list)
				{
					int index2;
					for (int l = 0, listCount = list.Count(); l < listCount; l++)
					{
						ProcessContainer(list.Get(l), foundCount, objectsCopy, indexesCopy, path + string.Format(" > %1[%2]", varName, l), index2);
					}
				}
			}
		}
		
		//--- Process children
		if (container)
		{
			for (int e = 0, count = container.GetNumChildren(); e < count; e++)
			{
				ProcessContainer(container.GetChild(e), foundCount, objectsCopy, indexesCopy, path);
			}
		}
	}
	protected void Run(BaseContainer source, bool logOnly, ResourceName stringTableOverride, string prefixOverride)
	{
		m_bLogOnly = logOnly;
		if(stringTableOverride)
			m_StringTablePath = stringTableOverride;
		
		if(prefixOverride)
			m_sPrefix = prefixOverride;
		
		if (!Workbench.OpenModule(LocalizationEditor))
			return;
		
		m_LocEditor = Workbench.GetModule(LocalizationEditor);
		m_LocEditor.SetOpenedResource(m_StringTablePath);
		m_StringTable = m_LocEditor.GetTable();
		if (!m_StringTable)
		{
			Print(string.Format("Unable to open string table '%1'!", m_StringTablePath.GetPath()), LogLevel.WARNING);
			return;
		}
		
		//--- Get all items, so we can check if item exists
		BaseContainerList items = m_StringTable.GetObjectArray("Items");
		BaseContainer item;
		string itemId, itemText;
		int itemsCount = items.Count();
		for (int i = 0; i < itemsCount; i++)
		{
			item = items.Get(i);
			item.Get("Id", itemId);
			item.Get("Target_en_us", itemText);
			m_IDs.Insert(itemId, itemText);
		}
		
		if (!m_bLogOnly)
		{
			if (!Workbench.OpenModule(WorldEditor))
				return;
			
			WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
			m_API = worldEditor.GetApi();
			m_API.BeginEntityAction("Automatic localization");
			m_LocEditor.BeginModify("Automatic localization");
		}
		
		
		//--- Initialize rules
		BaseContainerList rulesSource = source.GetObjectArray("m_aRules"); //--- Save sources, because CustomStringTableItem objects cannot be created in script
		foreach (int r, LocParserRule rule: m_aRules)
		{
			m_aRuleSources.Insert(rulesSource.Get(r).GetObject("m_ItemTemplate"));
			rule.InitRule(this, m_LocEditor);
		}
		
		//--- Get selected files
		array<ResourceName> selection = new array<ResourceName>;
		SCR_WorkbenchSearchResourcesCallbackArray context = new SCR_WorkbenchSearchResourcesCallbackArray(selection);
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		resourceManager.GetResourceBrowserSelection(context.Insert, true);
		WBProgressDialog progress = new WBProgressDialog("Processing...", resourceManager);
		
		//--- Process files
		int foundCount, foundCountPrev;
		int selectionCount = selection.Count();
		Resource resource;
		BaseResourceObject baseResource;
		BaseContainer container;
		Managed instance;
		for (int i = 0; i < selectionCount; i++)
		{
			progress.SetProgress(i / selectionCount);
			
			m_File = selection[i];
			
			resource = Resource.Load(m_File);
			if (!resource || !resource.IsValid())
				continue;
			
			baseResource = resource.GetResource();
			if (!baseResource)
				continue;
			
			container = baseResource.ToBaseContainer();
			if (!container)
				continue;
			
			foundCountPrev = foundCount;
			m_sFileName = FilePath.StripExtension(FilePath.StripPath(m_File), m_sFileExt);
			m_sFileLink = string.Format("@\"%1\"", m_File.GetPath());
			array<BaseContainer> objects = new array<BaseContainer>;
			array<int> indexes = new array<int>;
			ProcessContainer(container, foundCount, objects, indexes, m_sFileLink);
			
			if (!m_bLogOnly && foundCount != foundCountPrev)
			{
				BaseContainerTools.SaveContainer(container, m_File);
			}
		}
		
		if (!m_bLogOnly)
		{
			m_API.EndEntityAction();
			m_LocEditor.EndModify();
			if (foundCount > 0)
			{
				m_LocEditor.Save();
			
				//--- Show only newly added strings
				int itemsCountNew = items.Count();
				array<int> newIds = new array<int>;
				for (int i = itemsCount; i <= itemsCountNew; i++)
				{
					newIds.Insert(i);
				}
				m_LocEditor.AddUserFilter(newIds, "New strings");
			}
		}
		
		if (m_bLogOnly)
		{
			LogLevel logLevel = LogLevel.DEBUG;
			if (foundCount > 0) logLevel = LogLevel.WARNING;
			Print(string.Format("%1 file(s) processed, %2 unlocalized string(s) found\n               Logging only, no files were modified.", selectionCount, foundCount), logLevel);
		}
		else
		{
			Print(string.Format("%1 file(s) processed, %2 string(s) localized", selectionCount, foundCount), LogLevel.DEBUG);
		}
		
		m_IDs = null;
		m_StringTable = null;
		m_LocEditor = null;
	}
	
	/*!
	Run localization process.
	\param configPath Path to LocParser manager config
	\param logOnly True to only log unlocolized strings, without modifying any files
	*/
	static void Run(ResourceName configPath, bool logOnly, ResourceName stringTableOverride, string prefixOverride)
	{
		Resource configResource = Resource.Load(configPath);
		if (!configResource.IsValid())
		{
			Print(string.Format("Cannot load config '%1'!", configPath), LogLevel.WARNING);
			return;
		}
		
		BaseResourceObject configContainer = configResource.GetResource();
		if (!configContainer) return;
		
		BaseContainer configBase = configContainer.ToBaseContainer();
		if (!configBase) return;
		
		if (configBase.GetClassName().ToType() != LocParserManager)
		{
			Print(string.Format("Config '%1' is of type '%2', must be 'LocParserManager'!", configPath, configBase.GetClassName()), LogLevel.WARNING);
			return;
		}
				
		LocParserManager manager = LocParserManager.Cast(BaseContainerTools.CreateInstanceFromContainer(configBase));
		if (manager)
			manager.Run(configBase, logOnly, stringTableOverride, prefixOverride);
		else
			Print(string.Format("Unknown error when creating instance of config '%1'!", configPath), LogLevel.WARNING);
	}
};
