[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotBaseClass : SCR_ScenarioFrameworkLayerBaseClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkSlotBase : SCR_ScenarioFrameworkLayerBase
{
	[Attribute(desc: "Resource name of the object to be spawned", category: "Asset")]
	protected ResourceName 		m_sObjectToSpawn;
	
	[Attribute(desc: "Name of the entity used for identification", category: "Asset")];
	protected string					m_sID;
	
	[Attribute(desc: "This won't spawn new object, but it will rather use the object already existing in the world", category: "Asset")]
 	protected bool 		m_bUseExistingWorldAsset;
	
	[Attribute(desc: "Overrides display name of the spawned object for task purposes", category: "Asset")]
	protected string 	m_sOverrideObjectDisplayName;
	
	[Attribute(desc: "Randomize spawned asset(s) per Faction Attribute which needs to be filled as well. Will override Object To Spawn Attribute.", category: "Randomization")]
	protected bool						m_bRandomizePerFaction;
	
	[Attribute("0", UIWidgets.ComboBox, "Select Entity Catalog type for random spawn", "", ParamEnumArray.FromEnum(EEntityCatalogType), category: "Randomization")]
	protected EEntityCatalogType	m_eEntityCatalogType;
	
	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to optionally include to random spawn. If you want to spawn everything, you can leave it out empty and also leave Include Only Selected Labels attribute to false.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Randomization")]
	protected ref array<EEditableEntityLabel> 		m_aIncludedEditableEntityLabels;
	
	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to exclude from random spawn", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Randomization")]
	protected ref array<EEditableEntityLabel> 		m_aExcludedEditableEntityLabels;
	
	[Attribute(desc: "If true, it will spawn only the entities that are from Included Editable Entity Labels and also do not contain Label to be Excluded.", category: "Randomization")]
	protected bool										m_bIncludeOnlySelectedLabels;
	
	[Attribute(defvalue: "0", category: "Composition", desc: "When disabled orientation to terrain will be skipped for the next composition")]
	protected bool m_bIgnoreOrientChildrenToTerrain;
		
	protected bool						m_bSelected = false;
	protected ref EntitySpawnParams 	m_SpawnParams = new EntitySpawnParams;
	vector 								m_Size;
	protected ResourceName 				m_sRandomlySpawnedObject; 	
	protected vector 					m_vPosition;
	
#ifdef WORKBENCH
	[Attribute(defvalue: "1", desc: "Show the debug shapes in Workbench", category: "Debug")];
	protected bool							m_bShowDebugShapesInWorkbench;
	
	protected WorldEditorAPI m_API;
	protected IEntity m_PreviewEntity;
