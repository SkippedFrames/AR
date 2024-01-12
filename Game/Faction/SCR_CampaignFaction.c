#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
class SCR_CampaignFaction : SCR_Faction
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Defenders group prefab", "et")]
	private ResourceName m_DefendersGroupPrefab;

	[Attribute("", params: "et")]
	protected ref array<ResourceName> m_aStartingVehicles;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	private ResourceName m_MobileHQPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "For radio operators", "et")]
	private ResourceName m_RadioPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "HQ composition in small bases", "et")]
	private ResourceName m_BaseBuildingHQ;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition", "et")]
	private ResourceName m_BaseBuildingSupplyDepot;
	
	protected SCR_CampaignMilitaryBaseComponent m_MainBase;
	protected SCR_CampaignMilitaryBaseComponent m_PrimaryTarget;
	
	protected SCR_CampaignMobileAssemblyStandaloneComponent m_MobileAssembly;
	
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fVictoryTimestamp;
	protected float m_fPauseByBlockTimestamp;
	#else
	protected WorldTimestamp m_fVictoryTimestamp;
	protected WorldTimestamp m_fPauseByBlockTimestamp;
	#endif
	
	protected int m_iActiveRespawnRadios;
	protected int m_iControlPointsHeld;
	
	//------------------------------------------------------------------------------------------------
	void GetStartingVehiclePrefabs(out notnull array<ResourceName> prefabs)
	{
		prefabs.Copy(m_aStartingVehicles);
	}
	
	//------------------------------------------------------------------------------------------------
	void SendHQMessage(SCR_ERadioMsg msgType, int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN, int calledID = SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX, bool public = true, int param = SCR_CampaignRadioMsg.INVALID_RADIO_MSG_PARAM)
	{
		if (msgType == SCR_ERadioMsg.NONE)
			return;
		
		SCR_CampaignMilitaryBaseComponent HQ = GetMainBase();
		
		if (!HQ)
			return;
		
		BaseRadioComponent radio = BaseRadioComponent.Cast(HQ.GetOwner().FindComponent(BaseRadioComponent));
		
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		
		if (!transmitter)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));
		
		if (!callsignManager)
			return;
		
		IEntity called = GetGame().GetPlayerManager().GetPlayerControlledEntity(calledID);
		int companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex;
		
		if (called && !callsignManager.GetEntityCallsignIndexes(called, companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex))
	    	return;
		
		SCR_CampaignRadioMsg msg = new SCR_CampaignRadioMsg;
		msg.SetRadioMsg(msgType);
		msg.SetFactionId(GetGame().GetFactionManager().GetFactionIndex(this));
		msg.SetBaseCallsign(baseCallsign);
		msg.SetCalledCallsign(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex);
		msg.SetIsPublic(public);
		msg.SetParam(param);
		msg.SetPlayerID(calledID);
		msg.SetEncryptionKey(radio.GetEncryptionKey());

		transmitter.BeginTransmission(msg);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetControlPointsHeld(int count)
	{
		m_iControlPointsHeld = count;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetControlPointsHeld()
	{
		return m_iControlPointsHeld;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActiveRespawnRadios(int count)
	{
		m_iActiveRespawnRadios = count;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActiveRespawnRadios()
	{
		return m_iActiveRespawnRadios;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMainBase(SCR_CampaignMilitaryBaseComponent mainBase)
	{
		m_MainBase = mainBase;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent target)
	{
		m_PrimaryTarget = target;
		
		// Give tasks time to get created before refreshing the priorities
		GetGame().GetCallqueue().CallLater(RefreshTaskPriorities, SCR_GameModeCampaign.DEFAULT_DELAY);
	}
	
	//------------------------------------------------------------------------------------------------
	void RefreshTaskPriorities()
	{
		SCR_BaseTaskManager taskManager = GetTaskManager();

		if (!taskManager)
			return;
		
		SCR_CampaignMilitaryBaseManager baseManager = SCR_GameModeCampaign.GetInstance().GetBaseManager();

		if (!baseManager)
			return;

		array<SCR_BaseTask> tasks = {};
		taskManager.GetFilteredTasks(tasks, this);

		foreach (SCR_BaseTask task : tasks)
		{
			SCR_CampaignTask conflictTask = SCR_CampaignTask.Cast(task);

			if (!conflictTask)
				continue;

			SCR_CampaignMilitaryBaseComponent base = conflictTask.GetTargetBase();

			if (!base || base.GetFaction() == conflictTask.GetTargetFaction())
				continue;
			
			conflictTask.SetIsPriority(m_PrimaryTarget == base);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetPrimaryTarget()
	{
		return m_PrimaryTarget;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMobileAssembly(SCR_CampaignMobileAssemblyStandaloneComponent mobileAssembly)
	{
		m_MobileAssembly = mobileAssembly;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRadioPrefab()
	{
		return m_RadioPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefendersGroupPrefab()
	{
		return m_DefendersGroupPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetMobileHQPrefab()
	{
		return m_MobileHQPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetBuildingPrefab(EEditableEntityLabel type)
	{
		switch (type)
		{
			case EEditableEntityLabel.SERVICE_HQ: {return m_BaseBuildingHQ;};
			case EEditableEntityLabel.SERVICE_SUPPLY_STORAGE: {return m_BaseBuildingSupplyDepot;};
		}
		
		return ResourceName.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFactionNameUpperCase()
	{
		SCR_FactionUIInfo UI = SCR_FactionUIInfo.Cast(GetUIInfo());
		
		if (UI)
			return UI.GetFactionNameUpperCase();
		else
			return "";
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetMainBase()
	{
		return m_MainBase;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMobileAssemblyStandaloneComponent GetMobileAssembly()
	{
		return m_MobileAssembly;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetVictoryTimestamp(float timestamp)
	#else
	void SetVictoryTimestamp(WorldTimestamp timestamp)
	#endif
	{
		m_fVictoryTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetVictoryTimestamp()
	#else
	WorldTimestamp GetVictoryTimestamp()
	#endif
	{
		return m_fVictoryTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	void SetPauseByBlockTimestamp(float timestamp)
	#else
	void SetPauseByBlockTimestamp(WorldTimestamp timestamp)
	#endif
	{
		m_fPauseByBlockTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	#ifndef AR_CAMPAIGN_TIMESTAMP
	float GetPauseByBlockTimestamp()
	#else
	WorldTimestamp GetPauseByBlockTimestamp()
	#endif
	{
		return m_fPauseByBlockTimestamp;
	}
};
