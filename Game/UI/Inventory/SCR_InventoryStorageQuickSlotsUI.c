
//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory slots for the Quick Bar ( Item/Weapon switching )

class SCR_InventoryStorageQuickSlotsUI: SCR_InventoryStorageBaseUI
{
	protected SCR_InventorySlotUI						m_pLastSelectedSlot;
	protected static int 								s_iLastSelectedSlotIndex;
	protected static int 								s_iInitSelectedSlotIndex;
	protected static bool 								s_bQuickBarClosed;
	
	protected ResourceName m_sGamepadIcons = "{F7FD1672FECA05E8}UI/Textures/Icons/icons_gamepad_64.imageset";
	//------------------------------------------- USER METHODS  -----------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override void InitPaging() { return; }
	
	//------------------------------------------------------------------------------------------------
	// ! 
	protected void HighlightSlot( int iSlotIndex, bool bHighlight )
	{
		if ( m_aSlots.IsEmpty() || iSlotIndex == -1 )
			return;

		if (m_pLastSelectedSlot)
			m_pLastSelectedSlot.SetSelected(false);

		m_pLastSelectedSlot = m_pSelectedSlot;
		if ( iSlotIndex > m_aSlots.Count() - 1 )
			return;
		m_pSelectedSlot = m_aSlots.Get( iSlotIndex );
 		if ( !m_pSelectedSlot )
			return;	

		m_pSelectedSlot.SetSelectedQuickSlot( bHighlight );
		CheckIfQuickSlotActionsAvailable(m_Player);
	}

