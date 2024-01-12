void ScriptInvoker_EntityCoreBudgetUpdated(EEditableEntityBudget type, int originalBudgetValue, int budgetChange, int updatedBudgetValue, SCR_EditableEntityComponent entity);
typedef func ScriptInvoker_EntityCoreBudgetUpdated;
typedef ScriptInvokerBase<ScriptInvoker_EntityCoreBudgetUpdated> ScriptInvoker_EntityCoreBudgetUpdatedEvent;

/// @ingroup Editor_Core GameCore Editable_Entities
/*!
Core component to manage SCR_EditableEntityComponent.

The list of editable entities tracked here is local!
*/
[BaseContainerProps(configRoot: true)]
class SCR_EditableEntityCore: SCR_GameCoreBase
{
	[Attribute(desc: "Settings for every entity type.")]
	private ref array<ref SCR_EditableEntityCoreTypeSetting> m_TypeSettings;
	
	[Attribute("1000", desc: "Draw distance override for player characters.")]
	protected float m_fPlayerDrawDistance;
	
	[Attribute(defvalue: "0.01", desc: "The distance modifier for players in vehicles which will be used to determine player filter's visibility.")]
	protected float m_fPlayerVehicleDistanceModifier;

	[Attribute(desc: "Budget settings for every entity type.")]
	private ref array<ref SCR_EditableEntityCoreBudgetSetting> m_BudgetSettings;
	
	[Attribute(desc: "Label Groups")]
	private ref array<ref SCR_EditableEntityCoreLabelGroupSetting> m_LabelGroupSettings;
	
	[Attribute(desc: "Label configs")]
	private ref array<ref SCR_EditableEntityCoreLabelSetting> m_EntityLabels;
	
	private ref map<EEditableEntityType, SCR_EditableEntityCoreTypeSetting> m_TypeSettingsMap = new map<EEditableEntityType, SCR_EditableEntityCoreTypeSetting>;
	
	private ref map<EEditableEntityLabelGroup, ref array<SCR_EditableEntityCoreLabelSetting>> m_LabelListMap = new map<EEditableEntityLabelGroup, ref array<SCR_EditableEntityCoreLabelSetting>>;
	private ref map<EEditableEntityLabelGroup, SCR_EditableEntityCoreLabelGroupSetting> m_LabelGroupSettingsMap = new map<EEditableEntityLabelGroup, SCR_EditableEntityCoreLabelGroupSetting>;
	private ref map<EEditableEntityLabel, SCR_EditableEntityCoreLabelSetting> m_LabelSettingsMap = new map<EEditableEntityLabel, SCR_EditableEntityCoreLabelSetting>;
	
	private ref set<SCR_EditableEntityComponent> m_Entities;
	ref map<RplId, ref array<RplId>> m_OrphanEntityIds = new map<RplId, ref array<RplId>>();
	private SCR_EditableEntityComponent m_CurrentLayer;
	
	/*! Called when an entity is made editable */
	ref ScriptInvoker Event_OnEntityRegistered = new ScriptInvoker;
	/*! Called when an entity is made not editable */
	ref ScriptInvoker Event_OnEntityUnregistered = new ScriptInvoker;
	/*! Called when manual refresh of the entity is triggered */
	ref ScriptInvoker Event_OnEntityRefreshed = new ScriptInvoker;
	/*! Called when entity is changes parent */
	ref ScriptInvoker Event_OnParentEntityChanged = new ScriptInvoker;
	/*! Called when entity access key is modified */
	ref ScriptInvoker Event_OnEntityAccessKeyChanged = new ScriptInvoker;
	/*! Called when entity access keys are updated */
	ref ScriptInvoker Event_OnEntityVisibilityChanged = new ScriptInvoker;
	/*! Called when entity transformation is changed by the editor */
	ref ScriptInvoker Event_OnEntityTransformChanged = new ScriptInvoker;
	/*! Called when entity transformation is changed by the editor only server only. Sends EditableEntity and Prev Transform*/
	ref ScriptInvoker Event_OnEntityTransformChangedServer = new ScriptInvoker;
	/*! Called when entity budget is updated */
	ref ScriptInvoker_EntityCoreBudgetUpdatedEvent Event_OnEntityBudgetUpdated = new ScriptInvoker_EntityCoreBudgetUpdatedEvent();
	
