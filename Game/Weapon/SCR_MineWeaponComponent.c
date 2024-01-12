[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_MineWeaponComponentClass : WeaponComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MineWeaponComponent : WeaponComponent
{
	protected IEntity m_FlagEntity;
	
	//------------------------------------------------------------------------------------------------
	bool IsFlagged()
	{
		return m_FlagEntity != null;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFlagRemoved(SCR_MineFlagPickUpAction action)
	{
		m_FlagEntity = null;
		action.GetOnItemPickUp().Remove(OnFlagRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	event protected bool RplLoad(ScriptBitReader reader)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	event protected bool RplSave(ScriptBitWriter writer)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFlagParentSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		if (newSlot != null) // slot is null when removed from inventory
			m_FlagEntity = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterToFlag(IEntity flag)
	{
		SCR_MineFlagPickUpAction action = FindAction(flag);
		if (action)
			action.GetOnItemPickUp().Insert(OnFlagRemoved);
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(flag.FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnParentSlotChangedInvoker.Insert(OnFlagParentSlotChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_MineFlagPickUpAction FindAction(IEntity flag)
	{
		BaseActionsManagerComponent actionManager = BaseActionsManagerComponent.Cast(flag.FindComponent(BaseActionsManagerComponent));
		if (!actionManager)
			return null;
		
		array<BaseUserAction> actions = {};
		int count = actionManager.GetActionsList(actions);
		SCR_MineFlagPickUpAction action;
		
		for (int i = 0; i < count; i++)
		{
			action = SCR_MineFlagPickUpAction.Cast(actions[i]);
			if (action)
				break;
		}
		
		return action;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call only on server!
	void SetFlag(IEntity flag)
	{
		if (m_FlagEntity)
			return; // This should never happen!
		
		m_FlagEntity = flag;
		
		if (flag)
			RegisterToFlag(flag);
	}
};
