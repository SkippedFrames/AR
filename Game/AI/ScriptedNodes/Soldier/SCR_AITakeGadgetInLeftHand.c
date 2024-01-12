class SCR_AITakeGadgetInLeftHand : AITaskScripted
{
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECommonItemType) )]
	private ECommonItemType m_eItemType;
	
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EGadgetType) )]
	private EGadgetType m_eGadgetType;
	
	protected const float TIMEOUT_S = 6.0;
	
	protected IEntity m_OwnerEntity;
	protected CharacterControllerComponent m_CharacterController;
	protected SCR_InventoryStorageManagerComponent m_InventoryMgr;
	protected SCR_GadgetManagerComponent m_GadgetManager;
	
	protected IEntity m_ItemEntity;	// Item which we have requested to equip
	protected bool m_bWaiting;		// When true, we are waiting for item to be equipped
	protected float m_fTimer_s;		// Timer how long we've been waiting
	
	protected static const string PORT_ITEM = "ItemOut";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_ITEM
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	// -----------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_OwnerEntity = owner.GetControlledEntity();
		m_CharacterController = CharacterControllerComponent.Cast(m_OwnerEntity.FindComponent(CharacterControllerComponent));
		m_InventoryMgr = SCR_InventoryStorageManagerComponent.Cast(m_OwnerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(m_OwnerEntity);
	}

	// -----------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CharacterController || !m_InventoryMgr)
			return ENodeResult.FAIL;
			
		if (m_bWaiting)
		{
			if (IsItemEquipped(m_ItemEntity))
			{
				SetVariableOut(PORT_ITEM, m_ItemEntity);
				Reset();
				return ENodeResult.SUCCESS;
			}
			else if (m_fTimer_s > TIMEOUT_S)
			{
				ClearVariable(PORT_ITEM);
				Reset();
				return ENodeResult.FAIL;
			}
			else
			{
				m_fTimer_s += dt;
				return ENodeResult.RUNNING;
			}
		}
		else
		{
			IEntity item = FindItem();
			
			if (!item)
				return ENodeResult.FAIL;
			else
			{
				if (!m_CharacterController.CanEquipGadget(item))
					return ENodeResult.RUNNING;
				else
				{
					if (EquipInventoryItem(item))
					{
						m_ItemEntity = item;
						m_fTimer_s = 0;
						m_bWaiting = true;
						return ENodeResult.RUNNING;
					}
					else
						return ENodeResult.FAIL;
				}
			}
		}
	}

	//-----------------------------------------------------------------------------------------------
	protected void Reset()
	{
		m_ItemEntity = null;
		m_fTimer_s = 0;
		m_bWaiting = false;
	}

	//-----------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		Reset();
	}
	
	//-----------------------------------------------------------------------------------------------
	IEntity FindItem()
	{
		IEntity item;
		
		if (m_eItemType != 0)
		{
			SCR_HoldableItemPredicate predicate = new SCR_HoldableItemPredicate();
			predicate.wanted = m_eItemType;
			item = m_InventoryMgr.FindItem(predicate);
		}
		else
		{
			item = m_GadgetManager.GetGadgetByType(m_eGadgetType);
		}

		return item;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool EquipInventoryItem(IEntity item)
	{
		m_GadgetManager.SetGadgetMode(item, EGadgetMode.IN_HAND);
		//m_CharacterController.TakeGadgetInLeftHand(item, 1);
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool IsItemEquipped(IEntity item)
	{
		if (!item)
			return false;
		
		IEntity itemAtSlot = m_CharacterController.GetAttachedGadgetAtLeftHandSlot();
		
		return itemAtSlot == item;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		if (m_eItemType != 0)
			return string.Format("Item type: %1", typename.EnumToString(ECommonItemType, m_eItemType));
		else
			return string.Format("Gadget type: %1", typename.EnumToString(EGadgetType, m_eGadgetType));
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool CanReturnRunning()
	{
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Equips inventory item, returns success when done and item ref.";
	}
};
