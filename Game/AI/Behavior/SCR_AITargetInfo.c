enum EAITargetInfoCategory
{
	UNKNOWN,
	DESTROYED,	// Destroyed or deleted
	DISARMED,	// Disarmed
	LOST,		// Lost - not seen for a lot of time
	DETECTED,	// Was detected (heard)
	IDENTIFIED	// Was seen visually
}

enum EAITargetInfoFlags
{
}

class SCR_AITargetInfo
{
	IEntity m_Entity;
	Faction m_Faction;
	DamageManagerComponent m_DamageManager;
	PerceivableComponent m_Perceivable;
	vector m_vWorldPos;
	float m_fTimestamp; // Perception mgr time
	
	EAITargetInfoCategory m_eCategory;
	
	//----------------------------------------------------------------------------------------------------------
	void Init(IEntity entity = null,
		vector worldPos = vector.Zero,
		float timestamp = 0.0,
		EAITargetInfoCategory category = 0)
	{
		m_Entity = entity;
		m_vWorldPos = worldPos;
		m_fTimestamp = timestamp;	
		m_eCategory = category;
		
		if (entity)
		{
			// Init m_Faction
			FactionAffiliationComponent fcomp;
			
			SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
			if (character)
				fcomp = character.m_pFactionComponent; // Common case
			else
			{
				Vehicle veh = Vehicle.Cast(entity);
				if (veh)
					fcomp = veh.m_pFactionComponent;
				else
					fcomp = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent)); // Last resort
			}
			
			if (fcomp)
				m_Faction = fcomp.GetAffiliatedFaction();
			
			// Init pointers to components
			m_DamageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
			m_Perceivable = PerceivableComponent.Cast(entity.FindComponent(PerceivableComponent));
		}
	}
	
	//----------------------------------------------------------------------------------------------------------
	void InitFromBaseTarget(BaseTarget tgt)
	{
		IEntity entity = tgt.GetTargetEntity();
		vector pos;
		float timestamp;
		EAITargetInfoCategory category;
		if (tgt.GetTargetCategory() == ETargetCategory.ENEMY)
		{
			pos = tgt.GetLastSeenPosition();
			timestamp = tgt.GetTimeLastSeen();
			category = EAITargetInfoCategory.IDENTIFIED; 
		}
		else
		{
			pos = tgt.GetLastDetectedPosition();
			timestamp = tgt.GetTimeLastDetected();
			category = EAITargetInfoCategory.DETECTED;
		}
		Init(entity, pos, timestamp, category);
	}
	
	//----------------------------------------------------------------------------------------------------------
	void InitFromGunshot(IEntity entity, vector posWorld, float timestamp)
	{
		Init(entity, posWorld, timestamp, category: EAITargetInfoCategory.DETECTED);
	}
	
	//----------------------------------------------------------------------------------------------------------
	void UpdateFromBaseTarget(BaseTarget baseTarget)
	{
		ETargetCategory category = baseTarget.GetTargetCategory();

		if (category == ETargetCategory.ENEMY)
			m_eCategory = EAITargetInfoCategory.IDENTIFIED;
		else if (category == ETargetCategory.DETECTED)
			m_eCategory = EAITargetInfoCategory.DETECTED;
		
		// Update timestamp and position
		if (category == ETargetCategory.ENEMY)
		{
			m_vWorldPos = baseTarget.GetLastSeenPosition();
			m_fTimestamp = baseTarget.GetTimeLastSeen();
		}
		else
		{
			m_vWorldPos = baseTarget.GetLastDetectedPosition();
			m_fTimestamp = baseTarget.GetTimeLastDetected();
		}
	}
	
	//----------------------------------------------------------------------------------------------------------
	void UpdateFromGunshot(vector worldPos, float timestamp)
	{
		m_vWorldPos = worldPos;
		m_fTimestamp = timestamp;
		
		if (m_eCategory == EAITargetInfoCategory.LOST)
			m_eCategory = EAITargetInfoCategory.DETECTED;
	}
	
	//----------------------------------------------------------------------------------------------------------
	void CopyFrom(SCR_AITargetInfo other)
	{
		m_Entity = other.m_Entity;
		m_vWorldPos = other.m_vWorldPos;
		m_fTimestamp = other.m_fTimestamp;
		m_eCategory = other.m_eCategory;
		m_Faction = other.m_Faction;
	}
};
