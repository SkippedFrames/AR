//------------------------------------------------------------------------------------------------
class SCR_VehicleTagData : SCR_NameTagData
{
	bool m_bIsControlledPresent = false; 			// is player within this vehicle
	ref array<SCR_NameTagData> m_aPassengers = {};	// tracked passangers, meaning those who have their own nametag
	
	//------------------------------------------------------------------------------------------------
	//! Update current tracked passangers within the vehicle
	void UpdatePassenger(SCR_NameTagData tag, bool IsEntering, bool isControlledEntity = false)
	{
		if (IsEntering)
		{
			if (!m_aPassengers.Contains(tag))
				m_aPassengers.Insert(tag);
			
			if (m_Flags & ENameTagFlags.VEHICLE_DISABLE)		// in case we disabled last frame and added new passenger before the nametag was cleared, remove the flag
			{
				m_Flags &= ~ENameTagFlags.VEHICLE_DISABLE;
				m_Flags &= ~ENameTagFlags.DISABLED;
			}
			
			tag.m_VehicleParent = this;
			
			if (isControlledEntity)
				m_bIsControlledPresent = true;
		}
		else
		{
			tag.m_VehicleParent = null;
			m_aPassengers.RemoveItem(tag);
			if (m_aPassengers.IsEmpty())
			{
				m_Flags |= (ENameTagFlags.VEHICLE_DISABLE | ENameTagFlags.DISABLED);
			}
			
			if (isControlledEntity)
				m_bIsControlledPresent = false;
		}
		
		m_Flags |= ENameTagFlags.NAME_UPDATE;
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetName(out string name, out notnull array<string> nameParams)
	{
		int count = m_aPassengers.Count();
		if (count <= 0)
		{
			m_sName = string.Empty;
			nameParams.Copy(m_aNameParams);
			return;
		}
		else 
		{
			m_aPassengers[0].GetName(m_sName, nameParams);

			if (m_sName == string.Empty)	// passenger tag might need entity update in case of lost connection 
			{
				m_aPassengers[0].UpdateEntityType();
				m_aPassengers[0].GetName(m_sName, nameParams);
			}
			
			if (count > 1)
				m_sName = m_sName + "  (+" + (count - 1).ToString() + ")";
		}
			
		name = m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool UpdateEntityStateFlags()
	{
		m_Flags = ENameTagFlags.DISABLED | ENameTagFlags.NAME_UPDATE;	// this has a default flag because if tag never reaches a visibile state, it needs a disable flag for clean up
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateTagPos()
	{		
		vector origin = m_Entity.GetOrigin();
		vector mins, maxs;
		m_Entity.GetBounds(mins, maxs);
		float height = maxs[1];
		maxs = {0, height/2, 0};
		
		m_vEntWorldPos = origin + maxs;		// angling & tracing
		m_vEntHeadPos = m_vEntWorldPos;

		maxs = {0, height + height/5 , 0};	// visual display
		m_vTagWorldPos = origin + maxs;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void InitDefaults()
	{
		super.InitDefaults();

		m_bIsControlledPresent = false;
		m_aPassengers.Clear();
	}

};