	/*! Called when entity is extended or cease to be extended */
	ref ScriptInvoker Event_OnEntityExtendedChange = new ScriptInvoker;
	
	void AddToRoot(SCR_EditableEntityComponent entity)
	{
		if (!m_Entities) m_Entities = new set<SCR_EditableEntityComponent>;
		if (m_Entities.Find(entity) < 0) m_Entities.Insert(entity);
	}
	void RemoveFromRoot(SCR_EditableEntityComponent entity)
	{
		if (!m_Entities) return;
		
		int index = m_Entities.Find(entity);
		if (index != -1) m_Entities.Remove(index);
	}
	void RegisterEntity(SCR_EditableEntityComponent entity)
	{
		//--- Get settings for the entity based on its type
		SCR_EditableEntityCoreTypeSetting setting = null;
		if (m_TypeSettingsMap.Find(entity.GetEntityType(), setting))
		{
			//--- Set default max draw distance
			if (entity.GetMaxDrawDistanceSq() <= 0)
				entity.SetMaxDrawDistance(setting.GetMaxDrawDistance());
		}
		else
		{
			Print(string.Format("Default type settings not found for '%1'!", Type().EnumToString(EEditableEntityType, entity.GetEntityType())), LogLevel.ERROR);
		}
		
		Event_OnEntityRegistered.Invoke(entity);

		//:| Player ownership isn't ready if we don't wait for a frame before calling UpdateBudgets.
		//:| This results in Player ownership related checks to have an undefined behavior.
		GetGame().GetCallqueue().CallLater(UpdateBudgets, 0, false, entity, true, entity.GetOwnerScripted());
	}
	void UnRegisterEntity(SCR_EditableEntityComponent entity, IEntity owner = null)
	{
		UpdateBudgets(entity, false, owner);
		Event_OnEntityUnregistered.Invoke(entity);
	}
	
	/*
	Add orphaned entity.
	Than can happen when an entity is intialized, but its parent was not yet streamed in.
	In such case the entity is added to the list of orphans. Once the parent is initialized, it will be added to it.
	\param parentId Replication ID of the parent entity
	\param parentId Replication ID of the orphan entity
	*/
	void AddOrphan(RplId parentId, RplId orphanId)
	{
		array<RplId> orphanIds;
		if (!m_OrphanEntityIds.Find(parentId, orphanIds))
			orphanIds = {};
		
		orphanIds.Insert(orphanId);
		m_OrphanEntityIds.Insert(parentId, orphanIds);
	}
	/*
	Remove orphaned entities belonging to given parent.
	\param parentId Replication ID of the parent entity
	\param[out] outOrphans Array to be filled with orphaned entities
	\return Number of orphans
	*/
	int RemoveOrphans(RplId parentId, out notnull array<SCR_EditableEntityComponent> outOrphans)
	{
		if (!parentId.IsValid())
			return 0;
		
		//--- Check if there are some orphans for given parent. If not, terminate
		array<RplId> orphanIds = {};
		if (!m_OrphanEntityIds.Find(parentId, orphanIds))
			return 0;
		
		//--- Find all editable orphans
		SCR_EditableEntityComponent orphan;
		for (int i = orphanIds.Count() - 1; i >= 0; i--)
		{
			orphan = SCR_EditableEntityComponent.Cast(Replication.FindItem(orphanIds[i]));
			if (orphan)
			{
				outOrphans.Insert(orphan);
				orphanIds.Remove(i);
			}
		}
		
		//--- No orphans left, unregister the parent
		if (orphanIds.IsEmpty())
			m_OrphanEntityIds.Remove(parentId);
		
		return outOrphans.Count();
	}
	
