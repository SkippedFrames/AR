[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Manager that holds a reference to all Support Stations.")]
class SCR_SupportStationManagerComponentClass : ScriptComponentClass
{
	
};
class SCR_SupportStationManagerComponent : ScriptComponent
{
	// List of all support stations
	protected ref map<ESupportStationType, ref array<SCR_BaseSupportStationComponent>> m_mSupportStations = new  map<ESupportStationType, ref array<SCR_BaseSupportStationComponent>>();
	
	protected static SCR_SupportStationManagerComponent s_Instance;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get instance of SCR_SupportStationManagerComponent from GameMode
	\return SupportStationManager
	*/
	static SCR_SupportStationManagerComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get array of all Support stations of given type
	protected int GetArrayOfSupportStationType(ESupportStationType type, notnull out array<SCR_BaseSupportStationComponent> supportStations)
	{
		supportStations.Clear();
		
		//~ Add new type to make sure it is never null
		if (!m_mSupportStations.Find(type, supportStations))
		{
			m_mSupportStations.Insert(type, new ref array<SCR_BaseSupportStationComponent>);
			supportStations = m_mSupportStations[type];
		}	
		
		return supportStations.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns closest valid support station of given type. Will only get support stations of highest availible priority
	\param type Support Station Type
	\param actionUser User that started the action
	\param actionOwner entity on which the action manager was executed
	\param[out] reasonInvalid Reason why interaction was invalid (Will always take the highest reason why)
	\param[out] supply cost of closest valid support station, -1 if supplies are not enabled or if no support station is valid
	\return Closest valid support station
	*/
	SCR_BaseSupportStationComponent GetClosestValidSupportStation(ESupportStationType type, notnull IEntity actionUser, notnull IEntity actionOwner, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyCost)
	{
		array<SCR_BaseSupportStationComponent> supportStations = {};
		
		GetArrayOfSupportStationType(type, supportStations);
		
		reasonInvalid = ESupportStationReasonInvalid.NOT_IN_RANGE;
		ESupportStationReasonInvalid newReasonInvalid;
		
		SCR_BaseSupportStationComponent closestSupportStation = null;
		ESupportStationPriority validPriority = -1;
		float closestDistance = float.MAX;
		int minSuppliesRequired = int.MAX;
		int suppliesRequired, validSupplyCost;
		
		foreach (SCR_BaseSupportStationComponent station: supportStations)
		{
			if (!station)
				continue;
			
			//~ Found all support stations of same priority and closest support station is already set
			if (closestSupportStation != null && station.GetSupportStationPriority() < validPriority)
				break;
			
			//~ Check if support station is valid
			if (station.IsValid(actionOwner, actionUser, action, actionPosition, newReasonInvalid, suppliesRequired))
			{
				//~ station is valid so make sure to save the priority level which makes sure on the same priority stations are ever checked
				if (validPriority < 0)
					validPriority = station.GetSupportStationPriority();
				
				//~ Get distance to user
				float distancesq = vector.DistanceSq(actionPosition, station.GetPosition());
				
				//~ Support station is closer to user so set this as valid return
				if (distancesq < closestDistance)
				{
					closestDistance = distancesq;
					closestSupportStation = station;
					validSupplyCost = suppliesRequired;
				}
			}
			//~ Support station was invalid set new invalid reason if it has an higher priority
			else if (newReasonInvalid > reasonInvalid)
			{
				reasonInvalid = newReasonInvalid;
			}
			
			//~ Supply station was valid but not enough supplies
			if (newReasonInvalid == ESupportStationReasonInvalid.NO_SUPPLIES && minSuppliesRequired > suppliesRequired)
				minSuppliesRequired = suppliesRequired;
		}
		
		//~ Set supply cost to closest station cost
		if (closestSupportStation)
			supplyCost = validSupplyCost;
		//~ No valid supply station as not enough supplies to execute. Still set min supplies needed to execute
		else if (newReasonInvalid == ESupportStationReasonInvalid.NO_SUPPLIES)
			supplyCost = minSuppliesRequired;
		//~ No valid supply station and reason was not lack of supplies
		else 
			supplyCost = -1;
		
		return closestSupportStation;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Add support station to manager
	\param supportStation Support Station to add
	*/
	void AddSupportStation(notnull SCR_BaseSupportStationComponent supportStation)
	{
		if (supportStation.GetSupportStationType() == ESupportStationType.NONE)
		{
			Print("Cannot add SupportStation of type NONE! Make sure to overwrite GetSupportStationType() in inherited class.", LogLevel.ERROR);
			return;
		}
		
		array<SCR_BaseSupportStationComponent> supportStations = {};
		GetArrayOfSupportStationType(supportStation.GetSupportStationType(), supportStations);
		
		//~ Already in list		
		if (supportStations.Contains(supportStation))
			return;
		
		for (int i = 0, count = supportStations.Count(); i < count; i++)
		{
			//~ Make sure it is added near the at priority in the array
			if (supportStation.GetSupportStationPriority() >= supportStations[i].GetSupportStationPriority())
			{
				//~ Insert in list at the correct location
				m_mSupportStations[supportStation.GetSupportStationType()].InsertAt(supportStation, i);
				return;
			}
		}
		
		//~ Add to last position in array as loop above did not add it
		m_mSupportStations[supportStation.GetSupportStationType()].Insert(supportStation);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Remove support station from manager
	\param supportStation Support Station to remove
	*/
	void RemoveSupportStation(notnull SCR_BaseSupportStationComponent supportStation)
	{
		if (supportStation.GetSupportStationType() == ESupportStationType.NONE)
			return;
		
		array<SCR_BaseSupportStationComponent> supportStations = {};
		GetArrayOfSupportStationType(supportStation.GetSupportStationType(), supportStations);
		
		int index = supportStations.Find(supportStation);
		
		//~ Not in list
		if (index < 0)
			return;
		
		//~ Remove from list
		 m_mSupportStations[supportStation.GetSupportStationType()].RemoveOrdered(index);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Used in DamageSystemSupport station to return a rough percentage of the health the given damageManagercompnent has. 
	Take note that this will never be a precise number and it will ignore the default hitZone until all other hitZone percentage is greater than it
	\param damageManagerComponent Damage Manager to check
	\param maxHealPercentage How much can be healed max 1 == 100%
	\param[out] roughHealPercentageStatus Rough percentage of health of the entity to display to the player the status
	\param[out] allHitZonesMaxHealth True if all hitZones are max health using maxHealPercentage
	\param hitZonesToIgnore HitZones to ignore when checking health
	*/
	static void GetCombinedHitZonesStateForDamageSupportStation(notnull SCR_DamageManagerComponent damageManagerComponent, notnull array<HitZone> hitZones, float maxHealPercentage, out float roughHealPercentageStatus, out bool allHitZonesMaxHealth = true)
	{
		HitZone defaultHitZone = damageManagerComponent.GetDefaultHitZone();
		
		//~ Out values
		allHitZonesMaxHealth = true;
		roughHealPercentageStatus = 0;
		
		int hitZonesChecked = 0;
		bool defaultHitZoneIncluded = false;
		
		foreach (HitZone hitZone : hitZones)
		{
			if (defaultHitZone == hitZone)
			{
				defaultHitZoneIncluded = true;
				continue;
			}
				
			
			//~ Get percentage of hitZone and add it to the pool
			hitZonesChecked++;
			roughHealPercentageStatus += hitZone.GetHealthScaled();
			
			//~ Check if entity can be healed more by this support station if not yet set false
			if (allHitZonesMaxHealth && hitZone.GetHealthScaled() < maxHealPercentage)
				allHitZonesMaxHealth = false;
		}
		
		//~ Get average health of all hitZones that can be healed
		if (hitZonesChecked > 0)
			roughHealPercentageStatus = roughHealPercentageStatus / hitZonesChecked;
		else 
			roughHealPercentageStatus = 1;
		
		if (defaultHitZoneIncluded && defaultHitZone)
		{
			//~ If parts heal is higher than defualt hitZone health use default hitZone instead
			if (roughHealPercentageStatus > defaultHitZone.GetHealthScaled())
				roughHealPercentageStatus = defaultHitZone.GetHealthScaled();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (s_Instance)
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		s_Instance = SCR_SupportStationManagerComponent.Cast(gameMode.FindComponent(SCR_SupportStationManagerComponent));		
	}
};
