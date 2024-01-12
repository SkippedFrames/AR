#include "scripts/Game/config.c"
[EntityEditorProps(category: "GameScripted/Campaign", description: "Allow respawning at this vehicle in Campaign", color: "0 0 255 255")]
class SCR_CampaignMobileAssemblyComponentClass: ScriptComponentClass
{
	[Attribute("{6D282026AB95FC81}Prefabs/MP/Campaign/CampaignMobileAssemblySpawnpoint.et", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sSpawnpointPrefab;

	//------------------------------------------------------------------------------------------------
	ResourceName GetSpawnpointPrefab()
	{
		return m_sSpawnpointPrefab;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignMobileAssemblyComponent : ScriptComponent
{
	static const float MAX_WATER_DEPTH = 2.5;
	
	protected RplComponent m_RplComponent;
	
	protected SCR_CampaignFaction m_ParentFaction;
	
	protected bool m_bIsInRadioRange;
	
	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aBasesInRadioRange = {};
	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aBasesInRadioRangeOld = {};
	
	protected SCR_SpawnPoint m_SpawnPoint;
	
	protected SCR_CampaignMobileAssemblyStandaloneComponent m_StandaloneComponent;

	[RplProp(onRplName: "OnParentFactionIDSet")]
	protected int m_iParentFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;
	
	[RplProp(onRplName: "OnSpawnpointCreated")]
	protected int m_iSpawnpointId = RplId.Invalid();
	
	[RplProp(onRplName: "OnDeployChanged")]
	protected bool m_bIsDeployed;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		super.OnPostInit(owner);
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		GetGame().GetCallqueue().CallLater(UpdateRadioCoverage, 1000, true);
		
		if (IsProxy())
		{
			campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Insert(OnParentFactionIDSet);
			campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Insert(OnDeployChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		GetGame().GetCallqueue().Remove(UpdateRadioCoverage);
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Remove(OnParentFactionIDSet);
		campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Remove(OnDeployChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBasesInRadioRange()
	{
		array<SCR_MilitaryBaseComponent> bases = {};
		SCR_MilitaryBaseManager.GetInstance().GetBases(bases);
		
		float radioRange = GetRadioRange();
		radioRange = radioRange * radioRange;	// We're checking square distance
		vector truckPosition = GetOwner().GetOrigin();
		SCR_CampaignMilitaryBaseComponent campaignBase;
		
		m_aBasesInRadioRange = {};
		
		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			
			if (!campaignBase || !campaignBase.IsInitialized())
				continue;
			
			if (campaignBase.IsHQ() && campaignBase.GetFaction() != m_ParentFaction)
				continue;
			
			if (vector.DistanceSqXZ(truckPosition, campaignBase.GetOwner().GetOrigin()) < radioRange)
				m_aBasesInRadioRange.Insert(campaignBase);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanReachByRadio(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		return m_aBasesInRadioRange.Contains(base);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParentFactionID(int factionID)
	{
		m_iParentFaction = factionID;
		Replication.BumpMe();
		OnParentFactionIDSet();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnParentFactionIDSet()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		m_ParentFaction = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(m_iParentFaction));
	}
	
	//------------------------------------------------------------------------------------------------
	int GetParentFactionID()
	{
		return m_iParentFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetParentFaction()
	{
		return m_ParentFaction;
	}

	//------------------------------------------------------------------------------------------------
	int GetBasesInRange(notnull out array<SCR_CampaignMilitaryBaseComponent> basesInRange)
	{
		return basesInRange.Copy(m_aBasesInRadioRange);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRadioRange()
	{
		IEntity truck = GetOwner().GetParent();
		
		if (!truck)
			return 0;
		
		BaseRadioComponent comp = BaseRadioComponent.Cast(truck.FindComponent(BaseRadioComponent));
		
		if (!comp)
			return 0;
		
		BaseTransceiver tsv = comp.GetTransceiver(0);
		
		if (!tsv)
			return 0;
		
		return tsv.GetRange();
	}
	
	//------------------------------------------------------------------------------------------------
	bool Deploy(SCR_EMobileAssemblyStatus status, int playerId = 0)
	{
		if (!m_bIsInRadioRange && status == SCR_EMobileAssemblyStatus.DEPLOYED)
			return false;
		
		if (!m_ParentFaction)
			return false;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return false;
		
		if (m_ParentFaction.GetMobileAssembly() && status == SCR_EMobileAssemblyStatus.DEPLOYED)
			return false;
		
		if (status == SCR_EMobileAssemblyStatus.DEPLOYED)
		{
			CreateSpawnpoint();
			m_bIsDeployed = true;
		}
		else
		{
			m_bIsDeployed = false;
			
			if (m_SpawnPoint)
				RplComponent.DeleteRplEntity(m_SpawnPoint, false);
			
			campaign.GetBaseManager().RecalculateRadioCoverage(m_ParentFaction);
		}
		
		OnDeployChanged();
		Replication.BumpMe();
		campaign.BroadcastMHQFeedback(status, playerId, GetGame().GetFactionManager().GetFactionIndex(m_ParentFaction));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnDeployChanged()
	{
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		BaseRadioComponent radioComponent;
		
		IEntity vehicle = GetOwner().GetParent();
		
		if (vehicle)
			radioComponent = BaseRadioComponent.Cast(vehicle.FindComponent(BaseRadioComponent));
		
		m_aBasesInRadioRangeOld.Copy(m_aBasesInRadioRange);
		
		if (m_bIsDeployed)
		{
			UpdateBasesInRadioRange();

			if (!IsProxy())
			{
				if (radioComponent)
					radioComponent.SetPower(true);
				
				if (GetTaskManager())
				{
					SCR_CampaignTaskSupportEntity supportClass = SCR_CampaignTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignTaskSupportEntity));
					
					if (supportClass)
						supportClass.GenerateCaptureTasks(GetOwner());
				}
				
				GetGame().GetCallqueue().CallLater(CheckStatus, 500, true);
			}
		}
		else
		{
			m_aBasesInRadioRange = {};

			if (!IsProxy())
			{
				if (radioComponent)
					radioComponent.SetPower(false);
				
				GetGame().GetCallqueue().Remove(CheckStatus);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckStatus()
	{
		IEntity truck = GetOwner().GetParent();
		
		if (!truck)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
			
		if (!campaign)
			return;
		
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(truck.FindComponent(DamageManagerComponent));
		
		// Destroyed?
		if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
		{
			Deploy(SCR_EMobileAssemblyStatus.DESTROYED);
			
			return;
		}
		
		// Moved?
		Physics physicsComponent = truck.GetPhysics();
		
		if (!physicsComponent)
			return;
		
		vector vel = physicsComponent.GetVelocity();
		vel[1] = 0;
		
		if (vel.LengthSq() <= 0.01)
			return;
		
		Deploy(SCR_EMobileAssemblyStatus.DISMANTLED);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSpawnpointCreated()
	{
		// Delay so spawnpoint has time to be streamed in for clients
		GetGame().GetCallqueue().CallLater(RegisterSpawnpoint, 1000);
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterSpawnpoint()
	{
		// On server the assignment is done in CreateSpawnpoint()
		if (!IsProxy())
			return;
		
		m_SpawnPoint = SCR_SpawnPoint.Cast(Replication.FindItem(m_iSpawnpointId));
		
		if (!m_SpawnPoint)
			return;
		
		m_StandaloneComponent = SCR_CampaignMobileAssemblyStandaloneComponent.Cast(m_SpawnPoint.FindComponent(SCR_CampaignMobileAssemblyStandaloneComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMobileAssemblyStandaloneComponent GetStandaloneComponent()
	{
		return m_StandaloneComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDeployed()
	{
		return m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsInRadioRange()
	{
		return m_bIsInRadioRange;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateRadioCoverage()
	{
		if (!m_ParentFaction)
			return;
		
		bool inRangeNow = SCR_GameModeCampaign.GetInstance().GetBaseManager().IsEntityInFactionRadioSignal(GetOwner(), m_ParentFaction);
		bool refreshLinks = inRangeNow != m_bIsInRadioRange;
		m_bIsInRadioRange = inRangeNow;
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateSpawnpoint()
	{
		SCR_CampaignMobileAssemblyComponentClass componentData = SCR_CampaignMobileAssemblyComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return;
		
		Resource spawnpointResource = Resource.Load(componentData.GetSpawnpointPrefab());
		
		if (!spawnpointResource || !spawnpointResource.IsValid())
			return;
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		GetOwner().GetTransform(params.Transform);
		m_SpawnPoint = SCR_SpawnPoint.Cast(GetGame().SpawnEntityPrefab(spawnpointResource, null, params));
		
		if (!m_SpawnPoint)
			return;
		
		m_StandaloneComponent = SCR_CampaignMobileAssemblyStandaloneComponent.Cast(m_SpawnPoint.FindComponent(SCR_CampaignMobileAssemblyStandaloneComponent));

		if (m_StandaloneComponent)
		{
			m_StandaloneComponent.SetRadioRange(GetRadioRange());
			m_StandaloneComponent.SetVehicle(SCR_EntityHelper.GetMainParent(GetOwner(), true));
			
			// Delay so map item can initialize
			GetGame().GetCallqueue().CallLater(m_StandaloneComponent.SetParentFactionID, SCR_GameModeCampaign.MINIMUM_DELAY, false, m_iParentFaction);
		}
		
		m_iSpawnpointId = Replication.FindId(m_SpawnPoint);
		OnSpawnpointCreated();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignMobileAssemblyComponent()
	{
		if (Replication.IsClient())
			return;
		
		if (m_SpawnPoint)
			RplComponent.DeleteRplEntity(m_SpawnPoint, false);
		
		if (!m_ParentFaction || !IsDeployed())
			return;
		
		Deploy(SCR_EMobileAssemblyStatus.DISMANTLED);
	}
};

enum SCR_EMobileAssemblyStatus
{
	DEPLOYED,
	DISMANTLED,
	DESTROYED
};
