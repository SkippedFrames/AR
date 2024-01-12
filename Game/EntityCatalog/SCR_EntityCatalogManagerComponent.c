/**
Manager for non-faction specific entity catalogs as well as getters for faction specific catalogs
*/
[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Manager for holding non-faction enities and to getter functions for entities from factions")]
class SCR_EntityCatalogManagerComponentClass : SCR_BaseGameModeComponentClass
{
};

class SCR_EntityCatalogManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute(desc: "List of non-faction related Entity catalogs. Each holds a list of entity Prefab and data of a given type. Catalogs of the same type are merged into one. Note this array is moved to a map on init and set to null")]
	protected ref array<ref SCR_EntityCatalog> m_aEntityCatalogs;

	//~ Catalog map for quicker obtaining the catalog using EEntityCatalogType
	protected ref map<EEntityCatalogType, ref SCR_EntityCatalog> m_mEntityCatalogs = new ref map<EEntityCatalogType, ref SCR_EntityCatalog>();

	//~ Faction manager ref
	protected SCR_FactionManager m_FactionManager;

	//~ Instance
	protected static SCR_EntityCatalogManagerComponent s_Instance;

	//------------------------------------------------------------------------------------------------
	static SCR_EntityCatalogManagerComponent GetInstance()
	{		
		return s_Instance;
	}

	//======================================== GET CATALOG ========================================\\
	//--------------------------------- Get General Entity Catalog ---------------------------------\\
	/*!
	Get Entity catalog of specific type not part of a faction
	The catalog contains all entities part of the specific type not part of a faction
	\param catalogType Type to get catalog of
	\return Catalog. Can be null if not found. Note that Only one catalog of each type can be in the list
	*/
	SCR_EntityCatalog GetEntityCatalogOfType(EEntityCatalogType catalogType)
	{
		//~ Get catalog
		SCR_EntityCatalog entityCatalog = m_mEntityCatalogs.Get(catalogType);

		if (entityCatalog)
			return entityCatalog;

		//~ No data found
		Print(string.Format("'SCR_EntityCatalogManagerComponent' trying to get entity list of type '%1' but there is no catalog with that type.", typename.EnumToString(EEntityCatalogType, catalogType)), LogLevel.WARNING);
		return null;
	}

	//--------------------------------- Get Faction Entity Catalog with Key ---------------------------------\\
	/*!
	Get Entity catalog of specific type linked to given faction
	The catalog contains all entities part of the faction of that specific type
	\param catalogType Type to get catalog of
	\param factionKey FactionKey of faction
	\return Catalog. Can be null if not found. Note that Only one catalog of each type can be in the list
	*/
	SCR_EntityCatalog GetFactionEntityCatalogOfType(EEntityCatalogType catalogType, FactionKey factionKey)
	{
		if (!m_FactionManager)
			return null;

		SCR_Faction faction = SCR_Faction.Cast(m_FactionManager.GetFactionByKey(factionKey));
		if (!faction)
		{
			Print(string.Format("'SCR_EntityCatalogManagerComponent', GetFactionEntityCatalogOfType could not get Catalog as faction '%1' is invalid or does not exist in current game", factionKey), LogLevel.ERROR);
			return null;
		}

		return GetFactionEntityCatalogOfType(catalogType, faction);
	}

	//--------------------------------- Get Faction Entity Catalog with Faction ---------------------------------\\
	/*!
	Get Entity catalog of specific type linked to given faction
	The catalog contains all entities part of the faction of that specific type
	\param catalogType Type to get catalog of
	\param faction Faction to find
	\return Catalog. Can be null if not found. Note that Only one catalog of each type can be in the list
	*/
	SCR_EntityCatalog GetFactionEntityCatalogOfType(EEntityCatalogType catalogType, notnull SCR_Faction faction)
	{
		return faction.GetFactionEntityCatalogOfType(catalogType);
	}

	//======================================== GET ALL CATALOGS ========================================\\
	//--------------------------------- Get All General Entity Catalogs ---------------------------------\\
	/*!
	Get all entity catalogs not part of a faction
	The catalogs contain all entities not part of a faction
	\param[out] List of all catalogs within the faction
	\return List size
	*/
	int GetAllEntityCatalogs(notnull out array<SCR_EntityCatalog> outEntityCatalogs)
	{
		outEntityCatalogs.Clear();
		foreach (SCR_EntityCatalog entityCatalog : m_mEntityCatalogs)
		{
			outEntityCatalogs.Insert(entityCatalog);
		}

		return outEntityCatalogs.Count();
	}

	//--------------------------------- Get All Faction Entity Catalogs with factionKey ---------------------------------\\
	/*!
	Get all entity catalogs within the faction
	The catalogs contain all entities part of the faction
	\param[out] List of all catalogs within the faction
	\param factionKey key of faction to get all catalogs from
	\return List size
	*/
	int GetAllFactionEntityCatalogs(notnull out array<SCR_EntityCatalog> outEntityCatalogs, FactionKey factionKey)
	{
		if (!m_FactionManager)
			return 0;

		SCR_Faction faction = SCR_Faction.Cast(m_FactionManager.GetFactionByKey(factionKey));
		if (!faction)
		{
			Print(string.Format("'SCR_EntityCatalogManagerComponent', GetAllFactionEntityCatalogs could not get Catalog as faction '%1' is invalid or does not exist in current game", factionKey), LogLevel.ERROR);
			return 0;
		}

		return GetAllFactionEntityCatalogs(outEntityCatalogs, faction);
	}

	//--------------------------------- Get All Faction Entity Catalogs with faction ---------------------------------\\
	/*!
	Get all entity catalogs within the faction
	The catalogs contain all entities part of the faction
	\param[out] List of all catalogs within the faction
	\param faction faction to get all catalogs from
	\return List size
	*/
	int GetAllFactionEntityCatalogs(notnull out array<SCR_EntityCatalog> outEntityCatalogs, notnull SCR_Faction faction)
	{
		return faction.GetAllFactionEntityCatalogs(outEntityCatalogs);
	}

	//======================================== GET SPECIFIC ENTRY WITH PREFAB ========================================\\
	//--------------------------------- Direct Getter general ---------------------------------\\
	/*!
	Return entry with specific prefab within general catalog
	Ignores Disabled Entries
	Note this search can be somewhat extensive
	\param catalogType Catalog type to get entry in
	\param prefabToFind Prefab the entry has that you are looking for
	\return Found Entry, can be null if not found
	*/
	SCR_EntityCatalogEntry GetEntryWithPrefabFromCatalog(EEntityCatalogType catalogType, ResourceName prefabToFind)
	{
		//~ No prefab given
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefabToFind))
			return null;

		//~ Get catalog
		SCR_EntityCatalog catalog = GetEntityCatalogOfType(catalogType);
		if (!catalog)
			return null;

		//~ Try and find prefab
		return catalog.GetEntryWithPrefab(prefabToFind);
	}

	//--------------------------------- Direct Getter faction ---------------------------------\\
	/*!
	Return entry with specific prefab within faction catalog
	Ignores Disabled Entries
	Note this search can be somewhat extensive
	\param catalogType Catalog type to get entry in
	\param prefabToFind Prefab the entry has that you are looking for
	\param faction faction to find entry in
	\return Found Entry, can be null if not found
	*/
	SCR_EntityCatalogEntry GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType catalogType, ResourceName prefabToFind, notnull SCR_Faction faction)
	{
		//~ No prefab given
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefabToFind))
			return null;

		//~ Get catalog
		SCR_EntityCatalog catalog = GetFactionEntityCatalogOfType(catalogType, faction);
		if (!catalog)
			return null;

		//~ Try and find prefab
		return catalog.GetEntryWithPrefab(prefabToFind);
	}

	//--------------------------------- Direct Getter general or faction ---------------------------------\\
	/*!
	Return entry with specific prefab within general or faction catalog
	Ignores Disabled Entries
	Note this search can be somewhat extensive
	\param catalogType Catalog type to get entry in
	\param prefabToFind Prefab the entry has that you are looking for
	\param prioritizeGeneralCatalog If true will search in general catalog then faction catalog. False to search the other way around
	\param faction faction to find entry in along with general catalog
	\return Found Entry, can be null if not found
	*/
	SCR_EntityCatalogEntry GetEntryWithPrefabFromGeneralOrFactionCatalog(EEntityCatalogType catalogType, ResourceName prefabToFind, notnull SCR_Faction faction, bool prioritizeGeneralCatalog = false)
	{
		//~ No prefab given
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefabToFind))
			return null;

		SCR_EntityCatalogEntry foundEntry;

		//~ Search in faction first
		if (!prioritizeGeneralCatalog)
		{
			//~ Search in faction
			foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, faction);
			if (foundEntry)
				return foundEntry;

			//~ Search in general
			foundEntry = GetEntryWithPrefabFromCatalog(catalogType, prefabToFind);
			if (foundEntry)
				return foundEntry;
		}
		//~ Search in general catalog first
		else
		{
			//~ Search in faction
			foundEntry = GetEntryWithPrefabFromCatalog(catalogType, prefabToFind);
			if (foundEntry)
				return foundEntry;

			//~ Search in general
			foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, faction);
			if (foundEntry)
				return foundEntry;
		}

		//~ Was not found
		return null;
	}

	//--------------------------------- Direct Getter general or any faction ---------------------------------\\
	/*!
	Return entry with specific prefab within general or ANY faction catalog
	Note this search can be quite extensive!
	Ignores Disabled Entries
	\param catalogType Catalog type to get entry in
	\param prefabToFind Prefab the entry has that you are looking for
	\param prioritizeGeneralOfPriorityFaction If true will search in general catalog then faction catalogs. False it will first search priorityFaction (if any) than General than any other faction
	\param priorityFaction If given it will first try and get the entry from this faction before searching all the other factions
	\return Found Entry, can be null if not found
	*/
	SCR_EntityCatalogEntry GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType catalogType, ResourceName prefabToFind, SCR_Faction priorityFaction = null, bool prioritizeGeneralOfPriorityFaction = false)
	{
		SCR_EntityCatalogEntry foundEntry;

		if (!prioritizeGeneralOfPriorityFaction)
		{
			//~ Search in priority faction first
			if (priorityFaction)
			{
				foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, priorityFaction);
				if (foundEntry)
					return foundEntry;
			}

			//~ Search in general
			foundEntry = GetEntryWithPrefabFromCatalog(catalogType, prefabToFind);
			if (foundEntry)
				return foundEntry;
		}
		else
		{
			//~ Search in general first
			foundEntry = GetEntryWithPrefabFromCatalog(catalogType, prefabToFind);
			if (foundEntry)
				return foundEntry;

			//~ Search in priority faction
			if (priorityFaction)
			{
				foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, priorityFaction);
				if (foundEntry)
					return foundEntry;
			}
		}

		if (!m_FactionManager)
			return null;

		//~ Search all faction (ignoring priorityFaction) and try to find the prefab
		array<Faction> factions = {};
		m_FactionManager.GetFactionsList(factions);

		foreach (Faction faction : factions)
		{
			SCR_Faction scrFaction = SCR_Faction.Cast(faction);

			//~ SCR_Faction not found or already searched
			if (!scrFaction || scrFaction == priorityFaction)
				continue;

			//~ Try and find the prefab
			foundEntry = GetEntryWithPrefabFromFactionCatalog(catalogType, prefabToFind, scrFaction);
			if (foundEntry)
				return foundEntry;
		}

		return null;
	}
	
	//======================================== ARSENAL ========================================\\	
	//--------------------------------- Get arsenal items from general catalog ---------------------------------\\
	/*!
	Get all arsenal items configured on the faction
	Values taken from General Catalog ITEM
	\param[out] arsenalItems output array
	\param typeFilter filter for Types (-1 is ignore filter)
	\param modeFilter filter for Modes (-1 is ignore filter)
	\param requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	\return bool False if no config is set and when 0 items are configured
	*/
	bool GetArsenalItems(out array<SCR_ArsenalItem> arsenalItems, SCR_EArsenalItemType typeFilter = -1, SCR_EArsenalItemMode modeFilter = -1, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		arsenalItems.Clear();
		
		SCR_EntityCatalog itemCatalog = GetEntityCatalogOfType(EEntityCatalogType.ITEM);
		if (!itemCatalog)
			return false;
		
		return GetArsenalItems(arsenalItems, itemCatalog, typeFilter, modeFilter, requiresDisplayType);
	}
	
	//--------------------------------- Get arsenal Items from Faction ---------------------------------\\
	/*!
	Get all arsenal items configured on the faction
	Values taken from Faction Catalog ITEM
	\param[out] arsenalItems output array
	\param faction faction to get items from
	\param typeFilter filter for Types (-1 is ignore filter)
	\param modeFilter filter for Modes (-1 is ignore filter)
	\param requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	\return bool False if no config is set and when 0 items are configured
	*/
	bool GetFactionArsenalItems(out array<SCR_ArsenalItem> arsenalItems, SCR_Faction faction, SCR_EArsenalItemType typeFilter = -1, SCR_EArsenalItemMode modeFilter = -1, EArsenalItemDisplayType requiresDisplayType = -1)
	{		
		arsenalItems.Clear();
		
		SCR_EntityCatalog itemCatalog = GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM, faction);
		if (!itemCatalog)
			return false;
		
		return GetArsenalItems(arsenalItems, itemCatalog, typeFilter, modeFilter, requiresDisplayType);
	}
	
	//--------------------------------- Get Arsenal items from catalog ---------------------------------\\
	//~ Gets a filtered list of arsenal items
	protected bool GetArsenalItems(out array<SCR_ArsenalItem> arsenalItems, notnull SCR_EntityCatalog itemCatalog, SCR_EArsenalItemType typeFilter = -1, SCR_EArsenalItemMode modeFilter = -1, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		array<SCR_EntityCatalogEntry> arsenalEntries = {};
		array<SCR_BaseEntityCatalogData> arsenalDataList = {};
		itemCatalog.GetEntityListWithData(SCR_ArsenalItem, arsenalEntries, arsenalDataList);
		
		SCR_ArsenalItem arsenalItem;
		
		foreach (SCR_BaseEntityCatalogData arsenalData: arsenalDataList)
		{
			//~ Get Arsenal data
			arsenalItem = SCR_ArsenalItem.Cast(arsenalData);
			
			//~ Does not have type so skip
			if (typeFilter != -1 && !SCR_Enum.HasPartialFlag(arsenalItem.GetItemType(), typeFilter))
				continue;
			
			//~ Does not have mode so skip
			if (modeFilter != -1 && !SCR_Enum.HasPartialFlag(arsenalItem.GetItemMode(), modeFilter))
				continue;
			
			//~ Get if has display data, ignore if it doesn't
			if (requiresDisplayType != -1 && !arsenalItem.GetDisplayDataOfType(requiresDisplayType))
				continue;
			
			arsenalItems.Insert(arsenalItem);
		}
		
		return !arsenalItems.IsEmpty();
	}
	
	
	//--------------------------------- Get Arsenal items from catalog ---------------------------------\\
	/*!
	Get all arsenal items configured in any faction and the non-faction catalogs. Note this is a very intensive seach
	Values taken from Faction Catalog ITEM
	\param[out] allArsenalItems output array
	\param typeFilter filter for Types (-1 is ignore filter)
	\param modeFilter filter for Modes (-1 is ignore filter)
	\param requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	\return int count of items found
	*/
	int GetAllArsenalItems(out array<SCR_ArsenalItem> allArsenalItems, SCR_EArsenalItemType typeFilter = -1, SCR_EArsenalItemMode modeFilter = -1, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		allArsenalItems.Clear();
		
		array<Faction> factions = {};
		m_FactionManager.GetFactionsList(factions);
		SCR_Faction scrFaction;
		array<SCR_ArsenalItem> arsenalItems = {};
		
		foreach (Faction faction : factions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;
			
			GetFactionArsenalItems(arsenalItems, scrFaction, typeFilter, modeFilter, requiresDisplayType);
			allArsenalItems.InsertAll(arsenalItems);
		}
		
		GetArsenalItems(arsenalItems, typeFilter, modeFilter, requiresDisplayType);
		allArsenalItems.InsertAll(arsenalItems);
		
		return allArsenalItems.Count();
	}
	
	//--------------------------------- Returns an array of filtered arsenal items ---------------------------------\\
	/*!
	Get arsenal items filtered by SCR_EArsenalItemType filter, caches values
	\param typeFilter Combined flags for available items for this faction (RIFLE, MAGAZINE, EQUIPMENT, RADIOBACKPACK etc.)
	\param modeFilter Things like AMMO and CONSUMABLE
	\param faction If empty will take non-faction data else will take the data from the faction
	\param requiresDisplayData if true also requires display data for displaying item in arsenal rack
	\param requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	\return array with availabe arsenal items of give filter types
	*/
	array<SCR_ArsenalItem> GetFilteredArsenalItems(SCR_EArsenalItemType typeFilter, SCR_EArsenalItemMode modeFilter, SCR_Faction faction = null, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		array<SCR_ArsenalItem> refFilteredItems = {};
		array<SCR_ArsenalItem> filteredItems = {};
		
		if (faction)
			GetFactionArsenalItems(refFilteredItems, faction, typeFilter, modeFilter, requiresDisplayType);
		else 
			GetArsenalItems(refFilteredItems, typeFilter, modeFilter, requiresDisplayType);
		
		foreach (SCR_ArsenalItem item : refFilteredItems)
		{
			filteredItems.Insert(item);
		}
			
		return filteredItems;
	}

	//======================================== INIT ========================================\\
	/*!
	Init Catalog lists to move the arrays into a map for faster processing
	\param entityCatalogArray Array of entity catalog to move into the map
	\param entityCatalogMap the map to move the catalogs to. Catalogs with the same type will be merged
	*/
	static void InitCatalogs(notnull array<ref SCR_EntityCatalog> entityCatalogArray, notnull map<EEntityCatalogType, ref SCR_EntityCatalog> entityCatalogMap)
	{
		SCR_EntityCatalog foundCatalog;
		
		//~ Move catalogs to map for quicker processing
		foreach (SCR_EntityCatalog entityCatalog : entityCatalogArray)
		{
			//~ Catalog not part of map so add it
			if (!entityCatalogMap.Find(entityCatalog.GetCatalogType(), foundCatalog))
				entityCatalogMap.Insert(entityCatalog.GetCatalogType(), entityCatalog);
			//~ Catalog is part of map so merge them
			else 
				foundCatalog.MergeCatalogs(entityCatalog);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!m_FactionManager)
			Debug.Error2("SCR_EntityCatalogManagerComponent", "Could not find SCR_FactionManager, this is required for many the Getter Functions!");

		//~ Init the catalog
		InitCatalogs(m_aEntityCatalogs, m_mEntityCatalogs);
		
		//~ Clear array as no longer needed
		m_aEntityCatalogs = null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		if (s_Instance)
		{
			Print("More than one 'SCR_EntityCatalogManagerComponent' excist in the world! Make sure there is only 1!", LogLevel.ERROR);
			return;
		}

		//~ Set instance
		s_Instance = this;

		SetEventMask(owner, EntityEvent.INIT);
	}
};
