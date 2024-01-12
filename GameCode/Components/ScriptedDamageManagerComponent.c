//------------------------------------------------------------------------------------------------
class ScriptedDamageManagerComponentClass: BaseScriptedDamageManagerComponentClass
{
};

//------------------------------------------------------------------------------------------------
class ScriptedDamageManagerComponent : BaseScriptedDamageManagerComponent
{
	protected static const int MIN_MOMENTUM_RESPONSE_INDEX = 1;
	protected static const int MAX_MOMENTUM_RESPONSE_INDEX = 5;
	protected static const int MIN_DESTRUCTION_RESPONSE_INDEX = 6;
	protected const float SIMULATION_IMPRECISION_MULTIPLIER = 1.1;
	static const int MAX_DESTRUCTION_RESPONSE_INDEX = 10;
	static const string MAX_DESTRUCTION_RESPONSE_INDEX_NAME = "HugeDestructible";
	private static int s_iFirstFreeDamageManagerData = -1;
	private static ref array<ref SCR_ScriptedDamageManagerData> s_aScriptedDamageManagerData = {};
	
	protected int m_iTimetickInstigator = System.GetTickCount();
	protected int m_iTimeThresholdInstigatorReplacement = 180000; //180000 miliseconds = 3 minutes
	protected int m_iPlayerId = 0;
	
	private int m_iDamageManagerDataIndex = -1;
	
	//------------------------------------------------------------------------------------------------
	float CalculateMomentum(Contact contact, float ownerMass, float otherMass)
	{
		float dotMultiplier = vector.Dot(contact.VelocityAfter1.Normalized(), contact.VelocityBefore1.Normalized());
		float momentumBefore = ownerMass * contact.VelocityBefore1.Length() * SIMULATION_IMPRECISION_MULTIPLIER;
		float momentumAfter = ownerMass * contact.VelocityAfter1.Length() * dotMultiplier;
		float momentumA = Math.AbsFloat(momentumBefore - momentumAfter);
		
		dotMultiplier = vector.Dot(contact.VelocityAfter2.Normalized(), contact.VelocityBefore2.Normalized());
		momentumBefore = otherMass * contact.VelocityBefore2.Length() * SIMULATION_IMPRECISION_MULTIPLIER;
		momentumAfter = otherMass * contact.VelocityAfter2.Length() * dotMultiplier;
		float momentumB = Math.AbsFloat(momentumBefore - momentumAfter);
		return momentumA + momentumB;
	}
	
