/** @defgroup BatchPrefabsOperationsTool Batch Prefabs Operations Tool
Workbench tool for batch operation on multiple objects. It
*/

/** @ingroup BatchPrefabsOperationsTool
*/

/*!
Workbench tool for generation of windows & glass prefabs.

With this tool you can either create dozens of prefabs with selected configuration from XOB models
by pressing "Create" button or modify configuration of existing prefabs with "Modify" button.
*/
[WorkbenchToolAttribute("Batch prefabs operations tool", "### Creating new prefabs ###\nTo create new prefabs you need to fill 'Prefab Config', 'Path To Save' and\n'Models To Process'\n\nIf Generate Names is checked, there is no need to fill in New Names array\nUse Prefab Config to store additional settings in new prefab.\n\nOnce everything is set, click on 'Create' button to create new prefabs\n\n### Modifying  existing prefabs ###\nTo modify prefabs you need to have at least one prefab in\n'Prefabs To Process' array selected & valid config selected in\n'Prefabs Config'\n\nWith those 2 parameters set, click on 'Modify' button to modify selected prefabs", "2", awesomeFontCode: 0xf085)]
class BatchPrefabsOperationsTool : WorldEditorTool
{
	[Attribute("1", UIWidgets.ComboBox, "File system where new prefabs will be created", "",ParamEnumAddons.FromEnum(), "General")]
	private int m_AddonToUse;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Choose a base class of your prefabs.", "et", NULL, "Create")]
	private ResourceName m_BaseClass;

	[Attribute("", UIWidgets.ResourceNamePicker, "Choose a path to save your prefabs.", "unregFolders", NULL, "Create")]
	private ResourceName m_PathToSave;

	[Attribute("", UIWidgets.ResourceAssignArray, "Choose XOB files to process.", "xob", NULL, "Create")]
	protected ref array<ResourceName> m_ModelsToProcess;

	[Attribute("", UIWidgets.EditBox, "Choose new names of your prefabs.", "et", NULL, "Create")]
	protected ref array<string> m_NewNames;

	[Attribute("", UIWidgets.CheckBox, "Generate names from source files.","", NULL, "Create")]
	private bool m_GenerateNames;

	[Attribute("", UIWidgets.ResourceAssignArray, "Choose prefabs to process.", "et", NULL, "Modify")]
	protected ref array<ResourceName> m_PrefabsToProcess;

	[Attribute(defvalue: "", desc: "All prefabs in this directory will be processed. Replaces Prefabs To Process array", params: "unregFolders", category: "Modify")]
	private ResourceName m_FolderToProcess;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Pick a config with template configuration.", "conf", NULL, "Modify")]
	private ResourceName m_PrefabConfig;

	private IEntityComponentSource m_Component;


	/*!
	Process all components listed in config and try to apply all changes defined in
	m_ComponentAttributesArray by calling ProcessAttributes function.
	If component is not found, new one is created.
	\param entSrc Entity Source from which components should be read
	\param m_PrefabGeneratorConfig Handle to config file containing instructions for the tool
	*/
	void ProcessComponents(IEntitySource entSrc, SCR_BatchPrefabTemplates m_PrefabGeneratorConfig)
	{
		auto containerPath = new array<ref ContainerIdPathEntry>();
		foreach (int currentCompIndex, SCR_BatchPrefabTemplatesData currentComp: m_PrefabGeneratorConfig.m_ComponentsArray)
		{
			int action = currentComp.m_ComponentAction;
			
			// Check if component already exists
			m_Component = SCR_BaseContainerTools.FindComponentSource(entSrc,currentComp.m_ComponentName);
			if(!m_Component) 
			{ 
				// Return if modify only flag is used
				if( action == 2 ) 
					return;
				// Create new component
				m_Component = m_API.CreateComponent(entSrc,currentComp.m_ComponentName);
			}
			
			// Delete component
			if (action == 4)
			{
				bool success = m_API.DeleteComponent(entSrc, m_Component);
				if (!success) 
				{
					Print("Unable to delete " + currentComp.m_ComponentName + " from " + entSrc + ". Component is probably inherited", LogLevel.WARNING);
				} 
				return;
			}
			
			containerPath.Clear();
			containerPath.Insert(new ContainerIdPathEntry(currentComp.m_ComponentName));
			ProcessAttributes(entSrc, m_Component, containerPath,currentComp.m_ComponentAttributesArray);
			//ProcessAttributes(entSrc,containerPath,currentComp.m_SubAttributesArray);
		}
	}

