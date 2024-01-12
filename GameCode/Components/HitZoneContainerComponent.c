class SCR_HitZoneContainerComponentClass: HitZoneContainerComponentClass
{

};

class SCR_HitZoneContainerComponent: HitZoneContainerComponent
{
	protected bool m_bRplReady;
	
	//------------------------------------------------------------------------------------------------
	//! Check if replication loading is completed. Important for join in progress and when streaming entities in.
	bool IsRplReady()
	{
		return m_bRplReady;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event bool OnRplSave(ScriptBitWriter writer)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		SCR_FlammableHitZone flammableHitZone;
		foreach (HitZone hitZone: hitZones)
		{
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (flammableHitZone)
				writer.WriteInt(flammableHitZone.GetFireState());
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event bool OnRplLoad(ScriptBitReader reader)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		SCR_FlammableHitZone flammableHitZone;
		EFireState fireState;
		foreach (HitZone hitZone: hitZones)
		{
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (flammableHitZone)
			{
				reader.ReadInt(fireState);
				flammableHitZone.SetFireState(fireState);
			}
		}
		
		m_bRplReady = true;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*! Get the HitZone that matches the provided name. Case sensitivity is optional.
	\param hitZoneName String name of hitzone
	\param caseSensitive Case sensitivity
	\return hitZone HitZone matching the provided name
	*/
	HitZone GetHitZoneByName(string hitZoneName, bool caseSensitive = false)
	{
		if (hitZoneName.IsEmpty())
			return null;
		
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		foreach (HitZone hitZone: hitZones)
		{
			if (hitZone && hitZoneName.Compare(hitZone.GetName(), caseSensitive) == 0)
				return hitZone;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return hitzones with colliders assigned
	void GetPhysicalHitZones(out notnull array<HitZone> physicalHitZones)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		foreach (HitZone hitZone: hitZones)
		{
			if (hitZone && hitZone.HasColliderNodes())
				physicalHitZones.Insert(hitZone);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set fire rate of a flammable hitzone
	\param hitZoneIndex Index of the hitzone to set fire rate for
	\param fireRate Rate of fire to be applied
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SetFireState(int hitZoneIndex, EFireState fireState)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(hitZones.Get(hitZoneIndex));
		if (flammableHitZone)
			flammableHitZone.SetFireState(fireState);
	}
};