	/*!
	Get all entities.
	\param[out] entities Array to be filled with child entities
	\param onlyDirect When true, only the direct descendants are returned, otherwise all children, children of children etc. are returned.
	\param skipIgnored When true, entities flagged by IGNORE_LAYERS will not be included in the list
	*/
	void GetAllEntities(out notnull set<SCR_EditableEntityComponent> entities, bool onlyDirect = false, bool skipIgnored = false)
	{
		entities.Clear();
		if (!m_Entities) return;
		foreach (SCR_EditableEntityComponent entity: m_Entities)
		{
			entities.Insert(entity);
			if (!onlyDirect) entity.GetChildren(entities, onlyDirect, skipIgnored);
		}
	}
	/*!
	Get all editable entities with specified access keys.
	\param[out] entities Array to be filled with editable entities
	\param accessKey Access key. Only entities with at least one compatible key will be returned.
	*/
	void GetAllEntities(out notnull set<SCR_EditableEntityComponent> entities, EEditableEntityAccessKey accessKey)
	{
		entities.Clear();
		if (!m_Entities) return;
		foreach (SCR_EditableEntityComponent entity: m_Entities)
		{
			if (!entity.HasAccessSelf(accessKey)) continue;
			
			entities.Insert(entity);
			
			if (entity.IsLayer())
			{
				set<SCR_EditableEntityComponent> subEntities = new set<SCR_EditableEntityComponent>;
				entity.GetChildren(subEntities, false, accessKey);
				foreach (SCR_EditableEntityComponent child: subEntities)
				{
					entities.Insert(child);
				}
				//entities.InsertAll(subEntities)
			}
		}
	}
	
	/*!
	Find nearest entity to given position.
	\param pos Position from which the distance is measured
	\param type Required entity type
	\param flags Required entity flags
	\param onlyDirect True to scan only root entities, false to scan all editable entities in the world
	\return Editable entity
	*/
	SCR_EditableEntityComponent FindNearestEntity(vector pos, EEditableEntityType type, EEditableEntityFlag flags = 0, bool onlyDirect = true)
	{
		float nearestDis = float.MAX;
		SCR_EditableEntityComponent nearestEntity;
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		if (onlyDirect)
			entities = m_Entities;
		else
			GetAllEntities(entities);
		
		foreach (SCR_EditableEntityComponent entity: m_Entities)
		{
			if (entity.GetEntityType() != type || !entity.HasEntityFlag(flags))
				continue;
			
			vector entityPos;
			if (!entity.GetPos(entityPos))
				continue;
			
			float dis = vector.DistanceSq(entityPos, pos);
			if (dis > nearestDis)
				continue;
			
			nearestDis = dis;
			nearestEntity = entity;
		}
		return nearestEntity;
	}
	
	/*!
	Get interaction class for given entity type.
	\param type Entity type
	\return Interaction rules
	*/
	SCR_EditableEntityInteraction GetEntityInteraction(EEditableEntityType type)
	{
		SCR_EditableEntityCoreTypeSetting setting;
		if (m_TypeSettingsMap.Find(type, setting))
			return setting.GetInteraction();
		else
			return null;
	}
	/*!
	Check if entity can be controlled by player, used for delaying budget update/AI check
	\param EEditableEntityType type of the entity to check
	\return bool if given entity can be controlled by player
	*/
	bool GetEntityCanBeControlled(EEditableEntityType type)
	{
		SCR_EditableEntityCoreTypeSetting setting;
		return m_TypeSettingsMap.Find(type, setting) && setting.GetCanBePlayer();
	}
	
	EEditableEntityBudget GetBudgetForEntityType(EEditableEntityType entityType)
	{
		EEditableEntityBudget budget;
		switch (entityType)
		{
			case EEditableEntityType.GENERIC:
			case EEditableEntityType.ITEM:
				budget = EEditableEntityBudget.PROPS;
				break;
			case EEditableEntityType.CHARACTER:
			case EEditableEntityType.GROUP:
				budget = EEditableEntityBudget.AI;
				break;
			case EEditableEntityType.VEHICLE:
				budget = EEditableEntityBudget.VEHICLES;
				break;
			case EEditableEntityType.WAYPOINT:
			case EEditableEntityType.COMMENT:
			case EEditableEntityType.SYSTEM:
			case EEditableEntityType.TASK:
				budget = EEditableEntityBudget.SYSTEMS;
				break;
			default:
				budget = EEditableEntityBudget.PROPS;
				break;
		}
		return budget;
	}
	
