[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "Inventory 2.0 Storages List UI class")]

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout - Shows the storages slots on player ( backpack, ALICE, vest, etc )
class SCR_InventoryStoragesListUI : SCR_InventoryStorageBaseUI
{
	protected ref array<SCR_UniversalInventoryStorageComponent>		m_UniStorages			= new ref array<SCR_UniversalInventoryStorageComponent>();
	protected ref array<IEntity>									m_Items					= new ref array<IEntity>();
	
	//TODO: move to inventory manager
	protected const ResourceName INVENTORY_CONFIG = "{024B56A4DE577001}Configs/Inventory/InventoryUI.conf";
	protected ref SCR_InventoryConfig m_pInventoryUIConfig;
		
	
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
		
	//------------------------------------------------------------------------------------------------
	//!
	SCR_InventorySlotUI GetUISlotBySlotID( ESlotID slotID )
	{
		return m_aSlots.Get( slotID );
	}
	
	SCR_InventorySlotUI FindItemSlot(InventoryItemComponent item)
	{
		for (int i = 0; i < m_aSlots.Count(); i++)
		{
			SCR_InventorySlotUI pSlot = m_aSlots.Get(i);
			if ( pSlot )
			{
				if ( pSlot.GetInventoryItemComponent() == item )
				return pSlot;
			}
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override void InitPaging() { return; }
	
	
	//------------------------------------------------------------------------------------------------
	// ! 
	// TODO: maybe not needed as method?
	void GetAllStorages()
	{
		if( m_InventoryStorage )
			m_InventoryStorage.GetStorages( m_UniStorages );		
	}
	
	
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	protected SCR_InventorySlotUI CreateStorageSlotUI( InventoryItemComponent pItemComponent )
	{
		//Salinebags and tourniquets must not be visible in inventory when applied
		if (pItemComponent.GetParentSlot().Type() == SCR_TourniquetStorageSlot || pItemComponent.GetParentSlot().Type() == SCR_SalineBagStorageSlot)
			return null;

		if ( pItemComponent.Type() == ClothNodeStorageComponent )
		{
			return new SCR_InventorySlotLBSUI( pItemComponent, this );		
		}
		else if ( BaseInventoryStorageComponent.Cast( pItemComponent ) )
		{
			return new SCR_InventorySlotStorageUI( pItemComponent, this );		
		}
		else
		{
			return new SCR_InventorySlotUI( pItemComponent, this );		
		}
	}
	//------------------------------------------------------------------------------------------------
	// ! 
	override int CreateSlots( )
	{
		if( !m_InventoryStorage )
			return -1;
		m_Items.Clear();
		m_InventoryStorage.GetAll(m_Items);		
		/*
		foreach ( SCR_InventorySlotUI pSlot: m_aSlots )
			pSlot.Destroy();
		*/
		m_aSlots.Clear();
		foreach ( IEntity item: m_Items )
		{			
			if ( item )
				m_aSlots.Insert( CreateStorageSlotUI( InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent) )) );
			else
				m_aSlots.Insert( CreateStorageSlotUI( null ) );
		}
		return m_aSlots.Count()-1;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! sorting slots by their index, Changing Loadoaut layout in BaseLoadoutManager will affect it.
	override void SortSlots()
	{		
		array<int> aCoordinates;
		int iWidgetColumnSize = 1;
		int iWidgetRowSize = 1;
		int numberOfSlots = m_aSlots.Count();
		int iIndex = 0;
		int iCol = 1, iRow = 1;
		Widget pActualWidgetUI;
		SCR_InventorySlotUI pWidgetHandler;
			
		//reset all elements to 0 - free it
		m_iMatrix.Reset();
				
		
		foreach ( SCR_InventorySlotUI pSlot: m_aSlots )
		{
			if( pSlot )
			{
				auto pStorageSlotUI = SCR_InventorySlotUI.Cast( pSlot );
				if( pStorageSlotUI )
				{
					auto pItem = pStorageSlotUI.GetInventoryItemComponent();
					if( pItem )
					{
						#ifdef DEBUG_INVENTORY20
							ItemAttributeCollection pAttrs = pItem.GetAttributes();
							if ( pAttrs )
							{
								string sName = pAttrs.GetUIInfo().GetName();
								Print( sName );
							}
						#endif
						Widget w = pStorageSlotUI.GetWidget();
						if( w )
						{
							//reserve the position based on the enum
							int iLoadoutArea = m_pInventoryUIConfig.GetRowByArea( pStorageSlotUI.GetLoadoutArea() );
							if ( iLoadoutArea == - 1 )
								iLoadoutArea = m_pInventoryUIConfig.GetRowByCommonItemType( pStorageSlotUI.GetCommonItemType() );
							if ( iLoadoutArea != -1 )
								aCoordinates = m_iMatrix.ReservePlace( iWidgetColumnSize, iWidgetRowSize, 0, iLoadoutArea );	//if area exists in the config, reserve the index
							else
								aCoordinates = m_iMatrix.Reserve1stFreePlace( iWidgetColumnSize, iWidgetRowSize );				//if it doesn't exist, reserve the 1st free place
							if( ( aCoordinates[0] != -1 ) && ( aCoordinates[1] != -1 ) )
							{
								GridSlot.SetColumn( w, aCoordinates[0] );
								if ( iLoadoutArea < m_iMaxRows )
								{
									GridSlot.SetRow( w, iLoadoutArea );	
								}
							}
							
							SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(m_Storage);
							
							if (characterStorage)
							{
								pSlot.SetSlotBlocked(characterStorage.IsAreaBlocked(pSlot.GetLoadoutArea().Type()));
							}
							
							GridSlot.SetColumnSpan( w, iWidgetColumnSize );
							GridSlot.SetRowSpan( w, iWidgetRowSize );
							iCol += iWidgetColumnSize;			
						}
					}
				}
			}
		}
		FillWithEmptySlots();
	}

	//------------------------------------------------------------------------------------------------
	override protected void FillWithEmptySlots()
	{
		bool bFilling = true;
		array<int> aCoordinates = {};
		while ( bFilling )
		{
			aCoordinates = m_iMatrix.Reserve1stFreePlace( 1, 1 );
			if( ( aCoordinates[0] != -1 ) && ( aCoordinates[1] != -1 ) )
			{
				SCR_ItemAttributeCollection pAttrib = new SCR_ItemAttributeCollection();
				pAttrib.SetSlotType( ESlotID.SLOT_ANY );
				pAttrib.SetSlotSize( ESlotSize.SLOT_1x1 );
				SCR_InventorySlotUI pSlot = new SCR_InventorySlotUI( null, this, true, -1, pAttrib );
				m_aSlots.Insert( pSlot );
				GridSlot.SetColumn( pSlot.GetWidget(), aCoordinates[0] );
				GridSlot.SetRow( pSlot.GetWidget(), aCoordinates[1] );
				ResourceName icon = m_pInventoryUIConfig.GetIconByRow( aCoordinates[1] );
				if ( icon != ResourceName.Empty )
					pSlot.SetIcon( icon );
				
				SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(m_Storage);
				if (characterStorage)
				{
					LoadoutAreaType loadoutArea = m_pInventoryUIConfig.GetAreaByRow(aCoordinates[1]);
					
					if (loadoutArea)
						pSlot.SetSlotBlocked(characterStorage.IsAreaBlocked(loadoutArea.Type()));
				}
			}	
			else
			{
				bFilling = false;
			}
			
		}
	}
	//------------------------------------------------------------------------------------------------
	// ! do we want to delete all slots in the actual grid and show the content of the selected child storage? ( no )
	override bool IsTraversalAllowed()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! selects the actual slot
	/*
	override void SelectSlot( SCR_InventorySlotUI pSlot )
	{
		m_MenuHandler.StoreSelectedStorageSlot( SCR_InventorySlotStorageUI.Cast( pSlot ) );
	}
	*/
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		super.HandlerAttached( w );
		LayoutSlot.SetSizeMode( m_widget, LayoutSizeMode.Fill );
		LayoutSlot.SetFillWeight( m_widget, 0.8889 );
		
		
		RefreshList();
	}
	
	void RefreshList()
	{
		CreateEmptyLayout();
		CreateSlots();	
		SortSlots();
	}
			
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryStoragesListUI(BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		m_MenuHandler = menuManager;
		m_iMaxRows 		= 10;
		m_iMaxColumns 	= 1;
		m_iMatrix = new SCR_Matrix( m_iMaxColumns, m_iMaxRows );
		m_sGridPath = "CharacterGrid";
		m_Storage = storage;
		m_pInventoryUIConfig = SCR_InventoryConfig.Cast( BaseContainerTools.CreateInstanceFromContainer( BaseContainerTools.LoadContainer( INVENTORY_CONFIG ).GetResource().ToBaseContainer() ) );
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStoragesListUI()
	{
		if( m_UniStorages )
			delete m_UniStorages;
	}
	
};
