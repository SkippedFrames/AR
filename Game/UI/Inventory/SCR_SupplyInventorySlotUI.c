//------------------------------------------------------------------------------------------------
class SCR_SupplyInventorySlotUI : SCR_InventorySlotUI
{
	const string SLOT_LAYOUT_SUPPLY = "{EA29CC1952F8B019}UI/layouts/Menus/Inventory/SupplyInventoryItemSlot.layout";	
	const float SLOT_UNAVAILABLE_OPACITY = 0.35;
	
	protected const string RESOURCES_TEXT_WIDGET_NAME = "SuppliesText";
	protected const string STORED_RESOURCES_WIDGET_NAME = "SuppliesStored";
	protected const string AVAILABLE_RESOURCES_WIDGET_NAME = "SuppliesAvailable";
	protected const string COST_RESOURCES_WIDGET_NAME = "SuppliesCost";
	
	protected Widget m_StoredResourcesHolder;
	protected TextWidget m_StoredResourcesText;
	
	protected Widget m_AvailableResourcesHolder;
	protected TextWidget m_AvailableResourcesText;
	
	//~ What supplies type are displayed
	protected EResourceType m_eResourceType = EResourceType.SUPPLIES;

	//~ ArsenalInventorySlotUI is the cost one
	
	protected LocalizedString m_sCurrentAndMaxResourceFormat = "#AR-Campaign_BaseSuppliesAmount";
	
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected ref SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandle;
	
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
	override void SetItemFunctionality()
	{
		if (m_pItem.GetOwner().FindComponent(BaseInventoryStorageComponent))
			m_eSlotFunction = ESlotFunction.TYPE_STORAGE;			
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetSlotVisible(bool bVisible)
	{
		super.SetSlotVisible(bVisible);
		
		if (!m_wDimmerEffect)
			m_wDimmerEffect = m_widget.FindAnyWidget("Dimmer");
		
		m_StoredResourcesHolder = m_widget.FindAnyWidget(STORED_RESOURCES_WIDGET_NAME);
		if (m_StoredResourcesHolder)
			m_StoredResourcesText = TextWidget.Cast(m_StoredResourcesHolder.FindAnyWidget(RESOURCES_TEXT_WIDGET_NAME));
		
		m_AvailableResourcesHolder = m_widget.FindAnyWidget(AVAILABLE_RESOURCES_WIDGET_NAME);
		if (m_AvailableResourcesHolder)
			m_AvailableResourcesText = TextWidget.Cast(m_AvailableResourcesHolder.FindAnyWidget(RESOURCES_TEXT_WIDGET_NAME));
		
		//~ Never show cost
		Widget resourceCost = m_widget.FindAnyWidget(COST_RESOURCES_WIDGET_NAME);
		if (resourceCost)
			resourceCost.SetVisible(false);
		
		Refresh();
      }
	
	//------------------------------------------------------------------------------------------------
	override void Refresh()
	{
		super.Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateConsumer()
	{
		if (!m_ResourceComponent)
			return;
		
		float totalResources, maxResources;
		
		//~ Stored supplies
		bool showUI = SCR_ResourceSystemHelper.GetMaxAndStoredResources(m_ResourceComponent, totalResources, maxResources, m_eResourceType);
		UpdateStoredResources(showUI, totalResources, maxResources);
		
		//~ Available Supplies
		showUI = SCR_ResourceSystemHelper.GetAvailableResources(m_ResourceComponent, totalResources, m_eResourceType);
		UpdateAvailableResources(showUI, totalResources);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateStoredResources(bool showUI, float totalResources, float maxResources)
	{
		if (!m_StoredResourcesHolder || !m_StoredResourcesText)
			return;
		
		if (!showUI)
		{
			m_StoredResourcesHolder.SetVisible(false);
			return;
		}
		
		//~ Update supplies
		m_StoredResourcesText.SetTextFormat(m_sCurrentAndMaxResourceFormat, totalResources.ToString(), maxResources.ToString());
		m_StoredResourcesHolder.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateAvailableResources(bool showUI, float totalResources)
	{
		if (!m_AvailableResourcesHolder || !m_AvailableResourcesText)
			return;
		
		if (!showUI)
		{
			m_AvailableResourcesHolder.SetVisible(false);
			return;
		}
		
		//~ Update supplies
		m_AvailableResourcesText.SetText(totalResources.ToString());
		m_AvailableResourcesHolder.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		m_ResourceSubscriptionHandle = null;	
		
		if (m_ResourceComponent)
			m_ResourceComponent.TEMP_GetOnInteractorReplicated().Remove(UpdateConsumer);
	}
	
	void SCR_SupplyInventorySlotUI(InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null)
	{
		if (!m_pItem)
			return;
		
		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(m_pItem.GetOwner());
		if (!m_ResourceComponent)
			return;
	
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Insert(UpdateConsumer);;
		UpdateConsumer();
		
		m_ResourceConsumer = SCR_ResourceSystemHelper.GetFirstValidConsumer(m_ResourceComponent);
		if (!m_ResourceConsumer)
			return;
		
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryPlayerComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourceInventoryPlayerComponent)
			return;
		
		RplId resourceInventoryPlayerComponentRplId = Replication.FindId(resourceInventoryPlayerComponent);
		
		if (!resourceInventoryPlayerComponentRplId.IsValid())
			return;
		
		m_ResourceSubscriptionHandle = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandle(m_ResourceConsumer, resourceInventoryPlayerComponentRplId);
		
	}
};
