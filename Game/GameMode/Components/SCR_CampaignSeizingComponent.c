//------------------------------------------------------------------------------------------------
class SCR_CampaignSeizingComponentClass : SCR_SeizingComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignSeizingComponent : SCR_SeizingComponent
{
	[Attribute("60", params: "0 inf 0.1", category: "Campaign")]
	protected float m_fExtraTimePerService;

	protected SCR_CampaignMilitaryBaseComponent m_Base;
	
	//------------------------------------------------------------------------------------------------
	protected override SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		if (!m_Base || m_Base.IsHQ() || !m_Base.IsInitialized())
			return null;
		
		SCR_Faction faction = super.EvaluateEntityFaction(ent);
		
		if (!faction)
			return null;
		
		// Players of faction not covering this base with radio signal should not be able to capture or prevent capture
		SCR_CampaignFaction cFaction = SCR_CampaignFaction.Cast(faction);
		
		if (!cFaction)
			return null;
		
		if (faction.IsPlayable() && !m_Base.IsHQRadioTrafficPossible(cFaction))
			return null;
		
		return faction;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void RefreshSeizingTimer()
	{
		int servicesCount;

		if (m_Base)
		{
			array<SCR_EServicePointType> checkedTypes = {
				SCR_EServicePointType.ARMORY,
				SCR_EServicePointType.HELIPAD,
				SCR_EServicePointType.BARRACKS,
				SCR_EServicePointType.FUEL_DEPOT,
				SCR_EServicePointType.RADIO_ANTENNA,
				SCR_EServicePointType.FIELD_HOSPITAL,
				SCR_EServicePointType.LIGHT_VEHICLE_DEPOT,
				SCR_EServicePointType.HEAVY_VEHICLE_DEPOT
			};

			foreach (SCR_EServicePointType type : checkedTypes)
			{
				if (m_Base.GetServiceByType(type))
					servicesCount++;
			}
		}

		float seizingTimeVar = m_fMaximumSeizingTime - m_fMinimumSeizingTime;
		float deduct;

		if (m_iMaximumSeizingCharacters > 1)	// Avoid division by 0
		{
			float deductPerPlayer = seizingTimeVar / (m_iMaximumSeizingCharacters - 1);
			deduct = deductPerPlayer * (m_iSeizingCharacters - 1);
		}
		
		float servicesMultiplier = 1;
		
		if ((m_fMaximumSeizingTime - m_fMinimumSeizingTime) > 0)
			servicesMultiplier = 1 + (servicesCount * (m_fExtraTimePerService / (m_fMaximumSeizingTime - m_fMinimumSeizingTime)));

		#ifndef AR_CAMPAIGN_TIMESTAMP
		m_fSeizingEndTimestamp = m_fSeizingStartTimestamp + (servicesMultiplier * (m_fMaximumSeizingTime - deduct) * 1000);
		#else
		m_fSeizingEndTimestamp = m_fSeizingStartTimestamp.PlusSeconds(servicesMultiplier * (m_fMaximumSeizingTime - deduct));
		#endif

		if (m_bGradualTimerReset && m_fInterruptedCaptureDuration != 0)
			HandleGradualReset();
		
		Replication.BumpMe();
		OnSeizingTimestampChanged();
	}
	
	/*static void TestValues(float maxSeizingTime, float minSeizingTime, float extraTime, int players, int services)
	{
		float seizingTimeVar = maxSeizingTime - minSeizingTime;
		float deductPerPlayer = seizingTimeVar / 11;
		float deduct = deductPerPlayer * (players - 1);
		float servicesMultiplier = 1 + (services * (extraTime / (maxSeizingTime - minSeizingTime)));
		Print(servicesMultiplier * (maxSeizingTime - deduct));
	}*/
	
	//------------------------------------------------------------------------------------------------
	override void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
		super.OnBaseRegistered(base);
		
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		
		if (!campaignBase)
		{
			UnregisterBase(base);
			base.UnregisterLogicComponent(this);
		}
		
		if (!campaignBase || campaignBase.IsHQ())
			return;
		
		m_Base = campaignBase;
	}
};