#endif

	//------------------------------------------------------------------------------------------------
	//! Get objects of type defined in m_sObjectToSpawn in the range
	protected void QueryObjectsInRange(float fRange = 2.5)
	{
		BaseWorld pWorld = GetGame().GetWorld();
		if (!pWorld)
			return;
			
		pWorld.QueryEntitiesBySphere(GetOwner().GetOrigin(), fRange, GetEntity, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Entity)
			return;
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Entity.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
	 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);

		if (m_bEnableRepeatedSpawn)
		{
			if (m_iRepeatedSpawnNumber <= 1 && m_iRepeatedSpawnNumber != -1)
				SetIsTerminated(true);
			
			return;
		}
		
		SetIsTerminated(true);				
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		m_bExcludeFromDynamicDespawn = true;			
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSpawnedEntityDisplayName() 
	{ 
		SCR_EditableEntityComponent editableEntityComp = SCR_EditableEntityComponent.Cast(m_Entity.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntityComp)
			return string.Empty;
		
		return editableEntityComp.GetDisplayName();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get object of type defined in m_sObjectToSpawn
	protected bool GetEntity(notnull IEntity entity)
	{
		IEntity pParent = SCR_EntityHelper.GetMainParent(entity, true);
		EntityPrefabData prefabData = pParent.GetPrefabData();
		if (!prefabData)
			return true;

		ResourceName resource = prefabData.GetPrefabName();
		if (resource.IsEmpty())
		{
			resource = prefabData.GetPrefab().GetAncestor().GetName();
			if (resource.IsEmpty())
			{
				if (!m_sID.IsEmpty())
				{
					if (entity.GetName() != m_sID)
						return true;
				}
				else
				{
					return true;
				}			
			}
			else
			{
				if (m_sObjectToSpawn.IsEmpty())
					return true;
				
				TStringArray strs = new TStringArray;
				resource.Split("/", strs, true);
				string resourceName = strs[strs.Count() - 1];
				
				TStringArray strsObject = new TStringArray;
				m_sObjectToSpawn.Split("/", strsObject, true);
				string resourceObject = strsObject[strsObject.Count() - 1];
				
				if (resourceName == resourceObject)
				{
					m_Entity = entity;
					return false;
				}
				else
				{
					return true;
				}
			}
		}
		else
		{	
			if (resource != m_sObjectToSpawn)
				return true;
		}
		
		m_Entity = entity;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the Layer Task which is parent of this Slot 
	SCR_ScenarioFrameworkArea GetAreaWB() 
	{ 
		SCR_ScenarioFrameworkArea area;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (area)
				return area;
			
			entity = entity.GetParent();
		}
		
		return area;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSpawnedObjectName()
	{
		return m_sID;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetObjectToSpawn()
	{
		return m_sObjectToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRandomlySpawnedObject()
	{
		return m_sRandomlySpawnedObject;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRandomlySpawnedObject(ResourceName name)
	{
		m_sRandomlySpawnedObject = name;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DynamicDespawn()
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_Entity && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sObjectToSpawn))
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}
		
		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;
		
		if (m_Entity)
			m_vPosition = m_Entity.GetOrigin();
		
		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		foreach (IEntity entity : m_aSpawnedEntities)
		{
			if (!entity)
				continue;
			
			m_vPosition = entity.GetOrigin();
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		
		m_aSpawnedEntities.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_bIsTerminated)
			return;
		
		if (m_OnAllChildrenSpawned)
			m_OnAllChildrenSpawned.Remove(DynamicDespawn);
		
		if (!m_ParentLayer)
		{
			IEntity entity = GetOwner().GetParent();
			if (!entity)
				return;
			
			m_ParentLayer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		}
	
		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
			return;

		if (m_Entity && !m_bEnableRepeatedSpawn)
		{
			IEntity entity = GetOwner().GetParent();
			if (!entity)
				return;
				
			SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layerBase)
				return;
				
			if (!layerBase.GetEnableRepeatedSpawn())
			{
				Print(string.Format("ScenarioFramework: Object %1 already exists and won't be spawned for %2, exiting...", m_Entity, GetOwner().GetName()), LogLevel.ERROR);
				return;
			}
		}
		
		// Handles inheritance of faction settings from parents
		if (m_sFactionKey.IsEmpty() && m_ParentLayer && !m_ParentLayer.GetFactionKey().IsEmpty())
			SetFactionKey(m_ParentLayer.GetFactionKey());
		
		if (!m_bUseExistingWorldAsset)
		{
			m_Entity = SpawnAsset();
		}
		else
		{
			QueryObjectsInRange();	//sets the m_Entity in subsequent callback
		}
		
		GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawned);
		
		if (!m_Entity)
		{
			InvokeAllChildrenSpawned();
			return;
		}
		
		if (!m_sID.IsEmpty())
			m_Entity.SetName(m_sID);	
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Entity.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		
		if (Vehicle.Cast(m_Entity))
		{
			EventHandlerManagerComponent ehManager = EventHandlerManagerComponent.Cast(m_Entity.FindComponent(EventHandlerManagerComponent));
			if (ehManager)
				ehManager.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered, true);
		}
		
		if (!area)
		{
			SCR_GameModeSFManager gameModeComp = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
			if (gameModeComp)
				area = gameModeComp.GetParentArea(GetOwner());
		}
		m_Area = area;

		InvokeAllChildrenSpawned();
	}
	
	//------------------------------------------------------------------------------------------------
	override void AfterAllChildrenSpawned()
	{
		m_bInitiated = true;
		
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}
		
		foreach(SCR_ScenarioFrameworkActionBase activationAction : m_aActivationActions)
		{
			activationAction.Init(GetOwner());
		}

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAllAreasInitiatedInvoker()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;
		
		SCR_GameModeSFManager pGameModeMgr = SCR_GameModeSFManager.Cast(gameMode.FindComponent(SCR_GameModeSFManager));
		if (!pGameModeMgr)
			return null;
		
		return pGameModeMgr.GetOnAllAreasInitiated();
	}
	
	//------------------------------------------------------------------------------------------------
	// Slot cannot have children
	override void SpawnChildren(bool previouslyRandomized = false)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRandomAsset(out ResourceName prefab)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
		{
			Print("ScenarioFramework Randomized Spawn: Faction manager not found.", LogLevel.ERROR);
			return string.Empty;
		}

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey(m_sFactionKey));
		if (!faction)
		{
			Print(string.Format("ScenarioFramework Randomized Spawn: Selected faction not found for %1.", GetOwner().GetName()), LogLevel.ERROR);
			return string.Empty;
		}
			
		SCR_EntityCatalog entityCatalog = faction.GetFactionEntityCatalogOfType(m_eEntityCatalogType);
		if (!entityCatalog)
		{
			Print(string.Format("ScenarioFramework Randomized Spawn: Faction Entity Catalog not found for selected type for %1.", GetOwner().GetName()), LogLevel.ERROR);
			return string.Empty;
		}
						
		array<SCR_EntityCatalogEntry> aFactionEntityEntry = {};
		entityCatalog.GetFullFilteredEntityListWithLabels(aFactionEntityEntry, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, m_bIncludeOnlySelectedLabels);
		if (aFactionEntityEntry.IsEmpty())
		{
			Print(string.Format("ScenarioFramework Randomized Spawn: Applied labels resulted in no viable prefabs to be randomly selected for %1.", GetOwner().GetName()), LogLevel.ERROR);
			return string.Empty;
		}
			
		Math.Randomize(-1);
		prefab = aFactionEntityEntry.GetRandomElement().GetPrefab();
		m_sRandomlySpawnedObject = prefab;
		return prefab;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity SpawnAsset()
	{
		//If Randomization is enabled, it will try to apply settings from Attributes.
		//If it fails anywhere, original m_sObjectToSpawn will be used.
		if (m_bRandomizePerFaction)
		{
			ResourceName randomAsset; 
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(GetRandomlySpawnedObject()))
			{
				GetRandomAsset(randomAsset);
				if (!SCR_StringHelper.IsEmptyOrWhiteSpace(randomAsset)) 
					m_sObjectToSpawn = randomAsset;
			}
			else
			{
				randomAsset = GetRandomlySpawnedObject();
			}
		}
		
		Resource resource = Resource.Load(m_sObjectToSpawn);
		if (!resource)
			return null;
		
		GetOwner().GetWorldTransform(m_SpawnParams.Transform);
		m_SpawnParams.TransformMode = ETransformMode.WORLD;
		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles(m_SpawnParams.Transform);
		Math3D.AnglesToMatrix(angles, m_SpawnParams.Transform);
		
		//--- Spawn the prefab
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return null;
		
		if (m_bIgnoreOrientChildrenToTerrain)
		{
			IEntityComponentSource slotCompositionComponent = SCR_BaseContainerTools.FindComponentSource(resourceObject, SCR_SlotCompositionComponent);
			if (slotCompositionComponent )
			{
				SCR_SlotCompositionComponent.IgnoreOrientChildrenToTerrain();
			}
		}
		
		IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), m_SpawnParams);
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(entity);
		
		m_aSpawnedEntities.Insert(entity);
		
		if (m_vPosition != vector.Zero)
			entity.SetOrigin(m_vPosition);
		
		return entity;
	}
		
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		DrawDebugShape(m_bShowDebugShapesInWorkbench);
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_SetTransform(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		if (!m_PreviewEntity)
			return;
		
		BaseGameEntity baseGameEntity = BaseGameEntity.Cast(m_PreviewEntity);
		if (baseGameEntity)
			baseGameEntity.Teleport(mat);
		else
			m_PreviewEntity.SetTransform(mat);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnEntityPreview(IEntity owner, Resource resource)
	{
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetWorldTransform(spawnParams.Transform);
			
		m_PreviewEntity = GetGame().SpawnEntityPrefab(resource, owner.GetWorld(), spawnParams);
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
			
		Resource resource = Resource.Load(m_sObjectToSpawn);
		if (!resource)
			return;
			
		SpawnEntityPreview(owner, resource);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_bShowDebugShapesInWorkbench")
			DrawDebugShape(m_bShowDebugShapesInWorkbench);
		
		if (key == "coords")
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
			
			Resource resource = Resource.Load(m_sObjectToSpawn);
			if (!resource)
				return false;
			
			SpawnEntityPreview(owner, resource);
			return true;
		}
		else if (key == "m_sObjectToSpawn")
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
			return false;
		}
		return false;
	}
	
#endif	
			
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkSlotBase(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
#ifdef WORKBENCH		
		m_iDebugShapeColor = ARGB(100, 0x99, 0x10, 0xF2);
#endif		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ScenarioFrameworkSlotBase()
	{
#ifdef WORKBENCH		
		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
#endif	
	}
};

[BaseContainerProps()];
class SCR_WaypointSet	
{
	[Attribute("Name")]
 	string m_sName ;	
	
	[Attribute("Use random order")]
	bool m_bUseRandomOrder;
	
	[Attribute("Cycle the waypoints")]
	bool m_bCycleWaypoints;	
};
