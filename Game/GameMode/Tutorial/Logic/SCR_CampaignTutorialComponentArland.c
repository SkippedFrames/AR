//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandComponentClass : SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandComponent : SCR_BaseGameModeComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "conf")]
	ResourceName m_sStagesConfigAdvanced;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETutorialArlandStageMasters))]
	protected SCR_ETutorialArlandStageMasters m_eStartingStage;
	
	protected static const int TARGETS_SEARCH_DISTANCE = 1500;
	protected static const int INVALID_STANCE = -1;
	protected static const vector BACK_DOOR_OFFSET = {0,-0.7,0};
	protected static float WAYPOINT_FADE_THRESHOLD = 20;
	protected static float WAYPOINT_MINIMUM_OPACITY = 0.2;
	protected static float WAYPOINT_DISTANCE_INDICATOR_FADE_START = 100;
	protected static float WAYPOINT_DISTANCE_INDICATOR_FADE_END = 50;
	protected static const float SPAWN_STAGE_DISTANCE = 10;
	protected static const float FADE_DURATION = 1;
	protected static const float FADE_SPEED = 0.4;
	static string VEHICLE_MAINTENANCE_PREFAB = "{543C31BC05F032B6}PrefabsEditable/Auto/Compositions/Slotted/SlotFlatSmall/E_VehicleMaintenance_S_US_01.et";
	static string HELICOPTER_PREFAB = "{70BAEEFC2D3FEE64}Prefabs/Vehicles/Helicopters/UH1H/UH1H.et";
	static string HUMMER_PREFAB = "{751AFEEA19DDFB04}Prefabs/Vehicles/Wheeled/M998/M998_covered_long_MERDC.et";
	static string JEEP_PREFAB = "{94DE32169691AC34}Prefabs/Vehicles/Wheeled/M151A2/M151A2_transport_MERDC.et";
	static string TRUCK_PREFAB = "{C8656AECF5DF41D9}Prefabs/Vehicles/Wheeled/M923A1/M923A1_repair_MERDC.et";
	static string DRIVING_RANGE_PREFAB = "{B6F5006CBD24E9A1}Prefabs/TutorialDrivingRange.et";
	
	protected ref array<ref SCR_CampaignTutorialArlandStageInfo> m_aStageInfos = {};
	protected ref array<ref SCR_CampaignTutorialArlandStages> m_aStageConfigs = {};
	protected int m_iActiveStage;
	protected SCR_BaseCampaignTutorialArlandStage m_Stage;
	protected ImageWidget m_wFadeOut;
	protected SCR_GameModeCampaign m_CampaignGamemode;
	protected bool m_bPlayerSpawned;
	protected SCR_ECampaignTutorialArlandStage m_eStage = SCR_ECampaignTutorialArlandStage.START;
	protected SCR_ECampaignTutorialArlandStage m_eLastStage;
	protected ChimeraCharacter m_Player;
	protected int m_iStagesCnt;
	protected ref array<Widget> m_aWaypoints = {};
	protected bool m_bFirstStageSet;
	protected ref array<SCR_FiringRangeTarget> m_aFiringRangeTargets = {};
	protected SCR_CampaignSuppliesComponent m_SupplyTruckComponent;
	protected int m_iCountOfHits;
	protected SCR_CampaignMilitaryBaseComponent m_HQUS;
	protected BaseRadioComponent m_Radio;
	protected bool m_bIsMapOpen = false;
	protected bool m_bUsed3PV = false;
	protected ECharacterStance m_ePlayerStance = INVALID_STANCE;
	protected bool m_bCheckIsDeployed = false;
	protected bool m_bCheckIsDeployedBipod = false;
	protected bool m_bCheckLeaning = false;
	protected SCR_CampaignMobileAssemblyComponent m_MobileAssemblyComponent;
	protected bool m_bMovedOutOfVehicleByScript = false;
	protected bool m_fDelayedControlSchemeChangeRunning = false;
	protected bool m_bIsUsingGamepad;
	protected SCR_ETutorialArlandSupplyTruckWaypointMode m_eWaypointTruckPosition = SCR_ETutorialArlandSupplyTruckWaypointMode.NONE;
	protected float m_fStageTimestamp;
	protected SCR_CampaignTutorialArlandStages m_ActiveConfig;
	protected ResourceName m_WaypointWidget = "{B80E30D68492B2AE}UI/Imagesets/TutorialHud/TutorialHUD.imageset";
	protected static ref ScriptInvoker m_OnStructureBuilt = new ScriptInvoker();
	protected ref array<IEntity> m_aPlacedCompositions = {};
	protected IEntity m_PreviewBunker;
	protected SCR_PlacingEditorComponent m_PlacingComponent;
	protected bool m_bIsFirstRun = false;
	protected Vehicle m_RepairTruck;
	protected Vehicle m_Helicopter;
	protected Vehicle m_Hummer;
	protected Vehicle m_Jeep;
	protected Vehicle m_HummerRepairable;
	
	//------------------------------------------------------------------------------------------------
	protected void RemovePlayerMapMarkers()
	{
		SCR_MapMarkerManagerComponent mapMarkerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (!mapMarkerManager)
			return;
		
		array <ref SCR_MapMarkerBase> markerArray = {};
		markerArray = mapMarkerManager.GetLocalMarkers();
		
		for (int index = markerArray.Count()-1; index >= 0; index--)
		{
			mapMarkerManager.RemoveLocalMarker(markerArray[index]);
		}
		
		markerArray = mapMarkerManager.GetStaticMarkers();
		for (int index = markerArray.Count()-1; index >= 0; index--)
		{
			mapMarkerManager.RemoveStaticMarker(markerArray[index]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_Helicopter()
	{
		m_Helicopter = Vehicle.Cast(SpawnAsset("UH1COURSE", HELICOPTER_PREFAB));
		if (!m_Helicopter)
			return;
		
		//general helicopter damage check
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Helicopter.GetDamageManager());
		if (damageManager)
			damageManager.GetOnDamageStateChanged().Insert(OnHelicopterDamaged);
		
		//Drowned periodical check
		GetGame().GetCallqueue().CallLater(HelicopterDrownedCheck, 1000, true);
		
		//Rotor damage check
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_Helicopter.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
		
		IEntity rotor;
		EntitySlotInfo slot = slotManager.GetSlotByName("RotorMain");
		if (slot)
		{
			rotor = slot.GetAttachedEntity();
			SCR_RotorDamageManagerComponent rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			rotorDmgComp.GetOnDamageStateChanged().Insert(OnHelicopterDamaged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HelicopterDrownedCheck()
	{
		if (!m_Helicopter)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Helicopter.FindComponent(VehicleControllerComponent_SA));
		if (!vehicleController)
			return;
		
		if (!vehicleController.GetEngineDrowned())
			return;
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);	
	}
	
	
	//------------------------------------------------------------------------------------------------
	void RemoveHelicopterInvokers()
	{
		if (!m_Helicopter)
			return;
		
		//General damage invoker
		SCR_VehicleDamageManagerComponent heliDamageManager = SCR_VehicleDamageManagerComponent.Cast(m_Helicopter.GetDamageManager());
		if (heliDamageManager)
			heliDamageManager.GetOnDamageStateChanged().Remove(OnHelicopterDamaged);
		
		//Drowned periodical check
		GetGame().GetCallqueue().Remove(HelicopterDrownedCheck);
		
		//Rotor damage invoker
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_Helicopter.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
		
		EntitySlotInfo slot = slotManager.GetSlotByName("RotorMain");
		if (slot)
		{
			IEntity rotor = slot.GetAttachedEntity();
			if (!rotor)
				return;
			
			SCR_RotorDamageManagerComponent rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			if (!rotorDmgComp)
				return;
			
			rotorDmgComp.GetOnDamageStateChanged().Remove(OnHelicopterDamaged);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHelicopterDamaged(EDamageState state)
	{	
		if (state == EDamageState.UNDAMAGED)
			return;
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVehicleDamaged(EDamageState state)
	{	
		if (state == EDamageState.UNDAMAGED)
			return;
		
		if (m_Jeep)
		{
			SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Jeep.GetDamageManager());
			if (damageManager)
			{
				damageManager.GetOnDamageStateChanged().Remove(OnVehicleDamaged);
			
				ScriptedHitZone engine = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
				if (engine)
					engine.GetOnDamageStateChanged().Remove(OnVehicleDamaged);
		
				ScriptedHitZone gearbox = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
				if (gearbox)
					gearbox.GetOnDamageStateChanged().Remove(OnVehicleDamaged);
			}
		}
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEngineStoppedJeep()
	{
		if (!m_Jeep)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Jeep.FindComponent(VehicleControllerComponent_SA));
		if (!vehicleController || !vehicleController.GetEngineDrowned())
			return;
		
		vehicleController.GetOnEngineStop().Remove(OnEngineStoppedJeep);
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEngineStoppedHmw()
	{
		if (!m_Hummer)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Hummer.FindComponent(VehicleControllerComponent_SA));
		if (!vehicleController || !vehicleController.GetEngineDrowned())
			return;
		
		vehicleController.GetOnEngineStop().Remove(OnEngineStoppedHmw);
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEngineStoppedTruck()
	{
		if (!m_RepairTruck)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_RepairTruck.FindComponent(VehicleControllerComponent_SA));
		if (!vehicleController || !vehicleController.GetEngineDrowned())
			return;
		
		vehicleController.GetOnEngineStop().Remove(OnEngineStoppedTruck);
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTruckDamaged(EDamageState state)
	{	
		if (state == EDamageState.UNDAMAGED)
			return;
		
		if (m_RepairTruck)
		{
			SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_RepairTruck.GetDamageManager());
			if (damageManager)
			{
				damageManager.GetOnDamageStateChanged().Remove(OnTruckDamaged);
			
				ScriptedHitZone engine = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
				if (engine)
					engine.GetOnDamageStateChanged().Remove(OnTruckDamaged);
		
				ScriptedHitZone gearbox = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
				if (gearbox)
					gearbox.GetOnDamageStateChanged().Remove(OnTruckDamaged);
		
			}
		}
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
		//------------------------------------------------------------------------------------------------
	protected void OnHmwDamaged(EDamageState state)
	{	
		if (state == EDamageState.UNDAMAGED)
			return;
		
		if (m_Hummer)
		{
			SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Hummer.GetDamageManager());
			if (damageManager)
			{
				damageManager.GetOnDamageStateChanged().Remove(OnHmwDamaged);
			
				ScriptedHitZone engine = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
				if (engine)
					engine.GetOnDamageStateChanged().Remove(OnHmwDamaged);
		
				ScriptedHitZone gearbox = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
				if (gearbox)
					gearbox.GetOnDamageStateChanged().Remove(OnHmwDamaged);
			}
		}
		
		SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity FindBuiltComposition(string prefabName)
	{
		if (!m_aPlacedCompositions)
			return null;
		
		foreach (IEntity composition : m_aPlacedCompositions)
		{
			if (composition && composition.GetPrefabData().GetPrefabName() == prefabName)
				return composition; 
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActiveStage()
	{
		return m_iActiveStage;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActiveStage(int i)
	{
		m_iActiveStage = i;
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowMapDescriptor(string descriptorOwnerName, bool enable)
	{
		IEntity ent = IEntity.Cast(GetGame().GetWorld().FindEntityByName(descriptorOwnerName));
		if (!ent)
			return;

		SCR_MapDescriptorComponent descr = SCR_MapDescriptorComponent.Cast(ent.FindComponent(SCR_MapDescriptorComponent));
		if (!descr)
			return;

		descr.Item().SetVisible(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! ToDo - replace with proper Math.MapAngle usage
	bool IsMyAngleInRange(float angle, float range)
	{
		float cameraAngle = Math.RAD2DEG * m_Player.GetCharacterController().GetInputContext().GetAimingAngles()[0];
		return (cameraAngle > angle-range) && (cameraAngle < angle+range);
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns angle to entity from players current position.
	float GetEntityCompassAngle(notnull IEntity ent)
	{
		if (!m_Player)
			return 0;
		
		vector direction = ent.GetOrigin() - m_Player.GetOrigin();
		direction.Normalize();
		
		float angle = Math.Atan2(direction[0], direction[2]) * Math.RAD2DEG;
		
		return Math.Round(angle);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPreviewBunker(IEntity ent)
	{
		m_PreviewBunker = ent;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetPreviewBunker()
	{
		return m_PreviewBunker;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnModeAdded(SCR_EditorModeEntity modeEntity)
	{
		if (modeEntity.GetModeType() != EEditorMode.BUILDING)
			return;
		
		m_PlacingComponent = SCR_PlacingEditorComponent.Cast(modeEntity.FindComponent(SCR_PlacingEditorComponent));
		if (!m_PlacingComponent)
			return;
		
		m_PlacingComponent.GetOnPlaceEntityServer().Insert(OnPlacedPreview);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupEditorModeListener()
	{	
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return;
		
		editorManager.GetOnModeAdd().Insert(OnModeAdded);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_PlacingEditorComponent GetPlacingComponent()
	{
		return m_PlacingComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlacedPreview(int prefabID, SCR_EditableEntityComponent ent)
	{
		if (!ent)
			return;
		
		m_aPlacedCompositions.Insert(ent.GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnObjectDestroyed(EDamageState state)
	{
		if (state == EDamageState.DESTROYED)
			SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnStructureBuilt()
	{
		return m_OnStructureBuilt;
	}									   
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity);
		
		delete(m_Stage);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!playerController)
			return;
		
		array<SCR_SpawnPoint> availableSpawnpoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_CampaignGamemode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR));
		
		if (availableSpawnpoints.Count() < 2)
			return;
		
		TrySpawnPlayer(playerController, availableSpawnpoints[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (!SCR_HintManagerComponent.GetInstance().IsShown())
			return;
		
		m_bIsUsingGamepad = isGamepad;
		
		if (m_fDelayedControlSchemeChangeRunning)
			return;
		
		m_fDelayedControlSchemeChangeRunning = true;
		GetGame().GetCallqueue().CallLater(OnInputDeviceChangedDelayed, 250);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInputDeviceChangedDelayed()
	{
		m_fDelayedControlSchemeChangeRunning = false;
		bool switchedToKeyboard = !m_bIsUsingGamepad;
		m_Stage.OnInputDeviceChanged(switchedToKeyboard);
	}
	
	//------------------------------------------------------------------------------------------------
	ChimeraCharacter GetPlayer()
	{
		return m_Player;
	}	
	
	//------------------------------------------------------------------------------------------------
	void FadeToBlack(bool fadeOut)
	{
		SCR_FadeInOutEffect fade = SCR_FadeInOutEffect.Cast(GetGame().GetHUDManager().FindInfoDisplay(SCR_FadeInOutEffect));
		fade.FadeOutEffect(fadeOut, FADE_DURATION);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActiveConfig(SCR_ETutorialArlandStageMasters config)
	{	
		RemovePlayerMapMarkers();
		
		if (m_ActiveConfig && m_ActiveConfig.GetConfigClassName() == SCR_ETutorialArlandStageMasters.HELICOPTERS)
			RemoveHelicopterInvokers();
				
		foreach(SCR_CampaignTutorialArlandStages configs:m_aStageConfigs)
		{
			if(configs.GetStagesFromConfig() == config)
			{
				m_ActiveConfig = configs;
				m_ActiveConfig.GetStages(m_aStageInfos);
				if (m_Stage)
					m_Stage.FlushWaypoints();
				FlushWaypoints();
				delete m_Stage;
				m_iActiveStage = 0;
				SetStage(m_aStageInfos[0].GetIndex(), m_aStageInfos[m_aStageInfos.Count()-1].GetIndex());
				ResetPlayerPosition()
			}
		}
		SetupStageCounts();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWorldPostProcess(World world)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void FiringRangeInit()
	{
		typename stages = SCR_ECampaignTutorialArlandStage;
		IEntity shootingRangePos = GetGame().GetWorld().FindEntityByName(string.Format("PP_%1", stages.GetVariableName(SCR_ECampaignTutorialArlandStage.WEAPON_PICK)));
		IEntity baseBeauregard = GetGame().GetWorld().FindEntityByName("TownBaseBeauregard");
		
		if (shootingRangePos)
			GetGame().GetWorld().QueryEntitiesBySphere(shootingRangePos.GetOrigin(), TARGETS_SEARCH_DISTANCE, CollectAllTargets, null, EQueryEntitiesFlags.ALL);
		
		if (baseBeauregard)
			GetGame().GetWorld().QueryEntitiesBySphere(baseBeauregard.GetOrigin(), 50, CollectAllTargets, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	void MobileHQInit()
	{
		IEntity mHQ = GetGame().GetWorld().FindEntityByName("MobileHQ");
		
		if (!mHQ)
			return;
		
		SCR_CampaignFaction f = m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		
		if (!f)
			return;
		
		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(mHQ.FindComponent(BaseRadioComponent));
		
		if (radioComponent && radioComponent.TransceiversCount() > 0)
		{
			radioComponent.SetPower(false);
			radioComponent.GetTransceiver(0).SetFrequency(f.GetFactionRadioFrequency());
			radioComponent.SetEncryptionKey(f.GetFactionRadioEncryptionKey());
		}
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(mHQ.FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return;
		
		array<EntitySlotInfo> slots = new array<EntitySlotInfo>;
		slotManager.GetSlotInfos(slots);
		
		foreach (EntitySlotInfo slot: slots)
		{
			if (!slot)
				continue;
			
			IEntity truckBed = slot.GetAttachedEntity();
			
			if (!truckBed)
				continue;
			
			m_MobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));
			
			if (m_MobileAssemblyComponent)
			{
				m_MobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(f));
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SupplyTruckInit()
	{
		IEntity supplyTruck = GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck");
		
		if (!supplyTruck)
			return;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(supplyTruck.FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return;
		
		array<EntitySlotInfo> slots = new array<EntitySlotInfo>;
		slotManager.GetSlotInfos(slots);
		
		foreach (EntitySlotInfo slot: slots)
		{
			if (!slot)
				continue;
			
			IEntity truckBed = slot.GetAttachedEntity();
			
			if (!truckBed)
				continue;
			
			SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(truckBed.FindComponent(SCR_CampaignSuppliesComponent));
			
			if (suppliesComponent)
			{
				m_SupplyTruckComponent = suppliesComponent;
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_RifleRespawn()
	{
		SpawnAsset("M16", "{3E413771E1834D2F}Prefabs/Weapons/Rifles/M16/Rifle_M16A2.et");
		SpawnAsset("M21", "{81EB948E6414BD6F}Prefabs/Weapons/Rifles/M14/Rifle_M21_ARTII.et");
		SpawnAsset("M249", "{D2B48DEBEF38D7D7}Prefabs/Weapons/MachineGuns/M249/MG_M249.et");
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_MoveInJeep(IEntity position = null)
	{
		SpawnAsset("Jeep", "{5168FEA3054D6D15}Prefabs/Vehicles/Wheeled/M151A2/M151A2_M2HB_MERDC.et", position);
		MoveInVehicle("Jeep", ECompartmentType.Pilot);
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_SpawnTruck(IEntity position = null)
	{
		SpawnAsset("SupplyTruck", "{F37113A988304565}Prefabs/MP/Campaign/Assets/CampaignSupplyTruckWest.et", position);
		SupplyTruckInit();
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_ProcessTruck()
	{
		//m_SupplyTruckComponent.AddSupplies(1000);
		MoveInVehicle("SupplyTruck", ECompartmentType.Pilot);
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
		
		if (accessComp)
		{
			accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
			accessComp.GetOnCompartmentLeft().Insert(OnSupplyTruckLeft);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_PrepareFarm()
	{
		/*m_SupplyTruckComponent.AddSupplies(500);
		SCR_CampaignMilitaryBaseComponent baseFarm = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseFarm").FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (baseFarm && baseFarm.GetSupplies() < 950)
			baseFarm.AddSupplies(950 - baseFarm.GetSupplies());*/
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_ResetSeizing()
	{
		SCR_CampaignMilitaryBaseComponent baseBeauregard = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseBeauregard").FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (baseBeauregard)
		{
			baseBeauregard.SetFaction(m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
			baseBeauregard.RefreshTasks();
		}
		
		SCR_CampaignMilitaryBaseComponent baseMossHill = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("MainBaseMossHill").FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (baseMossHill)
		{
			baseMossHill.SetFaction(m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
			baseMossHill.RefreshTasks();
		}
		
		SpawnAsset("Seizing_Car", "{6B24D5AFD884D64C}Prefabs/Vehicles/Wheeled/M998/M998_MERDC.et");
		SpawnAsset("Seizing_Seizing_Car_Pub", "{6B24D5AFD884D64C}Prefabs/Vehicles/Wheeled/M998/M998_MERDC.et");
		IEntity mhq = SpawnAsset("MobileHQ", "{9CB496688A3BCC3E}Prefabs/MP/Campaign/Assets/CampaignMobileAssemblyWest.et");
		if (!mhq)
			return;
		
		SCR_CampaignFaction bluforFaction = m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		if (!bluforFaction)
			return;
		
		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(mhq.FindComponent(BaseRadioComponent));
		if (radioComponent && radioComponent.TransceiversCount() > 0)
		{
			radioComponent.SetPower(false);
			radioComponent.GetTransceiver(0).SetFrequency(bluforFaction.GetFactionRadioFrequency());
			radioComponent.SetEncryptionKey(bluforFaction.GetFactionRadioEncryptionKey());
		}
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(mhq.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
		
		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		foreach (EntitySlotInfo slot : slots)
		{
			IEntity truckBed = slot.GetAttachedEntity();

			if (!truckBed)
				continue;

			SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));
			if (mobileAssemblyComponent)
			{
				mobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(bluforFaction));
				break;
			}
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_HandleRespawnRadios()
	{
		while (m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetActiveRespawnRadios() > 0)
			m_CampaignGamemode.RemoveActiveRespawnRadio(m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
	}
	
	//------------------------------------------------------------------------------------------------
	void StageReset_DeployMHQ()
	{
		if (m_MobileAssemblyComponent && !m_MobileAssemblyComponent.IsDeployed())
		{
			m_MobileAssemblyComponent.UpdateRadioCoverage();
			m_MobileAssemblyComponent.Deploy(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool PrepareMedicalAmbulance()
	{
		IEntity ambulance = SpawnAsset("Ambulance", "{3B1EB924602C7A07}Prefabs/Vehicles/Wheeled/M998/M997_maxi_ambulance_MERDC.et");
		if (!ambulance)
			return false;
		
		SCR_VehicleDamageManagerComponent vehicleDamageManager = SCR_VehicleDamageManagerComponent.Cast(ambulance.FindComponent(SCR_VehicleDamageManagerComponent));
		if (!vehicleDamageManager)
			return false;
		
		vehicleDamageManager.GetOnDamageStateChanged().Insert(OnObjectDestroyed);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool PrepareAccidentArea()
	{
		IEntity victim = SpawnAsset("Victim", "{2F912ED6E399FF47}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Unarmed.et");
		if (!victim)
			return false;
		
		SCR_CharacterDamageManagerComponent damageComp = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageComp)
			return false;
		
		IEntity vehicle = SpawnAsset("AccidentJeep", "{86D830868F026D54}Prefabs/Vehicles/Wheeled/M151A2/M151A2_MERDC.et");
		if (!vehicle)
			return false;
		
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(victim.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return false;
		
		if (!compartmentAccess.MoveInVehicle(vehicle, ECompartmentType.Pilot))
			return false;
		
		//prepare injuries on victim
		damageComp.AddParticularBleeding("LThigh");
		damageComp.AddParticularBleeding("Chest");
		damageComp.GetHitZoneByName("LThigh").SetHealth(0);
		damageComp.GetHitZoneByName("Chest").SetHealth(0);
		damageComp.ForceUnconsciousness();
		damageComp.UpdateBloodClothes();
		damageComp.GetOnDamageStateChanged().Insert(OnObjectDestroyed);
		
		GetGame().GetCallqueue().Remove(RefreshVictimBloodLevel);
		GetGame().GetCallqueue().Remove(RefreshVictimResilience);
		GetGame().GetCallqueue().CallLater(RefreshVictimBloodLevel, 1000, true);
		GetGame().GetCallqueue().CallLater(RefreshVictimResilience, 1000, true);
		
		//Lock jeep to prevent players from driving away with it. However, this behavior cannot break any stages.
		SCR_VehicleSpawnProtectionComponent spawnProtectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(vehicle.FindComponent(SCR_VehicleSpawnProtectionComponent));
		if (spawnProtectionComponent)
		{
			spawnProtectionComponent.SetProtectOnlyDriverSeat(false);
			spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_BuildBlocked-UC");
			spawnProtectionComponent.SetVehicleOwner(-2);
		}
		
		//Disable damage handling on jeep to prevent players or uncanny physics from accidentally destroying it
		SCR_VehicleDamageManagerComponent vehicleDamageManager = SCR_VehicleDamageManagerComponent.Cast(vehicle.FindComponent(SCR_VehicleDamageManagerComponent));
		if (vehicleDamageManager)
			vehicleDamageManager.EnableDamageHandling(false);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStage_Medical()
	{
		//Return back to HUB should stage preparation fail
		if (!PrepareMedicalAmbulance())
			SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
		
		if (!PrepareAccidentArea())
			SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
		
		SpawnAsset("Backpack", "{4805E67E2AE30F8D}Prefabs/Items/Equipment/Backpacks/Backpack_Medical_M5.et");
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStage_VehiclesSimple()
	{
		m_Jeep = Vehicle.Cast(SpawnAsset("SmallJeep", JEEP_PREFAB));
		if (!m_Jeep)
			return;
		
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Jeep.GetDamageManager());
		if (damageManager)
		{
			damageManager.GetOnDamageStateChanged().Insert(OnVehicleDamaged);
		
			ScriptedHitZone engine = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
			if (engine)
				engine.GetOnDamageStateChanged().Insert(OnVehicleDamaged);
		
			ScriptedHitZone gearbox = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
			if (gearbox)
				gearbox.GetOnDamageStateChanged().Insert(OnVehicleDamaged);
		}
			
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Jeep.FindComponent(VehicleControllerComponent_SA));
		if (vehicleController)
			vehicleController.GetOnEngineStop().Insert(OnEngineStoppedJeep);
		
		IEntity trainingRange = SpawnAsset("TutorialDrivingRange", DRIVING_RANGE_PREFAB);
		if (!trainingRange)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStage_VehiclesHeavy()
	{
		m_RepairTruck = Vehicle.Cast(SpawnAsset("Truck",TRUCK_PREFAB ));
		if (!m_RepairTruck)
			return;
		
		m_Hummer = Vehicle.Cast(SpawnAsset("Hummer", HUMMER_PREFAB));
		if (!m_Hummer)
			return;
		
		m_HummerRepairable = Vehicle.Cast(SpawnAsset("Hummer2", HUMMER_PREFAB));
		if (!m_HummerRepairable)
			return;
		
		SCR_DamageManagerComponent damageManagerBroken = SCR_DamageManagerComponent.GetDamageManager(m_HummerRepairable);
		
		if (damageManagerBroken)
		{
			HitZone engineHitZone = damageManagerBroken.GetHitZoneByName("Engine_01");
			
			if (engineHitZone)
			{
				// This will work only if hitzone belongs to this damage manager
				vector transform[3];
				transform[0] = m_HummerRepairable.GetOrigin();
				transform[1] = vector.Forward;
				transform[2] = vector.Up;
				damageManagerBroken.HandleDamage(EDamageType.TRUE, engineHitZone.GetMaxHealth(), transform, m_HummerRepairable, engineHitZone, Instigator.CreateInstigator(null), null, -1, -1);
			}
		}
		
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_RepairTruck.GetDamageManager());
		if (damageManager)
		{
			damageManager.GetOnDamageStateChanged().Insert(OnTruckDamaged);
		
			ScriptedHitZone engine = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
			if (engine)
				engine.GetOnDamageStateChanged().Insert(OnTruckDamaged);
		
			ScriptedHitZone gearbox = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
			if (gearbox)
				gearbox.GetOnDamageStateChanged().Insert(OnTruckDamaged);
		}
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_RepairTruck.FindComponent(VehicleControllerComponent_SA));
		if (vehicleController)
			vehicleController.GetOnEngineStop().Insert(OnEngineStoppedTruck);
		
		damageManager = SCR_VehicleDamageManagerComponent.Cast(m_Hummer.GetDamageManager());
		if (damageManager)
		{
			damageManager.GetOnDamageStateChanged().Insert(OnHmwDamaged);
		
			ScriptedHitZone engine = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
			if (engine)
				engine.GetOnDamageStateChanged().Insert(OnHmwDamaged);
		
			ScriptedHitZone gearbox = ScriptedHitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
			if (gearbox)
				gearbox.GetOnDamageStateChanged().Insert(OnHmwDamaged);
		}
		
		vehicleController = VehicleControllerComponent_SA.Cast(m_Hummer.FindComponent(VehicleControllerComponent_SA));
		if (vehicleController)
			vehicleController.GetOnEngineStop().Insert(OnEngineStoppedHmw);
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStage_ShootingRange()
	{
		IEntity ambulance = SpawnAsset("Ambulance", "{3B1EB924602C7A07}Prefabs/Vehicles/Wheeled/M998/M997_maxi_ambulance_MERDC.et");
		if (!ambulance)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStage_CampaignBuilding()
	{
		//Delete built compositions.
		foreach (IEntity ent : m_aPlacedCompositions)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(ent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RefreshVictimResilience()
	{
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		SCR_CharacterDamageManagerComponent damageComp = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageComp)
			return;
		
		damageComp.GetResilienceHitZone().SetHealth(0);
	}
	
	//------------------------------------------------------------------------------------------------
	void RefreshVictimBloodLevel()
	{
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		SCR_CharacterDamageManagerComponent damageComp = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageComp)
			return;
		
		damageComp.GetBloodHitZone().SetHealth(2000);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CharacterDamageManagerComponent GetVictimDamageManager()
	{
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return NULL;
		
		return SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetStage(bool forcePlayerReset = false)
	{
		if (!m_Player.IsInVehicle() || forcePlayerReset)
			ResetPlayerPosition();
		
		if (m_Stage)
			m_Stage.Reset();
	}
	
	//------------------------------------------------------------------------------------------------
	Vehicle GetHummer()
	{
		return m_HummerRepairable;
	}
		
	//------------------------------------------------------------------------------------------------
	Vehicle GetRepairTruck()
	{
		return m_RepairTruck;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity SpawnAsset(string name, ResourceName type, IEntity spawnpoint = null)
	{
		string posName;
		
		if (!spawnpoint)
			spawnpoint = GetGame().GetWorld().FindEntityByName(string.Format("SpawnPos_%1", name));
		
		if (!spawnpoint)
			return null;
		
		IEntity oldEntity = GetGame().GetWorld().FindEntityByName(name);

		if (oldEntity)
			SCR_EntityHelper.DeleteEntityAndChildren(oldEntity);
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		spawnpoint.GetWorldTransform(params.Transform);
		Resource res = Resource.Load(type);
		
		if (!res)
			return null;
		
		IEntity newEntity = GetGame().SpawnEntityPrefab(res, null, params);
		newEntity.SetName(name);
		
		Vehicle veh = Vehicle.Cast(newEntity);
		
		if (veh)
		{
			vector pos = veh.GetOrigin();
			pos[1] = pos[1] + 1;
			
			Physics physicsComponent = veh.GetPhysics();
			
			if (physicsComponent)
			{
				veh.SetOrigin(pos);
				physicsComponent.SetVelocity("0 -1 0");
			}
		}
		
		return newEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteVehicleWhenEmpty(Vehicle veh)
	{
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (!compartmentAccessComponent)
		{
			delete veh;
			GetGame().GetCallqueue().Remove(DeleteVehicleWhenEmpty);
		}
		
		if (compartmentAccessComponent.GetVehicle() != veh)
		{
			delete veh;
			GetGame().GetCallqueue().Remove(DeleteVehicleWhenEmpty);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CollectAllTargets(IEntity ent)
	{
		SCR_FiringRangeTarget target = SCR_FiringRangeTarget.Cast(ent);
		if (!target)
			return true;
		
		target.SetState(ETargetState.TARGET_DOWN);
		m_aFiringRangeTargets.Insert(target);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_FiringRangeTarget> GetAllTargets()
	{
		return m_aFiringRangeTargets;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTargetHits()
	{
		return m_iCountOfHits;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCheckLeaning(bool check)
	{
		m_bCheckLeaning = check;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerStanceToCheck(ECharacterStance stance)
	{
		m_ePlayerStance = stance;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerDeployedCheck(bool check)
	{
		m_bCheckIsDeployed = check;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerDeployedBipodCheck(bool check)
	{
		m_bCheckIsDeployedBipod = check;
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterStance GetPlayerStanceToCheck()
	{
		return m_ePlayerStance;
	}
	
	//------------------------------------------------------------------------------------------------
	void CountTargetHit(ETargetState state, SCR_FiringRangeTarget target)
	{
		if (state != ETargetState.TARGET_DOWN)
			return; 
		
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
		if (!comp)
			return;
		
		if (GetPlayerStanceToCheck() != INVALID_STANCE && comp.GetStance() != GetPlayerStanceToCheck())
			return;
		
		if (m_bCheckIsDeployed && !comp.GetIsWeaponDeployed())
			return;
		
		if (m_bCheckIsDeployedBipod && !comp.GetIsWeaponDeployedBipod())
			return;
		
		// Leaning doesn't matter or is wrong.
		if (m_bCheckLeaning && !IsCharacterLeaning())
			return;
		
		target.SetAutoResetTarget(false);
		target.Event_TargetChangeState.Remove(CountTargetHit);
		m_iCountOfHits ++;	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWaypointTruckPosition(SCR_ETutorialArlandSupplyTruckWaypointMode mode)
	{
		m_eWaypointTruckPosition = mode;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		
		BaseWorld world = GetGame().GetWorld();
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("MainBaseHQ").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(0);
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("TownBaseFarm").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(1);
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("TownBaseBeauregard").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(2);
		SCR_CampaignMilitaryBaseComponent.Cast(world.FindEntityByName("MainBaseMossHill").FindComponent(SCR_CampaignMilitaryBaseComponent)).SetCallsignIndex(4);
		
		// Attempt to spawn the player automatically, cease after spawn is successful in OnPlayerSpawned
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		ForcePlayerFaction(playerController);
		

	}
	
	//------------------------------------------------------------------------------------------------
	void ForcePlayerFaction(notnull PlayerController pc)
	{
		if (!m_CampaignGamemode)
			return;
		
		int playerId = pc.GetPlayerId();
		
		// Skip faction and spawnpoint selection
		SCR_RespawnSystemComponent respawnSystem = m_CampaignGamemode.GetRespawnSystemComponent();
		
		if (!respawnSystem)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if (!factionManager)
			return;

		Faction factionUS =  m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		if (!factionUS)
			return;
		
		SCR_BasePlayerLoadout loadout = GetGame().GetLoadoutManager().GetRandomFactionLoadout(factionUS);
		if (!loadout)
			return;
		
		array<SCR_SpawnPoint> availableSpawnpoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_CampaignGamemode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR));
		
		if (availableSpawnpoints.Count() < 1)
			return;
		
		if (!pc)
			return;

		SCR_PlayerFactionAffiliationComponent playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		SCR_PlayerLoadoutComponent playerLoadout = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
		SCR_RespawnComponent respawnComp = SCR_RespawnComponent.Cast(pc.FindComponent(SCR_RespawnComponent));

		
		playerFactionAffiliation.GetOnPlayerFactionResponseInvoker_S().Insert(OnPlayerFactionResponse);
		playerLoadout.GetOnPlayerLoadoutResponseInvoker_S().Insert(OnPlayerLoadoutResponse);
		respawnComp.GetOnRespawnResponseInvoker_S().Insert(OnPlayerSpawnResponse);
		
		TrySetPlayerFaction(pc, factionUS);
		TrySetPlayerLoadout(pc, loadout);
		TrySpawnPlayer(pc, availableSpawnpoints[0]);		
	}

	//------------------------------------------------------------------------------------------------	
	protected void OnPlayerFactionResponse(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		PlayerController controller = PlayerController.Cast(component.GetOwner());
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if (!factionManager)
			return;

		Faction factionUS = factionManager.GetFactionByIndex(factionIndex);
		
		if (!factionUS)
			return;
		
		if(!response)
			TrySetPlayerFaction(controller, factionUS);
		else
			component.GetOnPlayerFactionResponseInvoker_S().Remove(OnPlayerFactionResponse);
	}

	//------------------------------------------------------------------------------------------------		
	protected void OnPlayerLoadoutResponse(SCR_PlayerLoadoutComponent component, int loadoutIndex, bool response)
	{
		PlayerController controller = PlayerController.Cast(component.GetOwner());
		SCR_BasePlayerLoadout loadout = GetGame().GetLoadoutManager().GetLoadoutByIndex(loadoutIndex);
		
		if(!response)
			TrySetPlayerLoadout(controller, loadout);
		else
			component.GetOnPlayerLoadoutResponseInvoker_S().Remove(OnPlayerLoadoutResponse);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawnResponse(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		PlayerController controller = PlayerController.Cast(requestComponent.GetOwner());
		array<SCR_SpawnPoint> availableSpawnpoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_CampaignGamemode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR));
		SCR_RespawnComponent respawnComp = SCR_RespawnComponent.Cast(controller.FindComponent(SCR_RespawnComponent));
		
		if(response != SCR_ESpawnResult.OK)
			TrySpawnPlayer(controller, availableSpawnpoints[1]);
		else
			respawnComp.GetOnRespawnResponseInvoker_S().Remove(OnPlayerSpawnResponse);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void TrySetPlayerFaction(notnull PlayerController playerController, Faction faction)
	{
		
		SCR_PlayerFactionAffiliationComponent playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!playerFactionAffiliation.RequestFaction(faction))
		{
			// Request not sent, failed locally
			return;
		}
		
		// Request sent, await response.
		// Response delivered via SCR_PlayerFactionAffiliationComponent.GetOnCanPlayerFactionResponseInvoker..., explained above.
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TrySetPlayerLoadout(notnull PlayerController playerController, SCR_BasePlayerLoadout loadout)
	{
		
		SCR_PlayerLoadoutComponent playerLoadoutComponent = SCR_PlayerLoadoutComponent.Cast(playerController.FindComponent(SCR_PlayerLoadoutComponent));
		if (!playerLoadoutComponent.RequestLoadout(loadout))
		{
			// Request not sent, failed locally
			return;
		}
		
		// Request sent, await response.
		// Response delivered via SCR_PlayerLoadoutComponent.GetOnPlayerLoadoutResponseInvoker...(), explained above.
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TrySpawnPlayer(notnull PlayerController playerController, notnull SCR_SpawnPoint spawnPoint)
	{
		
		SCR_RespawnComponent spawnComponent = SCR_RespawnComponent.Cast(playerController.FindComponent(SCR_RespawnComponent));
		SCR_PlayerLoadoutComponent loadoutComponent = SCR_PlayerLoadoutComponent.Cast(playerController.FindComponent(SCR_PlayerLoadoutComponent));
		
		// Retrieve loadout the user had stored prior to this request; this is synchronized as an ask/response
		SCR_BasePlayerLoadout loadout = loadoutComponent.GetAssignedLoadout();
		ResourceName loadoutResource = loadout.GetLoadoutResource();
		SCR_SpawnData spawnData = new SCR_SpawnPointSpawnData(loadoutResource, spawnPoint.GetRplId());		
		if (!spawnComponent.RequestSpawn(spawnData))
		{
			// Request not sent, failed locally
			return;
		}
		
		// Request sent, await response.
		// Response delivered via SCR_RespawnComponent.GetOnRespawnResponseInvoker...(), explained above.
	}		 
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		m_bIsMapOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapClose(MapConfiguration config)
	{
		m_bIsMapOpen = false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsMapOpen()
	{
		return m_bIsMapOpen;
	}
	
	//------------------------------------------------------------------------------------------------
	BaseRadioComponent GetPlayerRadio()
	{
		return m_Radio;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateSupplyTruckWaypoint(IEntity waypoint, bool useTruck, string contextName, vector offset = vector.Zero)
	{
		if (contextName.IsEmpty() || !m_SupplyTruckComponent)
			return;
		
		IEntity ent = m_SupplyTruckComponent.GetOwner();
		
		if (useTruck)
			ent = ent.GetParent();
		
		ActionsManagerComponent amc = ActionsManagerComponent.Cast(ent.FindComponent(ActionsManagerComponent));
			
		if (!amc)
			return;
			
		UserActionContext context = amc.GetContext(contextName);
				
		if (!context)
			return;
				
		vector pos = context.GetOrigin();
		pos = pos + offset;
		waypoint.SetOrigin(pos);
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckRadioPickup(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		m_Radio = BaseRadioComponent.Cast(item.FindComponent(BaseRadioComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	bool CheckCharacterStance(ECharacterCommandIDs command)
	{
		CharacterAnimationComponent comp = CharacterAnimationComponent.Cast(m_Player.FindComponent(CharacterAnimationComponent));
		bool ret = false;
					
		if (!comp)
			return false;
		
		CharacterMovementState mState = new CharacterMovementState;
		comp.GetMovementState(mState);
		
		if (mState)
			ret = mState.m_CommandTypeId == command;
	
		delete mState;
		return ret;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetFirstRun()
	{
		return m_bIsFirstRun;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFirstRun(bool firstRun)
	{
		m_bIsFirstRun = firstRun;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsCharacterLeaning()
	{
		SCR_CharacterControllerComponent comp = SCR_CharacterControllerComponent.Cast(m_Player.FindComponent(SCR_CharacterControllerComponent));
		if (!comp)
			return false;
		
		return comp.IsLeaning();
	}
	
	//------------------------------------------------------------------------------------------------
	void Check3rdPersonViewUsed()
	{
		if (!m_Player)
			return;
		
		CameraHandlerComponent comp = CameraHandlerComponent.Cast(m_Player.FindComponent(CameraHandlerComponent));
		
		if (!comp)
			return;
		
		if (comp.IsInThirdPerson())
		{
			m_bUsed3PV = true;
			GetGame().GetCallqueue().Remove(Check3rdPersonViewUsed);
			
			if (m_eStage == SCR_ECampaignTutorialArlandStage.WALL)
				GetGame().GetCallqueue().Remove(DelayedPopup);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetWas3rdPersonViewUsed()
	{
		return m_bUsed3PV;
	}
	
	//------------------------------------------------------------------------------------------------
	void MoveInVehicle(string vehName, ECompartmentType seat)
	{
		Vehicle veh = Vehicle.Cast(GetGame().GetWorld().FindEntityByName(vehName));
		
		if (!veh)
			return;
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (!compartmentAccessComponent)
			return;
		
		if (compartmentAccessComponent.GetVehicle())
			GetGame().GetCallqueue().CallLater(MoveInVehicle, 100, false, vehName, seat);
		else
		{
			if (m_wFadeOut)
			{
				m_wFadeOut.SetOpacity(1);
				AnimateWidget.Opacity(m_wFadeOut, 0, 0.8);
			}
			
			m_bMovedOutOfVehicleByScript = false;
			compartmentAccessComponent.MoveInVehicle(veh, seat);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool CheckCharacterInVehicle(ECompartmentType seat)
	{
		CompartmentAccessComponent compartmentAccessComponent = CompartmentAccessComponent.Cast(m_Player.FindComponent(CompartmentAccessComponent));
				
		if (!compartmentAccessComponent)
			return false;
	
		BaseCompartmentSlot compartmentSlot = compartmentAccessComponent.GetCompartment();
	
		if (!compartmentSlot)
			return false;
		
		return SCR_CompartmentAccessComponent.GetCompartmentType(compartmentSlot) == seat;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSupplyTruckLeft(IEntity vehicle)
	{
		if (m_bMovedOutOfVehicleByScript)
			return;
		
		if (!m_Player)
			return;
		
		CharacterControllerComponent charController = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
				
		if (!charController)
			return;
				
		if (charController.IsDead())
			return;
		
		SCR_PopUpNotification.GetInstance().HideCurrentMsg();
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
				
		if (accessComp)
			accessComp.GetOnCompartmentLeft().Remove(OnSupplyTruckLeft);
		
		FinishStage(m_Stage, SCR_ECampaignTutorialArlandStage.CONFLICT_UNLOADING_SUPPLIES);
		m_iActiveStage = 17;
		ResetStage(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void MovePlayer(notnull IEntity newPos, ResourceName loadoutResourceName)
	{		
		GetGame().GetCallqueue().CallLater(ResetPlayerCharacter, FADE_SPEED*100, false, newPos, loadoutResourceName);
		
		if (m_wFadeOut)
		{
			AnimateWidget.Opacity(m_wFadeOut, 0, FADE_SPEED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetPlayerCharacter(IEntity newPos, ResourceName loadoutResourceName)
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		newPos.GetWorldTransform(params.Transform);
		
		Resource res = Resource.Load(loadoutResourceName);
		if (!res.IsValid())
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		IEntity oldCharacter = playerController.GetControlledEntity();
		if (!oldCharacter)
		{
			Print("SCR_CampaignTutorialArlandComponent:: No playercontrolled entity found!", LogLevel.WARNING);
			return;
		}
		
		//Save old identity to keep same face
		Identity oldIdentity;
		SCR_CharacterIdentityComponent identityComp = SCR_CharacterIdentityComponent.Cast(oldCharacter.FindComponent(SCR_CharacterIdentityComponent));
		if (identityComp)
			oldIdentity = identityComp.GetIdentity();
		
		IEntity newCharacter = GetGame().SpawnEntityPrefab(res, null, params);
		if (!newCharacter)
			return;
		
		identityComp = SCR_CharacterIdentityComponent.Cast(newCharacter.FindComponent(SCR_CharacterIdentityComponent));
		if (identityComp && oldIdentity)
			identityComp.SetIdentity(oldIdentity);
		
		m_Player = ChimeraCharacter.Cast(newCharacter);
		playerController.SetControlledEntity(newCharacter);
		
		SCR_EntityHelper.DeleteEntityAndChildren(oldCharacter);
		
		if (!m_Stage)
			return;
		
		m_Stage.SetPlayer(m_Player);
	}
	
	//------------------------------------------------------------------------------------------------
	void DelayedPopup(string text = "", string subtitle = "", float duration = SCR_PopUpNotification.DEFAULT_DURATION, string param1 = "", string param2 = "", string subtitleParam1 = "", string subtitleParam2 = "")
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: subtitle, param1: param1, param2: param2, text2param1: subtitleParam1, text2param2: subtitleParam2, category: SCR_EPopupMsgFilter.TUTORIAL);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnStructureBuilt(SCR_CampaignMilitaryBaseComponent base, IEntity structure)
	{
		if (m_Stage)
			m_Stage.OnStructureBuilt(base, structure);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		m_bPlayerSpawned = true;
		m_Player = ChimeraCharacter.Cast(entity);
		
		if (m_Stage)
			m_Stage.SetPlayer(m_Player);
		
		if (!m_bFirstStageSet)
		{
			if (m_CampaignGamemode.IsTutorial())
			{
				SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
				
				if (hudManager)
				{
					SCR_XPInfoDisplay display = SCR_XPInfoDisplay.Cast(hudManager.FindInfoDisplay(SCR_XPInfoDisplay));
					
					if (display)
						display.AllowShowingInfo(false);
				}
			}
			
			m_bFirstStageSet = true;
			m_HQUS = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("MainBaseHQ").FindComponent(SCR_CampaignMilitaryBaseComponent));
			m_HQUS.AlterSupplyIncomeTimer(9999999);
			FiringRangeInit();
			MobileHQInit();
			IEntity WP_cottage = GetGame().GetWorld().FindEntityByName("WP_CONFLICT_COMPAS_MOVE");
			IEntity WP_mobileHQ = GetGame().GetWorld().FindEntityByName("WP_CONFLICT_MAP_MOVE");
			SCR_MapDescriptorComponent cottageDescr = SCR_MapDescriptorComponent.Cast(WP_cottage.FindComponent(SCR_MapDescriptorComponent));
			SCR_MapDescriptorComponent mobileTruckDescr = SCR_MapDescriptorComponent.Cast(WP_mobileHQ.FindComponent(SCR_MapDescriptorComponent));
			cottageDescr.Item().SetVisible(false);
			mobileTruckDescr.Item().SetVisible(false);
			
			while (m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetActiveRespawnRadios() < m_CampaignGamemode.GetMaxRespawnRadios())
				m_CampaignGamemode.AddActiveRespawnRadio(m_CampaignGamemode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
			
			m_wFadeOut = ImageWidget.Cast(GetGame().GetHUDManager().CreateLayout("{265245C299401BF6}UI/layouts/Menus/ContentBrowser/DownloadManager/ScrollBackground.layout", EHudLayers.OVERLAY));
			m_wFadeOut.SetOpacity(1);
			int stage = m_eStage;
			SetActiveConfig(0);
#ifdef WORKBENCH
			stage = m_eStartingStage;
#endif
			//TODO: @yuri pls fix for the future, very dirty indeed.
			GetGame().GetCallqueue().CallLater(Check3rdPersonViewUsed, 500, true);
			
#ifdef ENABLE_DIAG
			SetEventMask(m_CampaignGamemode, EntityEvent.FRAME);
#endif
		}
		else
			SetStage(m_eStage);
		
		ResetStage();
	}

	//------------------------------------------------------------------------------------------------	
	void CreateWaypoint()
	{
		Widget waypointFrame = GetGame().GetHUDManager().CreateLayout("{825C6D728AC3E029}UI/layouts/Tutorial/TutorialWaypointEdited.layout", EHudLayers.BACKGROUND);
		m_aWaypoints.Insert(waypointFrame.FindAnyWidget("RootTutorialHud"));
		
		//setup base image
		ImageWidget image = ImageWidget.Cast(waypointFrame.FindAnyWidget("Icon"));
		image.SetOpacity(0);
		image.LoadImageFromSet(0, m_WaypointWidget, "MISC");
		image.SetColor(Color.Yellow);
		
		//setup distance
		RichTextWidget text = RichTextWidget.Cast(waypointFrame.FindAnyWidget("Distance"));
		text.SetOpacity(0);
		text.SetColor(Color.Yellow);
		
		//setup misc
		image = ImageWidget.Cast(waypointFrame.FindAnyWidget("IconExtra"));
		image.SetOpacity(0);
		image.SetVisible(false);
		image.LoadImageFromSet(0, m_WaypointWidget, "CUSTOM");
		image.SetColor(Color.Yellow);
	}
	
	//------------------------------------------------------------------------------------------------	
	void FlushWaypoints()
	{
		foreach (Widget waypoints : m_aWaypoints)
			waypoints.RemoveFromHierarchy();
		m_aWaypoints.Clear();
	}	
	
	//------------------------------------------------------------------------------------------------	
	void SetWaypointMiscImage(string imageName, bool visible, int index = 0)
	{
		ImageWidget waypointMisc;
		
		if (m_aWaypoints.IsIndexValid(index))
		{
			waypointMisc = ImageWidget.Cast(m_aWaypoints[index].FindAnyWidget("IconExtra"));
			waypointMisc.LoadImageFromSet(0, m_WaypointWidget, imageName);
			waypointMisc.SetVisible(visible);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResumeStage(SCR_ECampaignTutorialArlandStage stage)
	{
		IEntity playerPos;
		typename stages = SCR_ECampaignTutorialArlandStage;
		
		while (!playerPos && stage > 0)
		{
			playerPos = GetGame().GetWorld().FindEntityByName(string.Format("PP_%1", stages.GetVariableName(stage)));
			
			if (playerPos)
			{
				m_eStage = stage;
				return;
			}
			stage--;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ECampaignTutorialArlandStage GetStage()
	{
		return m_eStage;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetStageDuration()
	{
		return GetGame().GetWorld().GetWorldTime() - m_fStageTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	void FinishStage(SCR_BaseCampaignTutorialArlandStage stage, SCR_ECampaignTutorialArlandStage nextStage = -1)
	{	
		SetPlayerStanceToCheck(-1);
		m_iCountOfHits = 0;
		m_bCheckLeaning = false;
		m_eWaypointTruckPosition = SCR_ETutorialArlandSupplyTruckWaypointMode.NONE;

		if (GetStage() == SCR_ECampaignTutorialArlandStage.END_STAGE)
		{
			SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(m_pGameMode);
			SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId: 1);
			gamemode.EndGameMode(endData)
		}
		
		FlushWaypoints();
		stage.FlushWaypoints();
		
		delete stage;
		if (nextStage == -1)
		{
			m_iActiveStage++;
			if(m_aStageInfos.IsIndexValid(m_iActiveStage))
				SetStage(m_aStageInfos[m_iActiveStage].GetIndex());
			else
				SetActiveConfig(0);
		}
		else
			SetStage(nextStage);	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStage(SCR_ECampaignTutorialArlandStage stage, SCR_ECampaignTutorialArlandStage lastStage = 0)
	{	
		m_eStage = stage;
		if(lastStage != 0)
			m_eLastStage = lastStage;
		
#ifdef ENABLE_DIAG
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, m_eStage);
#endif
		
		if (!m_Player)
			return;
		
		// Run the next stage
		if(!m_aStageInfos)
		{
			SetActiveConfig(0);
			return;
		}
		
		foreach (SCR_CampaignTutorialArlandStageInfo stageInfo : m_aStageInfos)
		{
			if (stageInfo.GetIndex() == m_eStage)
			{
				m_fStageTimestamp = GetGame().GetWorld().GetWorldTime();
				m_Stage = SCR_BaseCampaignTutorialArlandStage.Cast(GetGame().SpawnEntity(stageInfo.GetClassName().ToType()));
				break;
			}
			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetPlayerPosition()
	{
		IEntity playerPos;
		typename stages = SCR_ECampaignTutorialArlandStage;
		int stage = m_eStage;
		
		// If the current stage does not have its starting location set, use the previous one
		while (!playerPos && stage >= 0)
		{
			playerPos = GetGame().GetWorld().FindEntityByName(string.Format("PP_%1", stages.GetVariableName(stage)));
			stage--;
		}
		
		if (playerPos)
		{
			if (m_wFadeOut)
			{
				//m_wFadeOut.SetOpacity(1);
				AnimateWidget.Opacity(m_wFadeOut, 1, 4);
			}
			
			GetGame().GetCallqueue().CallLater(MovePlayer, 500, false, playerPos, m_ActiveConfig.GetPlayerLoadout());
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleWaypoints(bool hide)
	{
		foreach(Widget waypoint: m_aWaypoints)
		{
			waypoint.SetVisible(hide);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateWaypoints(array<IEntity> waypoints, float heightOffset)
	{
		int x = 0;
		if (waypoints.IsEmpty())
			return;
		foreach (IEntity waypoint : waypoints)
		{
			if (!waypoint)
				continue;
			vector WPPos = waypoint.GetOrigin();
			ImageWidget markerPos;
			ImageWidget markerExtra;
			RichTextWidget markerDistance;
			WPPos[1] = WPPos[1] + heightOffset;
			vector pos = GetGame().GetWorkspace().ProjWorldToScreen(WPPos, GetGame().GetWorld());
			int dist = vector.Distance(m_Player.GetOrigin(), WPPos);
			
			// Handle off-screen coords
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			int winX = workspace.GetWidth();
			int winY = workspace.GetHeight();
			int posX = workspace.DPIScale(pos[0]);
			int posY = workspace.DPIScale(pos[1]);
			
			if (posX < 0)
				pos[0] = 0;
			else if (posX > winX)
				pos[0] = workspace.DPIUnscale(winX);
			
			if (posY < 0)
				pos[1] = 0;
			else if (posY > winY || pos[2] < 0)
				pos[1] = workspace.DPIUnscale(winY);
			
			FrameSlot.SetPos(m_aWaypoints[x], pos[0], pos[1]);
			
			int distShown = dist;
			
			if (dist > 1000)
			{
				distShown = dist - (dist % 1000);
			}
			else if (dist > 100)
			{
				distShown = dist - (dist % 100);
			}
			else if (dist > 50)
			{
				distShown = dist - (dist % 50);
			}
			
			
			
			float opacity = 1;
			float distanceOpacity = 1;
			
			if (dist < WAYPOINT_FADE_THRESHOLD)
				opacity = Math.Lerp(WAYPOINT_MINIMUM_OPACITY, 1, dist / WAYPOINT_FADE_THRESHOLD);
			
			if (dist >= WAYPOINT_DISTANCE_INDICATOR_FADE_START)
			{
				distanceOpacity = 1;
			}
			else if (dist <= WAYPOINT_DISTANCE_INDICATOR_FADE_END)
			{
				distanceOpacity = 0;
			}
			else
			{
				distanceOpacity = Math.InverseLerp(WAYPOINT_DISTANCE_INDICATOR_FADE_END, WAYPOINT_DISTANCE_INDICATOR_FADE_START, dist);
			}
			
			markerPos = ImageWidget.Cast(m_aWaypoints[x].FindAnyWidget("Icon"));
			markerExtra = ImageWidget.Cast(m_aWaypoints[x].FindAnyWidget("IconExtra"));
			markerDistance = RichTextWidget.Cast(m_aWaypoints[x].FindAnyWidget("Distance"));
			
			markerPos.SetOpacity(opacity);
			markerExtra.SetOpacity(opacity);
			markerDistance.SetOpacity(distanceOpacity);
			markerDistance.SetTextFormat("#AR-Tutorial_WaypointUnits_meters", distShown);
			x++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignSuppliesComponent GetSupplyTruckComponent()
	{
		return m_SupplyTruckComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	bool FindSupplyTruck(IEntity ent)
	{
		SCR_CampaignSuppliesComponent supplyTruckComponent = SCR_CampaignSuppliesComponent.Cast(ent.FindComponent(SCR_CampaignSuppliesComponent));
		
		if (!supplyTruckComponent)
			return true;
		
		Vehicle truck = Vehicle.Cast(ent.GetParent());
		
		if (!truck)
			return true;
		
		m_SupplyTruckComponent = supplyTruckComponent;
		truck.SetName("BuildingSupplyTruck");
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	protected void SetupStageCounts(int stage = 0)
	{
		array<ref SCR_CampaignTutorialArlandStageInfo> stagelist = {};
				
		m_ActiveConfig.GetStages(stagelist);
		m_iStagesCnt = stagelist.Count();
			
#ifdef ENABLE_DIAG
		if (m_eStage > m_iStagesCnt - 1)
		{
			int diff = m_eStage - m_iStagesCnt - 1;
			int numStage = m_eStage - diff;		
		}		
		
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, "", "Stage", "Tutorial", string.Format("0 %1 0 1", m_iStagesCnt - 1));
#endif			
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_CampaignGamemode = SCR_GameModeCampaign.GetInstance();
		
		if (!m_CampaignGamemode)
			return;
		
		m_CampaignGamemode.SetIsTutorial(true);
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		ChimeraWorld world = GetGame().GetWorld();
		
		if (world)
		{
			MusicManager musicMan = world.GetMusicManager();
			
			if (musicMan)
			{
				array<int> musicCategories = {};
				SCR_Enum.GetEnumValues(MusicCategory, musicCategories);
				
				foreach (int category : musicCategories)
				{
					musicMan.MuteCategory(category, true, true);
				}
			}
		}
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_TUTORIAL_MENU, "Tutorial", "");
		DiagMenu.RegisterItem(SCR_DebugMenuID.DEBUGUI_TUTORIAL_CONFIG, "", "Config", "Tutorial", "Hub,Movement,Weapons,ConflictSimple,ConflictSige,Driving");
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, "", "Stage", "Tutorial", string.Format("0 %1 0 1", m_iStagesCnt - 1));
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE, m_eStage);
#endif
		
		Resource holder = BaseContainerTools.LoadContainer(m_sStagesConfigAdvanced);
		
		if (!holder)
			return;
		
		BaseContainer container = holder.GetResource().ToBaseContainer();
		
		if (!container)
			return;
		
		SCR_CampaignTutorialArlandStagesConfig configsConf = SCR_CampaignTutorialArlandStagesConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		
		if (!configsConf)
			return;
		
		configsConf.GetConfigs(m_aStageConfigs);
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		core.Event_OnEditorManagerCreatedServer.Insert(SetupEditorModeListener);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{		
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE) != m_eStage && m_Stage)
		{
			FinishStage(m_Stage, DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_TUTORIAL_STAGE));
			ResetStage();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CleanUpWaypoints()
	{
		if (m_aWaypoints.IsEmpty())
			return;
		foreach (Widget waypoint : m_aWaypoints)
		{
			if (!waypoint)
				continue;
			waypoint.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTutorialArlandComponent()
	{	
		if (m_wFadeOut)
			m_wFadeOut.RemoveFromHierarchy();
		
		CleanUpWaypoints();
		
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_CampaignTutorialArlandStagesConfig
{
	[Attribute()]
	private ref array<ref SCR_CampaignTutorialArlandStages> m_TutorialArlandStagesConfigs;
	
	//------------------------------------------------------------------------------------------------
	void GetConfigs(out notnull array<ref SCR_CampaignTutorialArlandStages> TutorialArlandStagesConfigs)
	{
		TutorialArlandStagesConfigs = m_TutorialArlandStagesConfigs;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CampaignTutorialArlandStages
{
	[Attribute()]
	private ref array<ref SCR_CampaignTutorialArlandStageInfo> m_TutorialArlandStages;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETutorialArlandStageMasters))]
	protected SCR_ETutorialArlandStageMasters m_ConfigClassName;
	
	[Attribute(defvalue :"{D54A817EA5E7184F}Prefabs/Characters/Campaign/Final/Campaign_US_Player_Tutorial.et", UIWidgets.ResourceNamePicker, "Player loadout for specified stage", "et")]
	protected ResourceName m_sPlayerLoadoutName;
	
	//------------------------------------------------------------------------------------------------
	SCR_ETutorialArlandStageMasters GetConfigClassName()
	{
		return m_ConfigClassName;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetStages(out notnull array<ref SCR_CampaignTutorialArlandStageInfo> TutorialArlandStages)
	{
		TutorialArlandStages = m_TutorialArlandStages;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ETutorialArlandStageMasters GetStagesFromConfig()
	{
		return m_ConfigClassName;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetPlayerLoadout()
	{
		return m_sPlayerLoadoutName;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sStageClassName")]
class SCR_CampaignTutorialArlandStageInfo
{
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sStageClassName;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignTutorialArlandStage))]
	protected SCR_ECampaignTutorialArlandStage m_eStage;
	
	//------------------------------------------------------------------------------------------------
	SCR_ECampaignTutorialArlandStage GetIndex()
	{
		return m_eStage;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetClassName()
	{
		return m_sStageClassName;
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_ETutorialArlandSupplyTruckWaypointMode
{
	NONE,
	DRIVER,
	BACK
}

//------------------------------------------------------------------------------------------------
enum SCR_ETutorialArlandStageMasters
{
	HUB,
	BASIC_TUTORIAL_SIMPLE,
	WEAPONS_TUTORIAL_SIMPLE,
	CONFLICT_TUTORIAL_BASE_BUILDING,
	CONFLICT_TUTORIAL_BESIEGING,
	CONFLICT_TUTORIAL_BASE_TOUR,
	MEDICAL,
	HELICOPTERS_SIMPLE,
	HELICOPTERS_ADVANCED,
	DRIVING_SIMPLE,
	DRIVING_ADVANCED,
	WEAPONS_ARSENAL,
	NAVIGATION,
	HELICOPTERS,
	FALLBACKS,
}
	

//------------------------------------------------------------------------------------------------
enum SCR_ECampaignTutorialArlandStage
{
	// HUB
	START = 0,
	RESTART,
	
	// MOVEMENT
	ZIGZAG_1,
	ZIGZAG_2,
	ZIGZAG_3,
	OVERPASS,
	WIREMESH,
	VAULT,
	VAULT2,
	WINDOW,
	FLAG,
	WALL,
	WALL2,
	LADDER_UP,
	LADDER_PLATFORM,
	LADDER_DOWN,
	LADDER_OFF,
	OBSTACLE_END,
	
	// SHOOTING RANGE
	WEAPON_PICK,
	MAGAZINE_PICK,
	WEAPON_RELOAD,
	FIREPOZ_1,
	SHOOTING,
	SHOOTING_CROUCH,
	SHOOTING_PRONE,
	FIREPOZ_2,
	SHOOTING_LEANING,
	GRENADE_PICK,
	GRENADE_THROW,
	WEAPON_REST,
	WEAPON_BIPOD_PICK,
	WEAPON_BIPOD,
	MINE_PICK,
	MINE_PLANT,
	MINE_DETONATE,
	STATIC_GETIN,
	STATIC_SHOOTING,
	BOARDING,
	DRIVING_0,
	SWITCHING_SEATS,
	VEHICLE_SHOOTING,
	MOVE_TO_BALISTIC_PROTECTION,
	BALLISTIC_PROTECTION,
	WEAPON_PICK_SNIPER,
	SNIPER_SHOOT_CLOSER,
	SNIPER_SHOOT_FAR,
	FIREPOZ_3,
	SHOOTING_END,
	
	
	DRIVING_1,
	DRIVING_2,
	DRIVING_3,
	DRIVING_4,
	DRIVING_5,
	DRIVING_6,
	DRIVING_7,
	DRIVING_SERPENTINE,
	DRIVING_8,
	DRIVING_PRE_BASE,
	DRIVING_9,
	DISMOUNTING,
	MAP_INFO,
	CONFLICT_TOUR_GARAGE,
	CONFLICT_TOUR_END,
	CONFLICT_REQUESTING_TRUCK,
	CONFLICT_LOADING_SUPPLIES,
	DRIVING_10,
	DRIVING_11,
	DRIVING_12,
	DRIVING_13,
	DRIVING_14,
	DRIVING_15,
	CONFLICT_UNLOADING_SUPPLIES,
	CONFLICT_BUILD_SERVICE,
	CONFLICT_BUILDING_EXIT,
	CONFLICT_EQUIP_ENTRENCHING_TOOL,
	CONFLICT_CONSTRUCT_SERVICE,
	CONFLICT_BUILD_BUNKER,
	CONFLICT_MOVE_TO_VEHICLE_DEPOT,
	CONFLICT_BUILDING_LOAD_SUPPLY,
	CONFLICT_BOARD_TRUCK,
	CONFLICT_BUILDING_QUIT,
	CONFLICT_BUILDING_TO_FAR,
	CONFLICT_BUILDING_CHECKPOINT,
	CONFLICT_BUILDING_QUIT2,
	DRIVING_16,
	DRIVING_17,
	DRIVING_18,
	CONFLICT_BASE_SEIZING,
	RETURN_TO_RESPAWN,
	BREAKER_STAGE,
	END_STAGE,
	DRIVING_INTRODUCTION,
	DRIVING_SPAWNING,
	DRIVING_REQUESTING,
	DRIVING_TRAINING_0,
	DRIVING_TRAINING_1, 
	DRIVING_TRAINING_2,
	DRIVE,
	GETIN,
	CAROVERVIEW,
	DRIVING_19,
	DRIVING_20,
	DRIVING_21,
	DRIVING_22,
	DRIVING_23,
	DRIVING_24,
	DRIVING_25,
	DRIVING_26,
	DRIVING_27,
	DRIVING_28,
	DRIVEBASIC,
	GETIN2,
	CAROVERVIEW2,
	DRIVING_31,
	DRIVING_32,
	DRIVING_33,
	DRIVING_34,
	DRIVING_35,
	DRIVING_36,
	DRIVING_37,
	DRIVING_38,
	DRIVING_39,
	DRIVING_40,
	DRIVING_41,
	DRIVING_42,
	DRIVING_43,
	DRIVING_44,
	DRIVING_45,
	DRIVING_46,
	DRIVING_47,
	DRIVING_48,
	DRIVING_49,
	DRIVING_50,
	DRIVING_51,
	DRIVING_52,
	DRIVING_53,
	
	// MEDICAL
	MEDICAL_START,
	MEDICAL_GETIN_AMBULANCE,
	MEDICAL_DRIVE_1,
	MEDICAL_DRIVE_2,
	MEDICAL_DRIVE_3,
	MEDICAL_DRIVE_4,
	MEDICAL_PULLOUT_PATIENT,
	MEDICAL_SHOW_PATIENT_STATUS,
	MEDICAL_SHOW_PATIENT_STATUS_2,
	MEDICAL_EQUIP_TOURNIQUET,
	MEDICAL_USE_TOURNIQUET,
	MEDICAL_EQUIP_BANDAGE,
	MEDICAL_USE_BANDAGE_CHEST,
	MEDICAL_USE_BANDAGE_LEG,
	MEDICAL_EQUIP_SALINE,
	MEDICAL_USE_SALINE,
	MEDICAL_EQUIP_MORPHINE,
	MEDICAL_USE_MORPHINE,
	MEDICAL_END,
	
	// CONFLICT CAPTURING
	CONFLICT_CAPTURING_START,
	CONFLICT_CAPTURING_OPEN_MAP,
	CONFLICT_TASKS_MENU,
	CONFLICT_TASKS_INFO,
	CONFLICT_VOLUNTEERING,
	CONFLICT_VOLUNTEERING_INFO,
	CONFLICT_CAPTURING_MOVE_1,
	CONFLICT_BASE_NEARBY,
	CONFLICT_BASE_SEIZING_INFO,
	CONFLICT_SHOW_COMPAS,
	CONFLICT_COMPAS_DIRECTION_NORTH,
	CONFLICT_COMPAS_DIRECTION_2,
	CONFLICT_COMPAS_MOVE,
	CONFLICT_MAP_MOVE,
	CONFLICT_RADIO_PICKUP,
	CONFLICT_VOIP_SETUP,
	CONFLICT_VOIP_USE,
	CONFLICT_VOIP_INFO,
	CONFLICT_MHQ_DEPLOY,
	CONFLICT_MHQ_INFO,
	CONFLICT_SEIZE_ENEMY_HQ,
	CONFLICT_CAPTURING_END,

	// NAVIGATION
	NAVIGATION_INTRODUCTION,
	NAVIGATION_OPEN_MAP,
	NAVIGATION_MAP_INTRODUCTION,
	NAVIGATION_MAP_GRIDS,
	NAVIGATION_MAP_TOOLBAR,
	NAVIGATION_MAP_TOOLBAR_COMPASS,
	NAVIGATION_MAP_TOOLBAR_RULER,
	NAVIGATION_MAP_TOOLBAR_WATCH,
	NAVIGATION_MAP_CLOSE,
	NAVIGATION_COMPASS_EQUIP,
	NAVIGATION_COMPASS_FIND_NORTH,
	NAVIGATION_COMPASS_FIND_LIGHTHOUSE,
	NAVIGATION_COMPASS_FIND_CHURCH,
	NAVIGATION_COMPASS_FIND_VILLAGE,
	NAVIGATION_COMPASS_FIND_HILL_TOWER,
	NAVIGATION_OPEN_MAP_2,
	NAVIGATION_CREATING_MARKER,
	NAVITATION_CREATING_MARKER2,
	NAVIGATION_MOVE,
	NAVIGATION_END,
	
	// NAVIGATION FALLBACKS
	NAVIGATION_MAP_CLOSED_FALLBACK,
	NAVIGATION_MARKER_HELPER,
	
	// HELICOURSE
	HELI_START,
	HELI_GETIN,
	HELI_START_ENGINE,
	HELI_GAINING_RPM,
	HELI_LIFT_OFF,
	HELI_FLIGHT_1,
	HELI_FLIGHT_2,
	HELI_FLIGHT_3,
	HELI_FLIGHT_AUTOHOOVER,
	HELI_FLIGHT_AUTOHOOVER_DESCRIPTION,
	HELI_FLIGHT_LAND,
	HELI_TURN_OFF_ENGINE,
	HELI_GETOUT,
	HELI_END,
	
	//
	DRIVING_HEAVY_START,
	DRIVING_HEAVY_1,
	DRIVING_HEAVY_2,
	DRIVING_HEAVY_3,
	DRIVING_HEAVY_4,
	DRIVING_HEAVY_5,
	DRIVING_HEAVY_6,
	DRIVING_HEAVY_7,
	DRIVING_HEAVY_8,
	DRIVING_HEAVY_9,
	DRIVING_HEAVY_10,
	DRIVING_HEAVY_11,
	DRIVING_HEAVY_GETIN,
	DRIVING_HEAVY_12,
	DRIVING_HEAVY_13,
	DRIVING_HEAVY_14,
	DRIVING_HEAVY_15,
	DRIVING_HEAVY_16,
	DRIVING_HEAVY_17,
	DRIVING_HEAVY_18,
	DRIVING_HEAVY_GETOUT,
	DRIVING_HEAVY_REPAIR,
	DRIVING_HEAVY_END,

	// CONFLICT BASE TOUR 
	CONFLICT_BASE_TOUR_START,
	
	// CONFLICT BASEBUILDING 
	CONFLICT_BUILDING_START,
	CONFLICT_BUILDING_ENTER_BUILDING,
	CONFLICT_BUILDING_PLACE_SERVICE,
	CONFLICT_BUILDING_EXIT_BUILDING,
	CONFLICT_BUILDING_EQUIP_TOOL,
	CONFLICT_BUILDING_CONSTRUCT_SERVICE,
	CONFLICT_BUILDING_INFO_LOW_SUPPLIES,
	CONFLICT_BUILDING_GET_INTO_TRUCK,
	CONFLICT_BUILDING_MOVE_TO_FIA_DEPOT,
	CONFLICT_BUILDING_EXIT_TRUCK,
	CONFLICT_BUILDING_LOAD_SUPPLIES,
	CONFLICT_BUILDING_EQUIP_TOOL2,
	CONFLICT_BUILDING_DECONSTRUCT,
	CONFLICT_BUILDING_GET_INTO_TRUCK2,
	CONFLICT_BUILDING_MOVE_TO_START,
	CONFLICT_BUILDING_EXIT_TRUCK2,
	CONFLICT_BUILDING_UNLOAD_SUPPLIES,
	CONFLICT_BUILDING_END,
};

//------------------------------------------------------------------------------------------------
enum SCR_ECampaignTutorialArlandConfig
{
	
}
