//------------------------------------------------------------------------------------------------
class SCR_SaveArsenalLoadout : SCR_BaseFactionCheckUserAction
{
	[Attribute("0", desc: "If true will try and get the Arsenal component from parent if none found. This is to check if saving is allowed. If no arsenal is found then saving is always true")]
	protected bool m_bAllowGetArsenalFromParent;
	
	protected SCR_EArsenalSaveType m_eLastCheckedArsenalSaveType = -1; //~ Keeps track of the last checked save type so the display text is only updated when this is updated
	protected LocalizedString m_sSaveTypeDisplayName;
	
	//~ Ref
	protected SCR_ArsenalComponent m_ArsenalComponent;
	protected SCR_ArsenalManagerComponent m_ArsenalManager;
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanBeShownScript(IEntity user)
	{
#ifdef DISABLE_ARSENAL_LOADOUTS
		return false;
#endif		
		if (!m_ArsenalManager || !super.CanBeShownScript(user))
			return false;
		
		//~ If has arsenal component and arsenal has an inventory. Only show if saving is allowed and not disabled at the start
		if (!m_ArsenalComponent || !m_ArsenalComponent.GetArsenalInventoryComponent() || (m_ArsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.SAVING_DISABLED && !m_ArsenalComponent.IsArsenalSavingDisplayedIfDisabled()))
			return false;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		SCR_ArsenalManagerComponent arsenalManager;
		return (!playerController || !playerController.IsPossessing());
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanBePerformedScript(IEntity user)
	{
		if (m_ArsenalComponent.GetArsenalSaveType() != m_eLastCheckedArsenalSaveType || (m_eLastCheckedArsenalSaveType == SCR_EArsenalSaveType.SAVING_DISABLED && m_sCannotPerformReason.IsEmpty()))
			UpdateActionDisplayName(user);
		
		if (m_ArsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.SAVING_DISABLED)
			return false;
		
		if (!super.CanBePerformedScript(user))
		{
			m_sSaveTypeDisplayName = string.Empty;
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		super.Init(pOwnerEntity, pManagerComponent);
		
		SCR_ArsenalManagerComponent.GetArsenalManager(m_ArsenalManager);
		if (!m_ArsenalManager)
			return;
		
		//~ Get Arsenal component
		m_ArsenalComponent = SCR_ArsenalComponent.Cast(GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!m_ArsenalComponent && m_bAllowGetArsenalFromParent && GetOwner().GetParent())
			m_ArsenalComponent = SCR_ArsenalComponent.Cast(GetOwner().GetParent().FindComponent(SCR_ArsenalComponent));
		
		//~ Could not get arsenal
		if (!m_ArsenalComponent)
		{
			Print("SCR_SaveArsenalLoadout is attached to an entity that does not have an arsenal on self or parent (if m_bAllowGetArsenalFromParent is true)!", LogLevel.WARNING);
			return;
		}	
		
		//~ Allows editor to set the set the allowing of arsenal saving
		m_ArsenalComponent.SetHasSaveArsenalAction(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateActionDisplayName(notnull IEntity user)
	{
		//~ There is no arsenal component
		if (!m_ArsenalComponent)
			return;
		
		//~ Makes sure it is only updated when needed
		m_eLastCheckedArsenalSaveType = m_ArsenalComponent.GetArsenalSaveType();
		
		//~ There is no arsenal manager
		if (!m_ArsenalManager)
			return;
		
		//~ Get the arseal save type holder
		SCR_ArsenalSaveTypeInfoHolder saveTypesHolder = m_ArsenalManager.GetArsenalSaveTypeInfoHolder();
		if (!saveTypesHolder)
			return;
		
		//~ Get the save type UIinfo of the current arsenal save type
		SCR_ArsenalSaveTypeUIInfo saveTypeUIInfo = saveTypesHolder.GetUIInfoOfType(m_eLastCheckedArsenalSaveType);
		if (!saveTypeUIInfo)
		{
			m_sSaveTypeDisplayName = string.Empty;
			return;
		}
		
		//~ Get Action display name
		m_sSaveTypeDisplayName = saveTypeUIInfo.GetPlayerActionDisplayName();
		
		//~ No custom display name for the action so use default
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sSaveTypeDisplayName))
		{
			SetCannotPerformReason(string.Empty);
			return;
		}
		
		//~ Action is disabled so show the action name in disabled () and use default
		if (m_eLastCheckedArsenalSaveType == SCR_EArsenalSaveType.SAVING_DISABLED)
		{
			SetCannotPerformReason(m_sSaveTypeDisplayName);
			m_sSaveTypeDisplayName = string.Empty;
			return;
		}
		
		if (m_eLastCheckedArsenalSaveType == SCR_EArsenalSaveType.FACTION_ITEMS_ONLY)
		{
			FactionAffiliationComponent userFactionAffiliation = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
			if (!userFactionAffiliation)
			{
				m_sSaveTypeDisplayName = string.Empty;
				return;
			}
				
			Faction faction = userFactionAffiliation.GetAffiliatedFaction();
			if (!faction)
			{
				m_sSaveTypeDisplayName = string.Empty;
				return;
			}
				
			m_sSaveTypeDisplayName = WidgetManager.Translate(m_sSaveTypeDisplayName, faction.GetFactionName());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{				
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sSaveTypeDisplayName))
			outName = m_sSaveTypeDisplayName;
		else 
			return super.GetActionNameScript(outName);
		
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	override protected void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
		if (!m_ArsenalManager || !m_ArsenalComponent)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		if (playerId <= 0)
			return;
		
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		m_ArsenalManager.SetPlayerArsenalLoadout(playerId, GameEntity.Cast(pUserEntity), m_ArsenalComponent);
	}
};
