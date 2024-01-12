//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CampaignStruct : SCR_JsonApiStruct
{
	protected int m_iHours = -1;
	protected int m_iMinutes = -1;
	protected int m_iSeconds = -1;
	protected vector m_vMHQLocationBLUFOR;
	protected vector m_vMHQRotationBLUFOR;
	protected vector m_vMHQLocationOPFOR;
	protected vector m_vMHQRotationOPFOR;
	protected ref array<ref SCR_CampaignBaseStruct> m_aBasesStructs = {};
	protected ref array<ref SCR_CampaignRemnantInfoStruct> m_aRemnantsStructs = {};
	protected ref array<ref SCR_CampaignPlayerStruct> m_aPlayerStructs = {};
	protected int m_iTutorialStage = -1;
	protected bool m_bMatchOver;
	protected string m_sWeatherState;
	protected int m_iCallsignOffset = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;
	
	//------------------------------------------------------------------------------------------------
	int GetHours()
	{
		return m_iHours;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMinutes()
	{
		return m_iMinutes;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSeconds()
	{
		return m_iSeconds;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetMHQLocationBLUFOR()
	{
		return m_vMHQLocationBLUFOR;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetMHQRotationBLUFOR()
	{
		return m_vMHQRotationBLUFOR;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetMHQLocationOPFOR()
	{
		return m_vMHQLocationOPFOR;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetMHQRotationOPFOR()
	{
		return m_vMHQRotationOPFOR;
	}
	
	//------------------------------------------------------------------------------------------------
	array <ref SCR_CampaignBaseStruct>GetBasesStructs()
	{
		return m_aBasesStructs;
	}	
		
	//------------------------------------------------------------------------------------------------
	array <ref SCR_CampaignRemnantInfoStruct>GetRemnantsStructs()
	{
		return m_aRemnantsStructs;
	}
	
	//------------------------------------------------------------------------------------------------
	array <ref SCR_CampaignPlayerStruct>GetPlayersStructs()
	{
		return m_aPlayerStructs;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTutorialStage()
	{
		return m_iTutorialStage;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsMatchOver()
	{
		return m_bMatchOver;
	}		
	
	//------------------------------------------------------------------------------------------------
	string GetWeatherState()
	{
		return m_sWeatherState;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCallsignOffset()
	{
		return m_iCallsignOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Serialize()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return false;
		
		m_bMatchOver = campaign.GetIsMatchOver();
		
		Clear();
		
		ChimeraWorld world = campaign.GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		
		if (timeManager)
		{
			timeManager.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
			WeatherStateTransitionManager transitionManager = timeManager.GetTransitionManager();
			
			if (transitionManager)
				m_sWeatherState = transitionManager.GetCurrentState().GetStateName();
		}
		
		campaign.GetBaseManager().StoreBasesStates(m_aBasesStructs);
		
		SCR_CampaignTutorialArlandComponent tutorial = SCR_CampaignTutorialArlandComponent.Cast(campaign.FindComponent(SCR_CampaignTutorialArlandComponent));
		
		if (tutorial)
		{
			m_iTutorialStage = tutorial.GetStage();
			return true;
		}
		
		campaign.StoreRemnantsStates(m_aRemnantsStructs);
		
		SCR_CampaignFaction factionBLUFOR = campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignFaction factionOPFOR = campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);
		IEntity mobileHQ;
		
		if (factionBLUFOR && factionBLUFOR.GetMobileAssembly())
		{
			mobileHQ = factionBLUFOR.GetMobileAssembly().GetVehicle();
			
			if (mobileHQ)
			{
				m_vMHQLocationBLUFOR = mobileHQ.GetOrigin();
				m_vMHQRotationBLUFOR = mobileHQ.GetYawPitchRoll();
			}
		}
		
		if (factionOPFOR && factionOPFOR.GetMobileAssembly())
		{
			mobileHQ = factionOPFOR.GetMobileAssembly().GetVehicle();
			
			if (mobileHQ)
			{
				m_vMHQLocationOPFOR = mobileHQ.GetOrigin();
				m_vMHQRotationOPFOR = mobileHQ.GetYawPitchRoll();
			}
		}
		
		campaign.WriteAllClientsData();
		array<ref SCR_CampaignClientData> clients = {};
		campaign.GetClientsData(clients);
		
		foreach (SCR_CampaignClientData data : clients)
		{
			SCR_CampaignPlayerStruct struct = new SCR_CampaignPlayerStruct();
			struct.SetID(data.GetID());
			struct.SetXP(data.GetXP());
			struct.SetFactionIndex(data.GetFactionIndex());
			
			m_aPlayerStructs.Insert(struct);
		}
		
		m_iCallsignOffset = campaign.GetCallsignOffset();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return false;
		
		campaign.StoreLoadedData(this);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_aBasesStructs.Clear();
		m_aRemnantsStructs.Clear();
		m_aPlayerStructs.Clear();
		m_vMHQLocationBLUFOR = vector.Zero;
		m_vMHQRotationBLUFOR = vector.Zero;
		m_vMHQLocationOPFOR = vector.Zero;
		m_vMHQRotationOPFOR = vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the id of the given resource
	//! {76A14F180887D76E}Prefabs/Compositions/Slotted/SlotFlatSmall/AmmoStorage_S_US_01.et
	//! >> {76A14F180887D76E}
	static string GetResourceId(string resourceName)
	{
		int i = resourceName.IndexOf("}");
		
		if (i <= 0)
			return string.Empty;
	
		return resourceName.Substring(0, resourceName.IndexOf("}") + 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignStruct()
	{
		RegV("m_iHours");
		RegV("m_iMinutes");
		RegV("m_iSeconds");
		RegV("m_vMHQLocationBLUFOR");
		RegV("m_vMHQRotationBLUFOR");
		RegV("m_vMHQLocationOPFOR");
		RegV("m_vMHQRotationOPFOR");
		RegV("m_aBasesStructs");
		RegV("m_aRemnantsStructs");
		RegV("m_aPlayerStructs");
		RegV("m_iTutorialStage");
		RegV("m_bMatchOver");
		RegV("m_sWeatherState");
		RegV("m_iCallsignOffset");
	}
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBaseStruct : SCR_JsonApiStruct
{
	// We need to keep these short so save file size is manageable
	protected bool m_bIsHQ;
	protected int m_iCSI;
	protected vector m_vPos;
	protected int m_iF;
	protected int m_iS;
	/*protected string m_sHQP;
	protected vector m_vHQPos;
	protected vector m_vHQRot;
	protected string m_sVDP;
	protected vector m_vVDPos;
	protected vector m_vVDRot;
	protected string m_sArmP;
	protected vector m_vArmPos;
	protected vector m_vArmRot;
	protected string m_sHVDP;
	protected vector m_vHVDPos;
	protected vector m_vHVDRot;
	protected string m_sSDP;
	protected vector m_vSDPos;
	protected vector m_vSDRot;
	protected string m_sAP;
	protected vector m_vAPos;
	protected vector m_vARot;	
	protected string m_sFHP;
	protected vector m_vFHPos;
	protected vector m_vFHRot;
	protected string m_sBP;
	protected vector m_vBPos;
	protected vector m_vBRot;*/
	
	//------------------------------------------------------------------------------------------------
	bool IsHQ()
	{
		return m_bIsHQ;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCallsignIndex()
	{
		return m_iCSI;
	}
	
	vector GetPosition()
	{
		return m_vPos;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSupplies()
	{
		return m_iS;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFaction()
	{
		return m_iF;
	}
	
	//------------------------------------------------------------------------------------------------
	/*string GetHQPrefab()
	{
		return m_sHQP;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetHQPosition()
	{
		return m_vHQPos;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetHQRotation()
	{
		return m_vHQRot;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetServicePrefab(SCR_EServicePointType type)
	{
		switch (type)
		{
			case SCR_EServicePointType.LIGHT_VEHICLE_DEPOT: {return m_sVDP;};
			case SCR_EServicePointType.ARMORY: {return m_sArmP;};
			case SCR_EServicePointType.HEAVY_VEHICLE_DEPOT: {return m_sHVDP;};
			case SCR_EServicePointType.SUPPLY_DEPOT: {return m_sSDP;};
			case SCR_EServicePointType.RADIO_ANTENNA: {return m_sAP;};
			case SCR_EServicePointType.FIELD_HOSPITAL: {return m_sFHP;};
			case SCR_EServicePointType.BARRACKS: {return m_sBP;};
		}
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetServicePosition(SCR_EServicePointType type)
	{
		switch (type)
		{
			case SCR_EServicePointType.LIGHT_VEHICLE_DEPOT: {return m_vVDPos;};
			case SCR_EServicePointType.ARMORY: {return m_vArmPos;};
			case SCR_EServicePointType.HEAVY_VEHICLE_DEPOT: {return m_vHVDPos;};
			case SCR_EServicePointType.SUPPLY_DEPOT: {return m_vSDPos;};
			case SCR_EServicePointType.RADIO_ANTENNA: {return m_vAPos;};
			case SCR_EServicePointType.FIELD_HOSPITAL: {return m_vFHPos;};
			case SCR_EServicePointType.BARRACKS: {return m_vBPos;};
		}
		
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetServiceRotation(SCR_EServicePointType type)
	{
		switch (type)
		{
			case SCR_EServicePointType.LIGHT_VEHICLE_DEPOT: {return m_vVDRot;};
			case SCR_EServicePointType.ARMORY: {return m_vArmRot;};
			case SCR_EServicePointType.HEAVY_VEHICLE_DEPOT: {return m_vHVDRot;};
			case SCR_EServicePointType.SUPPLY_DEPOT: {return m_vSDRot;};
			case SCR_EServicePointType.RADIO_ANTENNA: {return m_vARot;};
			case SCR_EServicePointType.FIELD_HOSPITAL: {return m_vFHRot;};
			case SCR_EServicePointType.BARRACKS: {return m_vBRot;};
		}
		
		return vector.Zero;
	}*/
	
	//------------------------------------------------------------------------------------------------
	void SetIsHQ(bool hq)
	{
		m_bIsHQ = hq;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCallsignIndex(int callsign)
	{
		m_iCSI = callsign;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPosition(vector position)
	{
		m_vPos = position;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSupplies(int supplies)
	{
		m_iS = supplies;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOwningFaction(int owningFaction)
	{
		m_iF = owningFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	/*void SetBuildingsData(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		array<SCR_ServicePointComponent> services = {};
		base.GetServices(services);
		
		IEntity composition = SCR_EntityHelper.GetMainParent(base.GetHQRadio());
		EntityPrefabData data;
		ResourceName prefab;
		
		if (composition)
		{
			data = composition.GetPrefabData();
			
			if (data)
			{
				prefab = data.GetPrefabName();
				
				if (!prefab.IsEmpty())
				{
					m_sHQP = SCR_CampaignStruct.GetResourceId(prefab);
					m_vHQPos = composition.GetOrigin();
					m_vHQRot = composition.GetYawPitchRoll();
				}
			}
		}
		
		foreach (SCR_ServicePointComponent service : services)
		{
			composition = SCR_EntityHelper.GetMainParent(service.GetOwner(), true);
			
			if (!composition)
				continue;
			
			data = composition.GetPrefabData();
			
			if (!data)
				continue;
			
			prefab = data.GetPrefabName();
			
			if (prefab.IsEmpty())
				continue;
			
			switch (service.GetType())
			{
				case SCR_EServicePointType.LIGHT_VEHICLE_DEPOT:
				{
					m_sVDP = SCR_CampaignStruct.GetResourceId(prefab);
					m_vVDPos = composition.GetOrigin();
					m_vVDRot = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.ARMORY:
				{
					m_sArmP = SCR_CampaignStruct.GetResourceId(prefab);
					m_vArmPos = composition.GetOrigin();
					m_vArmRot = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.HEAVY_VEHICLE_DEPOT:
				{
					m_sHVDP = SCR_CampaignStruct.GetResourceId(prefab);
					m_vHVDPos = composition.GetOrigin();
					m_vHVDRot = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.SUPPLY_DEPOT:
				{
					m_sSDP = SCR_CampaignStruct.GetResourceId(prefab);
					m_vSDPos = composition.GetOrigin();
					m_vSDRot = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.RADIO_ANTENNA:
				{
					m_sAP = SCR_CampaignStruct.GetResourceId(prefab);
					m_vAPos = composition.GetOrigin();
					m_vARot = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.FIELD_HOSPITAL:
				{
					m_sFHP = SCR_CampaignStruct.GetResourceId(prefab);
					m_vFHPos = composition.GetOrigin();
					m_vFHRot = composition.GetYawPitchRoll();
					break;
				}
				
				case SCR_EServicePointType.BARRACKS:
				{
					m_sBP = SCR_CampaignStruct.GetResourceId(prefab);
					m_vBPos = composition.GetOrigin();
					m_vBRot = composition.GetYawPitchRoll();
					break;
				}
			}
		}
	}*/
	
	//------------------------------------------------------------------------------------------------
	override void OnExpand()
	{
		Print("OnExpand()");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnItemObject(int index, string name)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnObject( string name )
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError(int errorCode)
	{
		Print("OnError: " + errorCode);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess( int errorCode )
	{
        Print("OnSuccess: " + errorCode );
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignBaseStruct()
	{
		RegV("m_bIsHQ");
		RegV("m_iCSI");
		RegV("m_vPos");
		RegV("m_iF");
		RegV("m_iS");
		/*RegV("m_sHQP");
		RegV("m_vHQPos");
		RegV("m_vHQRot");
		RegV("m_sVDP");
		RegV("m_vVDPos");
		RegV("m_vVDRot");
		RegV("m_sArmP");
		RegV("m_vArmPos");
		RegV("m_vArmRot");
		RegV("m_sHVDP");
		RegV("m_vHVDPos");
		RegV("m_vHVDRot");
		RegV("m_sSDP");
		RegV("m_vSDPos");
		RegV("m_vSDRot");
		RegV("m_sAP");
		RegV("m_vAPos");
		RegV("m_vARot");
		RegV("m_sFHP");
		RegV("m_vFHPos");
		RegV("m_vFHRot");
		RegV("m_sBP");
		RegV("m_vBPos");
		RegV("m_vBRot");*/
	}
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignRemnantInfoStruct : SCR_JsonApiStruct
{
	protected int m_iID;
	protected int m_iSize;
	protected float m_fR;
	
	//------------------------------------------------------------------------------------------------
	int GetID()
	{
		return m_iID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMembersAlive()
	{
		return m_iSize;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRespawnTimer()
	{
		return m_fR;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetID(int ID)
	{
		m_iID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMembersAlive(int count)
	{
		m_iSize = count;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRespawnTimer(float timer)
	{
		m_fR = timer;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignRemnantInfoStruct()
	{
		RegV("m_iID");
		RegV("m_iSize");
		RegV("m_fR");
	}
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignPlayerStruct : SCR_JsonApiStruct
{
	static const string LOCAL_PLAYER_IDENTITY_ID = "identitySP";
	
	protected string m_sID;
	protected int m_iF = -1;
	protected int m_iXP;
	
	//------------------------------------------------------------------------------------------------
	string GetID()
	{
		return m_sID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFactionIndex()
	{
		return m_iF;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetXP()
	{
		return m_iXP;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetID(string ID)
	{
		m_sID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFactionIndex(int index)
	{
		m_iF = index;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetXP(int xp)
	{
		m_iXP = xp;
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetPlayerIdentity(int playerId)
	{
		if (playerId <= 0)
			return string.Empty;
		
		if (RplSession.Mode() == RplMode.None)
			return LOCAL_PLAYER_IDENTITY_ID;
		
		BackendApi api = GetGame().GetBackendApi();
		
		if (!api)
			return string.Empty;
		
		return api.GetPlayerIdentityId(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignPlayerStruct()
	{
		RegV("m_sID");
		RegV("m_iF");
		RegV("m_iXP");
	}
};
