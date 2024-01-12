#include "scripts/Game/config.c"
[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkLayerBaseClass : ScriptComponentClass
{
	// prefab properties here
}

enum SCR_EScenarioFrameworkSpawnChildrenType
{
	ALL,
	RANDOM_ONE,
	RANDOM_MULTIPLE,
	RANDOM_BASED_ON_PLAYERS_COUNT
}

class SCR_ScenarioFrameworkLayerBase : ScriptComponent
{
	[Attribute(defvalue: "0", UIWidgets.ComboBox, desc: "Spawn all children, only random one or random multiple ones?", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkSpawnChildrenType), category: "Children")];
	protected SCR_EScenarioFrameworkSpawnChildrenType			m_SpawnChildren;

	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager", category: "Asset")]
	protected FactionKey 		m_sFactionKey;

	[Attribute(defvalue: "100", desc: "If the RANDOM_MULTIPLE option is selected, what's the percentage? ", UIWidgets.Graph, "0 100 1", category: "Children")];
	protected int 							m_iRandomPercent;

	[Attribute(desc: "When enabled, it will repeatedly spawn childern according to other parameters set", category: "Children")];
	protected bool		m_bEnableRepeatedSpawn;

	[Attribute(defvalue: "-1", desc: "If Repeated Spawn is enabled, how many times can children be spawned? If set to -1, it is unlimited", category: "Children")];
	protected int 							m_iRepeatedSpawnNumber;

	[Attribute(defvalue: "-1", UIWidgets.Slider, desc: "If Repeated Spawn is enabled, how frequently it will spawn next wave of children? Value -1 means disabled, thus children won't be spawned by the elapsed time.", params: "-1 86400 1", category: "Children")]
	protected float 	m_fRepeatedSpawnTimer;

	[Attribute(desc: "Show the debug shapes during runtime", category: "Debug")];
	protected bool							m_bShowDebugShapesDuringRuntime;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType), category: "Activation")]
	protected SCR_ScenarioFrameworkEActivationType			m_eActivationType;
	
	[Attribute(desc: "Actions that will be activated when this Area gets activated", category: "OnActivation")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActivationActions;
	
	[Attribute(desc: "", category: "Activation")];
	protected bool							m_bExcludeFromDynamicDespawn;

	[Attribute(UIWidgets.Auto, category: "Plugins")];
	protected ref array<ref SCR_ScenarioFrameworkPlugin> m_aPlugins;

	protected ref array<SCR_ScenarioFrameworkLayerBase>		m_aChildren = {};
	protected ref array<SCR_ScenarioFrameworkLayerBase>		m_aRandomlySpawnedChildren = {};
	protected ref array<SCR_ScenarioFrameworkLogic>			m_aLogic = {};

	protected ref ScriptInvokerVoid 		m_OnAllChildrenSpawned;
	protected ref array<IEntity>			m_aSpawnedEntities = {};
	protected IEntity						m_Entity;
	protected SCR_ScenarioFrameworkArea						m_Area;
	protected SCR_ScenarioFrameworkLayerBase				m_ParentLayer;
	protected float							m_fDebugShapeRadius = 0.25;
	protected WorldTimestamp				m_fRepeatSpawnTimeStart;
	protected WorldTimestamp				m_fRepeatSpawnTimeEnd;
	protected int							m_iDebugShapeColor = ARGB(32, 0xFF, 0x00, 0x12);
	protected int 							m_iCurrentlySpawnedChildren;
	protected int 							m_iSupposedSpawnedChildren;
	protected bool							m_bInitiated = false;
	protected bool							m_bIsRegistered = false;
	protected bool 							m_bDynamicallyDespawned;
	protected bool							m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	
	static const int SPAWN_DELAY = 200;

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return GetOwner().GetName();
	}

	//------------------------------------------------------------------------------------------------
	void SetEntity(IEntity entity)
	{
		m_Entity = entity;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}

	//------------------------------------------------------------------------------------------------
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}

	//------------------------------------------------------------------------------------------------
	void SetRandomlySpawnedChildren(array<string> randomlySpawnedChildren)
	{
		IEntity entity;
		SCR_ScenarioFrameworkLayerBase layer;
		m_aRandomlySpawnedChildren.Clear();
		foreach (string child : randomlySpawnedChildren)
		{
			entity = GetGame().GetWorld().FindEntityByName(child);
			if (!entity)
				continue;

			layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
				continue;

			m_aRandomlySpawnedChildren.Insert(layer);
		}
	}

	//------------------------------------------------------------------------------------------------
	array<SCR_ScenarioFrameworkLayerBase> GetRandomlySpawnedChildren()
	{
		return m_aRandomlySpawnedChildren;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetPlayersCount(FactionKey factionName = "")
	{
		if (factionName.IsEmpty())
			return GetGame().GetPlayerManager().GetPlayerCount();

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return -1;

		int iCnt = 0;
		array<int> aPlayerIDs = {};
		SCR_PlayerController playerController;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID : aPlayerIDs)
		{
			playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
			if (!playerController)
				continue;

			if (playerController.GetLocalControlledEntityFaction() == factionManager.GetFactionByKey(factionName))
				iCnt++;
		}
		return iCnt;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetMaxPlayersForGameMode(FactionKey factionName = "")
	{
		//TODO: separate players by faction (attackers / defenders)
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());

		if (!header)
			return 4;	//TODO: make a constant

		return header.m_iPlayerCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Get parent area the object is nested into
	SCR_ScenarioFrameworkArea GetParentArea()
	{
		if (m_Area)
			return m_Area;
		
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			m_Area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (m_Area)
				return m_Area;

			entity = entity.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get layer task the object is nested into if there is some
	SCR_ScenarioFrameworkLayerTask GetLayerTask()
	{
		SCR_ScenarioFrameworkLayerTask layer;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
			if (layer)
				return layer;

			entity = entity.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get SlotTask from array of LayerBases if there is any
	SCR_ScenarioFrameworkSlotTask GetSlotTask(array<SCR_ScenarioFrameworkLayerBase> aLayers)
	{
		SCR_ScenarioFrameworkSlotTask slotTask;
		foreach (SCR_ScenarioFrameworkLayerBase layer : aLayers)
		{
			IEntity child = layer.GetOwner();
			slotTask = SCR_ScenarioFrameworkSlotTask.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotTask));
			if (slotTask)
				return slotTask;

			child = GetOwner().GetChildren();
			while (child)
			{
				slotTask = SCR_ScenarioFrameworkSlotTask.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotTask));
				if (slotTask)
					return slotTask;
				
				child = child.GetSibling();
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFactionKey(FactionKey factionKey)
	{
		m_sFactionKey = factionKey;
	}

	//------------------------------------------------------------------------------------------------
	protected FactionKey GetFactionKey()
	{
		return m_sFactionKey;
	}

	//------------------------------------------------------------------------------------------------
	void SetParentLayer(SCR_ScenarioFrameworkLayerBase parentLayer)
	{
		m_ParentLayer = parentLayer;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerBase GetParentLayer()
	{
		if (m_ParentLayer)
			return m_ParentLayer;
		
		IEntity entity = GetOwner().GetParent();
		if (!entity)
			return null;
		
		m_ParentLayer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		return m_ParentLayer;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EScenarioFrameworkSpawnChildrenType GetSpawnChildrenType()
	{
		return m_SpawnChildren;
	}

	//------------------------------------------------------------------------------------------------
	bool GetEnableRepeatedSpawn()
	{
		return m_bEnableRepeatedSpawn;
	}

	//------------------------------------------------------------------------------------------------
	void SetEnableRepeatedSpawn(bool value)
	{
		m_bEnableRepeatedSpawn = value;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkEActivationType GetActivationType()
	{
		return m_eActivationType;
	}

	//------------------------------------------------------------------------------------------------
	void SetActivationType(SCR_ScenarioFrameworkEActivationType activationType)
	{
		m_eActivationType = activationType;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsInitiated()
	{
		return m_bInitiated;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetDynamicDespawnExcluded()
	{
		return m_bExcludeFromDynamicDespawn;
	}

	//------------------------------------------------------------------------------------------------
	void SetDynamicDespawnExcluded(bool excluded)
	{
		m_bExcludeFromDynamicDespawn = excluded;
	}

	//------------------------------------------------------------------------------------------------
	array<IEntity> GetSpawnedEntities()
	{
		return m_aSpawnedEntities;
	}

	//------------------------------------------------------------------------------------------------
	array<SCR_ScenarioFrameworkLayerBase> GetChildrenEntities()
	{
		return m_aChildren;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the random Slot
	SCR_ScenarioFrameworkLayerBase GetRandomChildren()
	{
		if (m_aChildren.IsEmpty())
			return null;

		Math.Randomize(-1);
		return m_aChildren.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void GetChildren()
	{
		SCR_ScenarioFrameworkLayerBase slotComponent;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			slotComponent = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (slotComponent && !m_aChildren.Contains(slotComponent))
			{
				m_aChildren.Insert(slotComponent);
			}
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void GetLogics()
	{
		IEntity child = GetOwner().GetChildren();
		SCR_ScenarioFrameworkLogic logic;
		while (child)
		{
			logic = SCR_ScenarioFrameworkLogic.Cast(child);
			if (logic && !m_aLogic.Contains(logic))
				m_aLogic.Insert(logic);
			
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	array<SCR_ScenarioFrameworkLogic> GetSpawnedLogics()
	{
		return m_aLogic;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkPlugin> GetSpawnedPlugins()
	{
		return m_aPlugins;
	}

	//------------------------------------------------------------------------------------------------
	int GetRepeatedSpawnNumber()
	{
		return m_iRepeatedSpawnNumber;
	}

	//------------------------------------------------------------------------------------------------
	void SetRepeatedSpawnNumber(int number)
	{
		m_iRepeatedSpawnNumber = number;
	}

	//------------------------------------------------------------------------------------------------
	protected void RepeatedSpawn()
	{
		if (!m_bEnableRepeatedSpawn || m_iRepeatedSpawnNumber <= 1)
			return;

		//This calls the RepeatedSpawnCalled with set delay and is set in a way that it 
		//Can be both queued that way or called manually from different place (pseudo-looped CallLater)
		GetGame().GetCallqueue().CallLater(RepeatedSpawnCalled, 1000 * m_fRepeatedSpawnTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RepeatedSpawnCalled()
	{
		m_iRepeatedSpawnNumber--;
		SpawnChildren(true);
		
		if (!m_bEnableRepeatedSpawn || m_iRepeatedSpawnNumber <= 1)
			return;
		
		RepeatedSpawn();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnAllChildrenSpawned()
	{
		if (!m_OnAllChildrenSpawned)
			m_OnAllChildrenSpawned = new ScriptInvokerVoid();

		return m_OnAllChildrenSpawned;
	}

	//------------------------------------------------------------------------------------------------
	void InvokeAllChildrenSpawned()
	{
		if (m_OnAllChildrenSpawned)
			m_OnAllChildrenSpawned.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	void CheckAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer = null)
	{
		if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			m_iCurrentlySpawnedChildren = 0;
			m_iSupposedSpawnedChildren = 0;
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				int activationType = child.GetActivationType();
				if (activationType == SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION || activationType == SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION
					|| activationType == SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT)
					continue;

				if (child.GetIsInitiated())
					m_iCurrentlySpawnedChildren++;

				m_iSupposedSpawnedChildren++;
			}

			if (m_iCurrentlySpawnedChildren == m_iSupposedSpawnedChildren)
				InvokeAllChildrenSpawned();
		}
		else if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_ONE)
		{
			InvokeAllChildrenSpawned();
		}
		else
		{
			m_iCurrentlySpawnedChildren = 0;
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				int activationType = child.GetActivationType();
				if (activationType == SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION || activationType == SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION)
					continue;

				if (child.GetIsInitiated())
					m_iCurrentlySpawnedChildren++;
			}

			if (m_iCurrentlySpawnedChildren == m_iSupposedSpawnedChildren)
				InvokeAllChildrenSpawned();
		}
	}

	//------------------------------------------------------------------------------------------------
	void SpawnChildren(bool previouslyRandomized = false)
	{
		if (m_aChildren.IsEmpty())
		{
			InvokeAllChildrenSpawned();
			return;
		}
		
		if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			int slotCount;
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				if (SCR_ScenarioFrameworkSlotBase.Cast(child))
				{
					GetGame().GetCallqueue().CallLater(InitChild, SPAWN_DELAY * slotCount, false, child);
					slotCount++;
				}
				else
				{
					InitChild(child);
				}
			}
		}
		else if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_ONE)
		{
			//We need to introduce slight delay for the randomization by time seed to occur
			GetGame().GetCallqueue().CallLater(SpawnRandomOneChild, Math.RandomInt(1, 10), false, previouslyRandomized);
		}
		else
		{
			//We need to introduce slight delay for the randomization by time seed to occur
			GetGame().GetCallqueue().CallLater(SpawnRandomMultipleChildren, Math.RandomInt(1, 10), false, previouslyRandomized);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SpawnPreviouslyRandomizedChildren()
	{
		foreach (int i, SCR_ScenarioFrameworkLayerBase child : m_aRandomlySpawnedChildren)
		{
			GetGame().GetCallqueue().CallLater(InitChild, 200 * i, false, child);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SpawnRandomOneChild(bool previouslyRandomized = false)
	{
		if (previouslyRandomized)
			SpawnPreviouslyRandomizedChildren();
		else
		{
			SCR_ScenarioFrameworkLayerBase child = GetRandomChildren();
			m_aRandomlySpawnedChildren.Insert(child);
			InitChild(child);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SpawnRandomMultipleChildren(bool previouslyRandomized = false)
	{
		if (previouslyRandomized)
			SpawnPreviouslyRandomizedChildren();
		else
		{
			array<SCR_ScenarioFrameworkLayerBase> aChildren = {};
			aChildren.Copy(m_aChildren);
			
			if (aChildren.IsEmpty())
				return;
			
			if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_BASED_ON_PLAYERS_COUNT)
				m_iRandomPercent = Math.Ceil(GetPlayersCount() / GetMaxPlayersForGameMode() * 100);

			m_iSupposedSpawnedChildren = Math.Round(m_aChildren.Count() / 100 * m_iRandomPercent);
			SCR_ScenarioFrameworkLayerBase child;
			for (int i = 1; i <= m_iSupposedSpawnedChildren; i++)
			{
				if (aChildren.IsEmpty())
					continue;
				
				Math.Randomize(-1);
				child = aChildren.GetRandomElement();
				m_aRandomlySpawnedChildren.Insert(child);
				InitChild(child);
				aChildren.RemoveItem(child);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void InitChild(SCR_ScenarioFrameworkLayerBase child)
	{
		if (!child)
			return;

		child.SetParentLayer(this);
		child.Init(GetParentArea(), SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetSpawnedEntity()
	{
		return m_Entity;
	}

	//------------------------------------------------------------------------------------------------
	protected void ActivateLogic()
	{
		GetLogics();
		foreach (SCR_ScenarioFrameworkLogic logic : m_aLogic)
		{
			logic.Init();
		}
	}

	//------------------------------------------------------------------------------------------------
	void DynamicDespawn()
	{
		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;

		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
		{
			child.DynamicDespawn();
		}

		m_aChildren.Clear();

		foreach (IEntity entity : m_aSpawnedEntities)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}

		m_aSpawnedEntities.Clear();
	}

	//------------------------------------------------------------------------------------------------
	void DynamicReinit()
	{
		Init(GetParentArea(), SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}

	//------------------------------------------------------------------------------------------------
	void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_bInitiated)
			return;

		if (m_bIsTerminated)
		{
			if (!m_ParentLayer)
				return;

			SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(m_ParentLayer);
			if (!layerTask && !GetParentArea().GetLayerTask())
				return;
		}

		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
			return;

		if (!area)
		{
			SCR_GameModeSFManager gameModeComp = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
			if (gameModeComp)
				area = gameModeComp.GetParentArea(GetOwner());
		}
		m_Area = area;

		bool previouslyRandomized;
		if (!m_aRandomlySpawnedChildren.IsEmpty())
			previouslyRandomized = true;

		// Handles inheritance of faction settings from parents
		if (m_sFactionKey.IsEmpty() && m_ParentLayer && !m_ParentLayer.GetFactionKey().IsEmpty())
			SetFactionKey(m_ParentLayer.GetFactionKey());

		GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawned);

		GetChildren();
		SpawnChildren(previouslyRandomized);
	}

	//------------------------------------------------------------------------------------------------
	void AfterAllChildrenSpawned()
	{
		m_bInitiated = true;
		
		ActivateLogic();
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

		if (m_fRepeatedSpawnTimer >= 0)
			RepeatedSpawn();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		if (m_bShowDebugShapesDuringRuntime)
			DrawDebugShape(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (m_bShowDebugShapesDuringRuntime || m_fRepeatedSpawnTimer >= 0)
		{
			//TODO: deactivate once the slots are not needed (after entity was spawned)
			SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		}
	}
	//------------------------------------------------------------------------------------------------
	void SetDebugShapeSize(float fSize)
	{
		m_fDebugShapeRadius = fSize;
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShape(bool draw)
	{
		Shape dbgShape = null;
		if (!draw)
			return;

		dbgShape = Shape.CreateSphere(
										m_iDebugShapeColor,
										ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
										GetOwner().GetOrigin(),
										m_fDebugShapeRadius
								);
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntity owner, IEntitySource src)
	{
		RenameOwnerEntity(owner);
		
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			RenameOwnerEntity(child);
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RenameOwnerEntity(IEntity owner)
	{
		GenericEntity genericEntity = GenericEntity.Cast(owner);

		WorldEditorAPI api = genericEntity._WB_GetEditorAPI();
		if(!api.UndoOrRedoIsRestoring())
			api.RenameEntity(owner, api.GenerateDefaultEntityName(api.EntityToSource(owner)));
	}
#endif	

	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLayerBase(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
#ifdef WORKBENCH	
		m_iDebugShapeColor = ARGB(32, 0x99, 0xF3, 0x12);
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.OnWBKeyChanged(this);
		}
#endif		
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPlugin : ScriptAndConfig
{
	protected SCR_ScenarioFrameworkLayerBase m_Object;

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerBase GetObject()
	{
		return m_Object;
	}

	//------------------------------------------------------------------------------------------------
	void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		m_Object = object;
	}

	//------------------------------------------------------------------------------------------------
	void OnWBKeyChanged(SCR_ScenarioFrameworkLayerBase object)
	{
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginTrigger : SCR_ScenarioFrameworkPlugin
{
	[Attribute(defvalue: "5.0", UIWidgets.Slider, params: "1.0 1000.0 0.5", desc: "Radius of the trigger if selected", category: "Trigger")];
	protected float						m_fAreaRadius;

	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(TA_EActivationPresence), category: "Trigger Activation")]
	protected TA_EActivationPresence	m_eActivationPresence;

	[Attribute(desc: "If SPECIFIC_CLASS is selected, fill the class name here.", category: "Trigger")]
	protected ref array<string> 	m_aSpecificClassNames;

	[Attribute(desc: "Which Prefabs and if their children will be detected by the trigger. Is combined with other filters using OR.", category: "Trigger")]
	protected ref array<ref SCR_ScenarioFrameworkPrefabFilter> m_aPrefabFilter;

	[Attribute("", category: "Trigger Activation")]
	protected FactionKey 		m_sActivatedByThisFaction;

	[Attribute(desc: "Here you can input custom trigger conditions that you can create by extending the SCR_CustomTriggerConditions", uiwidget: UIWidgets.Object)]
	protected ref array<ref SCR_CustomTriggerConditions> m_aCustomTriggerConditions;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "If you set some vehicle to be detected by the trigger, it will also search the inventory for vehicle prefabs/classes that are set", category: "Trigger")]
	protected bool m_bSearchVehicleInventory;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")];
	protected bool		m_bOnce;

	[Attribute(defvalue: "1", UIWidgets.Slider, desc: "How frequently is the trigger updated and performing calculations. Lower numbers will decrease performance.", params: "0 86400 1", category: "Trigger")]
	protected float 	m_fUpdateRate;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Minimum players needed to activate this trigger when PLAYER Activation presence is selected", params: "0 1 0.01", precision: 2, category: "Trigger")]
	protected float		m_fMinimumPlayersNeededPercentage;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "For how long the trigger conditions must be true in order for the trigger to activate. If conditions become false, timer resets", params: "0 86400 1", category: "Trigger")]
	protected float 	m_fActivationCountdownTimer;

	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the notification is allowed to be displayed", category: "Trigger")]
	protected bool		m_bNotificationEnabled;

	[Attribute(desc: "Notification title text that will be displayed when the PLAYER Activation presence is selected", category: "Trigger")]
	protected string 	m_sPlayerActivationNotificationTitle;

	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the audio sound is played and affected by the trigger", category: "Trigger")]
	protected bool		m_bEnableAudio;

	[Attribute(desc: "Audio sound that will be playing when countdown is active.", category: "Trigger")]
	protected string 	m_sCountdownAudio;

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		SCR_CharacterTriggerEntity trigger;
		IEntity entity = object.GetSpawnedEntity();

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(object);
		if (area)
		{
			trigger = SCR_CharacterTriggerEntity.Cast(area.GetTrigger());
		}
		else
		{
			if (!BaseGameTriggerEntity.Cast(entity))
			{
				Print("ScenarioFramework: SlotTrigger - The selected prefab is not trigger!", LogLevel.ERROR);
				return;
			}
			trigger = SCR_CharacterTriggerEntity.Cast(entity);
		}

		if (trigger)
		{
			trigger.SetSphereRadius(m_fAreaRadius);
			trigger.SetActivationPresence(m_eActivationPresence);
			trigger.SetOwnerFaction(m_sActivatedByThisFaction);
			trigger.SetSpecificClassName(m_aSpecificClassNames);
			trigger.SetPrefabFilters(m_aPrefabFilter);
			trigger.SetCustomTriggerConditions(m_aCustomTriggerConditions);
			trigger.SetSearchVehicleInventory(m_bSearchVehicleInventory);
			trigger.SetOnce(m_bOnce);
			trigger.SetUpdateRate(m_fUpdateRate);
			trigger.SetNotificationEnabled(m_bNotificationEnabled);
			trigger.SetEnableAudio(m_bEnableAudio);
			trigger.SetMinimumPlayersNeeded(m_fMinimumPlayersNeededPercentage);
			trigger.SetPlayerActivationNotificationTitle(m_sPlayerActivationNotificationTitle);
			trigger.SetActivationCountdownTimer(m_fActivationCountdownTimer);
			trigger.SetCountdownAudio(m_sCountdownAudio);

			return;
		}

		SCR_BaseFactionTriggerEntity factionTrigger = SCR_BaseFactionTriggerEntity.Cast(entity);
		if (factionTrigger)
		{
			factionTrigger.SetSphereRadius(m_fAreaRadius);
			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
				factionTrigger.SetOwnerFaction(factionManager.GetFactionByKey(m_sActivatedByThisFaction));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnWBKeyChanged(SCR_ScenarioFrameworkLayerBase object)
	{
		super.OnWBKeyChanged(object);
		object.SetDebugShapeSize(m_fAreaRadius);
		//src.Set("m_sAreaName", m_fAreaRadius);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginOnDestroyEvent : SCR_ScenarioFrameworkPlugin
{
	[Attribute(UIWidgets.Auto, desc: "What to do once object gets destroyed", category: "OnDestroy")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnDestroy;

	protected IEntity m_Asset;

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		SCR_CharacterTriggerEntity trigger;
		IEntity entity = object.GetSpawnedEntity();
		if (!entity)
			return;

		m_Asset = entity;
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(ScriptedDamageManagerComponent.GetDamageManager(m_Asset));
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		else
			PrintFormat("ScenarioFramework: Registering OnDestroy of entity %1 failed! The entity doesn't have damage manager", entity, LogLevel.ERROR);
		
		if (Vehicle.Cast(m_Asset))
		{
			VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Asset.FindComponent(VehicleControllerComponent_SA));
			if (vehicleController)
				vehicleController.GetOnEngineStop().Insert(CheckEngineDrowned);
			
			// Since there is no invoker and no reliable way how to tackle drowned vehicles, in order to make it reliable,
			// We cannot solely rely on GetOnEngineStop because vehicle could have been pushed/moved into the water without started engine.
			GetGame().GetCallqueue().CallLater(CheckEngineDrowned, 5000, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Asset)
			return;

		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(ScriptedDamageManagerComponent.GetDamageManager(m_Asset));
		if (objectDmgManager)
		{
			objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
			GetGame().GetCallqueue().Remove(CheckEngineDrowned);
			
			VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Asset.FindComponent(VehicleControllerComponent_SA));
			if (vehicleController)
				vehicleController.GetOnEngineStop().Remove(CheckEngineDrowned);
		}

		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnDestroy)
		{
			action.OnActivate(m_Asset);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckEngineDrowned()
	{
		if (!m_Asset)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Asset.FindComponent(VehicleControllerComponent_SA));
		if (vehicleController && vehicleController.GetEngineDrowned())
		{
			vehicleController.GetOnEngineStop().Remove(CheckEngineDrowned);
			GetGame().GetCallqueue().Remove(CheckEngineDrowned);
			
			ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(ScriptedDamageManagerComponent.GetDamageManager(m_Asset));
			if (objectDmgManager)
		 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
			
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnDestroy)
			{
				action.OnActivate(m_Asset);
			}
		}
	}

}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginOnInventoryChange : SCR_ScenarioFrameworkPlugin
{
	[Attribute(UIWidgets.Auto, desc: "What to do once object inventory has changed by item addition")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemAdded;
	
	[Attribute(UIWidgets.Auto, desc: "What to do once object inventory has changed by item removal")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemRemoved;

	protected IEntity m_Asset;

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		IEntity entity = object.GetSpawnedEntity();
		if (!entity)
			return;

		m_Asset = entity;
		
		//Inventory system is a mess and since different entities have different storage managers that don't have this properly inherited, we need to account for that here
		SCR_InventoryStorageManagerComponent storageManager1 = SCR_InventoryStorageManagerComponent.Cast(m_Asset.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManager1)
		{
			storageManager1.m_OnItemAddedInvoker.Insert(OnItemAdded);
			storageManager1.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			return;
		}
		
		SCR_VehicleInventoryStorageManagerComponent storageManager2 = SCR_VehicleInventoryStorageManagerComponent.Cast(m_Asset.FindComponent(SCR_VehicleInventoryStorageManagerComponent));
		if (storageManager2)
		{
			storageManager2.m_OnItemAddedInvoker.Insert(OnItemAdded);
			storageManager2.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			return;
		}
		
		SCR_ArsenalInventoryStorageManagerComponent storageManager3 = SCR_ArsenalInventoryStorageManagerComponent.Cast(m_Asset.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		if (storageManager3)
		{
			storageManager3.m_OnItemAddedInvoker.Insert(OnItemAdded);
			storageManager3.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemAdded(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemAdded)
		{
			action.OnActivate(m_Asset);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemRemoved)
		{
			action.OnActivate(m_Asset);
		}
	}

}
