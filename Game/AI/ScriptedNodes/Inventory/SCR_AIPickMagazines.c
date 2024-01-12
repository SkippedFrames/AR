class SCR_AIPickMagazines : AITaskScripted
{
	static const string PORT_MAGAZINE_WELL = "MagazineWell";
	
	[Attribute("", UIWidgets.EditBox, "Name of magazine well" )]
	protected string m_sMagazineWellType;

	private IEntity m_OwnerEntity;
	private SCR_InventoryStorageManagerComponent m_Inventory;
	private typename m_oMagazineWell;
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_MAGAZINE_WELL
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }	
	
	override void OnInit(AIAgent owner)
	{
		m_OwnerEntity = owner.GetControlledEntity();
		if (!m_OwnerEntity)
			Debug.Error("Owner must be a character!");
		m_Inventory = SCR_InventoryStorageManagerComponent.Cast(m_OwnerEntity.FindComponent(SCR_InventoryStorageManagerComponent));			
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Inventory)
			ENodeResult.FAIL;
		
		if (!GetVariableIn(PORT_MAGAZINE_WELL,m_oMagazineWell))
			m_oMagazineWell = m_sMagazineWellType.ToType();
		
		CharacterVicinityComponent vicinity = CharacterVicinityComponent.Cast(m_OwnerEntity.FindComponent(CharacterVicinityComponent));
		if (vicinity)
		{
			array<IEntity> items = {};
			vicinity.GetAvailableItems(items);
			
			foreach (IEntity item : items)
			{
				MagazineComponent magComp = MagazineComponent.Cast(item.FindComponent(MagazineComponent));
				if (!magComp)
					continue;
				if (magComp.GetMagazineWell().Type() == m_oMagazineWell && !m_Inventory.TryInsertItem(item,EStoragePurpose.PURPOSE_DEPOSIT))
				{
					CharacterControllerComponent charCtrlComp = CharacterControllerComponent.Cast(ChimeraCharacter.Cast(m_OwnerEntity).GetCharacterController());
					charCtrlComp.ReloadWeaponWith(item);
				}
			}
			
			if (items.Count() == 0)
			{
				PrintFormat("No items found to pick up by %1!",m_OwnerEntity);
				return ENodeResult.FAIL;
			}
			else
				return ENodeResult.SUCCESS;
		}
		return ENodeResult.FAIL;
	}
	
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	protected override string GetOnHoverDescription()
	{
		return "AI task that picks up all magazines of provided MagazineWell type in the vicinity of its inventory.";
	}
};