	//------------------------------------------------------------------------------------------------
	// ! 
	void SelectSlot( float fWheelValue )
	{
		if ( fWheelValue == 0 )
			return;
		
		if ( m_aSlots.IsEmpty() )
			return;
			
		if ( fWheelValue == 1 )
			fWheelValue = -1;			//in case it's controler dpad uses
				
		if ( s_iLastSelectedSlotIndex < 0 )
		{
			if (fWheelValue > 0)
				s_iLastSelectedSlotIndex = m_aSlots.Count() - 1;
			else
				s_iLastSelectedSlotIndex = 0;
				
		}
		else
		{
			if (m_pInputManager.IsUsingMouseAndKeyboard() && s_iLastSelectedSlotIndex < 0)
				s_iLastSelectedSlotIndex = m_aSlots.Count()-1;
			//PrintFormat( "INV: index to deselect: %1 | m_aSlotsCount: %2", s_iLastSelectedSlotIndex, m_aSlots.Count() );
			HighlightSlot( s_iLastSelectedSlotIndex, false );
			if (fWheelValue > 0)
			{
				if (m_pInputManager.IsUsingMouseAndKeyboard())
					s_iLastSelectedSlotIndex = Math.Max( ( s_iLastSelectedSlotIndex - 1 ), 0 );
				else
					s_iLastSelectedSlotIndex = ( s_iLastSelectedSlotIndex - 1 ) % ( m_aSlots.Count() );			//version for cyclic rotation through the items in slots
			}
			else
			{
				if (m_pInputManager.IsUsingMouseAndKeyboard())
					s_iLastSelectedSlotIndex = Math.Min( ( s_iLastSelectedSlotIndex + 1 ), m_aSlots.Count()-1 );
				else
					s_iLastSelectedSlotIndex = ( s_iLastSelectedSlotIndex + 1 ) % ( m_aSlots.Count() );			//version for cyclic rotation through the items in slots
			}


			//if ( )																//version for cyclic rotation through the items in slots
			//PrintFormat( "INV: index to select: %1 | m_aSlotsCount: %2", s_iLastSelectedSlotIndex, m_aSlots.Count() );
			
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_SCROLL);
		}	
		HighlightSlot( s_iLastSelectedSlotIndex, true );
	}
		
		
	//------------------------------------------------------------------------------------------------
	// ! 
	void SelectSlot( int iSlotIndex )
	{
		HighlightSlot( iSlotIndex, true );
		s_iLastSelectedSlotIndex = iSlotIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! Hide all, but actual slot
	void FilterOutSlots()
	{
		foreach ( SCR_InventorySlotUI pSlot : m_aSlots )
		{
			if ( pSlot && pSlot != m_pSelectedSlot )
				pSlot.SetSlotVisible( false );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override protected void UpdateOwnedSlots(notnull array<IEntity> pItemsInStorage)
	{
		int count = pItemsInStorage.Count();
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && !m_MenuHandler)
			count = Math.Min(count, 4); // todo: get this from radial menu config
		
		if (count < m_aSlots.Count())
		{
			for (int i = m_aSlots.Count() - count; i > 0; i--)
			{
				auto slotUI = m_aSlots[m_aSlots.Count() - 1];
				if (slotUI)
					slotUI.Destroy();
			}
			
		}
		
		m_aSlots.Resize(count);
		for (int i = 0; i < count; i++)
		{
			InventoryItemComponent pComponent = GetItemComponentFromEntity( pItemsInStorage[i] );
			if (m_aSlots[i])
				m_aSlots[i].UpdateReferencedComponent(pComponent);
			else
				m_aSlots[i] = CreateSlotUI(pComponent, i);
		}
		CheckIfQuickSlotActionsAvailable(m_Player);
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override protected void GetAllItems( out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null )
	{
		if (!m_Player)
			return;
		
		if ( m_InventoryStorage && m_InventoryStorage.GetQuickSlotItems() )
			pItemsInStorage.Copy( m_InventoryStorage.GetQuickSlotItems() );		
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override protected int CreateSlots()
	{
		array<IEntity> itemsInStorage = {};
		GetAllItems(itemsInStorage);
		UpdateOwnedSlots(itemsInStorage);
		CheckIfQuickSlotActionsAvailable(m_Player);
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	// ! creates the the grid from array of UI items
	override protected void SortSlots()
	{
		array<int> aCoordinates;
		int iWidgetColumnSize, iWidgetRowSize;
		int iPageCounter = 0;				
		int iRelativeOffset = 0; 				//if there's an item taking more than one slot, offset the following items to the right

		//reset all elements to 0 - free it
		m_iMatrix.Reset(); 		
									
		foreach ( SCR_InventorySlotUI pSlot: m_aSlots )
		{
			if( !pSlot )
				continue;
			Widget w = pSlot.GetWidget();
			if( !w )
				continue;
			
			iWidgetColumnSize = pSlot.GetColumnSize();
			iWidgetRowSize = pSlot.GetRowSize();
		
			int iCol = pSlot.GetSlotIndex() + iRelativeOffset;
			m_iMatrix.ReservePlace( iWidgetColumnSize, iWidgetRowSize, iCol, 0 );	
			GridSlot.SetColumn( w, iCol );
			iRelativeOffset += ( iWidgetColumnSize - 1 );
			GridSlot.SetRow( w, 0 );
			pSlot.SetPage( iPageCounter );
			
			GridSlot.SetColumnSpan( w, iWidgetColumnSize );
			GridSlot.SetRowSpan( w, iWidgetRowSize );	
		}
		m_iNrOfPages = iPageCounter + 1;
		FillWithEmptySlots();	
		CheckIfQuickSlotActionsAvailable(m_Player);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckIfQuickSlotActionsAvailable(IEntity player)
	{
		foreach (int iIndex, SCR_InventorySlotUI pSlot: m_aSlots)
		{
			if(!pSlot)
				continue;
			
			Widget w = pSlot.GetWidget();
			if(!w)
				continue;
			
			if (!pSlot.GetInventoryItemComponent())
				w.SetOpacity(0.35);
			
			if (!pSlot.CanUseItem(m_Player))
				w.SetOpacity(0.35);
			else
				w.SetOpacity(1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void FillWithEmptySlots()
	{
		array<int> aCoordinates;
		int iWidgetColumnSize, iWidgetRowSize;
		int iPageCounter = 0;				
		int iRelativeOffset = 0; 				//if there's an item taking more than one slot, offset the following items to the right
		int i = 0;
		
		foreach ( int iIndex, SCR_InventorySlotUI pSlot: m_aSlots )
		{
			if( !pSlot )			//if pSlot doesn't exist it means no assigned item in this slot and we create the "empty" slot
			{
				ESlotSize eSlotSize;
				if ( iIndex > 1 )							
					eSlotSize = ESlotSize.SLOT_1x1;		
				else
					eSlotSize = ESlotSize.SLOT_2x1;	//first 2 slots are dedicated to weapons ( thus 2x1 layout)
				
				SCR_ItemAttributeCollection pAttrib = new SCR_ItemAttributeCollection();
				pAttrib.SetSlotType( ESlotID.SLOT_ANY );
				pAttrib.SetSlotSize( eSlotSize );
				pSlot = SCR_InventorySlotQuickSlotUI.Cast( CreateSlotUI( null, iIndex, pAttrib ) );
				m_aSlots.Set( iIndex, pSlot );
			}
			Widget w = pSlot.GetWidget();
			if( !w )
				continue;
			
			iWidgetColumnSize = pSlot.GetColumnSize();
			iWidgetRowSize = pSlot.GetRowSize();
		
			int iCol = i + iRelativeOffset;
			m_iMatrix.ReservePlace( iWidgetColumnSize, iWidgetRowSize, iCol, 0 );	
			GridSlot.SetColumn( w, iCol );
			GridSlot.SetRow( w, 0 );
			
			if (GetInventoryMenuHandler() && iIndex < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT)
			{
				pSlot.SetSlotVisible(false);
				i = 0;
			}
			else
			{
				iRelativeOffset += iWidgetColumnSize - 1;
				i++;
			}
			
			GridSlot.SetColumnSpan( w, iWidgetColumnSize );
			GridSlot.SetRowSpan( w, iWidgetRowSize );	
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetItem( InventoryItemComponent pInventoryComponent, int iSlotIndex )
	{
		m_aSlots.InsertAt( CreateSlotUI( pInventoryComponent ), iSlotIndex );
	}
		
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	protected SCR_InventorySlotQuickSlotUI CreateSlotUI( InventoryItemComponent pComponent, int iSlotIndex, SCR_ItemAttributeCollection pAttributes = null )
	{
		return SCR_InventorySlotQuickSlotUI( pComponent, this, true, iSlotIndex, pAttributes );			//creates the slot 
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetRowsAndColumns()
	{
		// leave this empty because we need to initialize rows and columns in different place
	}
	
	//------------------------------------------------------------------------------------------------
	bool UseItemInSlot()
	{
		if (!m_pSelectedSlot)
			return false;
		FilterOutSlots();

		bool useItem = true;
		if (s_bQuickBarClosed)
		{
			useItem = (s_iInitSelectedSlotIndex != s_iLastSelectedSlotIndex);
			s_bQuickBarClosed = false;
		}

		if (useItem)
		{
			m_pSelectedSlot.UseItem(m_Player);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_CONFIRM);
		}
		else
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_CLOSE);
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInitialQuickSlot()
	{
		IEntity currentItem = m_InventoryStorage.GetSelectedItem();
		if (!currentItem)
			s_iInitSelectedSlotIndex = -1;
		
		array<IEntity> items = m_InventoryStorage.GetQuickSlotItems();
		foreach (int i, IEntity item : items)
		{
			if (item != currentItem)
				continue;
			
			s_iInitSelectedSlotIndex = i;
			SelectSlot(s_iInitSelectedSlotIndex);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetQuickBarClosed()
	{
		s_bQuickBarClosed = true;
	}

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		// s_iLastSelectedSlotIndex = -1;
	}
	
	//------------------------------------------------------------------------------------------------
	void RefreshList()
	{
		CreateEmptyLayout();
		RefreshQuickSlots();
	}
	
	//------------------------------------------------------------------------------------------------
	void RefreshQuickSlots()
	{
		CreateSlots();	
		InitMatrix();
		SortSlots();
	}
	
	//------------------------------------------------------------------------------------------------	
	override void RefreshSlot(int index)
	{
		if (!m_aSlots.IsIndexValid(index))
			return;

		IEntity item = m_InventoryStorage.GetItemFromQuickSlot(index);
		if (!item)
			return;

		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComp)
			return;

		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(itemComp.GetAttributes());
		m_aSlots[index].UpdateReferencedComponent(itemComp, attributes);
		SortSlots();
	}

	//------------------------------------------------------------------------------------------------
	int GetActualSlotsUsedCount()
	{
		int result = 0;
		int slotCount = m_aSlots.Count();
		SCR_InventorySlotUI slot;
		for (int id = 0; id < slotCount; ++id)
		{
			if (GetInventoryMenuHandler() && id < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT)
				continue;

			slot = m_aSlots[id];
			if (slot)
			{
				result += slot.GetColumnSize();
			}
			else
			{
				if (id < 2) // first two doubleslots reserved for weapons
				{
					result += ESlotSize.SLOT_2x1;
				}
				else
				{
					result += ESlotSize.SLOT_1x1;
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	void InitMatrix()
	{
		if (!m_wGrid)
			return;
		
		m_iMaxColumns = GetActualSlotsUsedCount();
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, m_iMaxRows);
		m_iPageSize = m_iMaxRows * m_iMaxColumns;

		for (int y = 0; y < m_iMaxRows; ++y)
		{
			m_wGrid.SetRowFillWeight(y, 1);
			for (int x = 0; x < m_iMaxColumns; ++x)
			{
				m_wGrid.SetColumnFillWeight(x, 1);
			}
		}

		SizeLayoutWidget wSizeContainer = SizeLayoutWidget.Cast(m_widget.FindAnyWidget("SizeLayout0"));
		if (wSizeContainer)
			wSizeContainer.SetAspectRatio(m_iMaxColumns / m_iMaxRows);
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetGamepadIcons()
	{
		return m_sGamepadIcons;
	}

	InventoryItemComponent GetCurrentSlotItem()
	{
		SCR_InventorySlotUI slot = m_aSlots.Get(s_iLastSelectedSlotIndex);
		if (!slot.GetInventoryItemComponent())
			return null;

		return slot.GetInventoryItemComponent();
	}

	//------------------------------------------------------------------------------------------------
	void HighlightLastSelectedSlot()
	{
		SelectSlot(s_iLastSelectedSlotIndex);
	}

	//------------------------------------------- COMMON METHODS  -----------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		super.HandlerAttached( w );
		RefreshList();
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryStorageQuickSlotsUI( BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		m_Storage = null;					// quick slots don't use any storage. They will get m_aQuickSlotItems from CharacterInventoryStorageManager
		//m_MenuHandler 	= menuManager;
		m_eSlotAreaType = null;
		m_iMaxRows 		= 1;
		m_iMaxColumns	= 0;
		m_iLastShownPage = iPage;
	}
};