	/*!
	Get all entity budget settings
	\param[out] Array with entity budget settings
	*/
	void GetBudgets(out notnull array<ref SCR_EditableEntityCoreBudgetSetting> budgets)
	{
		foreach (SCR_EditableEntityCoreBudgetSetting budget : m_BudgetSettings)
		{
			budgets.Insert(budget);
		}
	}
	
	/*!
	Get current budget settings of given type
	\param[out] SCR_EditableEntityCoreBudgetSetting
	*/
	bool GetBudget(EEditableEntityBudget budgetType, out SCR_EditableEntityCoreBudgetSetting budgetSettings)
	{
		foreach (SCR_EditableEntityCoreBudgetSetting budget : m_BudgetSettings)
		{
			if (budget.GetBudgetType() == budgetType)
			{
				budgetSettings = budget;
				return true;
			}
		}
		return false;
	}
	
	/*!
	Get all entity label group settings
	\param[out] Array with current entity label groups
	*/
	void GetLabelGroups(out notnull array<ref SCR_EditableEntityCoreLabelGroupSetting> labelGroups)
	{
		foreach ( SCR_EditableEntityCoreLabelGroupSetting labelGroup : m_LabelGroupSettings)
		{
			int index = labelGroups.Count();
			
			for (int i = 0; i < index; i++)
			{
				if (labelGroups[i].GetOrder() >= labelGroup.GetOrder())
				{
					index = i;
					break;
				}
			}
			
			labelGroups.InsertAt(labelGroup, index);
		}
	}
	
	/*!
	Get the order of the given group type
	\param groupLabel given group type
	\return order
	*/
	int GetLabelGroupOrder(EEditableEntityLabelGroup groupLabel)
	{
		foreach ( SCR_EditableEntityCoreLabelGroupSetting labelGroup : m_LabelGroupSettings)
		{
			if (labelGroup.GetLabelGroupType() == groupLabel)
				return labelGroup.GetOrder();
		}
		
		return -1;
	}
	
	/*!
	Get group enum value of passed entity label
	\param EEditableEntityLabel enum value
	\param[out] Label group enum value this entity label belongs to.
	\return True when label is part of a group and found valid result.
	*/
	bool GetLabelGroupType(EEditableEntityLabel label, out EEditableEntityLabelGroup labelGroup)
	{
		SCR_EditableEntityCoreLabelSetting labelSettings;
		if (!m_LabelSettingsMap.Find(label, labelSettings))
			return false;
		
		labelGroup = labelSettings.GetLabelGroupType();
		return labelGroup != EEditableEntityLabelGroup.NONE;
	}
	
	/*!
	Get all label settings of the passed label group
	\param EEditableEntityLabelGroup enum value of group
	\param[out] Array with all label settings of labels in this group
	\return True if group has labels defined
	*/
	bool GetLabelsOfGroup(EEditableEntityLabelGroup groupType, out notnull array<SCR_EditableEntityCoreLabelSetting> labels)
	{
		return m_LabelListMap.Find(groupType, labels);
	}
	
	/*!
	Get UI info of passed entity label
	\param EEditableEntityLabel enum value
	\param[out] SCR_UIInfo matching the passed label enum value
	\return True if UI Info was found and is valid
	*/
	bool GetLabelUIInfo(EEditableEntityLabel entityLabel, out SCR_UIInfo uiInfo)
	{
		SCR_EditableEntityCoreLabelSetting labelSetting = m_LabelSettingsMap.Get(entityLabel);
		if (labelSetting)
			uiInfo = labelSetting.GetInfo();

		return uiInfo != null;
	}
	
