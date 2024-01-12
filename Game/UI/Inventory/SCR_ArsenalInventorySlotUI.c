//------------------------------------------------------------------------------------------------
class SCR_ArsenalInventorySlotUI : SCR_InventorySlotUI
{
	static const string SLOT_LAYOUT_SUPPLY = "{EA29CC1952F8B019}UI/layouts/Menus/Inventory/SupplyInventoryItemSlot.layout";
	
	protected const string RESOURCES_TEXT_WIDGET_NAME = "SuppliesText";
	protected const string STORED_RESOURCES_WIDGET_NAME = "SuppliesStored";
	protected const string AVAILABLE_RESOURCES_WIDGET_NAME = "SuppliesAvailable";
	protected const string COST_RESOURCES_WIDGET_NAME = "SuppliesCost";
	
	protected Widget m_CostResourceHolder;
	protected TextWidget m_CostResourceHolderText;
	
	protected bool m_bIsAvailable = true;
	protected TextWidget m_wResourceText;
	protected Widget m_wResourceDisplay;
	
	protected SCR_ResourceComponent m_ArsenalResourceComponent;
	protected SCR_ResourceContainer m_ResourceContainer;
	
	protected float m_fSupplyCost;
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceComponent GetArsenalResourceComponent()
	{
		return m_ArsenalResourceComponent;
	}
	
	void SetArsenalResourceComponent(SCR_ResourceComponent component)
	{
		m_ArsenalResourceComponent = component;
	}
	
    //------------------------------------------------------------------------------------------------
	void SetStorageUI(SCR_InventoryStorageBaseUI storageUI)
	{
		m_pStorageUI = storageUI;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected string SetSlotSize()
	{
		string slotLayout = SLOT_LAYOUT_SUPPLY;
        
		switch ( m_Attributes.GetItemSize() ) 
		{
			case ESlotSize.SLOT_1x1:
                m_iSizeX = 1;
                m_iSizeY = 1;

                break;
			case ESlotSize.SLOT_2x1:
                m_iSizeX = 2;
                m_iSizeY = 1;

                break;
			case ESlotSize.SLOT_2x2:
                m_iSizeX = 2; 
                m_iSizeY = 2;
                
                break;
			case ESlotSize.SLOT_3x3:
                m_iSizeX = 3;
                m_iSizeY = 3;
                
                break;
		}

		return slotLayout;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetSlotVisible(bool bVisible)
	{
		super.SetSlotVisible(bVisible);
		
		if (!m_wDimmerEffect)
			m_wDimmerEffect = m_widget.FindAnyWidget("Dimmer");
		
		if (!m_pItem || !m_pItem.GetOwner())
			return;
		
		//~ Never show Stored resources
		Widget resourceStored = m_widget.FindAnyWidget(STORED_RESOURCES_WIDGET_NAME);
		if (resourceStored)
			resourceStored.SetVisible(false);
		
		//~ Never show Available resources
		Widget resourceAvailable = m_widget.FindAnyWidget(AVAILABLE_RESOURCES_WIDGET_NAME);
		if (resourceAvailable)
			resourceAvailable.SetVisible(false);
		
		//~ Get cost widget
		m_CostResourceHolder = m_widget.FindAnyWidget(COST_RESOURCES_WIDGET_NAME);
		if (m_CostResourceHolder)
			m_CostResourceHolderText = TextWidget.Cast(m_CostResourceHolder.FindAnyWidget(RESOURCES_TEXT_WIDGET_NAME));
		
		if (!m_CostResourceHolder || !m_CostResourceHolderText)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(m_pItem.GetOwner());
		SCR_ResourceContainer resourceContainer;
		
		if (resourceComponent)
			resourceContainer = resourceComponent.GetContainer(EResourceType.SUPPLIES);
		
		UpdateTotalResources(GetTotalResources());
		m_CostResourceHolder.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Refresh()
	{
		super.Refresh();
		
		if (!m_pItem || !m_pItem.GetOwner())
			return;
		
		UpdateTotalResources(GetTotalResources());
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTotalResources(float totalResources)
	{
		if (!m_CostResourceHolderText)
			return;
		
		m_CostResourceHolderText.SetText(totalResources.ToString());
		
		if (!m_ArsenalResourceComponent)
		{
			SetItemAvailability(true);
			return;
		}
		
		SCR_ResourceConsumer consumer = m_ArsenalResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (!consumer)
		{	
			SetItemAvailability(true);
			return;
		}
		
		SetItemAvailability(totalResources <= consumer.GetAggregatedResourceValue());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsDraggable()
	{
		return m_bIsAvailable && super.IsDraggable();
	}
	
 	//------------------------------------------------------------------------------------------------
	float GetTotalResources()
	{
		if (!m_pItem || !m_pItem.GetOwner())
			return 0;
		
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return 0;
		
		IEntity storageEnt = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		
		SCR_Faction faction;
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(storageEnt.FindComponent(SCR_ArsenalComponent));
		if (arsenalComponent)
			faction = arsenalComponent.GetAssignedFaction();
		
		Resource resource = Resource.Load(m_pItem.GetOwner().GetPrefabData().GetPrefabName());
		
		SCR_EntityCatalogEntry entry;
		if (faction)
			entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, resource.GetResource().GetResourceName(), faction);
		else 
			entry = entityCatalogManager.GetEntryWithPrefabFromCatalog(EEntityCatalogType.ITEM, resource.GetResource().GetResourceName());
		
		if (!entry)
			return 0;
		
		SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!data)
			return 0;
		
		m_fSupplyCost = data.GetSupplyCost();;
		
		if (!m_ArsenalResourceComponent)
			return m_fSupplyCost;
		
		
		SCR_ResourceConsumer consumer = m_ArsenalResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (!consumer)
			return m_fSupplyCost;
		
		m_fSupplyCost = m_fSupplyCost * consumer.GetBuyMultiplier();
		
		return m_fSupplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetItemAvailability(bool isAvailable)
	{
		if (!m_wDimmerEffect)
			return;
		
		m_bIsAvailable = isAvailable;
		
		if (!m_wDimmerEffect.IsVisible() == isAvailable)
			return;
		
		m_wDimmerEffect.SetVisible(!isAvailable);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetItemSupplyCost()
	{
		return m_fSupplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ArsenalInventorySlotUI(InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null)
	{
	}
	
	//-------------------------------------S-----------------------------------------------------------
	void ~SCR_ArsenalInventorySlotUI()
	{
		SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Remove(this);
	}
};
