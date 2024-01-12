#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
class SCR_CampaignMilitaryBaseComponentClass : SCR_MilitaryBaseComponentClass
{
	[Attribute("{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset", UIWidgets.ResourceNamePicker, "Imageset with foreground textures", "imageset", category: "Campaign")]
	ResourceName m_sBuildingIconImageset;

	[Attribute("{C35F29E48086221A}Configs/Campaign/CampaignGraphLinesConfig.conf", UIWidgets.ResourceNamePicker, "", "conf", category: "Campaign")]
	protected ref SCR_GraphLinesData m_GraphLinesData;

	[Attribute("3", category: "Campaign")]
	protected float m_fLineWidth;

	//------------------------------------------------------------------------------------------------
	ResourceName GetBuildingIconImageset()
	{
		return m_sBuildingIconImageset;
	}

	//------------------------------------------------------------------------------------------------
	SCR_GraphLinesData GetGraphLinesData()
	{
		return m_GraphLinesData;
	}

	//------------------------------------------------------------------------------------------------
	float GetLineWidth()
	{
		return m_fLineWidth;
	}
}

void OnSpawnPointAssignedDelegate(SCR_SpawnPoint spawnpoint);
typedef func OnSpawnPointAssignedDelegate;
typedef ScriptInvokerBase<OnSpawnPointAssignedDelegate> OnSpawnPointAssignedInvoker;

//------------------------------------------------------------------------------------------------
class SCR_CampaignMilitaryBaseComponent : SCR_MilitaryBaseComponent
{
	[Attribute("0", category: "Campaign"), RplProp()]
	protected bool m_bIsControlPoint;

	[Attribute("0", desc: "Can this base be picked as a faction's main base?", category: "Campaign")]
	protected bool m_bCanBeHQ;

	[Attribute("1", category: "Campaign")]
	protected bool m_bDisableWhenUnusedAsHQ;
	
	[Attribute("0", desc: "If enabled, this base will be treated as an HQ when handling supply income.", category: "Campaign")]
	protected bool m_bIsSupplyHub;

	[Attribute("#AR-MapSymbol_Installation", desc: "The display name of this base.", category: "Campaign")]
	protected string m_sBaseName;

	[Attribute("#AR-MapSymbol_Installation", desc: "The display name of this base, in upper case.", category: "Campaign")]
	protected string m_sBaseNameUpper;