	/*!
	Process all attributes listed in config and apply change to selected entity source.
	Function supports basic math  (multiplying, dividing & adding) &
	bitwise operations.
	\param entSrc Entity Source where new values should be stored
	\param entSrcAttribute Entity Source from which script should read values
	\param containerPath Path to container where values will be stored
	\param TemplateData Array of values to be processed
	*/
	void ProcessAttributes(IEntitySource entSrc, IEntitySource entSrcAttribute, array<ref ContainerIdPathEntry> containerPath, array<ref SCR_BatchPrefabTemplatesAttributes> TemplateData)
	{
		// Add atributes to currently processed component
		foreach (int currentAtributeIndex, SCR_BatchPrefabTemplatesAttributes currentAttribute: TemplateData)
		{
			string attributeName = currentAttribute.m_AttributeName;
			string attributeValue = currentAttribute.m_AttributeValue;
			int attributeOperation = currentAttribute.m_AttributeOperation;
			if(attributeOperation == EBatchPrefabOperationFlag.MATH)
			{
				float m_fCurrentValue;
				entSrcAttribute.Get(attributeName,m_fCurrentValue);

				// Remove * & / signs from string since ToFloat cannot cope with it
				string attributeValueShort = attributeValue;
				attributeValueShort.Replace("*","");
				attributeValueShort.Replace("/","");
				float operand = attributeValueShort.ToFloat();
				if(attributeValue.StartsWith("+") || attributeValue.StartsWith("-"))
				{
					m_fCurrentValue = m_fCurrentValue + operand;
				}
				if(attributeValue.StartsWith("/"))
				{
					// TODO: Catch division by zero
					m_fCurrentValue = m_fCurrentValue / operand;
				}
				if(attributeValue.StartsWith("*"))
				{
					m_fCurrentValue = m_fCurrentValue * operand;
				}
				attributeValue = m_fCurrentValue.ToString();
			}
			if(attributeOperation == EBatchPrefabOperationFlag.ENUM)
			{
				int m_iCurrentValue;
				entSrcAttribute.Get(attributeName,m_iCurrentValue);
				if(attributeValue.StartsWith("+"))
				{
					m_iCurrentValue = m_iCurrentValue | attributeValue.ToInt();
				}
				if(attributeValue.StartsWith("-"))
				{
					m_iCurrentValue = m_iCurrentValue &~ attributeValue.ToInt();
				}
				attributeValue = m_iCurrentValue.ToString();
			}
			m_API.SetVariableValue(entSrc, containerPath, attributeName, attributeValue);
			//ProcessAttributes(entSrc,containerPath,currentAttribute.m_SubAttributesArray);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Create bunch of inherited prefabs with selected models in m_ModelsToProcess array.
	By minimum, new prefabs will have attached models in MeshObject component and will inherit
	from m_BaseClass prefab. They will be located in folder selected in m_PathToSave & will
	have name based on XOB file.

	Optionaly, you can also use m_PrefabConfig to apply more values to your new prefab.

	If m_NewNames array is used, number of entries in this array must match the number of models
	*/
	[ButtonAttribute("Create")]
	void Execute()
	{
		// Input parameters verification
		if (!m_ModelsToProcess)
		{
			Print("Models To Process is empty. Please fill it with at least one model", LogLevel.ERROR);
			return;
		}
		if (!m_PathToSave)
		{
			Print("Path To Save is empty. Please select valid save location", LogLevel.ERROR);
			return;
		}
		if (!m_BaseClass)
		{
			Print("Base Class is empty. Please select valid base prefab for your window", LogLevel.ERROR);
			return;
		}
		if (!m_GenerateNames && !m_NewNames)
		{
			Print("New Names and Generate Names are empty. Please either check Generate Names or type in new names", LogLevel.ERROR);
			return;
		}

		WorldEditor we = Workbench.GetModule(WorldEditor);
		ResourceManager rb = Workbench.GetModule(ResourceManager);

		// Strip GUID
		if (m_PathToSave.Contains("{") && m_PathToSave.Contains("}"))
		{
			int guidBracket = m_PathToSave.IndexOf("}");
			m_PathToSave = m_PathToSave.Substring(guidBracket+1, m_PathToSave.Length() - guidBracket - 1);
		}
	
		// Get addon 
		string addon = SCR_AddonTool.GetAddonIndex(m_AddonToUse);
		addon = SCR_AddonTool.ToFileSystem(addon);
	
		// Get absolute path for CreateEntityTemplate
		string absPath;
		Workbench.GetAbsolutePath(addon + m_PathToSave, absPath);

		string m_sTempName;

		foreach (int currentIndex, ResourceName currentElement: m_ModelsToProcess)
		{
			// Load metafile and get resource GUID
			private string modelName = currentElement;

			string m_sFileLink = string.Format("@\"%1\"", currentElement.GetPath());
			Print("Processing " + m_sFileLink, LogLevel.NORMAL);
		
			m_API.BeginEntityAction("Processing " + modelName);

			// Create entity with selected base class
			IEntity m_Entity = m_API.CreateEntity(m_BaseClass, "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);

			IEntitySource entSrc = m_API.EntityToSource(m_Entity);
			auto containerPath = new array<ref ContainerIdPathEntry>();

			// Try to find MeshObject component and if it doesn't exist, then create new one
			if(!(SCR_BaseContainerTools.FindComponentSource(entSrc,"MeshObject"))) { m_API.CreateComponent(entSrc,"MeshObject");}
			containerPath.Clear();
			containerPath.Insert(new ContainerIdPathEntry("MeshObject"));
			m_API.SetVariableValue(entSrc, containerPath, "Object", modelName);

			if (m_PrefabConfig)
			{
				Resource holder = BaseContainerTools.LoadContainer(m_PrefabConfig);
				if (!holder)
					return;

				// Process attributes from config
				SCR_BatchPrefabTemplates m_PrefabGeneratorConfig = SCR_BatchPrefabTemplates.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
				ProcessAttributes(entSrc, entSrc, containerPath,m_PrefabGeneratorConfig.m_AttributesArray);
				ProcessComponents(entSrc, m_PrefabGeneratorConfig);
			}

			// Generate new name based on xob file name. If not checked, then custom one is used
			if(m_GenerateNames)
			{
				array<string> tempArray = new array<string>();
				modelName.Split("/",tempArray,false);
				m_sTempName = tempArray[tempArray.Count()-1];
				m_sTempName.Replace(".xob","");
			}else{
				m_sTempName = m_NewNames[currentIndex];
			}

			// Create file
			bool fileCreated;
			fileCreated = m_API.CreateEntityTemplate(entSrc,FilePath.Concat(absPath, m_sTempName  + ".et"));

			// End entity manipulation
			m_Entity = m_API.SourceToEntity(entSrc);
			m_API.DeleteEntity(m_Entity);
			m_API.EndEntityAction();
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Modify all prefabs listed in m_PrefabsToProcess with instructions contained in m_PrefabConfig.
	After values are changed, function is also triggering saving of currently open scene
	World Editor to store all changes on the user drive.
	*/
	[ButtonAttribute("Modify")]
	void ExecuteModify()
	{
		// Input parameters verification
		if (!m_PrefabsToProcess && !m_FolderToProcess)
		{
			Print("m_PrefabsToProcess & m_FolderToProcess is empty. Please fill with at least one prefab", LogLevel.ERROR);
			return;
		}
		if (!m_PrefabConfig)
		{
			Print("Missing config file for batch prefab operation!", LogLevel.ERROR);
			return;
		}

		WorldEditor we = Workbench.GetModule(WorldEditor);
	
		if( m_FolderToProcess != "" )
		{
			string addon = SCR_AddonTool.GetAddonIndex(m_AddonToUse);
			addon = SCR_AddonTool.ToFileSystem(addon);
		
			// Get all prefabs from selected folder
			Workbench.SearchResources(m_PrefabsToProcess.Insert, {"et"},null, addon + m_FolderToProcess.GetPath());
			
			Print("Processing " + m_PrefabsToProcess.Count(), LogLevel.NORMAL);
		
			// Exit if there are no valid prefabs in selected folder
			if ( m_PrefabsToProcess.Count() == 0 )
			{
				Print("There are no valid prefabs in selected folder", LogLevel.ERROR);
				return;
			}
		}
		

		foreach (int currentIndex, ResourceName currentElement: m_PrefabsToProcess)
		{
			// Load metafile and get resource GUID
			private string modelName = currentElement;

			string m_sFileLink = string.Format("@\"%1\"", currentElement.GetPath());
			Print("Processing " + m_sFileLink, LogLevel.NORMAL);
			//m_API.BeginEntityAction("Processing - " + modelName);

			// Create entity with selected base class
			IEntity m_Entity = m_API.CreateEntity(currentElement, "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);

			// Get parent entity source
			IEntitySource entSrc = m_API.EntityToSource(m_Entity);
			IEntitySource prefab = entSrc.GetAncestor();

			auto containerPath = new array<ref ContainerIdPathEntry>();

			// Load config and verify if its valid one
			Resource holder = BaseContainerTools.LoadContainer(m_PrefabConfig);
			if (!holder)
				return;

			// Process all attributes from config
			SCR_BatchPrefabTemplates m_PrefabGeneratorConfig = SCR_BatchPrefabTemplates.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			ProcessAttributes(prefab, prefab, containerPath, m_PrefabGeneratorConfig.m_AttributesArray);
			ProcessComponents(prefab, m_PrefabGeneratorConfig);

			// if we use 'entSrc' to modify prefab instance
			// if we use 'entSrc.GetAncestor()' to modify prefab (first prefab in prefab hierarchy)
			m_Entity = m_API.SourceToEntity(entSrc);
			m_API.DeleteEntity(m_Entity);
			//m_API.EndEntityAction();
		}
		we.Save(); // invoke world editor save to save changes to the prefab
	}

	//------------------------------------------------------------------------------------------------
	void BatchPrefabsOperationsTool(IEntitySource src, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~BatchPrefabsOperationsTool()
	{
	}
};

//! Custom title for BatchPrefabTemplates config parameters
class BatchPrefabTitle: BaseContainerCustomTitle
{
	string m_ParamString[4];

	void BatchPrefabTitle(string paramString1, string paramString2 = "", string paramString3 = "", string paramString4 = "")
	{
		m_ParamString[0] = paramString1;
		m_ParamString[1] = paramString2;
		m_ParamString[2] = paramString3;
		m_ParamString[3] = paramString4;
	}

	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		string temp;
		string prop;
		for (int i = 1; i <= 4; i++)
		{
			temp = "";
			prop = m_ParamString[i - 1];
			if (!source.Get(prop, temp)) temp = prop;
			title = title + temp;
		}
		if(title == "") return false;
		return true;
	}
};

//! Unique flags for the batch operations
enum EBatchPrefabOperationFlag
{
	REPLACE = 1, ///< Default state - replace current value with anything that
	MATH = 2, ///< Math operations are performed on the attribute
	ENUM = 4 ///< Bitwise operations on enum
};

enum EBatchPrefabOperationComponentFlag
{
	CREATE = 1, ///< Create new component - if component exist then it will be modified
	MODIFY = 2, ///< Only modifies component - if component is missing then all actions related to this component will be skipped
	DELETE = 4 ///< Delete component
};


//------------------------------------------------------------------------------------------------
//! SCR_BatchPrefabTemplates config
[BaseContainerProps(configRoot: true)]
class SCR_BatchPrefabTemplates
{
	[Attribute(desc: "Array of attributes which be aplied to prefab (not to its components)" )]
	ref array<ref SCR_BatchPrefabTemplatesAttributes> m_AttributesArray;
	[Attribute(desc: "Array of components which will be added or modified" )]
	ref array<ref SCR_BatchPrefabTemplatesData> m_ComponentsArray;
};

[BaseContainerProps(configRoot: false), BatchPrefabTitle("Component: ","m_ComponentName")]
class SCR_BatchPrefabTemplatesData
{
	[Attribute(desc: "Name of component to modify or add" )] string m_ComponentName;
	[Attribute(defvalue: "1", UIWidgets.ComboBox ,desc: "Create new component if its missing prefab", enums: ParamEnumArray.FromEnum(EBatchPrefabOperationComponentFlag))]
	EBatchPrefabOperationComponentFlag m_ComponentAction;
	[Attribute(desc: "Array of attributes which will be modified inside component belonging to prefab. If component is not present, new one will be created" )]
	ref array<ref SCR_BatchPrefabTemplatesAttributes> m_ComponentAttributesArray;
};

[BaseContainerProps(configRoot: false), BatchPrefabTitle( "m_AttributeName",": ","m_AttributeValue")]
class SCR_BatchPrefabTemplatesAttributes
{
	[Attribute(desc: "Name of attribute to modify" )] string m_AttributeName;
	[Attribute(desc: "Value which will be stored in prefab. Simple math (adding, multiplying & dividing) & bitwise operations are allowed (use + to add and - to remove flag)" )] string m_AttributeValue;
	[Attribute("1", UIWidgets.ComboBox, category: "", desc: "Specifies type of operation that is performed.", enums: ParamEnumArray.FromEnum(EBatchPrefabOperationFlag))]
	EBatchPrefabOperationFlag m_AttributeOperation;
	[Attribute(desc: "Feature not implemented yet!")]
	ref array<ref SCR_BatchPrefabTemplatesAttributes> m_SubAttributesArray;
};