	//------------------------------------------------------------------------------------------------
	// This method uses similar logic to the logic of DamageSurroundingHitzones, but not the same.
	int GetSurroundingHitzones(vector origin, Physics physics, float maxDistance, out array<HitZone> outHitzones)
	{
		array<HitZone> hitzones = {};
		outHitzones = {};
		int hitzonesCount;
		
		int count = GetAllHitZones(hitzones);
		float maxDistanceSq = maxDistance * maxDistance; //SQUARE it for faster calculations of distance
		array<string> hitzoneColliderNames = {};
		
		float minDistance, currentDistance;
		int colliderCount, geomIndex;
		vector mat[4];
		for (int i = count - 1; i >= 0; i--)
		{
			minDistance = float.MAX;
			colliderCount = hitzones[i].GetAllColliderNames(hitzoneColliderNames); //The array is cleared inside the GetAllColliderNames method
			
			if (colliderCount == 0)
				continue;
			
			for (int y = colliderCount - 1; y >= 0; y--)
			{
				geomIndex = physics.GetGeom(hitzoneColliderNames[y]);
				if (geomIndex == -1)
					continue;
				
				physics.GetGeomWorldTransform(geomIndex, mat);
				currentDistance = vector.DistanceSq(origin, mat[3]);
				
				if (currentDistance < minDistance)
					minDistance = currentDistance;
			}
			
			if (minDistance > maxDistanceSq)
				continue;
			
			minDistance = Math.Sqrt(minDistance);
			
			hitzonesCount++;
			outHitzones.Insert(hitzones[i]);
		}
		
		return hitzonesCount;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\ param damageType determinese which damage multipliers are taken into account
	//! Made specifically for cases where hitzones are not parented 
	*/
	float GetMinDestroyDamage(EDamageType damageType, array<HitZone> hitzones, int count)
	{
		float damage;
		float damageMultiplier;
		HitZone defaultHitzone = GetDefaultHitZone();
		if (!IsDamageHandlingEnabled() || defaultHitzone.GetDamageMultiplier(damageType) * defaultHitzone.GetBaseDamageMultiplier() == 0)
			return -1; // invalid damage value, because this vehicle cannot be destroyed
		
		for (int i = 0; i < count; i++)
		{
			damageMultiplier = hitzones[i].GetDamageMultiplier(damageType) * hitzones[i].GetBaseDamageMultiplier();
			if (damageMultiplier != 0)
				damage += (hitzones[i].GetMaxHealth() + hitzones[i].GetDamageReduction()) / damageMultiplier;
		}
		
		return damage;
	}
	
	//------------------------------------------------------------------------------------------------
	private notnull SCR_ScriptedDamageManagerData GetScriptedDamageManagerData()
	{
		if (m_iDamageManagerDataIndex == -1)
			m_iDamageManagerDataIndex = AllocateScriptedDamageManagerData();
		
		return s_aScriptedDamageManagerData[m_iDamageManagerDataIndex];
	}
	
	//------------------------------------------------------------------------------------------------
	private int AllocateScriptedDamageManagerData()
	{
		if (s_iFirstFreeDamageManagerData == -1)
			return s_aScriptedDamageManagerData.Insert(new SCR_ScriptedDamageManagerData());
		else
		{
			int returnIndex = s_iFirstFreeDamageManagerData;
			SCR_ScriptedDamageManagerData data = s_aScriptedDamageManagerData[returnIndex];
			s_iFirstFreeDamageManagerData = data.m_iNextFreeIndex;
			data.m_iNextFreeIndex = -1;
			return returnIndex;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void FreeScriptedDamageManagerData(int index)
	{
		s_aScriptedDamageManagerData[index].Reset();
		s_aScriptedDamageManagerData[index].m_iNextFreeIndex = s_iFirstFreeDamageManagerData;
		s_iFirstFreeDamageManagerData = index;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamage()
	{
		return GetScriptedDamageManagerData().GetOnDamage();
	}	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageOverTimeAdded()
	{
		return GetScriptedDamageManagerData().GetOnDamageOverTimeAdded();
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageOverTimeRemoved()
	{
		return GetScriptedDamageManagerData().GetOnDamageOverTimeRemoved();
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageStateChanged()
	{
		return GetScriptedDamageManagerData().GetOnDamageStateChanged();
	}
		
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	override protected void OnDamage(
				  EDamageType type,
				  float damage,
				  HitZone pHitZone,
				  notnull Instigator instigator, 
				  inout vector hitTransform[3], 
				  float speed,
				  int colliderID, 
				  int nodeID)
	{
		if (m_iDamageManagerDataIndex == -1)
			return;
		
		ScriptInvoker invoker = s_aScriptedDamageManagerData[m_iDamageManagerDataIndex].GetOnDamage(false);
		if (invoker)
			invoker.Invoke(type, damage, pHitZone, instigator, hitTransform, speed, colliderID, nodeID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invoked every time the DoT is added to certain hitzone.
	override void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz)
	{
		super.OnDamageOverTimeAdded(dType,  dps,  hz);
		
		if (m_iDamageManagerDataIndex == -1)
			return;
		
		ScriptInvoker invoker = s_aScriptedDamageManagerData[m_iDamageManagerDataIndex].GetOnDamageOverTimeAdded(false);
		if (invoker)
			invoker.Invoke(dType,  dps,  hz);
	}
	
	//------------------------------------------------------------------------------------------------
	//!	Invoked when provided damage type is removed from certain hitzone.
	override void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		super.OnDamageOverTimeRemoved(dType, hz);
		
		if (m_iDamageManagerDataIndex == -1)
			return;
		
		ScriptInvoker invoker = s_aScriptedDamageManagerData[m_iDamageManagerDataIndex].GetOnDamageOverTimeRemoved(false);
		if (invoker)
			invoker.Invoke(dType, hz);
	}
	
	//------------------------------------------------------------------------------------------------
	//!	Invoked when damage state changes.
	protected override void OnDamageStateChanged(EDamageState state)
	{
		super.OnDamageStateChanged(state);
		
		if (m_iDamageManagerDataIndex == -1)
			return;
		
		ScriptInvoker invoker = s_aScriptedDamageManagerData[m_iDamageManagerDataIndex].GetOnDamageStateChanged(false);
		if (invoker)
			invoker.Invoke(state);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called whenever an instigator is going to be set.
	\param currentInstigator: This damage manager's last instigator
	\param newInstigator: The new instigator for this damage manager
	\return If it returns true, newInstigator will become the new current instigator for the damage manager and it will receive kill credit.
	*/
	protected override bool ShouldOverrideInstigator(notnull Instigator currentInstigator, notnull Instigator newInstigator)
	{
		//If time difference since last instigator set is small, this kill was a suicide, and the previous instigator was an enemy, do not override
		int currentTimeTick = System.GetTickCount();
		
		if (m_iPlayerId == 0)
		{
			m_iPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(GetOwner());
		}
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (m_iPlayerId <= 0 || !factionManager)
		{
			m_iTimetickInstigator = currentTimeTick;
			return true;
		}
		
		int newId = newInstigator.GetInstigatorPlayerID();
		int oldId = currentInstigator.GetInstigatorPlayerID();
		
		Faction factionKiller = Faction.Cast(factionManager.GetPlayerFaction(newId));
		if (!factionKiller)
		{
			m_iTimetickInstigator = currentTimeTick;
			return true;
		}
		
		Faction factionPrevInstigator = Faction.Cast(factionManager.GetPlayerFaction(oldId));
		if (!factionPrevInstigator)
		{
			m_iTimetickInstigator = currentTimeTick;
			return true;
		}
		Faction factionPlayer = Faction.Cast(factionManager.GetPlayerFaction(m_iPlayerId));
		if (!factionPlayer)
		{
			m_iTimetickInstigator = currentTimeTick;
			return true;
		}
		
		if (newId == m_iPlayerId && (currentTimeTick - m_iTimetickInstigator) < m_iTimeThresholdInstigatorReplacement && !factionPrevInstigator.IsFactionFriendly(factionPlayer))
		{
			return false;
		}
		
		m_iTimetickInstigator = currentTimeTick;
		return true;
	}
	//------------------------------------------------------------------------------------------------
	void ~ScriptedDamageManagerComponent()
	{
		if (m_iDamageManagerDataIndex != -1)
			FreeScriptedDamageManagerData(m_iDamageManagerDataIndex);
	}
};