	[Attribute("", desc: "Name of associated map location (to hide its label)", category: "Campaign")]
	protected string m_sMapLocationName;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(SCR_ECampaignBaseType), category: "Campaign")]
	protected SCR_ECampaignBaseType m_eType;

	[Attribute("32", UIWidgets.Slider, "Radio frequency (MHz) for players operating at this base (US)", "32 68 4.0", category: "Campaign")]
	protected int m_iFreqWest;

	[Attribute("38", UIWidgets.Slider, "Radio frequency (MHz) for players operating at this base (USSR)", "38 54 2", category: "Campaign")]
	protected int m_iFreqEast;

	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected ref map<int, int> m_mDefendersData = new map<int, int>();
	#else
	protected ref map<int, WorldTimestamp> m_mDefendersData = new map<int, WorldTimestamp>();
	#endif

	static const float RADIO_RECONFIGURATION_DURATION = 20.0;
	static const float BARRACKS_REDUCED_DEPLOY_COST = 0.5;
	static const int UNDER_ATTACK_WARNING_PERIOD = 60;
	static const int INVALID_PLAYER_INDEX = -1;
	static const int INVALID_FACTION_INDEX = -1;
	static const int INVALID_FREQUENCY = -1;
	static const int RESPAWN_DELAY_AFTER_CAPTURE = 180000;
	static const int RELAY_BASE_RADIUS = 100;

	static const int SUPPLY_DEPOT_CAPACITY = 4500;
	static const int DEFENDERS_CHECK_PERIOD = 30000;
	static const int DEFENDERS_REWARD_PERIOD = 120000;
	static const int DEFENDERS_REWARD_MULTIPLIER = 3;

	protected static const int TICK_TIME = 1;
	protected static const float HQ_VEHICLE_SPAWN_RADIUS = 40;
	protected static const float HQ_VEHICLE_QUERY_SPACE = 6;
	protected static const string RADIO_CHATTER_SIGNAL_NAME = "RadioChatter";
	protected static const string ESTABLISH_ACTION_SIGNAL_NAME = "EstablishAction";

	protected ref OnSpawnPointAssignedInvoker m_OnSpawnPointAssigned;

	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aBasesInRadioRange = {};
	protected ref array<SCR_AmbientPatrolSpawnPointComponent> m_aRemnants = {};
	protected ref array<IEntity> m_aStartingVehicles = {};

	protected float m_fStartingSupplies;
	protected float m_fSuppliesArrivalTime = float.MAX;
	protected float m_fNextFrameCheck;
	protected float m_fTimer;
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fLastEnemyContactTimestamp;
	#else
	protected WorldTimestamp m_fLastEnemyContactTimestamp;
	#endif

	protected bool m_bLocalPlayerPresent;

	protected SCR_CampaignMilitaryBaseMapDescriptorComponent m_MapDescriptor;

	protected SCR_SpawnPoint m_SpawnPoint;

	protected SCR_CampaignFaction m_CapturingFaction;
	protected SCR_CampaignFaction m_OwningFactionPrevious;

	protected BaseRadioComponent m_RadioComponent;

	protected IEntity m_HQRadio;
	protected IEntity m_HQTent;

	protected SCR_CampaignMapUIBase m_UIElement;

	protected SCR_TimedWaypoint m_SeekDestroyWP;

	protected SCR_SmartActionWaypoint m_RetakeWP;

	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;

	protected SCR_CampaignBarracksComponent m_BarrackComponent;

	protected SCR_ArmoryComponent m_ArmoryComponent;
	protected SCR_ArmoryComponent m_RadioArmory;

	protected SCR_AIGroup m_DefendersGroup;

	[RplProp(onRplName: "OnHQSet")]
	protected bool m_bIsHQ;

	[RplProp(onRplName: "OnInitialized")]
	protected bool m_bInitialized;

	[RplProp(onRplName: "OnHasSignalChanged")]
	protected SCR_ECampaignHQRadioComms m_eRadioCoverageBLUFOR = SCR_ECampaignHQRadioComms.NONE;

	[RplProp(onRplName: "OnHasSignalChanged")]
	protected SCR_ECampaignHQRadioComms m_eRadioCoverageOPFOR = SCR_ECampaignHQRadioComms.NONE;

	[RplProp(onRplName: "OnCapturingFactionChanged")]
	protected int m_iCapturingFaction = INVALID_FACTION_INDEX;

	[RplProp()]
	protected int m_iReconfiguredBy = INVALID_PLAYER_INDEX;

	[RplProp(onRplName: "OnRadioRangeChanged")]
	protected float m_fRadioRange;

	[RplProp(onRplName: "OnRespawnCooldownChanged")]
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fRespawnAvailableSince;
	#else
	protected WorldTimestamp m_fRespawnAvailableSince;
	#endif

	[RplProp(onRplName: "OnAttackingFactionChanged")]
	protected int m_iAttackingFaction = -1;

	//------------------------------------------------------------------------------------------------
	bool IsControlPoint()
	{
		return m_bIsControlPoint;
	}

	//------------------------------------------------------------------------------------------------
	bool CanBeHQ()
	{
		return m_bCanBeHQ;
	}

	//------------------------------------------------------------------------------------------------
	bool DisableWhenUnusedAsHQ()
	{
		return m_bDisableWhenUnusedAsHQ;
	}

	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetRespawnTimestamp()
	#else
	WorldTimestamp GetRespawnTimestamp()
	#endif
	{
		return m_fRespawnAvailableSince;
	}

	//------------------------------------------------------------------------------------------------
	void Initialize()
	{
		if (IsProxy())
			return;

		m_bInitialized = true;
		Replication.BumpMe();
		OnInitialized();
	}

	//------------------------------------------------------------------------------------------------
	void Disable()
	{
		if (IsProxy())
			return;

		GetGame().GetCallqueue().Remove(SpawnBuilding);
		GetGame().GetCallqueue().Remove(SpawnStartingVehicles);
		GetGame().GetCallqueue().Remove(EvaluateDefenders);
		GetGame().GetCallqueue().Remove(SupplyIncomeTimer);
		//GetGame().GetCallqueue().Remove(SpawnSavedBuildings);
		DeleteStartingVehicles();

		m_FactionComponent.SetAffiliatedFaction(null);

		if (m_HQTent)
			RplComponent.DeleteRplEntity(m_HQTent, false);

		m_bInitialized = false;
		Replication.BumpMe();
		OnDisabled();
	}

	//------------------------------------------------------------------------------------------------
	void OnInitialized()
	{
		if (!m_bInitialized)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		// Register default base spawnpoint
		SCR_SpawnPoint spawnpoint;
		IEntity child = GetOwner().GetChildren();

		while (child)
		{
			spawnpoint = SCR_SpawnPoint.Cast(child);

			if (spawnpoint)
			{
				m_SpawnPoint = spawnpoint;

				SCR_CampaignBuildingProviderComponent buildingProvider = SCR_CampaignBuildingProviderComponent.Cast(GetOwner().FindComponent(SCR_CampaignBuildingProviderComponent));
				if (buildingProvider)
					m_SpawnPoint.SetSpawnPositionRange(buildingProvider.GetBuildingRadius());
				else
					m_SpawnPoint.SetSpawnPositionRange(m_iRadius);

				if (m_OnSpawnPointAssigned)
					m_OnSpawnPointAssigned.Invoke(spawnpoint);

				break;
			}

			child = child.GetSibling();
		}

		// Initialize registered services
		array<SCR_ServicePointComponent> services = {};
		GetServices(services);

		foreach (SCR_ServicePointComponent service : services)
		{
			OnServiceBuilt(service);
		}

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			ConnectToCampaignBasesSystem();

			Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

			if (!playerFaction)
				playerFaction = campaign.GetBaseManager().GetLocalPlayerFaction();

			if (playerFaction)
				OnLocalPlayerFactionAssigned(playerFaction);
			else
				campaign.GetOnFactionAssignedLocalPlayer().Insert(OnLocalPlayerFactionAssigned);
		}

		campaign.GetBaseManager().GetOnAllBasesInitialized().Insert(OnAllBasesInitialized);

		if (IsProxy())
		{
			campaign.GetBaseManager().AddActiveBase();
			return;
		}
		else if (!m_bIsHQ && SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent)))
		{
			Math.Randomize(-1);
			GetGame().GetCallqueue().CallLater(EvaluateDefenders, DEFENDERS_CHECK_PERIOD + Math.RandomIntInclusive(0, DEFENDERS_CHECK_PERIOD * 0.1), true);
		}

		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(GetOwner().FindComponent(SCR_CampaignSeizingComponent));

		if (seizingComponent)
		{
			seizingComponent.GetOnCaptureStart().Insert(OnCaptureStart);
			seizingComponent.GetOnCaptureInterrupt().Insert(EndCapture);
		}

		if (IsProxy())
			return;
		
		if (!campaign.IsTutorial())
			GetGame().GetCallqueue().CallLater(SupplyIncomeTimer, SCR_GameModeCampaign.UI_UPDATE_DELAY, true, false);
	}

	//------------------------------------------------------------------------------------------------
	void ApplyHeaderSettings(notnull SCR_CampaignCustomBase settings)
	{
		m_bCanBeHQ = settings.GetCanBeHQ();
		m_bDisableWhenUnusedAsHQ = settings.GetDisableWhenUnusedAsHQ();
		m_bIsControlPoint = settings.IsControlPoint();

		float radioRange = settings.GetRadioRange();

		if (radioRange < 0)
			return;

		BaseRadioComponent radio = BaseRadioComponent.Cast(GetOwner().FindComponent(BaseRadioComponent));

		if (!radio)
			return;

		BaseTransceiver transceiver = radio.GetTransceiver(0);

		if (!transceiver)
			return;

		transceiver.SetRange(radioRange);
		RecalculateRadioRange();
	}

	//------------------------------------------------------------------------------------------------
	override void OnServiceStateChanged(SCR_EServicePointStatus state, notnull SCR_ServicePointComponent serviceComponent)
	{
		switch (state)
		{
			case SCR_EServicePointStatus.UNDER_CONSTRUCTION:
			{
				return;
			}

			case SCR_EServicePointStatus.ONLINE:
			{
				OnServiceBuilt(serviceComponent);

				SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
				if (!campaign)
					return;

				SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
				if (!baseManager)
					return;

				baseManager.OnServiceBuilt(state, serviceComponent);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnServiceBuilt(notnull SCR_ServicePointComponent service)
	{
		bool duringInit = (GetGame().GetWorld().GetWorldTime() <= 0);

		// Delayed call so the composition has time to properly load and register its entire hierarchy on clients as well
		GetGame().GetCallqueue().CallLater(OnServiceBuilt_AfterInit, 500, false, service);

		EEditableEntityLabel serviceType = service.GetLabel();
		SCR_ERadioMsg radio = SCR_ERadioMsg.NONE;
		SCR_CampaignFaction owner = GetCampaignFaction();

		switch (serviceType)
		{
			case EEditableEntityLabel.SERVICE_SUPPLY_STORAGE:
			{
				radio = SCR_ERadioMsg.BUILT_SUPPLY;
				break;
			}

			case EEditableEntityLabel.SERVICE_FUEL:
			{
				radio = SCR_ERadioMsg.BUILT_FUEL;
				break;
			}

			case EEditableEntityLabel.SERVICE_ARMORY:
			{
				radio = SCR_ERadioMsg.BUILT_ARMORY;

				if (!IsProxy())
					m_ArmoryComponent = SCR_ArmoryComponent.Cast(service.GetOwner().FindComponent(SCR_ArmoryComponent));

				break;
			}

			case EEditableEntityLabel.SERVICE_VEHICLE_DEPOT_LIGHT:
			{
				radio = SCR_ERadioMsg.BUILT_VEHICLES_LIGHT;
				break;
			}

			case EEditableEntityLabel.SERVICE_VEHICLE_DEPOT_HEAVY:
			{
				radio = SCR_ERadioMsg.BUILT_VEHICLES_HEAVY;
				break;
			}

			case EEditableEntityLabel.SERVICE_ANTENNA:
			{
				radio = SCR_ERadioMsg.BUILT_ANTENNA;

				if (!IsProxy())
				{
					m_RadioArmory = SCR_ArmoryComponent.Cast(service.GetOwner().FindComponent(SCR_ArmoryComponent));
					RecalculateRadioRange();
				}

				break;
			}

			case EEditableEntityLabel.SERVICE_LIVING_AREA:
			{
				radio = SCR_ERadioMsg.BUILT_BARRACKS;

				if (!IsProxy())
				{
					SCR_CampaignBarracksComponent barracks = SCR_CampaignBarracksComponent.Cast(service);

					if (barracks)
					{
						AssignBarracks(barracks);
						barracks.SetBase(this);
					}
				}

				SCR_DefenderSpawnerComponent defenderComp = SCR_DefenderSpawnerComponent.Cast(service);

				if (defenderComp)
					defenderComp.AssignSupplyComponent(m_SuppliesComponent);

				break;
			}

			case EEditableEntityLabel.SERVICE_FIELD_HOSPITAL:
			{
				radio = SCR_ERadioMsg.BUILT_FIELD_HOSPITAL;
				break;
			}

			case EEditableEntityLabel.SERVICE_HELIPAD:
			{
				radio = SCR_ERadioMsg.BUILT_HELIPAD;
				break;
			}
		}

		if (IsProxy())
			return;

		SCR_CampaignBuildingCompositionComponent buildingComponent = SCR_CampaignBuildingCompositionComponent.Cast(SCR_EntityHelper.GetMainParent(service.GetOwner(), true).FindComponent(SCR_CampaignBuildingCompositionComponent));

		if (buildingComponent && !buildingComponent.GetProviderEntity())
			buildingComponent.SetProviderEntityServer(GetOwner());

		if (owner && IsHQRadioTrafficPossible(GetFaction(), SCR_ECampaignHQRadioComms.BOTH_WAYS) && !duringInit)
			owner.SendHQMessage(radio, m_iCallsign);
	}

	//------------------------------------------------------------------------------------------------
	void OnServiceRemoved(notnull SCR_ServicePointComponent service)
	{
		if (service.GetServiceState() != SCR_EServicePointStatus.ONLINE)
			return;

		if (service.GetType() == SCR_EServicePointType.RADIO_ANTENNA)
		{
			if (!IsProxy())
			{
				m_RadioArmory = null;
				RecalculateRadioRange();
			}

		}
	}

	//------------------------------------------------------------------------------------------------
	void OnServiceBuilt_AfterInit(SCR_ServicePointComponent service)
	{
		// In case the composition has been deleted in the meantime
		if (!service)
			return;

		if (!IsProxy())
		{
			ResourceName delegatePrefab = service.GetDelegatePrefab();
	
			if (delegatePrefab.IsEmpty())
				return;
	
			Resource delegateResource = Resource.Load(delegatePrefab);
	
			if (!delegateResource || !delegateResource.IsValid())
				return;
	
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			service.GetOwner().GetTransform(params.Transform);
			IEntity delegateEntity = GetGame().SpawnEntityPrefab(delegateResource, null, params);
	
			if (!delegateEntity)
				return;
	
			SCR_ServicePointDelegateComponent delegateComponent = SCR_ServicePointDelegateComponent.Cast(delegateEntity.FindComponent(SCR_ServicePointDelegateComponent));
	
			if (!delegateComponent)
				return;
	
			service.SetDelegate(delegateComponent);
			delegateComponent.SetParentBaseId(Replication.FindId(this));
		}

		if (RplSession.Mode() != RplMode.Dedicated)
			m_MapDescriptor.HandleMapInfo();
	}

	//------------------------------------------------------------------------------------------------
	void OnDisabled()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		if (m_SpawnPoint)
		{
			m_SpawnPoint.SetFactionKey(string.Empty);
			m_SpawnPoint = null;
		}

		if (m_MapDescriptor)
			m_MapDescriptor.Item().SetVisible(false);

		if (m_UIElement)
			m_UIElement.SetVisible(false);

		campaign.GetOnFactionAssignedLocalPlayer().Remove(OnLocalPlayerFactionAssigned);

		if (RplSession.Mode() != RplMode.Dedicated)
			DisconnectFromCampaignBasesSystem();

		campaign.GetBaseManager().GetOnAllBasesInitialized().Remove(OnAllBasesInitialized);

		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(GetOwner().FindComponent(SCR_CampaignSeizingComponent));

		if (seizingComponent)
		{
			seizingComponent.GetOnCaptureStart().Remove(OnCaptureStart);
			seizingComponent.GetOnCaptureInterrupt().Remove(EndCapture);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsInitialized()
	{
		return m_bInitialized;
	}

	//------------------------------------------------------------------------------------------------
	void SetAsHQ(bool isHQ)
	{
		if (IsProxy())
			return;

		m_bIsHQ = isHQ;
		SCR_CampaignFaction faction = GetCampaignFaction();

		if (m_bIsHQ)
		{
			SCR_CampaignMilitaryBaseComponent previousHQ = faction.GetMainBase();
			SCR_CampaignMilitaryBaseComponent enemyHQ = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetEnemyFaction(faction).GetMainBase();

			if (previousHQ && previousHQ != this && previousHQ != enemyHQ)
			{
				if (previousHQ.GetDisableWhenUnusedAsHQ())
					previousHQ.Disable();
			}
		}

		Replication.BumpMe();
		OnHQSet();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHQSet()
	{
		SCR_CampaignFaction owner = SCR_CampaignFaction.Cast(GetFaction());

		if (owner && m_bIsHQ)
			owner.SetMainBase(this);

		if (IsProxy())
			return;

		HandleSpawnPointFaction();

		GetGame().GetCallqueue().Remove(SpawnStartingVehicles);

		if (m_bIsHQ)
		{
			SupplyIncomeTimer(true);
			GetGame().GetCallqueue().CallLater(SpawnStartingVehicles, 1500, false);		// Delay so we don't spawn stuff during init
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsHQ()
	{
		return m_bIsHQ;
	}

	//------------------------------------------------------------------------------------------------
	OnSpawnPointAssignedInvoker GetOnSpawnPointAssigned()
	{
		if (!m_OnSpawnPointAssigned)
			m_OnSpawnPointAssigned = new OnSpawnPointAssignedInvoker();

		return m_OnSpawnPointAssigned;
	}

	//------------------------------------------------------------------------------------------------
	float GetRadioRange()
	{
		return m_fRadioRange;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetRelayRadioRange(notnull BaseRadioComponent radio)
	{
		float range;
		int transceiversCount;
		RelayTransceiver transceiver;
		float thisRange;

		for (int i = 0, count = radio.TransceiversCount(); i < count; i++)
		{
			transceiver = RelayTransceiver.Cast(radio.GetTransceiver(i));

			if (!transceiver)
				continue;

			thisRange = transceiver.GetRange();

			if (thisRange <= range)
				continue;

			range = thisRange;
		}

		return range;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateBasesInRadioRange()
	{
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();
		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);

		SCR_CampaignMilitaryBaseComponent campaignBase;
		float radioRange = GetRadioRange();
		radioRange = radioRange * radioRange;	// We're checking square distance
		vector basePosition = GetOwner().GetOrigin();

		m_aBasesInRadioRange = {};

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase || !campaignBase.IsInitialized() || campaignBase == this)
				continue;

			if (vector.DistanceSqXZ(basePosition, campaignBase.GetOwner().GetOrigin()) < radioRange)
				m_aBasesInRadioRange.Insert(campaignBase);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool CanReachByRadio(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		return m_aBasesInRadioRange.Contains(base);
	}

	//------------------------------------------------------------------------------------------------
	bool CanReachByRadio(notnull SCR_CampaignMobileAssemblyStandaloneComponent mobileAssembly)
	{
		return (vector.DistanceXZ(GetOwner().GetOrigin(), mobileAssembly.GetOwner().GetOrigin()) <= GetRadioRange());
	}

	//------------------------------------------------------------------------------------------------
	void SetHQRadioConverage(notnull SCR_Faction faction, SCR_ECampaignHQRadioComms coverage)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
		{
			if (m_eRadioCoverageBLUFOR == coverage)
				return;

			m_eRadioCoverageBLUFOR = coverage;
		}
		else if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR))
		{
			if (m_eRadioCoverageOPFOR == coverage)
				return;

			m_eRadioCoverageOPFOR = coverage;
		}

		Replication.BumpMe();
		OnHasSignalChanged();
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECampaignHQRadioComms GetHQRadioCoverage(notnull Faction faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return SCR_ECampaignHQRadioComms.NONE;

		if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
			return m_eRadioCoverageBLUFOR;
		else if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR))
			return m_eRadioCoverageOPFOR;

		return SCR_ECampaignHQRadioComms.NONE;
	}

	//------------------------------------------------------------------------------------------------
	bool IsHQRadioTrafficPossible(Faction faction, SCR_ECampaignHQRadioComms direction = SCR_ECampaignHQRadioComms.RECEIVE)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return false;

		bool isBLUFOR = (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));

		if (!isBLUFOR && faction != campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR))
			return false;

		switch (direction)
		{
			case SCR_ECampaignHQRadioComms.RECEIVE:
			{
				if (isBLUFOR)
					return (m_eRadioCoverageBLUFOR == SCR_ECampaignHQRadioComms.RECEIVE || m_eRadioCoverageBLUFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);
				else
					return (m_eRadioCoverageOPFOR == SCR_ECampaignHQRadioComms.RECEIVE || m_eRadioCoverageOPFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);

				break;
			}

			case SCR_ECampaignHQRadioComms.SEND:
			{
				if (isBLUFOR)
					return (m_eRadioCoverageBLUFOR == SCR_ECampaignHQRadioComms.SEND || m_eRadioCoverageBLUFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);
				else
					return (m_eRadioCoverageOPFOR == SCR_ECampaignHQRadioComms.SEND || m_eRadioCoverageOPFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);

				break;
			}
		}

		if (isBLUFOR)
			return (direction == m_eRadioCoverageBLUFOR);

		return (direction == m_eRadioCoverageOPFOR);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLocalPlayerFactionAssigned(Faction assignedFaction)
	{
		UpdateBasesInRadioRange();
		m_MapDescriptor.MapSetup(assignedFaction);
		m_MapDescriptor.HandleMapInfo(SCR_CampaignFaction.Cast(assignedFaction));
		HideMapLocationLabel();
	}

	//------------------------------------------------------------------------------------------------
	int GetBaseSpawnCost()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return -1;

		//reduce spawning cost if barracks are built
		if (GetServiceDelegateByType(SCR_EServicePointType.BARRACKS))
			return campaign.GetSpawnCost() * BARRACKS_REDUCED_DEPLOY_COST;
		else
			return campaign.GetSpawnCost();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCaptureStart(SCR_Faction faction)
	{
		SCR_CampaignFaction factionC = SCR_CampaignFaction.Cast(faction);

		if (!factionC)
			return;

		BeginCapture(factionC);
	}

	//------------------------------------------------------------------------------------------------
	void RefreshTasks()
	{
		if (IsProxy())
			return;

		SCR_CampaignTaskSupportEntity supportClass = SCR_CampaignTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignTaskSupportEntity));

		if (supportClass)
			supportClass.GenerateCaptureTasks(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	void NotifyAboutEnemyAttack(notnull Faction attackingFaction)
	{
		if (!GetFaction())
			return;

		if (!IsHQRadioTrafficPossible(GetFaction()))
			return;

		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (m_fLastEnemyContactTimestamp != 0 && m_fLastEnemyContactTimestamp > (Replication.Time() - (SCR_CampaignMilitaryBaseComponent.UNDER_ATTACK_WARNING_PERIOD * 1000)))
			return;
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		if (m_fLastEnemyContactTimestamp != 0)
		{
			if (m_fLastEnemyContactTimestamp.PlusSeconds(SCR_CampaignMilitaryBaseComponent.UNDER_ATTACK_WARNING_PERIOD).Greater(curTime))
				return;
		}
		#endif

		GetCampaignFaction().SendHQMessage(SCR_ERadioMsg.BASE_UNDER_ATTACK, GetCallsign());
		#ifndef AR_CAMPAIGN_TIMESTAMP
		m_fLastEnemyContactTimestamp = Replication.Time();
		#else
		m_fLastEnemyContactTimestamp = curTime;
		#endif

		if (!GetCapturingFaction())
			SetAttackingFaction(GetGame().GetFactionManager().GetFactionIndex(attackingFaction));
	}

	//------------------------------------------------------------------------------------------------
	//! Capturing has been terminated
	void EndCapture()
	{
		if (IsProxy())
			return;

		if (!m_CapturingFaction)
			return;

		m_CapturingFaction = null;

		m_iCapturingFaction = INVALID_FACTION_INDEX;
		Replication.BumpMe();
		OnCapturingFactionChanged();
	}

	//------------------------------------------------------------------------------------------------
	int GetReconfiguredByID()
	{
		return m_iReconfiguredBy;
	}

	//------------------------------------------------------------------------------------------------
	bool GetDisableWhenUnusedAsHQ()
	{
		return m_bDisableWhenUnusedAsHQ;
	}

	//------------------------------------------------------------------------------------------------
	//! Capturing has begun
	bool BeginCapture(SCR_CampaignFaction faction, int playerID = INVALID_PLAYER_INDEX)
	{
		if (IsProxy() || !faction)
			return false;

		// The capturing faction already owns this base, return
		if (faction == GetFaction())
			return false;

		// Change the capturing faction
		m_CapturingFaction = faction;

		m_iCapturingFaction = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetFactionIndex(faction);

		m_iReconfiguredBy = playerID;
		Replication.BumpMe();
		OnCapturingFactionChanged();
		NotifyAboutEnemyAttack(faction);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Changes the faction which can spawn on spawn point groups owned by this base
	protected void HandleSpawnPointFaction()
	{
		if (!m_SpawnPoint)
			return;

		SCR_CampaignFaction owner = GetCampaignFaction();
		FactionKey currentKey = m_SpawnPoint.GetFactionKey();
		FactionKey ownerKey;
		FactionKey finalKey;

		if (owner)
		{
			ownerKey = owner.GetFactionKey();
			finalKey = ownerKey;
		}

		if (ownerKey == FactionKey.Empty)
		{
			if (currentKey != FactionKey.Empty)
				m_SpawnPoint.SetFactionKey(FactionKey.Empty);

			return;
		}

		if (!m_bIsHQ && !IsHQRadioTrafficPossible(owner, SCR_ECampaignHQRadioComms.BOTH_WAYS))
			finalKey = FactionKey.Empty;

		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (Replication.Time() < m_fRespawnAvailableSince && !m_bIsHQ)
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		if (world.GetServerTimestamp().Less(m_fRespawnAvailableSince) && !m_bIsHQ)
		#endif
			finalKey = FactionKey.Empty;

		if (GetSupplies() < GetBaseSpawnCost() && !m_bIsHQ)
			finalKey = FactionKey.Empty;

		if (finalKey == currentKey)
			return;

		m_SpawnPoint.SetFactionKey(finalKey);
	}
	//------------------------------------------------------------------------------------------------
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Event which is triggered when the capturing faction changes
	override void OnCapturingFactionChanged()
	{
		super.OnCapturingFactionChanged();

		m_CapturingFaction = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetCampaignFactionByIndex(m_iCapturingFaction);

		// Play or stop radio tuning SFX
		if (m_HQRadio)
		{
			SignalsManagerComponent comp = SignalsManagerComponent.Cast(m_HQRadio.FindComponent(SignalsManagerComponent));

			if (comp)
			{
				if (m_CapturingFaction)
					comp.SetSignalValue(comp.AddOrFindSignal(ESTABLISH_ACTION_SIGNAL_NAME), 1);
				else
					comp.SetSignalValue(comp.AddOrFindSignal(ESTABLISH_ACTION_SIGNAL_NAME), 0);
			}
		}

		if (!IsProxy())
		{
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

			if (campaign)
				campaign.GetBaseManager().EvaluateControlPoints();
		}

		SCR_CampaignFeedbackComponent feedback = SCR_CampaignFeedbackComponent.GetInstance();

		if (!feedback)
			return;

		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (m_CapturingFaction && GetFaction() == playerFaction && IsHQRadioTrafficPossible(playerFaction))
			feedback.FlashBaseIcon(this, faction: m_CapturingFaction, infiniteTimer: true);
		else
			feedback.FlashBaseIcon(this, changeToDefault: true);
	}

	//------------------------------------------------------------------------------------------------
	void OnHasSignalChanged()
	{
		if (!IsProxy())
		{
			HandleSpawnPointFaction();
			SupplyIncomeTimer(true);
		}

		if (RplSession.Mode() != RplMode.Dedicated)
			m_MapDescriptor.HandleMapInfo();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (campaign)
			campaign.GetBaseManager().GetOnSignalChanged().Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSuppliesChanged()
	{
		if (!IsProxy() && !m_bIsHQ)
			HandleSpawnPointFaction();

		if (RplSession.Mode() != RplMode.Dedicated && m_UIElement)
			m_UIElement.SetIconInfoText();
	}

	//------------------------------------------------------------------------------------------------
	//! Event which is triggered when the owning faction changes
	override protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
		super.OnFactionChanged(owner, previousFaction, faction);

		if (!GetGame().InPlayMode())
			return;

		SCR_CampaignFaction newCampaignFaction = SCR_CampaignFaction.Cast(faction);

		if (!newCampaignFaction)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		float curTime = GetGame().GetWorld().GetWorldTime();

		if (!IsProxy())
		{
			EndCapture();
			m_mDefendersData.Clear();

			// Update signal coverage only if the base was seized during normal play, not at the start
			if (curTime > 10000)
			{
				campaign.GetBaseManager().RecalculateRadioCoverage(campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
				campaign.GetBaseManager().RecalculateRadioCoverage(campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
			}

			ChangeRadioSettings(newCampaignFaction);

			// Reset timer for reinforcements
			SupplyIncomeTimer(true);

			// Delay respawn possibility at newly-captured bases
			if (!m_bIsHQ && m_bInitialized && newCampaignFaction.IsPlayable() && campaign.GetBaseManager().IsBasesInitDone() && curTime > SCR_GameModeCampaign.BACKEND_DELAY)
			{
				#ifndef AR_CAMPAIGN_TIMESTAMP
				m_fRespawnAvailableSince = Replication.Time() + RESPAWN_DELAY_AFTER_CAPTURE;
				#else
				ChimeraWorld world = GetOwner().GetWorld();
				m_fRespawnAvailableSince = world.GetServerTimestamp().PlusMilliseconds(RESPAWN_DELAY_AFTER_CAPTURE);
				#endif
				OnRespawnCooldownChanged();
				Replication.BumpMe();
			}

			HandleSpawnPointFaction();
			SendHQMessageBaseCaptured();

			// If some Remnants live, send them to recapture
			if (newCampaignFaction.IsPlayable())
			{
				foreach (SCR_AmbientPatrolSpawnPointComponent remnants : m_aRemnants)
				{
					AIGroup grp = remnants.GetSpawnedGroup();

					if (!grp)
						continue;

					if (!m_RetakeWP && m_HQRadio)
					{
						EntitySpawnParams params = EntitySpawnParams();
						params.TransformMode = ETransformMode.WORLD;
						params.Transform[3] = m_HQRadio.GetOrigin();
						m_SeekDestroyWP = SCR_TimedWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(SCR_GameModeCampaign.GetInstance().GetSeekDestroyWaypointPrefab()), null, params));
						m_SeekDestroyWP.SetHoldingTime(60);
					}

					if (m_SeekDestroyWP)
						grp.AddWaypointAt(m_SeekDestroyWP, 0);
				}
			}

			// Change barrack group according to owner's faction, if it was built
			if (m_BarrackComponent)
			{
				if (!newCampaignFaction.IsPlayable())
					m_BarrackComponent.StopHandler();
				else
				{
					ResourceName defenderGroup = newCampaignFaction.GetDefendersGroupPrefab();

					if (defenderGroup)
					{
						m_BarrackComponent.EnableSpawning(false);
						m_BarrackComponent.SetGroupPrefab(defenderGroup);
						if (!m_BarrackComponent.IsInitialized())
							m_BarrackComponent.InitializeBarrack();

						GetGame().GetCallqueue().CallLater(m_BarrackComponent.EnableSpawning, m_BarrackComponent.GetRespawnDelay()* 1000, false, true);
					}
				}
			}

			// Change owner of assigned armory
			if (m_ArmoryComponent)
				m_ArmoryComponent.ChangeOwningFaction(GetFaction());

			// Change owner of assigned armory
			if (m_RadioArmory)
				m_RadioArmory.ChangeOwningFaction(GetFaction());

			if (GetGame().GetWorld().GetWorldTime() != 0)
				GetGame().GetSaveManager().Save(ESaveType.AUTO);
		}

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			SCR_CampaignFeedbackComponent feedback = SCR_CampaignFeedbackComponent.GetInstance();

			if (feedback)
				feedback.FlashBaseIcon(this, changeToDefault: true);

			if (m_MapDescriptor)
				m_MapDescriptor.HandleMapInfo();

			SetRadioChatterSignal(newCampaignFaction);

			SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
			IEntity player = SCR_PlayerController.GetLocalControlledEntity();

			// TODO: Move this to PlayRadioMsg so it is checked for player being inside radio range
			if (campaign)
			{
				if (player && playerFaction && playerFaction != GetCampaignFaction() && IsHQRadioTrafficPossible(playerFaction))
				{
					if (GetCampaignFaction())
						SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_BaseSeized-UC", prio: SCR_ECampaignPopupPriority.BASE_LOST, param1: GetCampaignFaction().GetFactionNameUpperCase(), param2: GetBaseNameUpperCase());
					else
					{
						SCR_CampaignFaction factionR = campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);

						if (factionR)
							SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_BaseSeized-UC", prio: SCR_ECampaignPopupPriority.BASE_LOST, param1: factionR.GetFactionNameUpperCase(), param2: GetBaseNameUpperCase());
					}

					if (playerFaction == m_OwningFactionPrevious)
						//campaign.ShowHint(SCR_ECampaignHints.BASE_LOST);
				}
			}
		}

		m_OwningFactionPrevious = newCampaignFaction;
	}

	//------------------------------------------------------------------------------------------------
	void OnSpawnPointFactionAssigned(FactionKey faction)
	{
		if (RplSession.Mode() != RplMode.Dedicated)
			m_MapDescriptor.HandleMapInfo();
	}

	//------------------------------------------------------------------------------------------------
	void ChangeRadioSettings(notnull SCR_Faction faction)
	{
		if (!m_RadioComponent || m_RadioComponent.TransceiversCount() == 0)
			return;

		BaseTransceiver tsv = BaseTransceiver.Cast(m_RadioComponent.GetTransceiver(0));

		if (!tsv)
			return;

		m_RadioComponent.SetEncryptionKey(faction.GetFactionRadioEncryptionKey());

		int factionFrequency = faction.GetFactionRadioFrequency();

		// Setting frequency outside of limits causes a VME
		if (factionFrequency < tsv.GetMinFrequency() || factionFrequency > tsv.GetMaxFrequency())
			return;

		tsv.SetFrequency(factionFrequency);
	}

	//------------------------------------------------------------------------------------------------
	void RegisterRemnants(notnull SCR_AmbientPatrolSpawnPointComponent remnants)
	{
		m_aRemnants.Insert(remnants);
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetBuildingIconImageset()
	{
		SCR_CampaignMilitaryBaseComponentClass componentData = SCR_CampaignMilitaryBaseComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return ResourceName.Empty;

		return componentData.GetBuildingIconImageset();
	}

	//------------------------------------------------------------------------------------------------
	SCR_GraphLinesData GetGraphLinesData()
	{
		SCR_CampaignMilitaryBaseComponentClass componentData = SCR_CampaignMilitaryBaseComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return null;

		return componentData.GetGraphLinesData();
	}

	//------------------------------------------------------------------------------------------------
	float GetLineWidth()
	{
		SCR_CampaignMilitaryBaseComponentClass componentData = SCR_CampaignMilitaryBaseComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return 0;

		return componentData.GetLineWidth();
	}

	//------------------------------------------------------------------------------------------------
	void SetDefendersGroup(SCR_AIGroup grp)
	{
		m_DefendersGroup = grp;
	}

	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetDefendersGroup()
	{
		return m_DefendersGroup;
	}

	//------------------------------------------------------------------------------------------------
	override void SetCallsign(notnull SCR_Faction faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		if (m_iCallsign == INVALID_BASE_CALLSIGN)
			return;

		int callsignOffset = campaign.GetCallsignOffset();
		campaign.GetOnCallsignOffsetChanged().Remove(OnCallsignAssigned);

		if (callsignOffset == INVALID_BASE_CALLSIGN)
		{
			campaign.GetOnCallsignOffsetChanged().Insert(OnCallsignAssigned);
			return;
		}

		SCR_MilitaryBaseCallsign callsignInfo;

		// Use index offset for OPFOR so callsigns are not comparable (i.e. Alabama will not always mean Avrora etc.)
		if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
			callsignInfo = faction.GetBaseCallsignByIndex(m_iCallsign);
		else
			callsignInfo = faction.GetBaseCallsignByIndex(m_iCallsign, callsignOffset);

		if (!callsignInfo)
			return;

		m_sCallsign = callsignInfo.GetCallsign();
		m_sCallsignNameOnly = callsignInfo.GetCallsignShort();
		m_sCallsignNameOnlyUC = callsignInfo.GetCallsignUpperCase();
		m_iCallsignSignal = callsignInfo.GetSignalIndex();
	}

	//------------------------------------------------------------------------------------------------
	override void SetCallsignIndexAutomatic(int index)
	{
		// Handled separately in SCR_CampaignMilitaryBaseManager.InitializeBases()
		return;
	}

	//------------------------------------------------------------------------------------------------
	//! Handles the timer for reinforcements arrival at this base
	//! \param reset Resets the current timer
	protected void SupplyIncomeTimer(bool reset = false)
	{
		BaseWorld world = GetGame().GetWorld();

		if (!world)
			return;

		float curTime = world.GetWorldTime();

		if (reset)
		{
			m_fSuppliesArrivalTime = curTime + (SCR_GameModeCampaign.GetInstance().GetSuppliesArrivalInterval() * 1000);
			return;
		}

		Faction owner = GetFaction();

		if (!owner || !IsHQRadioTrafficPossible(owner, SCR_ECampaignHQRadioComms.BOTH_WAYS))
			return;

		if (curTime >= m_fSuppliesArrivalTime)
		{
			AddRegularSupplyPackage(owner);
			m_fSuppliesArrivalTime = curTime + (SCR_GameModeCampaign.GetInstance().GetSuppliesArrivalInterval() * 1000);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reinforcements timer has finished, send in reinforcements
	protected void AddRegularSupplyPackage(notnull Faction faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		float currentSupplies = GetSupplies();
		float spaceLimit = campaign.GetSuppliesReplenishThreshold() - currentSupplies;

		if (spaceLimit <= 0)
			return;
		
		int quickReplenishThreshold = campaign.GetQuickSuppliesReplenishThreshold();
		float quickReplenishMultiplier = campaign.GetQuickSuppliesReplenishMultiplier();
		int income = campaign.GetRegularSuppliesIncomeBase();
		int incomeHQ = campaign.GetRegularSuppliesIncome();
		int toAdd;

		if (m_bIsHQ || m_bIsSupplyHub)
		{
			if (currentSupplies < quickReplenishThreshold)
			{
				int incomeHQQuick = incomeHQ * quickReplenishMultiplier;
				incomeHQQuick = Math.Min(incomeHQQuick, quickReplenishThreshold - currentSupplies);
				toAdd = Math.Max(incomeHQQuick, incomeHQ);
			}
			else
			{
				toAdd = incomeHQ;
			}
		}
		else
		{
			if (currentSupplies < quickReplenishThreshold)
			{
				int incomeQuick = income * quickReplenishMultiplier;
				incomeQuick = Math.Min(incomeQuick, quickReplenishThreshold - currentSupplies);
				toAdd = Math.Max(incomeQuick, income);
			}
			else
			{
				toAdd = income;
			}
		}

		int extraBasesCount;

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBasesInRadioRange)
		{
			if (base.GetType() != SCR_ECampaignBaseType.BASE)
				continue;

			if (base.GetFaction() != faction)
				continue;

			if (!base.IsHQRadioTrafficPossible(faction))
				continue;

			extraBasesCount++;
		}

		toAdd += (SCR_GameModeCampaign.GetInstance().GetRegularSuppliesIncomeExtra() * extraBasesCount);
		AddSupplies(Math.Min(toAdd, spaceLimit));
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseMapDescriptorComponent GetMapDescriptor()
	{
		return m_MapDescriptor;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMapUIBase GetMapUI()
	{
		return m_UIElement;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsEntityInMyRange(notnull IEntity entity)
	{
		return vector.Distance(entity.GetOrigin(), GetOwner().GetOrigin()) <= GetRadioRange();
	}

	//------------------------------------------------------------------------------------------------
	float GetSupplies()
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(GetOwner());

		if (!resourceComponent)
			return 0.0;

		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!resourceConsumer)
			return 0.0;

		if (!IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(resourceConsumer);
		
		return resourceConsumer.GetAggregatedResourceValue();
	}

	//------------------------------------------------------------------------------------------------
	float GetSuppliesMax()
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(GetOwner());

		if (!resourceComponent)
			return 0.0;

		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!resourceConsumer)
			return 0.0;

		if (!IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(resourceConsumer);

		return resourceConsumer.GetAggregatedMaxResourceValue();
	}

	//------------------------------------------------------------------------------------------------
	void RegisterHQRadio(notnull IEntity radio)
	{
		m_HQRadio = radio;
		SetRadioChatterSignal(GetCampaignFaction());
	}

	//------------------------------------------------------------------------------------------------
	override void RegisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		super.RegisterLogicComponent(component);

		SCR_FlagComponent flag = SCR_FlagComponent.Cast(component);

		// This is indeed a repeated call from super
		// Sandbox mil bases spawned via HQ tents are interfering with shown flag as they have different faction
		// It's a temporary fix until we switch to full free building and bases on top of each other will no longer be a thing
		if (flag && GetFaction())
			GetGame().GetCallqueue().CallLater(ChangeFlags, 1000, false, GetFaction());	
	}

	//------------------------------------------------------------------------------------------------
	void SetRadioChatterSignal(SCR_Faction faction)
	{
		if (!m_HQRadio)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		SignalsManagerComponent comp = SignalsManagerComponent.Cast(m_HQRadio.FindComponent(SignalsManagerComponent));

		if (!comp)
			return;

		if (!faction || faction.GetFactionKey() == campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR))
		{
			comp.SetSignalValue(comp.AddOrFindSignal(RADIO_CHATTER_SIGNAL_NAME), 0);
		}
		else
		{
			if (faction.GetFactionKey() == campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR))
			{
				comp.SetSignalValue(comp.AddOrFindSignal(RADIO_CHATTER_SIGNAL_NAME), 1);
			}
			else
			{
				comp.SetSignalValue(comp.AddOrFindSignal(RADIO_CHATTER_SIGNAL_NAME), 2);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetHQRadio()
	{
		return m_HQRadio;
	}

	//------------------------------------------------------------------------------------------------
	void HideMapLocationLabel()
	{
		if (m_sMapLocationName.IsEmpty() || !GetGame().GetWorld())
			return;

		GenericEntity ent = GenericEntity.Cast(GetGame().GetWorld().FindEntityByName(m_sMapLocationName));

		if (!ent)
			return;

		MapDescriptorComponent comp = MapDescriptorComponent.Cast(ent.FindComponent(MapDescriptorComponent));

		if (!comp)
			return;

		MapItem item = comp.Item();

		if (!item)
			return;

		item.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	int GetRadioFrequency(FactionKey faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return INVALID_FREQUENCY;

		switch (faction)
		{
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR): {return m_iFreqWest; };
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR): {return m_iFreqEast; };
		}

		return INVALID_FREQUENCY;
	}

	//------------------------------------------------------------------------------------------------
	//! Add delivered supplies
	void AddSupplies(int suppliesCount, bool replicate = true)
	{
		if (suppliesCount == 0)
			return;

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(GetOwner());

		if (!resourceComponent)
			return;

		if (suppliesCount > 0)
		{
			SCR_ResourceGenerator resourceGenerator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

			if (!resourceGenerator)
				return;

			resourceGenerator.RequestGeneration(suppliesCount);
		}
		else
		{
			SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

			if (!resourceConsumer)
				return;

			resourceConsumer.RequestConsumtion(-suppliesCount);
		}

		HandleSpawnPointFaction();
	}

	//------------------------------------------------------------------------------------------------
	void SetSupplies(float suppliesCount)
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(GetOwner());

		if (!resourceComponent)
			return;

		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!resourceConsumer)
			return;

		if (!IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(resourceConsumer);

		AddSupplies(suppliesCount - resourceConsumer.GetAggregatedResourceValue());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInitialSupplies(float suppliesCount)
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(GetOwner());

		if (!resourceComponent)
			return;

		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!resourceConsumer)
			return;

		if (!IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(resourceConsumer);
		
		resourceConsumer.RequestConsumtion(resourceConsumer.GetAggregatedResourceValue());
		SetSupplies(suppliesCount);
	}

	//------------------------------------------------------------------------------------------------
	void SetStartingSupplies(float suppliesCount)
	{
		m_fStartingSupplies = suppliesCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Change reinforcements timer
	//! \param time This number is added to the timer
	void AlterSupplyIncomeTimer(float time)
	{
		m_fSuppliesArrivalTime += time
	}

	//------------------------------------------------------------------------------------------------
	//! Outs all bases in range that suit the filter. Filter = allowed types.
	int GetBasesInRange(SCR_ECampaignBaseType filter, notnull out array<SCR_CampaignMilitaryBaseComponent> basesInRange)
	{
		int count = 0;

		if (!m_aBasesInRadioRange)
			return count;

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBasesInRadioRange)
		{
			SCR_ECampaignBaseType baseType = base.GetType();

			if ((filter & baseType) == baseType)
			{
				basesInRange.Insert(base);
				count++;
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	void SetFaction(SCR_CampaignFaction faction)
	{
		if (IsProxy())
			return;

		if (!m_FactionComponent)
			return;

		m_FactionComponent.SetAffiliatedFaction(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the owning faction
	SCR_CampaignFaction GetCampaignFaction()
	{
		return SCR_CampaignFaction.Cast(GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	//! Returns type of this base
	SCR_ECampaignBaseType GetType()
	{
		return m_eType;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the name of this base
	string GetBaseName()
	{
		return m_sBaseName;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the upper-case name of this base
	string GetBaseNameUpperCase()
	{
		return m_sBaseNameUpper;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether this base is being captured
	bool IsBeingCaptured()
	{
		// The capturing faction exists, return true
		if (m_CapturingFaction)
			return true;

		// No faction is capturing this base, return false
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetAttackingFaction(int enemyFaction)
	{
		m_iAttackingFaction = enemyFaction;

		if (m_iAttackingFaction < 0)
			return;

		Replication.BumpMe();
		OnAttackingFactionChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnAttackingFactionChanged()
	{
		Faction enemyFaction = GetGame().GetFactionManager().GetFactionByIndex(m_iAttackingFaction);

		if (!IsProxy())
			GetGame().GetCallqueue().CallLater(SetAttackingFaction, 1000, false, -1);
		else
			m_iAttackingFaction = -1;

		SCR_CampaignFeedbackComponent feedback = SCR_CampaignFeedbackComponent.GetInstance();
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (feedback && playerFaction == GetFaction() && IsHQRadioTrafficPossible(playerFaction))
			feedback.FlashBaseIcon(this, faction: enemyFaction);
	}

	//------------------------------------------------------------------------------------------------
	void OnRespawnCooldownChanged()
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		float curTime = Replication.Time();
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		#endif

		// Make sure the spawnpoint becomes available after timer runs out
		if (!IsProxy())
			#ifndef AR_CAMPAIGN_TIMESTAMP
			GetGame().GetCallqueue().CallLater(HandleSpawnPointFaction, (m_fRespawnAvailableSince - curTime) + SCR_GameModeCampaign.MEDIUM_DELAY);
			#else
			GetGame().GetCallqueue().CallLater(HandleSpawnPointFaction, m_fRespawnAvailableSince.DiffMilliseconds(curTime) + SCR_GameModeCampaign.MEDIUM_DELAY);
			#endif

		// Handle respawn cooldown UI
		if (RplSession.Mode() != RplMode.Dedicated)
			UpdateRespawnCooldown();
	}

	//------------------------------------------------------------------------------------------------
	void UpdateRespawnCooldown()
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		float curTime = Replication.Time();
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		#endif

		if (m_UIElement && SCR_FactionManager.SGetLocalPlayerFaction() == GetFaction())
			m_UIElement.SetIconInfoText();

		GetGame().GetCallqueue().Remove(UpdateRespawnCooldown);

		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (curTime < m_fRespawnAvailableSince)
		#else
		if (curTime.Less(m_fRespawnAvailableSince))
		#endif
			GetGame().GetCallqueue().CallLater(UpdateRespawnCooldown, SCR_GameModeCampaign.UI_UPDATE_DELAY);
	}

	//------------------------------------------------------------------------------------------------
	override void OnCallsignAssigned()
	{
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!faction)
			faction = SCR_GameModeCampaign.GetInstance().GetBaseManager().GetLocalPlayerFaction();

		if (!faction)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		campaign.GetOnCallsignOffsetChanged().Remove(OnCallsignAssigned);
		int callsignOffset = campaign.GetCallsignOffset();

		if (callsignOffset == INVALID_BASE_CALLSIGN)
		{
			campaign.GetOnCallsignOffsetChanged().Insert(OnCallsignAssigned);
			return;
		}

		SetCallsign(faction);
	}

	//------------------------------------------------------------------------------------------------
	void OnAllBasesInitialized()
	{
		if (IsProxy())
		{
			UpdateBasesInRadioRange();
			return;
		}

		// Spawn HQ composition
		if (GetType() == SCR_ECampaignBaseType.BASE)
		{
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
			ResourceName buildingPrefab = GetCampaignFaction().GetBuildingPrefab(EEditableEntityLabel.SERVICE_HQ);

			if (!buildingPrefab)
			{
				Math.Randomize(-1);

				if (Math.RandomFloat01() >= 0.5)
					buildingPrefab = campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetBuildingPrefab(EEditableEntityLabel.SERVICE_HQ);
				else
					buildingPrefab = campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetBuildingPrefab(EEditableEntityLabel.SERVICE_HQ);
			}

			if (buildingPrefab)
				GetGame().GetCallqueue().CallLater(SpawnBuilding, 1000, false, buildingPrefab, GetOwner().GetOrigin(), GetOwner().GetYawPitchRoll(), true);	// Delay so we don't spawn stuff during init
		}
	}

	//------------------------------------------------------------------------------------------------
	void RecalculateRadioRange()
	{
		float range;

		if (m_RadioComponent)
			range = GetRelayRadioRange(m_RadioComponent);

		float thisRange;
		array<SCR_ServicePointComponent> antennas = {};
		GetServicesByType(antennas, SCR_EServicePointType.RADIO_ANTENNA);
		BaseRadioComponent radio;

		// Find antenna services, read max radio range from the radio component on their owners
		foreach (SCR_ServicePointComponent service : antennas)
		{
			SCR_AntennaServicePointComponent antenna = SCR_AntennaServicePointComponent.Cast(service);
			radio = BaseRadioComponent.Cast(antenna.GetOwner().FindComponent(BaseRadioComponent));

			if (!radio)
				continue;

			thisRange = GetRelayRadioRange(radio);

			if (thisRange > range)
				range = thisRange;
		}

		if (m_fRadioRange == range)
			return;

		m_fRadioRange = range;
		Replication.BumpMe();
		OnRadioRangeChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnRadioRangeChanged()
	{
		UpdateBasesInRadioRange();

		if (m_MapDescriptor)
			m_MapDescriptor.HandleMapLinks();

		if (IsProxy())
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		SCR_CampaignMilitaryBaseManager bManager = campaign.GetBaseManager();

		if (GetGame().GetWorld().GetWorldTime() > campaign.BACKEND_DELAY)
		{
			// Process recalculation immediately unless we're still within save loading period
			bManager.RecalculateRadioCoverage(GetCampaignFaction());
		}
		else
		{
			// Otherwise process for both factions only once so we're not doing it for each antenna loaded
			GetGame().GetCallqueue().Remove(bManager.RecalculateRadioCoverage);
			GetGame().GetCallqueue().CallLater(bManager.RecalculateRadioCoverage, campaign.DEFAULT_DELAY, false, campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
			GetGame().GetCallqueue().CallLater(bManager.RecalculateRadioCoverage, campaign.DEFAULT_DELAY, false, campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
		}

		RefreshTasks();
	}

	//------------------------------------------------------------------------------------------------
	//! Called from SCR_MapCampaignUI when base UI elements are initialized
	void SetBaseUI(SCR_CampaignMapUIBase base)
	{
		m_UIElement = base;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign Barrack component
	void AssignBarracks(SCR_CampaignBarracksComponent barracksComp)
	{
		m_BarrackComponent = barracksComp;

		if (!m_BarrackComponent)
			return;

		if (m_SuppliesComponent)
			m_BarrackComponent.SetSuppliesComponent(m_SuppliesComponent);

		SCR_CampaignFaction owningFaction = GetCampaignFaction();

		if (!owningFaction || !owningFaction.IsPlayable())
			return;

		ResourceName defenderGroup = owningFaction.GetDefendersGroupPrefab();

		if (defenderGroup)
		{
			m_BarrackComponent.SetGroupPrefab(defenderGroup);
			m_BarrackComponent.InitializeBarrack();
		}
	}

	//------------------------------------------------------------------------------------------------
	void SendHQMessageBaseCaptured()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		SCR_CampaignFaction newOwner = GetCampaignFaction();
		if (!newOwner)
			return;

		int newOwnerPoints = newOwner.GetControlPointsHeld();

		if (newOwnerPoints == campaign.GetControlPointTreshold() && IsControlPoint())
		{
			newOwner.SendHQMessage(SCR_ERadioMsg.WINNING);

			SCR_CampaignFaction losingFaction;
			FactionManager factionManager = GetGame().GetFactionManager();
			if (!factionManager)
				return;

			if (newOwner.GetFactionKey() == campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR))
				losingFaction = SCR_CampaignFaction.Cast(factionManager.GetFactionByKey(campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR)));
			else
				losingFaction = SCR_CampaignFaction.Cast(factionManager.GetFactionByKey(campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR)));

			if (losingFaction)
				losingFaction.SendHQMessage(SCR_ERadioMsg.LOSING);
		}
		else
		{
			if (IsHQ())
				newOwner.SendHQMessage(SCR_ERadioMsg.SEIZED_MAIN, m_iCallsign);
			else if (IsControlPoint())
				newOwner.SendHQMessage(SCR_ERadioMsg.SEIZED_MAJOR, m_iCallsign);
			else if (GetType() == SCR_ECampaignBaseType.BASE)
				newOwner.SendHQMessage(SCR_ERadioMsg.SEIZED_SMALL, m_iCallsign);

			if (m_OwningFactionPrevious)
			{
				if (GetType() == SCR_ECampaignBaseType.RELAY)
					m_OwningFactionPrevious.SendHQMessage(SCR_ERadioMsg.RELAY_LOST, m_iCallsign);
				else
					m_OwningFactionPrevious.SendHQMessage(SCR_ERadioMsg.BASE_LOST, m_iCallsign);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnStartingVehicles()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		if (campaign.IsTutorial())
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		vector pos, oldPos;
		IEntity vehicleEntity;
		Physics physicsComponent;
		
		array<ResourceName> prefabNames = {};
		GetCampaignFaction().GetStartingVehiclePrefabs(prefabNames);

		foreach (ResourceName prefabName : prefabNames)
		{
			oldPos = pos;
			SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOwner().GetOrigin(), HQ_VEHICLE_SPAWN_RADIUS, HQ_VEHICLE_QUERY_SPACE);

			//Should found empty terrain position be too close to last one, do another search with previous position as areaCenter.
			if (vector.DistanceSqXZ(pos, oldPos) < 1)
				SCR_WorldTools.FindEmptyTerrainPosition(pos, oldPos, HQ_VEHICLE_SPAWN_RADIUS, HQ_VEHICLE_QUERY_SPACE);

			params.Transform[3] = pos;

			Resource veh = Resource.Load(prefabName);

			if (!veh || !veh.IsValid())
				return;

			vehicleEntity = GetGame().SpawnEntityPrefab(veh, GetGame().GetWorld(), params);

			if (vehicleEntity)
			{
				physicsComponent = vehicleEntity.GetPhysics();

				if (physicsComponent)
					physicsComponent.SetVelocity("0 -0.1 0"); // Make vehicle copy the terrain properly

				m_aStartingVehicles.Insert(vehicleEntity);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteStartingVehicles()
	{
		foreach (IEntity veh : m_aStartingVehicles)
		{
			if (veh)
				RplComponent.DeleteRplEntity(veh, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveFortifications()
	{
		IEntity child = GetOwner().GetChildren();
		IEntity prevChild;

		while (child)
		{
			if (child.FindComponent(SCR_CampaignBuildingCompositionComponent))
				prevChild = child;

			child = child.GetSibling();

			if (prevChild)
				SCR_EntityHelper.DeleteEntityAndChildren(prevChild);
		}
	}

	//------------------------------------------------------------------------------------------------
	void LoadState(notnull SCR_CampaignBaseStruct baseStruct)
	{
		SetCallsignIndex(baseStruct.GetCallsignIndex());

		if (!m_bInitialized)
			Initialize();

		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(factionManager.GetFactionByIndex(baseStruct.GetFaction()));

		if (faction && GetCampaignFaction() != faction)
		{
			SetFaction(faction);
			RemoveFortifications();
		}

		m_fStartingSupplies = -1;

		SetAsHQ(baseStruct.IsHQ());
		UpdateBasesInRadioRange();

		GetGame().GetCallqueue().Remove(SpawnBuilding);
		//GetGame().GetCallqueue().CallLater(SpawnSavedBuildings, SCR_GameModeCampaign.DEFAULT_DELAY, false, baseStruct);	// Delay so we don't spawn stuff during init

		if (m_HQTent)
			RplComponent.DeleteRplEntity(m_HQTent, false);

		// Starting vehicles have been presumably spent at this point
		GetGame().GetCallqueue().Remove(SpawnStartingVehicles);
		DeleteStartingVehicles();
	}

	//------------------------------------------------------------------------------------------------
	/*void SpawnSavedBuildings(notnull SCR_CampaignBaseStruct loadedData)
	{
		if (m_HQTent)
			RplComponent.DeleteRplEntity(m_HQTent, false);

		SpawnBuilding(loadedData.GetHQPrefab(), loadedData.GetHQPosition(), loadedData.GetHQRotation(), true);

		array<SCR_EServicePointType> types = {};
		SCR_Enum.GetEnumValues(SCR_EServicePointType, types);

		foreach (SCR_EServicePointType type : types)
		{
			if (GetServiceByType(type))
				continue;

			SpawnBuilding(loadedData.GetServicePrefab(type), loadedData.GetServicePosition(type), loadedData.GetServiceRotation(type));
		}
	}*/

	//------------------------------------------------------------------------------------------------
	void SpawnBuilding(ResourceName prefab, vector position, vector rotation, bool isMainTent = false)
	{
		if (prefab.IsEmpty())
			return;

		if (position == vector.Zero)
			return;

		EntitySpawnParams params = EntitySpawnParams();
		GetOwner().GetWorldTransform(params.Transform);
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(rotation, params.Transform);
		params.Transform[3] = position;

		IEntity composition = GetGame().SpawnEntityPrefab(Resource.Load(prefab), null, params);

		if (!composition)
			return;

		if (isMainTent)
		{
			m_HQTent = composition;

			// Delayed call so the supplies system has time to process all the containers first
			if (m_fStartingSupplies >= 0)
				GetGame().GetCallqueue().CallLater(SetInitialSupplies, SCR_GameModeCampaign.MEDIUM_DELAY, false, m_fStartingSupplies);
		}

		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());

		if (aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(composition);

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.FindComponent(SCR_EditableEntityComponent));
		vector transform[4];

		if (!editableEntity)
		{
			GetOwner().GetTransform(transform);
			SCR_TerrainHelper.SnapToTerrain(transform, composition.GetWorld());
			composition.SetTransform(transform);
			return;
		}

		editableEntity.GetTransform(transform);

		if (!SCR_TerrainHelper.SnapToTerrain(transform, composition.GetWorld()))
			return;

		editableEntity.SetTransformWithChildren(transform);
	}

	//------------------------------------------------------------------------------------------------
	//! Periodically add XP to players defending their base
	void EvaluateDefenders()
	{
		SCR_CampaignFaction baseFaction = GetCampaignFaction();

		if (!baseFaction.IsPlayable())
			return;

		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!factionManager)
			return;

		if (GetHQRadioCoverage(factionManager.GetEnemyFaction(baseFaction)) == SCR_ECampaignHQRadioComms.NONE)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		array<int> playerIdsPresent = {};
		playerManager.GetPlayers(playerIds);
		int radiusSq;
		vector basePos = GetOwner().GetOrigin();
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		bool enemiesPresent;
		
		if (m_eType == SCR_ECampaignBaseType.BASE)
			radiusSq = m_iRadius * m_iRadius;
		else
			radiusSq = RELAY_BASE_RADIUS * RELAY_BASE_RADIUS;

		foreach (int playerId : playerIds)
		{
			SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerId));

			if (!player)
				continue;

			CharacterControllerComponent charController = player.GetCharacterController();

			if (charController.IsDead())
				continue;

			if (vector.DistanceSqXZ(player.GetOrigin(), basePos) > radiusSq)
				continue;

			if (player.GetFaction() == baseFaction)
				playerIdsPresent.Insert(playerId);
			else
				enemiesPresent = true;
		}

		#ifdef AR_CAMPAIGN_TIMESTAMP
		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		#endif
		foreach (int playerId : playerIdsPresent)
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			int curTime = Replication.Time();
			#endif

			#ifndef AR_CAMPAIGN_TIMESTAMP
			int startedDefendingAt = m_mDefendersData.Get(playerId);
			#else
			WorldTimestamp startedDefendingAt = m_mDefendersData.Get(playerId);
			#endif
			if (startedDefendingAt == 0)
			{
				m_mDefendersData.Set(playerId, curTime)
			}
			#ifndef AR_CAMPAIGN_TIMESTAMP
			else if ((curTime - startedDefendingAt) >= DEFENDERS_REWARD_PERIOD)
			#else
			else if (curTime.DiffMilliseconds(startedDefendingAt) >= DEFENDERS_REWARD_PERIOD)
			#endif
			{
				m_mDefendersData.Set(playerId, curTime);

				if (enemiesPresent)
					compXP.AwardXP(playerManager.GetPlayerController(playerId), SCR_EXPRewards.BASE_DEFENDED, DEFENDERS_REWARD_MULTIPLIER);
				else
					compXP.AwardXP(playerManager.GetPlayerController(playerId), SCR_EXPRewards.BASE_DEFENDED);
			}
		}

		// Clean up non-present players from the list
		for (int i = m_mDefendersData.Count() - 1; i >= 0; i--)
		{
			int playerId = m_mDefendersData.GetKey(i);

			if (playerIdsPresent.Contains(playerId))
				continue;

			m_mDefendersData.Remove(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	void StoreState(out SCR_CampaignBaseStruct baseStruct)
	{
		baseStruct.SetIsHQ(IsHQ());
		baseStruct.SetCallsignIndex(GetCallsign());
		baseStruct.SetPosition(GetOwner().GetOrigin());
		baseStruct.SetOwningFaction(GetGame().GetFactionManager().GetFactionIndex(GetFaction()));
		baseStruct.SetSupplies(GetSupplies());
		//baseStruct.SetBuildingsData(this);
	}

	protected void ConnectToCampaignBasesSystem()
	{
		World world = GetOwner().GetWorld();
		CampaignBasesSystem updateSystem = CampaignBasesSystem.Cast(world.FindSystem(CampaignBasesSystem));
		if (!updateSystem)
			return;

		updateSystem.Register(this);
	}

	protected void DisconnectFromCampaignBasesSystem()
	{
		World world = GetOwner().GetWorld();
		CampaignBasesSystem updateSystem = CampaignBasesSystem.Cast(world.FindSystem(CampaignBasesSystem));
		if (!updateSystem)
			return;

		updateSystem.Unregister(this);
	}

	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		m_fTimer += timeSlice;

		// Periodic calls, add random delay to avoid spikes
		if (m_fTimer < m_fNextFrameCheck)
			return;

		m_fTimer = 0;
		m_fNextFrameCheck = TICK_TIME + (Math.RandomFloat01() * TICK_TIME * 0.5);
		bool playerPresentPreviously = m_bLocalPlayerPresent;
		m_bLocalPlayerPresent = GetIsLocalPlayerPresent();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (campaign && m_bLocalPlayerPresent != playerPresentPreviously)
			campaign.GetBaseManager().OnLocalPlayerPresenceChanged(this, m_bLocalPlayerPresent);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (RplSession.Mode() != RplMode.Dedicated)
			m_MapDescriptor = SCR_CampaignMilitaryBaseMapDescriptorComponent.Cast(GetOwner().FindComponent(SCR_CampaignMilitaryBaseMapDescriptorComponent));
		else
			return;

		if (!m_MapDescriptor)
			return;

		m_MapDescriptor.SetParentBase(this);
		m_MapDescriptor.Item().SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RadioComponent = BaseRadioComponent.Cast(GetOwner().FindComponent(BaseRadioComponent));
		m_SuppliesComponent = SCR_CampaignSuppliesComponent.Cast(owner.FindComponent(SCR_CampaignSuppliesComponent));

		if (m_fRadioRange == 0 && m_RadioComponent)
			m_fRadioRange = GetRelayRadioRange(m_RadioComponent);

		if (m_SuppliesComponent)
			m_SuppliesComponent.m_OnSuppliesChanged.Insert(OnSuppliesChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromCampaignBasesSystem();

		if (m_SuppliesComponent)
			m_SuppliesComponent.m_OnSuppliesChanged.Remove(OnSuppliesChanged);

		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(GetOwner().FindComponent(SCR_CampaignSeizingComponent));

		if (seizingComponent)
		{
			seizingComponent.GetOnCaptureStart().Remove(OnCaptureStart);
			seizingComponent.GetOnCaptureInterrupt().Remove(EndCapture);
		}

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (campaign)
		{
			campaign.GetOnFactionAssignedLocalPlayer().Remove(OnLocalPlayerFactionAssigned);
			campaign.GetBaseManager().GetOnAllBasesInitialized().Remove(OnAllBasesInitialized);
			campaign.GetOnCallsignOffsetChanged().Remove(OnCallsignAssigned);
		}

		super.OnDelete(owner);
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sBaseName", true)]
class SCR_CampaignCustomBase
{
	[Attribute("", UIWidgets.EditBox, "Base entity name as set up in World Editor.")]
	protected string m_sBaseName;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bIsControlPoint;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bCanBeHQ;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bDisableWhenUnusedAsHQ;

	[Attribute("-1", desc: "Use -1 for default base setting.")]
	protected float m_fRadioRange;

	//------------------------------------------------------------------------------------------------
	string GetBaseName()
	{
		return m_sBaseName;
	}

	//------------------------------------------------------------------------------------------------
	bool IsControlPoint()
	{
		return m_bIsControlPoint;
	}

	//------------------------------------------------------------------------------------------------
	bool GetCanBeHQ()
	{
		return m_bCanBeHQ;
	}

	//------------------------------------------------------------------------------------------------
	bool GetDisableWhenUnusedAsHQ()
	{
		return m_bDisableWhenUnusedAsHQ;
	}

	float GetRadioRange()
	{
		return m_fRadioRange;
	}
}

enum SCR_ECampaignBaseType
{
	BASE,
	RELAY
}

enum EFactionMapID
{
	UNKNOWN = 0,
	EAST = 1,
	WEST = 2,
	FIA = 3
}
