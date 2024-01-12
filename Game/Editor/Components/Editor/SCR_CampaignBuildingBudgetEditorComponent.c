[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingBudgetEditorComponentClass : SCR_BudgetEditorComponentClass
{
};

class SCR_CampaignBuildingBudgetEditorComponent : SCR_BudgetEditorComponent
{
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	protected SCR_ECharacterRank m_eHighestRank;
	
	protected static const int UNLIMITED_PROP_BUDGET = -1; 

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorActivateServer()
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		array<ref SCR_RankID> ranks = factionManager.GetAllAvailableRanks();
		if (!ranks)
			return;

		ranks.Sort();

		SCR_RankID highestRankID = ranks[ranks.Count() - 1];
		if (!highestRankID)
			return;

		m_eHighestRank = highestRankID.GetRankID();
		
		RefreshBudgetSettings();
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorActivate()
	{
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
		if (!m_CampaignBuildingComponent)
			return;

		RefreshBudgetSettings();
		m_CampaignBuildingComponent.GetOnProviderChanged().Insert(OnTargetBaseChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorDeactivate()
	{
		if (m_CampaignBuildingComponent)
			m_CampaignBuildingComponent.GetOnProviderChanged().Remove(OnTargetBaseChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void GetMainBudgetType(out array<ref EEditableEntityBudget> budgets)
	{
		budgets.Insert(EEditableEntityBudget.CAMPAIGN);
		budgets.Insert(EEditableEntityBudget.RANK_CORPORAL);
		budgets.Insert(EEditableEntityBudget.RANK_SERGEANT);
		
		if (!m_CampaignBuildingComponent)
			return;
		
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(m_CampaignBuildingComponent.GetProviderComponent());
		if (!providerComponent)
			return;
		
		if (providerComponent.GetMaxPropValue() != UNLIMITED_PROP_BUDGET)
			budgets.Insert(EEditableEntityBudget.PROPS);
		
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("SCR_CampaignBuildingBudgetEditorComponent.RefreshResourcesComponent() should be used instead.")]
	protected bool RefreshSuppliesComponent()
	{
		if (!m_CampaignBuildingComponent)
			return false;

		SCR_CampaignSuppliesComponent providerSuppliesComponent;
		if (m_CampaignBuildingComponent.GetProviderSuppliesComponent(providerSuppliesComponent))
		{
			if (m_SuppliesComponent)
			{
				if (m_SuppliesComponent != providerSuppliesComponent)
				{
					m_SuppliesComponent.m_OnSuppliesChanged.Remove(OnBaseSuppliesChanged);
				}
			}
			else
			{
				providerSuppliesComponent.m_OnSuppliesChanged.Insert(OnBaseSuppliesChanged);
			}
		}

		m_SuppliesComponent = providerSuppliesComponent;

		return m_SuppliesComponent != null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseResourcesChanged(SCR_ResourceConsumer consumer, int previousValue)
	{
		RefreshResourcesComponent();
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("SCR_CampaignBuildingBudgetEditorComponent.OnBaseResourcesChanged() should be used instead.")]
	protected void OnBaseSuppliesChanged(int supplies)
	{
		RefreshResourcesComponent();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTargetBaseChanged(IEntity targetEntity)
	{
		RefreshBudgetSettings();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetMaxBudgetValue(EEditableEntityBudget type, out int maxBudget)
	{
		switch (type)
		{
			case EEditableEntityBudget.CAMPAIGN:
				if (!m_ResourceComponent)
					return true;
				
				SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				
				if (consumer)
					maxBudget = consumer.GetAggregatedMaxResourceValue();
				
				return true;
			case EEditableEntityBudget.RANK_CORPORAL:
				maxBudget = m_eHighestRank;
				return true;
			case EEditableEntityBudget.RANK_SERGEANT:
				maxBudget = m_eHighestRank;
				return true;
			case EEditableEntityBudget.PROPS:
				maxBudget = GetProviderMaxPropValue();
				return true;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override int GetCurrentBudgetValue(EEditableEntityBudget type)
	{
		switch (type)
		{
			case EEditableEntityBudget.CAMPAIGN:
				if (!m_ResourceComponent)
					return -1;
				
				SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				
				if (!consumer)
					return -1;
				
				return consumer.GetAggregatedMaxResourceValue() - consumer.GetAggregatedResourceValue();
			case EEditableEntityBudget.RANK_CORPORAL:
				return m_eHighestRank - GetUserRank();
			case EEditableEntityBudget.RANK_SERGEANT:
				return m_eHighestRank - GetUserRank();
			case EEditableEntityBudget.PROPS:
				return GetCurrentProviderPropValue();
		}

		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return max prop value for a current provider. This number limits the number of prefabs (compositions) buildable with this provider.
	int GetProviderMaxPropValue()
	{
		if (!m_CampaignBuildingComponent)
			return UNLIMITED_PROP_BUDGET;
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return UNLIMITED_PROP_BUDGET;
		
		return providerComponent.GetMaxPropValue();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return current prop value for a current provider. This number limits the number of prefabs (compositions) buildable with this provider.
	int GetCurrentProviderPropValue()
	{
		if (!m_CampaignBuildingComponent)
			return UNLIMITED_PROP_BUDGET;
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return UNLIMITED_PROP_BUDGET;
		
		return providerComponent.GetCurrentPropValue();
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetUserRank()
	{
		int playerId = SCR_PlayerController.GetLocalPlayerId();

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return SCR_ECharacterRank.INVALID;
		
		return SCR_CharacterRankComponent.GetCharacterRank(playerController.GetControlledEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected override void OnEntityCoreBudgetUpdatedOwner(EEditableEntityBudget entityBudget, int budgetValue, int budgetChange, bool sendBudgetMaxEvent, bool budgetMaxReached)
	{
		int maxBudget;
		GetMaxBudgetValue(EEditableEntityBudget.PROPS, maxBudget);
		Event_OnBudgetUpdated.Invoke(EEditableEntityBudget.PROPS, GetCurrentBudgetValue(EEditableEntityBudget.PROPS), budgetValue, maxBudget);
		
		if (entityBudget != EEditableEntityBudget.CAMPAIGN)
			return;

		if (!m_ResourceComponent)
			return;
		
		SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		
		if (!consumer)
			return;
		
		if (consumer.GetAggregatedResourceValue() - budgetChange <= 0)
			Event_OnBudgetMaxReached.Invoke(entityBudget, true);
	}

	//------------------------------------------------------------------------------------------------
	protected bool RefreshResourcesComponent()
	{
		if (!m_CampaignBuildingComponent)
			return false;

		SCR_ResourceComponent providerResourceComponent;
		
		if (m_CampaignBuildingComponent.GetProviderResourceComponent(providerResourceComponent))
		{
			SCR_ResourceConsumer consumerNew = providerResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
			SCR_ResourceConsumer consumerOld;
			
			if (m_ResourceComponent)
				consumerOld = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
			
			if (consumerNew && consumerOld)
			{
				if (consumerNew != consumerOld)
					consumerOld.GetOnResourcesChanged().Remove(OnBaseResourcesChanged);
			}
			else if (consumerNew)
				consumerNew.GetOnResourcesChanged().Insert(OnBaseResourcesChanged);
		}

		m_ResourceComponent = providerResourceComponent;

		return m_ResourceComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void RefreshBudgetSettings()
	{
		array<ref EEditableEntityBudget> budgetsType = {};
		
		GetMainBudgetType(budgetsType);

		SCR_EditableEntityCoreBudgetSetting budget;
		
		for (int i = budgetsType.Count() - 1; i >= 0; i--)
		{
			EEditableEntityBudget budgetType = budgetsType[i];
			
			int maxBudgetValue;
			
			if (m_EntityCore.GetBudget(budgetType, budget) && GetMaxBudgetValue(budgetType, maxBudgetValue))
				m_BudgetSettingsMap.Insert(budgetType, budget);
		}

		RefreshResourcesComponent();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanPlaceEntitySource(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget, bool isPlacingPlayer = false, bool updatePreview = true, bool showNotification = true)
	{
		bool canPlace = true;
		if (IsBudgetCapEnabled() && editableEntitySource)
		{
			array<ref SCR_EntityBudgetValue> budgetCosts = {};
			if (!GetEntitySourcePreviewBudgetCosts(editableEntitySource, budgetCosts))
			{
				canPlace = false;
			}
			// Clear budget cost when placing as player
			if (isPlacingPlayer)
			{
				budgetCosts.Clear();
			}

			if (updatePreview)
			{
				UpdatePreviewCost(budgetCosts);
			}

			canPlace = canPlace && CanPlace(budgetCosts, blockingBudget);
		}
		return CanPlaceResultCampaignBuilding(canPlace, showNotification, blockingBudget);
	}

	protected bool CanPlaceResultCampaignBuilding(bool canPlace, bool showNotification, EEditableEntityBudget blockingBudget)
	{
		if (showNotification)
		{
			Rpc(CanPlaceOwnerCampaignBuilding, canPlace, blockingBudget);
		}
		return canPlace;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void CanPlaceOwnerCampaignBuilding(bool canPlace, EEditableEntityBudget blockingBudget)
	{
		if (!canPlace)
		{
			switch (blockingBudget)
			{
				case EEditableEntityBudget.CAMPAIGN:
				{
					GetManager().SendNotification(ENotification.EDITOR_PLACING_NO_ENOUGH_SUPPLIES);
					break;
				};
				case EEditableEntityBudget.RANK_CORPORAL:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_CORPORAL_NEEDED, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.CORPORAL);
					break;
				};
				case EEditableEntityBudget.RANK_SERGEANT:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_SERGEANT_NEEDED, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.SERGEANT);
					break;
				};
				case EEditableEntityBudget.PROPS:
				{
					GetManager().SendNotification(ENotification.EDITOR_PLACING_NO_MORE_COMPOSITIONS_AT_BASE);
					break;
				};
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool GetMaxBudget(EEditableEntityBudget type, out SCR_EntityBudgetValue budget)
	{
		foreach	(SCR_EntityBudgetValue maxBudget : m_MaxBudgets)
		{
			if (maxBudget.GetBudgetType() != type)
				continue;
			
			int newMaxBudget;

			switch (type)
			{
				case EEditableEntityBudget.CAMPAIGN:
				{
					if (!m_ResourceComponent)
						break;
					
					SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
					
					if (!consumer)
						break;
					
					newMaxBudget = consumer.GetAggregatedResourceValue();
					
					break;
				};
				case EEditableEntityBudget.RANK_CORPORAL:
				{
					newMaxBudget = GetUserRank();
					break;
				};
				case EEditableEntityBudget.RANK_SERGEANT:
				{
					newMaxBudget = GetUserRank();
					break;
				};
				case EEditableEntityBudget.PROPS:
				{					
					newMaxBudget = GetProviderMaxPropValue();
					break;
				};
			}

			maxBudget.SetBudgetValue(newMaxBudget);

			budget = maxBudget;

			return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the composition can be placed. 
	override bool CanPlace(notnull array<ref SCR_EntityBudgetValue> budgetCosts, out EEditableEntityBudget blockingBudget)
	{
		if (!IsBudgetCapEnabled() || budgetCosts.IsEmpty()) 
			return true;
		
		foreach (SCR_EntityBudgetValue budgetCost : budgetCosts)
		{
			if (!CanPlace(budgetCost, blockingBudget))
			{
				if (IsBudgetCapEnabled(blockingBudget))
					return false;
			};
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the given budget is on the list of budgets used by a provider.
	bool IsBudgetCapEnabled(EEditableEntityBudget blockingBudget)
	{
		if (!m_CampaignBuildingComponent)
			return true;
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return true;
		
		return providerComponent.IsBudgetToEvaluate(blockingBudget);
	}
};
