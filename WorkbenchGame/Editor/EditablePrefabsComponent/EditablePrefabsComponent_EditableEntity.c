[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Component", true)]
class EditablePrefabsComponent_EditableEntity: EditablePrefabsComponent_Base
{
	const string IMG_EXTENSION = "edds";
	const string META_EXTENSION = ".meta";
	
	[Attribute(defvalue: "(%1)", desc: "Format of autogenerated placeholder name.\n%1 is the prefab file name (without extension).")]
	private string m_sNameFormat;
	
	[Attribute(defvalue: "#AR-EditableEntity_%1_Name", desc: "Format of localized name.\nWhen found in the database, it will be applied instead of the placeholder name.\n%1 is the prefab file name (without extension).")]
	private string m_sLocKeyFormat;
	
	[Attribute(defvalue: "UI/Textures/EditorPreviews", desc: "Directory where placeholder image will be auto-generated.\nHierarchy inside the folder mimics prefab folder hierarchy.", params: "folders")]
	private ResourceName m_ImagesDirectory;
	
	[Attribute(defvalue: "", desc: "Placeholder image used when adding new editable entity", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds")]
	private ResourceName m_ImagePlaceholder;
	
	[Attribute(defvalue: "1", desc: "Enable budget update during autoconfiguration, costly procedure so can be disabled if budgets don't need to be updated")]
	private bool m_bUpdateBudgets;
	
	[Attribute(desc: "Label Rules")]
	private ref array<ref EditablePrefabsLabel_Base> m_EntityLabelRules;
	
	[Attribute(defvalue: "", desc: "Editor entity core config file", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "conf")]
	private ResourceName m_EntityCoreConfigPrefab;
	
	private string m_sImagesPath;
	private string m_sImagePlaceholderPath;
	private string m_sImagePlaceholderSource;
	private string m_sImagePlaceholderExt;
	private ResourceManager m_ResourceManager;
	private ref SCR_EditableEntityCore m_EntityCoreConfig;
	
	protected ref map<EEditableEntityBudget, int> m_MinBudgetCost = new map<EEditableEntityBudget, int>;
	
	protected bool SetUIInfo(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource, IEntitySource instanceEntitySource, IEntityComponentSource componentSource, IEntityComponentSource componentCurrent)
	{
		BaseContainer info = componentSource.GetObject("m_UIInfo");
		if (!info)
			return false;
		
		typename infoType = info.GetClassName().ToType();
		
		array<ref ContainerIdPathEntry> path = {ContainerIdPathEntry(componentSource.GetClassName()), ContainerIdPathEntry("m_UIInfo")};

		//--- Get existing UI info
		BaseContainer infoCurrent;
		if (componentCurrent)
			infoCurrent = componentCurrent.GetObject("m_UIInfo");
		
		//--- Name
		if (!m_sNameFormat.IsEmpty())
		{
			string name;
			
			//--- Preserve existing localized name
			if (infoCurrent)
				infoCurrent.Get("Name", name);
			
			if (!name.StartsWith("#"))
			{
				string prefabName = FilePath.StripExtension(FilePath.StripPath(prefab));
				
				string locKey = string.Format(m_sLocKeyFormat, prefabName);
				string locText = WidgetManager.Translate(locKey);
				if (locText != locKey)
					name = locKey; //--- Apply existing, but not assigned localized name
				else
					name = string.Format(m_sNameFormat, prefabName); //--- No localized name defined yet, generate the name based on prefab name
			}
			
			api.SetVariableValue(entitySource, path, "Name", name);
		}
		
		
		//--- Preserve existing localized description
		if (infoCurrent)
		{
			string description;
			if (infoCurrent.IsVariableSetDirectly("Description") && infoCurrent.Get("Description", description) && !description.IsEmpty())
			{
				api.SetVariableValue(entitySource, path, "Description", description);
			}
			
			//--- Preserve existing entity icon set name
			string entityIconSetName;
			if (infoCurrent.IsVariableSetDirectly("IconSetName") && infoCurrent.Get("IconSetName", entityIconSetName) && !entityIconSetName.IsEmpty())
			{
				api.SetVariableValue(entitySource, path, "IconSetName", entityIconSetName);
			}
			
			//--- Preserve existing entity icon
			ResourceName entityIcon;
			if (infoCurrent.IsVariableSetDirectly("Icon") && infoCurrent.Get("Icon", entityIcon) && !entityIcon.IsEmpty())
			{
				api.SetVariableValue(entitySource, path, "Icon", entityIcon);
			}
		}
		
		//--- Image
		string imagePath = targetPath;
		if (GetImagePath(config, imagePath))
		{
			//--- Create placeholder preview image
			string addonName = SCR_AddonTool.GetResourceLastAddon(prefab);
			addonName = SCR_AddonTool.ToFileSystem(addonName);
			CreatePreviewImage(config, api, imagePath, entitySource, addonName);
			
			api.SetVariableValue(entitySource, path, "m_Image", imagePath);
		}
		
		IEntityComponentSource horizontalAlignComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_HorizontalAlignComponent);
		if (horizontalAlignComponent)
		{
			bool isHorizontalAlign;
			horizontalAlignComponent.Get("Enabled", isHorizontalAlign);
			
			if (isHorizontalAlign)
			{
				int flags;
				componentSource.Get("m_Flags", flags);
				flags = flags | EEditableEntityFlag.HORIZONTAL;
			
				api.SetVariableValue(entitySource, {ContainerIdPathEntry(componentSource.GetClassName())}, "m_Flags", flags.ToString());
			}
		}
		
		//--- Labels
		
		/* Print label indices for debugging
		for	(int i = 0; i<50 ; i++)
		{
			string labelName = Type().EnumToString(EEditableEntityLabel, i);
			
			if (labelName != string.Empty)
			{
				PrintFormat("Label %1: %2", i, labelName);
			}			
		}
		*/
		
		// Read source entity
		array<EEditableEntityLabel> autoLabels = {};
		array<EEditableEntityLabel> authoredLabels = {};
		if (GetLabelsFromSource(api, config, prefab, targetPath, instanceEntitySource, componentSource, componentCurrent, autoLabels, authoredLabels))
		{
			api.SetVariableValue(entitySource, path, "m_aAutoLabels", SCR_BaseContainerTools.GetArrayValue(autoLabels));
			api.SetVariableValue(entitySource, path, "m_aAuthoredLabels", SCR_BaseContainerTools.GetArrayValue(authoredLabels));
		}
		
		if (m_bUpdateBudgets)
		{
			BaseContainer infoAncestor;
			IEntityComponentSource componentAncestor;
			if (componentCurrent)
			{
				componentAncestor = componentCurrent.GetAncestor();
				if (componentAncestor)
					infoAncestor = componentAncestor.GetObject("m_UIInfo");
			}
			
			//--- Update vehicle occupant budgets
			if (infoType.IsInherited(SCR_EditableVehicleUIInfo))
			{
				array<ref SCR_EntityBudgetValue> crewBudgetCosts = {}, passengerBudgetCosts = {};
				array<ECompartmentType> vehicleCompartmentTypes = {};
				
				GetEntityBudgetCostsFromVehicle(entitySource, crewBudgetCosts, passengerBudgetCosts, vehicleCompartmentTypes);
				
				string vehicleCompartmentTypesValue = SCR_BaseContainerTools.GetArrayValue(vehicleCompartmentTypes);
				api.SetVariableValue(entitySource, path, "m_aOccupantFillCompartmentTypes", vehicleCompartmentTypesValue);
				
				SetBudgets(api, entitySource, infoAncestor, infoCurrent, path, "m_aCrewEntityBudgetCost", crewBudgetCosts);
				SetBudgets(api, entitySource, infoAncestor, infoCurrent, path, "m_aPassengerEntityBudgetCost", passengerBudgetCosts);
			}
			
			//=== Calculate combined children budget costs
			array<ref SCR_EntityBudgetValue> entityChildrenBudgetCosts = {};
			GetEntityChildrenBudgetCostsFromSource(entitySource, entityChildrenBudgetCosts);
			
			SetBudgets(api, entitySource, infoAncestor, infoCurrent, path, "m_EntityChildrenBudgetCost", entityChildrenBudgetCosts);
			
			//--- Preserve existing values of the entity budget
			if (infoAncestor)
			{
				string varName = "m_EntityBudgetCost";
				array<ref SCR_EntityBudgetValue> entityBudgetCosts = {};
				PreserveBudgets(api, infoCurrent, varName, entityBudgetCosts);
				
				int budgetListCount;
				BaseContainerList budgetList = infoAncestor.GetObjectArray(varName);
				if (budgetList)
					budgetListCount = budgetList.Count();
				
				array<ref ContainerIdPathEntry> budgetPath = {ContainerIdPathEntry(componentSource.GetClassName()), ContainerIdPathEntry("m_UIInfo"), null};
				foreach (SCR_EntityBudgetValue budget: entityBudgetCosts)
				{
					api.CreateObjectArrayVariableMember(entitySource, path, varName, "SCR_EntityBudgetValue", budgetListCount);
					
					budgetPath.Set(budgetPath.Count() - 1, ContainerIdPathEntry(varName, budgetListCount));
					api.SetVariableValue(entitySource, budgetPath , "m_BudgetType", typename.EnumToString(EEditableEntityBudget, budget.GetBudgetType()));
					api.SetVariableValue(entitySource, budgetPath , "m_Value", budget.GetBudgetValue().ToString());
					budgetListCount++;
				}
			}
		}
		
		//--- Custom slot prefab
		IEntityComponentSource slotCompositionComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_SlotCompositionComponent);
		if (slotCompositionComponent)
		{
			ResourceName slotPrefab = SCR_SlotCompositionComponentClass.GetSlotPrefab(slotCompositionComponent);
			if (!slotPrefab.IsEmpty())
				api.SetVariableValue(entitySource, path, "m_SlotPrefab", slotPrefab);
		}
		
		//--- Copy group identity
		if (infoType.IsInherited(SCR_EditableGroupUIInfo))
		{
			IEntityComponentSource groupIdentitySource = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_GroupIdentityComponent);
			if (groupIdentitySource)
			{
				BaseContainer symbolSource = groupIdentitySource.GetObject("m_MilitarySymbol");
				if (symbolSource)
				{
					SCR_MilitarySymbol symbol = SCR_MilitarySymbol.Cast(BaseContainerTools.CreateInstanceFromContainer(symbolSource));
					Resource symbolResource = BaseContainerTools.CreateContainerFromInstance(symbol);
					info.SetObject("m_MilitarySymbol", symbolResource.GetResource().ToBaseContainer());
				}
			}
		}
		return true;
	}
	protected void SetBudgets(WorldEditorAPI api, IEntitySource entitySource, BaseContainer info, BaseContainer infoCurrent, array<ref ContainerIdPathEntry> path, string varName, array<ref SCR_EntityBudgetValue> budgets)
	{	
		PreserveBudgets(api, infoCurrent, varName, budgets);
		
		api.ClearVariableValue(entitySource, path, varName);
		
		//--- Copy path and add an entry to be set in the loop
		array<ref ContainerIdPathEntry> budgetPath = {};
		foreach (ContainerIdPathEntry entry: path)
		{
			budgetPath.Insert(ContainerIdPathEntry(entry.PropertyName, entry.Index));
		}
		budgetPath.Insert(null);
		
		EEditableEntityBudget listBudgetType;
		int listBudgetValue;
		
		BaseContainerList budgetList;
		int budgetListCount;
		if (info)
		{
			budgetList = info.GetObjectArray(varName);
			if (budgetList)
				budgetListCount = budgetList.Count();
		}
		
		foreach (int i, SCR_EntityBudgetValue entry: budgets)
		{
			EEditableEntityBudget budgetType = entry.GetBudgetType();
			int budgetValue = entry.GetBudgetValue();
			
			//--- Scan existing array to determine whether to override existing entry or create a new one
			int index = -1;
			for (int l = 0; l < budgetListCount; l++)
			{
				if (budgetList.Get(l).Get("m_BudgetType", listBudgetType) && listBudgetType == budgetType)
				{
					if (budgetList.Get(l).Get("m_Value", listBudgetValue) && listBudgetValue == budgetValue)
						index = -2; //--- Value found and is the same - don't save new value
					else
						index = l; //--- Value found and is different - override the existing one
					
					break;
				}
			}
			
			if (index == -2)
			{
				continue;
			}
			if (index == -1)
			{
				//--- Value not found, add a new one
				index = budgetListCount;
				//budgetListCount++;
				api.CreateObjectArrayVariableMember(entitySource, path, varName, "SCR_EntityBudgetValue", index);
			}
			
			//--- Set values
			budgetPath.Set(budgetPath.Count() - 1, ContainerIdPathEntry(varName, index));
			api.SetVariableValue(entitySource, budgetPath , "m_BudgetType", typename.EnumToString(EEditableEntityBudget, budgetType));
			api.SetVariableValue(entitySource, budgetPath , "m_Value", budgetValue.ToString());
		}
	}
	protected void PreserveBudgets(WorldEditorAPI api, BaseContainer infoCurrent, string varName, array<ref SCR_EntityBudgetValue> budgets)
	{
		BaseContainerList budgetList = infoCurrent.GetObjectArray(varName);
		int budgetListCount = budgetList.Count();
		EEditableEntityBudget listBudgetType;
		for (int l = 0; l < budgetListCount; l++)
		{
			budgetList.Get(l).Get("m_BudgetType", listBudgetType);
			
			bool isFound;
			foreach (int i, SCR_EntityBudgetValue entry: budgets)
			{
				if (entry.GetBudgetType() == listBudgetType)
				{
					isFound = true;
					break;
				}
			}
			if (!isFound)
			{
				//--- Add only budgets that are not defined anew
				EEditableEntityBudget budgetType;
				budgetList.Get(l).Get("m_BudgetType", budgetType);
				
				int budgetValue;
				budgetList.Get(l).Get("m_Value", budgetValue);
				
				budgets.Insert(new SCR_EntityBudgetValue(budgetType, budgetValue));
			}
		}
	}
	protected void CreatePreviewImage(EditablePrefabsConfig config, WorldEditorAPI api, out string targetPath, IEntitySource entitySource, string addonName)
	{
		if (!m_ResourceManager) return;
		
		//--- Cannot find image path
		//if (!GetImagePath(config, targetPath)) return;

		//--- Already exists, don't create placeholder, but get GUID of the file
		if (FileIO.FileExists(targetPath))
		{
			string absolutePath;
			Workbench.GetAbsolutePath(targetPath, absolutePath);
			MetaFile metaContainer = m_ResourceManager.GetMetaFile(absolutePath);
			targetPath = metaContainer.GetResourceID();
			return;
		}
		
		string sourceFile = FilePath.ReplaceExtension(FilePath.StripPath(targetPath), m_sImagePlaceholderExt);
		if (sourceFile.IsEmpty() || m_sImagePlaceholderSource.IsEmpty()) return;

		//--- Read source meta file
		string placeholderPath = m_ImagePlaceholder.GetPath();
		string absolutePath;
		Workbench.GetAbsolutePath(placeholderPath, absolutePath);
		
		//--- Create directory
		string imageDirectoryPath = FilePath.StripFileName(targetPath);
		if (!config.CreateDirectoryFor(imageDirectoryPath,addonName)) return;
		
		//--- Copy texture source
		FileIO.CopyFile(m_sImagePlaceholderPath + m_sImagePlaceholderSource, addonName  + FilePath.StripFileName(targetPath) + sourceFile);
		
		//--- Register the file
		Workbench.GetAbsolutePath( addonName + FilePath.StripFileName(targetPath) + sourceFile, absolutePath, false);
		MetaFile metaContainer = m_ResourceManager.RegisterResourceFile(absolutePath);
		if (metaContainer)
		{
			//--- Update meta file
			targetPath = metaContainer.GetResourceID();
			BaseContainerList configurations = metaContainer.GetObjectArray("Configurations");
			if (configurations)
			{
				configurations.Get(0).Set("ColorSpace", "ToSRGB"); //--- Assume PC is the first
				metaContainer.Save();
				Print(string.Format("Editable entity preview image ADDED: @\"%1\"", targetPath), LogLevel.DEBUG);
				return;
			}
		}
		
		Print(string.Format("Editable entity preview image creation FAILED: from @\"%1\"", entitySource.GetResourceName().GetPath()), LogLevel.WARNING);
	}
	protected void DeletePreviewImage(EditablePrefabsConfig config, string prefabPath)
	{
		//--- Delete the texture and its meta file
		if (!GetImagePath(config, prefabPath)) return;
		;
		FileIO.DeleteFile(prefabPath);
		FileIO.DeleteFile(prefabPath + META_EXTENSION);
		
		//--- Delete the source file
		prefabPath.Replace(IMG_EXTENSION, m_sImagePlaceholderExt);
		FileIO.DeleteFile(prefabPath);
	}
	protected void MovePreviewImage(EditablePrefabsConfig config, string currentPath, string newPath)
	{		
		//--- Move texture
		if (!GetImagePath(config, currentPath)) return;
		if (!GetImagePath(config, newPath)) return;
		
		string currentSourcePath = currentPath;
		string newSourcePath = newPath;
		currentSourcePath.Replace(IMG_EXTENSION, m_sImagePlaceholderExt);
		newSourcePath.Replace(IMG_EXTENSION, m_sImagePlaceholderExt);
		
		FileIO.CopyFile(currentSourcePath, newSourcePath);
		config.MoveFile(currentPath, newPath); //--- Move the texture
		FileIO.DeleteFile(currentSourcePath); //--- Delete the old source file only after the texture was renamed, otherwise registration will complain
	}
	protected bool GetImagePath(EditablePrefabsConfig config, out string prefabPath)
	{
		if (m_sImagesPath.IsEmpty()) return false;
		
		prefabPath = FilePath.ReplaceExtension(prefabPath, IMG_EXTENSION);
		if (prefabPath.Replace(config.GetTargetPath(), m_sImagesPath) != 0) return true;
		if (prefabPath.Replace(config.GetSourcePath(), m_sImagesPath) != 0) return true;
		return false;
	}
	protected void SetEntityFlags(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource, IEntityComponentSource componentSource, IEntityComponentSource componentCurrent)
	{
		EEditableEntityFlag flags, flagsOrig;
		componentSource.Get("m_Flags", flagsOrig);
		flags = flagsOrig;
		
		//--- Composition
		IEntityComponentSource compositionComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_SlotCompositionComponent);
		if (compositionComponent)
		{
			bool orientChildren;
			compositionComponent.Get("m_bOrientChildrenToTerrain", orientChildren);
			if (orientChildren)
				flags = flags | EEditableEntityFlag.ORIENT_CHILDREN;
			else
				flags = flags & ~EEditableEntityFlag.ORIENT_CHILDREN;
		}
		
		if (flags != flagsOrig)
			componentSource.Set("m_Flags", flags);
	}
	
	protected bool GetLabelsFromSource(WorldEditorAPI api,EditablePrefabsConfig config, ResourceName prefab, string targetPath, IEntitySource entitySource, IEntityComponentSource componentSource, IEntityComponentSource componentSourceCurrent, notnull array<EEditableEntityLabel> autoLabels, notnull array<EEditableEntityLabel> authoredLabels)
	{
		EEditableEntityType entityType = EEditableEntityType.GENERIC;
		
		// Read data from existing editable component first, or fall back on new editable component
		if (componentSourceCurrent)
		{
			ReadEditableEntityComponent(componentSourceCurrent, entityType, authoredLabels);
		}
		else
		{
			ReadEditableEntityComponent(componentSource, entityType, authoredLabels);
		}
		
		// Validate label rules, add label if unique
		foreach	(EditablePrefabsLabel_Base labelRule : m_EntityLabelRules)
		{
			EEditableEntityLabel label;
			if (labelRule.GetLabelValid(api, entitySource, componentSource, targetPath, entityType, authoredLabels, label))
			{
				if (!autoLabels.Contains(label) && !authoredLabels.Contains(label))
				{
					//PrintFormat("Autoconfig Label added: %1", typename.EnumToString(EEditableEntityLabel, label));
					autoLabels.Insert(label);	
				}
			}
		}
		return true;
	}
	
	protected void ReadEditableEntityComponent(IEntityComponentSource componentSource, out EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels)
	{
		// Read data from existing editable entity
		if (componentSource)
		{
			BaseContainer componentUIInfoCurrent = componentSource.GetObject("m_UIInfo");
			if (componentUIInfoCurrent)
			{
				array<EEditableEntityLabel> componentAuthoredLabels = {};
				componentUIInfoCurrent.Get("m_aAuthoredLabels", authoredLabels);
			}
			
			// Read editable entity type
			componentSource.Get("m_EntityType", entityType);
		}
	}
	
	protected void GetEntityChildrenBudgetCostsFromSource(IEntitySource entitySource, out notnull array<ref SCR_EntityBudgetValue> entityBudgetCosts)
	{
		IEntityComponentSource compositionLinkComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_EditorLinkComponent);
		
		if (entitySource.GetClassName().ToType().IsInherited(SCR_AIGroup))
		{
			GetEntityBudgetCostsFromGroup(entitySource, entityBudgetCosts);
		}
		else if (compositionLinkComponent)
		{
			GetEntityBudgetCostsFromLinkComponent(entitySource, compositionLinkComponent, entityBudgetCosts);
		}
	}
	
	protected void GetEntityBudgetCostsFromVehicle(IEntitySource entitySource, out notnull array<ref SCR_EntityBudgetValue> crewBudgetCosts, out notnull array<ref SCR_EntityBudgetValue> passengerBudgetCosts, out notnull array<ECompartmentType> vehicleCompartmentTypes)
	{
		IEntityComponentSource componentSource;
		typename componentType;
		BaseContainerList slots;
		BaseContainer slot, occupantData;
		ResourceName prefab;
		IEntitySource slotEntity;
		
		array<IEntitySource> queue = {entitySource};
		while (!queue.IsEmpty())
		{
			IEntitySource source = queue[0];
			queue.Remove(0);
			
			for (int c = 0, componentCount = source.GetComponentCount(); c < componentCount; c++)
			{
				componentSource = source.GetComponent(c);
				componentType = componentSource.GetClassName().ToType();
				
				if (componentType.IsInherited(BaseCompartmentManagerComponent))
				{
					//--- Find compartments and get budgets of their default occupants
					slots = componentSource.GetObjectArray("CompartmentSlots");
					for (int s = 0, slotCount = slots.Count(); s < slotCount; s++)
					{
						slot = slots[s];
						
						occupantData = slot.GetObject("m_DefaultOccupantData");
						if (!occupantData)
							continue;
						
						if (!occupantData.Get("m_sDefaultOccupantPrefab", prefab))
							continue;
						
						slotEntity = SCR_BaseContainerTools.FindEntitySource(Resource.Load(prefab));
						if (!slotEntity)
							continue;
						
						//--- Check for slot type (any type apart from cargo counts as crew)
						typename slotType = slot.GetClassName().ToType();
						ECompartmentType compartmentType;
						if (slotType.IsInherited(CargoCompartmentSlot))
						{
							AddBudgetCostsFromEntity(slotEntity, passengerBudgetCosts);
							compartmentType = ECompartmentType.Cargo;
						}
						else
						{
							AddBudgetCostsFromEntity(slotEntity, crewBudgetCosts);
							
							if (slotType.IsInherited(PilotCompartmentSlot))
								compartmentType = ECompartmentType.Pilot;
							else
								compartmentType = ECompartmentType.Turret;
						}
						if (!vehicleCompartmentTypes.Contains(compartmentType))
							vehicleCompartmentTypes.Insert(compartmentType);
					}
				}
				else if (componentType.IsInherited(SlotManagerComponent))
				{
					//--- Go deeper into base slots
					slots = componentSource.GetObjectArray("Slots");
					for (int s = 0, slotCount = slots.Count(); s < slotCount; s++)
					{
						if (!slots[s].Get("Prefab", prefab))
							continue;
						
						slotEntity = SCR_BaseContainerTools.FindEntitySource(Resource.Load(prefab));
						if (slotEntity)
							queue.Insert(slotEntity);
					}
				}
			}
		}
	}
	
	protected void GetEntityBudgetCostsFromGroup(IEntitySource entitySource, out notnull array<ref SCR_EntityBudgetValue> entityBudgetCosts)
	{
		array<ResourceName> memberPrefabs = {};
		
		entitySource.Get("m_aUnitPrefabSlots", memberPrefabs);
		
		AddBudgetCostsFromEntities(memberPrefabs, entityBudgetCosts);
	}
	
	protected void GetEntityBudgetCostsFromLinkComponent(IEntitySource entitySource, IEntityComponentSource compositionLinkComponent, out notnull array<ref SCR_EntityBudgetValue> entityBudgetCosts)
	{
		array<ResourceName> childPrefabs = {};
		
		if (compositionLinkComponent)
		{
			BaseContainerList entries = compositionLinkComponent.GetObjectArray("m_aEntries");
			for (int e = 0, count = entries.Count(); e < count; e++)
			{
				SCR_EditorLinkEntry compositionChild = SCR_EditorLinkEntry.Cast(BaseContainerTools.CreateInstanceFromContainer(entries.Get(e)));
				if (!compositionChild)
					continue;
				
				IEntitySource childEntitySource = SCR_BaseContainerTools.FindEntitySource(Resource.Load(compositionChild.m_Prefab));
				if (!childEntitySource)
					continue;
				
				IEntityComponentSource childEditableEntity = SCR_BaseContainerTools.FindComponentSource(childEntitySource, SCR_EditableEntityComponent);
				if (!childEditableEntity)
					continue;
				
				SCR_EditableEntityUIInfo childUiInfo = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(childEditableEntity));
				if (!childUiInfo)
					continue;
				
				array<ref SCR_EntityBudgetValue> entityPreviewBudgetCost = {};
				childUiInfo.GetEntityChildrenBudgetCost(entityPreviewBudgetCost);
				
				SCR_EntityBudgetValue.MergeBudgetCosts(entityBudgetCosts, entityPreviewBudgetCost);
				
				childPrefabs.Insert(compositionChild.m_Prefab);
			}
		}
		
		AddBudgetCostsFromEntities(childPrefabs, entityBudgetCosts);
	}
	
	protected void AddBudgetCostsFromEntities(array<ResourceName> childPrefabs, out notnull array<ref SCR_EntityBudgetValue> entityBudgetCosts)
	{
		int childCount = childPrefabs.Count();		
		for	(int i = 0; i < childCount; i++)
		{
			IEntitySource memberEntitySource = SCR_BaseContainerTools.FindEntitySource(Resource.Load(childPrefabs[i]));
			if (memberEntitySource)
			{
				AddBudgetCostsFromEntity(memberEntitySource, entityBudgetCosts);
			}
		}
	}
	
	protected void AddBudgetCostsFromEntity(IEntitySource entitySource, out notnull array<ref SCR_EntityBudgetValue> entityBudgetCosts)
	{
		IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource);
		if (editableEntitySource)
		{
			array<ref SCR_EntityBudgetValue> budgetCosts = {};
			if (SCR_EditableEntityComponentClass.GetEntitySourceBudgetCost(editableEntitySource, budgetCosts))
			{
				SCR_EntityBudgetValue.MergeBudgetCosts(entityBudgetCosts, budgetCosts);
			}
			else
			{
				EEditableEntityBudget entityBudgetType = m_EntityCoreConfig.GetBudgetForEntityType(SCR_EditableEntityComponentClass.GetEntityType(editableEntitySource));
				int minimumBudgetCost = m_MinBudgetCost.Get(entityBudgetType);
				
				SCR_EntityBudgetValue.MergeBudgetCosts(entityBudgetCosts, {new SCR_EntityBudgetValue(entityBudgetType, minimumBudgetCost)});
			}
		}
	}
	
	override void EOnCreate(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource, IEntitySource instanceEntitySource, IEntityComponentSource componentSource, IEntityComponentSource componentCurrent)
	{		
		SetEntityFlags(config, api, prefab, targetPath, entitySource, componentSource, componentCurrent);
		SetUIInfo(config, api, prefab, targetPath, entitySource, instanceEntitySource, componentSource, componentCurrent);
		
		//--- Preserve custom icon pos
		if (componentCurrent && componentCurrent.IsVariableSetDirectly("m_vIconPos"))
		{
			vector iconPos;
			componentCurrent.Get("m_vIconPos", iconPos);
			componentSource.Set("m_vIconPos", iconPos.ToString(false));
		}
	}
	override void EOnDelete(EditablePrefabsConfig config, WorldEditorAPI api, string prefabPath)
	{
		DeletePreviewImage(config, prefabPath);
	}
	override void EOnMove(EditablePrefabsConfig config, WorldEditorAPI api, string currentPath, string newPath)
	{
		MovePreviewImage(config, currentPath, newPath);
	}
	
	void EditablePrefabsComponent_EditableEntity()
	{
		if (m_ImagePlaceholder.IsEmpty())
		{
			Print("m_ImagePlaceholder not defined!", LogLevel.WARNING);
			return;
		}
		
		m_ResourceManager = Workbench.GetModule(ResourceManager);
		if (!m_ResourceManager) return;
		
		BaseContainer configContainer = BaseContainerTools.LoadContainer(m_EntityCoreConfigPrefab).GetResource().ToBaseContainer();
		// Combining two lines below will return null
		Managed configInstance = BaseContainerTools.CreateInstanceFromContainer(configContainer);
		m_EntityCoreConfig = SCR_EditableEntityCore.Cast(configInstance);
		if (!m_EntityCoreConfig)
		{
			Print("Editable entity core instance could not be created", LogLevel.ERROR);
			return;
		}
		
		array<ref SCR_EditableEntityCoreBudgetSetting> budgetSettings = {};
		m_EntityCoreConfig.GetBudgets(budgetSettings);
		
		foreach (SCR_EditableEntityCoreBudgetSetting budget : budgetSettings)
		{
			m_MinBudgetCost.Set(budget.GetBudgetType(), budget.GetMinBudgetCost());
		}
		
		string absolutePath;
		Workbench.GetAbsolutePath(m_ImagePlaceholder.GetPath(), absolutePath);
		
		m_sImagesPath = m_ImagesDirectory.GetPath();
		m_sImagePlaceholderPath = FilePath.StripFileName(m_ImagePlaceholder.GetPath());
		
		MetaFile metaContainer = m_ResourceManager.GetMetaFile(absolutePath);
		if (metaContainer)
		{			
			//--- Get source file and extension
			BaseContainerList configurations = metaContainer.GetObjectArray("Configurations");
			for (int i = 0, count = configurations.Count(); i < count; i++)
			{
				BaseContainer configuration = configurations.Get(i);
		
				//--- Read extension from source file - obsolete method
				configuration.Get("SourceFile", m_sImagePlaceholderSource);
				if (!m_sImagePlaceholderSource.IsEmpty())
				{
					FilePath.StripExtension(m_sImagePlaceholderSource, m_sImagePlaceholderExt);
					continue;
				}
				
				//--- Get extension from configuration class - only PNG is allowed
				if (configuration.GetClassName() == "PNGResourceClass")
				{
					m_sImagePlaceholderExt = "png";
					m_sImagePlaceholderSource = FilePath.ReplaceExtension(m_ImagePlaceholder, m_sImagePlaceholderExt);
					m_sImagePlaceholderSource = FilePath.StripPath(m_sImagePlaceholderSource);
					continue;
				}
				else
				{
					Print(string.Format("Placeholder file configuration is of type %1, must be PNGResourceClass!", configuration.GetClassName()), LogLevel.ERROR);
				}
			}			
		}
	}
	
	void ~EditablePrefabsComponent_EditableEntity()
	{
		m_EntityCoreConfig = null;
	}
};