	/*!
	Get UI info of passed entity label if the label is valid to display
	\param EEditableEntityLabel enum value
	\param currentMode Current editor mode
	\param[out] SCR_UIInfo matching the passed label enum value
	\return True if UI Info was found and is valid
	*/
	bool GetLabelUIInfoIfValid(EEditableEntityLabel entityLabel, EEditorMode currentMode, out SCR_UIInfo uiInfo)
	{
		SCR_EditableEntityCoreLabelSetting labelSetting = m_LabelSettingsMap.Get(entityLabel);
		if (labelSetting)
		{
			if (labelSetting.IsValid(currentMode))
				uiInfo = labelSetting.GetInfo();
			else 
				uiInfo = null;
		}
			
		return uiInfo != null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get UI infos and Linked Confict servece point of passed entity label for BuildMode specific Labels
	Will ignore any non-buildmode labels as well as any build mode labels that have neither UIInfo nor linked conflict service point
	\param entityLabel label to get data from
	\return A class with specific build mode label data. Which has such info as: Label, UiInfo and Linked conflict service point. Null if getting the data for the specific label fails
	*/
	SCR_EditableEntityCampaignBuildingModeLabelData GetBuildModeLabelData(EEditableEntityLabel entityLabel)
	{
		SCR_EditableEntityCampaignBuildingLabelSetting buildModeLabelSetting = SCR_EditableEntityCampaignBuildingLabelSetting.Cast(m_LabelSettingsMap.Get(entityLabel));
		if (!buildModeLabelSetting)
			return null;
		
		SCR_UIInfo uiInfo = buildModeLabelSetting.GetInfo();
		SCR_EServicePointType linkedConflictServicePoint = buildModeLabelSetting.GetLinkedConflictServicePoint();
		if (!uiInfo && linkedConflictServicePoint < 0)
			return null;
		
		return new SCR_EditableEntityCampaignBuildingModeLabelData(entityLabel, uiInfo, linkedConflictServicePoint);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get UI infos and Linked Confict servece point of all passed entity labels for BuildMode specific Labels
	Will ignore any non-buildmode labels as well as any build mode labels that have neither UIInfo nor linked conflict service point
	\param entityLabels labels to get data from
	\param[out] validBuildmodeLabelData array of specific build mode label data. Which has such info as: Label, UiInfo and Linked conflict service point
	\return Count of valid build mode lables.
	*/
	int GetCampaignBuildingModeLabelsData(notnull array<EEditableEntityLabel> entityLabels, notnull out array<ref SCR_EditableEntityCampaignBuildingModeLabelData> validBuildmodeLabelData)
	{
		validBuildmodeLabelData.Clear();
		
		SCR_EditableEntityCampaignBuildingLabelSetting buildModeLabelSetting;
		SCR_UIInfo uiInfo;
		SCR_EServicePointType linkedConflictServicePoint;
		
		foreach (EEditableEntityLabel entityLabel : entityLabels)
		{			
			if (entityLabel == EEditableEntityLabel.TRAIT_SERVICE)
				continue;
			
			buildModeLabelSetting = SCR_EditableEntityCampaignBuildingLabelSetting.Cast(m_LabelSettingsMap.Get(entityLabel));
			if (!buildModeLabelSetting)
				continue;
			
			uiInfo = buildModeLabelSetting.GetInfo();
			linkedConflictServicePoint = buildModeLabelSetting.GetLinkedConflictServicePoint();
			
			if (!uiInfo && linkedConflictServicePoint < 0)
				continue;
			
			validBuildmodeLabelData.Insert(new SCR_EditableEntityCampaignBuildingModeLabelData(entityLabel, uiInfo, linkedConflictServicePoint));
		}
		
		return validBuildmodeLabelData.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get UI info of passed entity label for BuildMode labels. Will return null if no UIInfo assigned or if not a building mode specific label
	\param entityLabel label to get data from
	\return UiInfo
	*/
	SCR_UIInfo GetBuildModeLabelUIInfo(EEditableEntityLabel entityLabel)
	{
		SCR_EditableEntityCampaignBuildingLabelSetting buildModeLabelSetting = SCR_EditableEntityCampaignBuildingLabelSetting.Cast(m_LabelSettingsMap.Get(entityLabel));
		if (!buildModeLabelSetting)
			return null;
		
		return buildModeLabelSetting.GetInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get linked conflict service point of passed entity label for BuildMode labels. Will ignore any non assigned values (-1 or less)
	\param entityLabel label to get data from
	\return Linked conflict service point
	*/
	SCR_EServicePointType GetBuildModeLabelLinkedConflictService(EEditableEntityLabel entityLabel)
	{
		SCR_EditableEntityCampaignBuildingLabelSetting buildModeLabelSetting = SCR_EditableEntityCampaignBuildingLabelSetting.Cast(m_LabelSettingsMap.Get(entityLabel));
		if (!buildModeLabelSetting)
			return -1;
		
		return buildModeLabelSetting.GetLinkedConflictServicePoint();
	}
		
	//------------------------------------------------------------------------------------------------
	/*!
	Get specific label order. Returns -1 if label not found
	\return int label order.
	*/
	int GetLabelOrder(EEditableEntityLabel entityLabel)
	{
		//~ Labels do not use order so get array index
		array<SCR_EditableEntityCoreLabelSetting> labels = {};
		EEditableEntityLabelGroup groupLabel;
		GetLabelGroupType(entityLabel, groupLabel);
		
		if (!GetLabelsOfGroup(groupLabel, labels))
			return -1;
		
		int count = labels.Count();
		
		for(int i = 0; i < count; i++)
        {
            if (labels[i].GetLabelType() == entityLabel)
				return i;
        }
		
		return -1;
	}
	
	/*!
	Get a label array and order it according to group and label index
	\return[inout] labels that will be ordered
	*/
	void OrderLabels(inout notnull array<EEditableEntityLabel> labels)
	{
		if (labels.IsEmpty())
			return;
		
		array<EEditableEntityLabelGroup> orderedGroups = {};
		map<EEditableEntityLabelGroup, ref array<EEditableEntityLabel>> groupsWithLabels = new map<EEditableEntityLabelGroup, ref array<EEditableEntityLabel>>();
		
		int groupLabelCount = 0;
		bool groupAdded = false;
		EEditableEntityLabelGroup groupLabel;

		//~ Get label groups of the given label and get order of the group
		foreach (EEditableEntityLabel label: labels)
		{
			GetLabelGroupType(label, groupLabel);
			
			if (!groupsWithLabels.Contains(groupLabel))
				groupsWithLabels.Insert(groupLabel, new ref array<EEditableEntityLabel>());
				
			groupsWithLabels[groupLabel].Insert(label);

			//~ New list so add it and continue
			if (orderedGroups.IsEmpty())
			{
				orderedGroups.Insert(groupLabel);
				groupLabelCount++;
				continue;
			}
			
			//~ Already in list so continue
			if (orderedGroups.Contains(groupLabel))
				continue;
			
			groupAdded = false;
			//~ Check existing and place it in the correct order
			for(int i = 0; i < groupLabelCount; i++)
	        {
				//~ Group Label is higher order so add it to the list and break loop
				if (GetLabelGroupOrder(groupLabel) <= GetLabelGroupOrder(orderedGroups[i]))
				{
					orderedGroups.InsertAt(groupLabel, i);
					groupLabelCount++;
					groupAdded = true;
					break;
				}
	        }
			
			if (groupAdded)
				continue;
			
			orderedGroups.Insert(groupLabel);
			groupLabelCount++;
		}
		
		array<EEditableEntityLabel> orderedLabels = {};
		int orderedLabelCount;
		bool labelAdded = false;
		array<EEditableEntityLabel> allOrderedLabels = {};
		
		array<EEditableEntityLabel> labelsTest = {};
		
		//~ For each ordered group get the labels and order those as well
		foreach (EEditableEntityLabelGroup group: orderedGroups)
		{			
			orderedLabelCount = 0;
			orderedLabels.Clear();
			
			//~ Go over each label in the group and order them from highest to lowest
			foreach (EEditableEntityLabel label: groupsWithLabels[group])
			{
				//~ New list so add it and continue
				if (orderedLabels.IsEmpty())
				{
					orderedLabels.Insert(label);
					orderedLabelCount++;
					continue;
				}
				
				labelAdded = false;
				//~ Check existing and place it in the correct order
				for(int i = 0; i < orderedLabelCount; i++)
		        {
					//~ Label is higher order so add it to the list and break loop
					if (GetLabelOrder(label) <= GetLabelOrder(orderedLabels[i]))
					{
						orderedLabels.InsertAt(label, i);
						orderedLabelCount++;
						labelAdded = true;
						break;
					}
		        }
				
				if (labelAdded)
					continue;
				
				//~ Lowest order so add it there
				orderedLabels.Insert(label);
				orderedLabelCount++;
			}
			
			//~ Add the ordered labels to the overal ordered list
			allOrderedLabels.InsertAll(orderedLabels);
		}
		
		//~ Set out labels to ordered labels
		labels.Copy(allOrderedLabels);
	}
	
	/*!
	Load global settings for given entity.
	\param entity Affected entity
	*/
	void LoadSettings(SCR_EditableEntityComponent entity)
	{
		if (!entity) return;
		
		//--- Get settings for the entity based on its type
		SCR_EditableEntityCoreTypeSetting setting = null;
		if (!m_TypeSettingsMap.Find(entity.GetEntityType(), setting))
		{
			Print(string.Format("Default type settings not found for '%1'!", Type().EnumToString(EEditableEntityType, entity.GetEntityType())), LogLevel.ERROR);
			return;
		}
		
		//--- Set default max draw distance
		if (entity.GetMaxDrawDistanceSq() <= 0)
			entity.SetMaxDrawDistance(setting.GetMaxDrawDistance());
	}
	/*!
	Get global settings for given entity.
	\param entity Editable entity
	*/
	SCR_EditableEntityCoreTypeSetting GetSettings(SCR_EditableEntityComponent entity)
	{
		SCR_EditableEntityCoreTypeSetting setting = null;
		if (entity && !m_TypeSettingsMap.Find(entity.GetEntityType(), setting))
			return setting;
		
		return null;
	}
	
	/*!
	Get draw distance override value for player characters.
	\return Draw distance value
	*/
	float GetPlayerDrawDistanceSq(bool isInVehicle)
	{
		float modifiedDrawDistance = m_fPlayerDrawDistance;
		if (isInVehicle)
			modifiedDrawDistance *= m_fPlayerVehicleDistanceModifier;

		return modifiedDrawDistance;
	}
	
	/*!
	Print out the hierarchy of all editable entities.
	*/
	void Log()
	{
		if (!m_Entities) return;
		Print("--------------------------------------------------", LogLevel.DEBUG);
		Print(string.Format("--- ALL (%1)", m_Entities.Count()), LogLevel.DEBUG);
		foreach (SCR_EditableEntityComponent entity: m_Entities)
		{
			entity.Log();
		}
		Print("--------------------------------------------------", LogLevel.DEBUG);
	}
	
	void UpdateBudgets(SCR_EditableEntityComponent entity, bool added, IEntity owner = null)
	{
		if (!entity)
			return;
		
		EEditableEntityType entityType = entity.GetEntityType();
		if (entity.HasEntityFlag(EEditableEntityFlag.LOCAL) || entity.HasEntityFlag(EEditableEntityFlag.NON_INTERACTIVE))
			return;
		
		if (!owner)
			owner = entity.GetOwner();
		
		if (added)
		{
			// Budget update is delayed by one frame since AI-control can not be determined directly on spawn
			// Update is delayed for these and potentionally other entities
			GetGame().GetCallqueue().CallLater(UpdateBudgetForEntity, 0, false, entity, added, owner);
		}
		else
		{
			UpdateBudgetForEntity(entity, added, owner);
		}
	}
	
	protected void UpdateBudgetForEntity(SCR_EditableEntityComponent entity, bool added, IEntity owner)
	{
		// Entity was deleted before delayed update could run, ignore entity.
		// Will be ignored for both delayed Register and Unregister so shouldn't affect total budget
		if (!entity)
			return;

		array<ref SCR_EntityBudgetValue> entityBudgetCosts = {};
		if (entity.GetEntityBudgetCost(entityBudgetCosts, owner))
		{
			foreach	(SCR_EntityBudgetValue budgetCost : entityBudgetCosts)
			{
				UpdateBudget(budgetCost.GetBudgetType(), added, entity, budgetCost);
			}
		}
		else
		{
			UpdateBudget(GetBudgetForEntityType(entity.GetEntityType(owner)), added, entity);
		}
	}
	
	protected void UpdateBudget(EEditableEntityBudget budgetType, bool added, SCR_EditableEntityComponent entity, SCR_EntityBudgetValue budgetCost = null)
	{	
		SCR_EditableEntityCoreBudgetSetting budgetSettings;
		if (!GetBudget(budgetType, budgetSettings))
		{
			return;
		}
		
		int originalBudgetValue = budgetSettings.GetCurrentBudget();	
		int budgetChange = 0;
		if (added)
		{
			budgetChange = budgetSettings.AddToBudget(budgetCost);
		}
		else
		{
			budgetChange = budgetSettings.SubtractFromBudget(budgetCost);
		}
		
		int updatedBudgetValue = originalBudgetValue + budgetChange;
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES))
		{
			PrintFormat("New budget for type %1: %2", budgetType, updatedBudgetValue);
		}
		
		Event_OnEntityBudgetUpdated.Invoke(budgetType, originalBudgetValue, budgetChange, updatedBudgetValue, entity);
	}
	
	override void OnUpdate(float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_ALL))
		{
			int type = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_TYPE) - 1;
			if (type == -1)
			{
				Log();
			}
			else
			{
				SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(Math.Pow(2, type));
				if (filter)
					filter.Log();
			}
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_ALL, false);
		}
	}
	override void OnGameStart()
	{
		typename state = EEditableEntityState;
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES, "Editable Entities", "Editor");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_ALL, "", "Log All", "Editable Entities");
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_TYPE, "", "Log Type", "Editable Entities", string.Format("-1 %1 -1 1", state.GetVariableCount() - 1));
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_DISABLE, "", "Disable entities", "Editable Entities");
	}
	override void OnGameEnd()
	{
		m_Entities = null;
		m_CurrentLayer = null;
		
		Event_OnEntityRegistered = new ScriptInvoker;
		Event_OnEntityUnregistered = new ScriptInvoker;
		Event_OnParentEntityChanged = new ScriptInvoker;
		Event_OnEntityAccessKeyChanged = new ScriptInvoker;
		Event_OnEntityVisibilityChanged = new ScriptInvoker;
		Event_OnEntityExtendedChange = new ScriptInvoker;
		Event_OnEntityBudgetUpdated = new ScriptInvoker_EntityCoreBudgetUpdatedEvent();
		
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_ALL);
	}
	
	void SCR_EditableEntityCore()
	{
		foreach (SCR_EditableEntityCoreTypeSetting setting: m_TypeSettings)
		{
			m_TypeSettingsMap.Insert(setting.GetType(), setting)
		}
		
		foreach (SCR_EditableEntityCoreLabelGroupSetting labelGroup : m_LabelGroupSettings)
		{
			array<SCR_EditableEntityCoreLabelSetting> groupLabels = {};
			EEditableEntityLabelGroup labelGroupType = labelGroup.GetLabelGroupType();
			m_LabelGroupSettingsMap.Insert(labelGroupType, labelGroup);
			
			foreach (SCR_EditableEntityCoreLabelSetting entityLabel : m_EntityLabels)
			{
				if (entityLabel.GetLabelGroupType() == labelGroupType)
				{
					groupLabels.Insert(entityLabel);
				}
			}
			m_LabelListMap.Insert(labelGroupType, groupLabels);
		}
		
		foreach (SCR_EditableEntityCoreLabelSetting labelSetting : m_EntityLabels)
		{
			m_LabelSettingsMap.Insert(labelSetting.GetLabelType(), labelSetting);
		}
		
		//--- Square the value
		m_fPlayerDrawDistance = m_fPlayerDrawDistance * m_fPlayerDrawDistance;
	}
};
