//~ Use the inherent versions!
[BaseContainerProps(), BaseContainerCustomStringTitleField("USE INHERENT VERSION ONLY!")]
class SCR_BaseResupplySupportStationAction : SCR_BaseUseSupportStationAction
{
	[Attribute(ESupportStationType.RESUPPLY_AMMO.ToString(), desc: "Which Resupply this action is: Medical or ammo", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ESupportStationType))]
	protected ESupportStationType m_eSupportStationType;
	
	[Attribute(desc: "This decides what items will be resupplied when using the action")]
	protected ref SCR_ResupplySupportStationData m_ResupplyData;
	
	[Attribute("-1", desc: "How much of the given item (or weapon magazines) can be in the inventory of the character. -1 means it is unlimited")]
	protected int m_iMaxResupplyCount;
	
	[Attribute("#AR-SupportStation_Resupply_ActionInvalid_NoInventorySpace", desc: "Text shown on action if player cannot resupply because inventory is full", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidNoInventorySpace;
	
	[Attribute("#AR-SupportStation_Resupply_ActionInvalid_InvalidNotInStorage", desc: "Text shown on action if player cannot resupply because the item is not in the storage of which the character wants to resupply", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidNotInStorage;
	
	[Attribute(ENotification.UNKNOWN.ToString(), desc: "Notification when the action is used. Shown to player using the action if resupply self or on the player that is being resupplied if resupply other. Leave unknown to ignore", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eNotificationOnUse; 
	
	protected SCR_InventoryStorageManagerComponent m_InventoryManagerTarget;
	
	protected EResupplyUnavailableReason m_eResupplyUnavailableReason;
	
	protected bool m_bCanResupply;
	protected ResourceName m_sItemToResupply;
	protected int m_iCurrentItemAmount = -1;
	
	protected const LocalizedString X_OUTOF_Y_FORMATTING = "#AR-SupportStation_ActionFormat_ItemAmount";
	protected const LocalizedString CURRENT_ITEM_AMOUNT_FORMATTING = "#AR-SupportStation_ActionFormat_CurrentItemAmount";
	
	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return m_eSupportStationType;	
	}
	
	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		if (reasonInvalid == ESupportStationReasonInvalid.RESUPPLY_ENOUGH_ITEMS)
			return WidgetManager.Translate(X_OUTOF_Y_FORMATTING, m_iCurrentItemAmount, m_iMaxResupplyCount);
		else if (reasonInvalid == ESupportStationReasonInvalid.RESUPPLY_INVENTORY_FULL)
			return m_sInvalidNoInventorySpace;
		else if (reasonInvalid == ESupportStationReasonInvalid.RESUPPLY_NOT_IN_STORAGE)
			return m_sInvalidNotInStorage;
		
		return super.GetInvalidPerformReasonString(reasonInvalid);
	}

	//------------------------------------------------------------------------------------------------
	//~ Returns true if the action is shown but disabled because of no supplies, max items or inventory full. Otherwise hide it to avoid to many options
	protected bool GetShowButDisabled()
	{
		if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.ENOUGH_ITEMS || m_eResupplyUnavailableReason == EResupplyUnavailableReason.INVENTORY_FULL)
			return true;
			
		if (m_eCannotPerformReason == ESupportStationReasonInvalid.NO_SUPPLIES)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	ENotification GetNotificationOnUse()
	{
		return m_eNotificationOnUse;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool CanHaveMultipleUsers()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		if (!m_ResupplyData)
			return false;
		
		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{		
		if (m_sItemToResupply.IsEmpty())
		{
			UpdateCanResupply(GetOwner(), user);
			if (m_sItemToResupply.IsEmpty())
				return false;
		}
		
		bool canBePerformed = super.CanBePerformedScript(user);
		
		//~ Cannot resupply as resupply action unavailible
		if (canBePerformed && !m_bCanResupply)
		{
			//~ Set reason action is unavailible
			if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.ENOUGH_ITEMS)
				SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_ENOUGH_ITEMS);
			else if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.INVENTORY_FULL)
				SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_INVENTORY_FULL);
			else if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.NOT_IN_GIVEN_STORAGE)
				SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_NOT_IN_STORAGE);
			else if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.NO_VALID_WEAPON)
				SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_NO_VALID_WEAPON);
			
			return false;
		}
		
		return canBePerformed;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void CanBePerformedUpdate(IEntity user)
	{
		//~ Check if can add to inventory. Only called every x seconds to save performance
		UpdateCanResupply(GetOwner(), user);
		super.CanBePerformedUpdate(user);
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetItemToResupply()
	{
		return m_sItemToResupply;
	}

	//------------------------------------------------------------------------------------------------
	//~ Can Resupply loops through entire inventory so only call this every x seconds
	protected void UpdateCanResupply(IEntity owner, IEntity user)
	{
		m_bCanResupply = false;
		
		//~ Reset reason
		m_eResupplyUnavailableReason = EResupplyUnavailableReason.NO_VALID_WEAPON;
			
		SetTargetInventory(user, owner);
		if (!m_InventoryManagerTarget)
			return;
	
		BaseMuzzleComponent muzzle;
		if (!m_ResupplyData.GetResupplyItemOrMuzzle(m_InventoryManagerTarget.GetOwner(), owner, m_SupportStationGadget, m_sItemToResupply, muzzle))
			return;
		
		//~ No valid item found
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sItemToResupply))
			return;
		
		//~ Check if resupply is availible (And get current item amount)
		if (muzzle)
			m_eResupplyUnavailableReason = m_InventoryManagerTarget.CanResupplyMuzzle(muzzle, m_iMaxResupplyCount, GetProviderInventory(), m_iCurrentItemAmount);
		else
			m_eResupplyUnavailableReason = m_InventoryManagerTarget.CanResupplyItem(m_sItemToResupply, m_iMaxResupplyCount, GetProviderInventory(), m_iCurrentItemAmount);
		
		m_bCanResupply = m_eResupplyUnavailableReason == EResupplyUnavailableReason.RESUPPLY_VALID;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		UpdateCanResupply(pOwnerEntity, pUserEntity);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void SetTargetInventory(IEntity user, IEntity owner)
	{
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageManagerComponent GetTargetInventory()
	{
		return m_InventoryManagerTarget;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get inventory of who ever provide the item. This simply checkes if the item is in the inventory. This should be an arsenal as it does not remove anything from the inventory
	protected InventoryStorageManagerComponent GetProviderInventory()
	{
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void ResetReferencesOnServer()
	{
		m_sItemToResupply = string.Empty;
		super.ResetReferencesOnServer();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		if (m_ResupplyData)
			m_ResupplyData.Init(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetActionStringParam()
	{		
		if (!m_bCanPerform)
		{
			if (m_eResupplyUnavailableReason != EResupplyUnavailableReason.INVENTORY_FULL)
				return string.Empty;
		}
		
		if (m_iMaxResupplyCount < 0)
			return WidgetManager.Translate(CURRENT_ITEM_AMOUNT_FORMATTING, m_iCurrentItemAmount);
		
		return WidgetManager.Translate(X_OUTOF_Y_FORMATTING, m_iCurrentItemAmount, m_iMaxResupplyCount);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE!")]
class SCR_ResupplySupportStationData
{
	//------------------------------------------------------------------------------------------------
	void Init(IEntity owner)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetResupplyItemOrMuzzle(notnull IEntity targetCharacter, notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget, out ResourceName item, out BaseMuzzleComponent muzzle)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected FactionAffiliationComponent GetFactionAffiliationGadgetOrOwner(notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget)
	{
		FactionAffiliationComponent factionAffiliationComponent;
		
		if (supportStationGadget)
		{
			factionAffiliationComponent = FactionAffiliationComponent.Cast(supportStationGadget.GetOwner().FindComponent(FactionAffiliationComponent));
			if (factionAffiliationComponent)
				return factionAffiliationComponent;
		}
			
		factionAffiliationComponent = FactionAffiliationComponent.Cast(actionOwner.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComponent)
		{
			IEntity parent = actionOwner.GetParent();
			if (parent)
				factionAffiliationComponent = FactionAffiliationComponent.Cast(parent.FindComponent(FactionAffiliationComponent));
		}
		
		return factionAffiliationComponent;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ResupplyItemSupportStationData : SCR_ResupplySupportStationData
{
	[Attribute(desc: "Prefab of entity", UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sItemPrefab;
	
	//------------------------------------------------------------------------------------------------
	override bool GetResupplyItemOrMuzzle(notnull IEntity targetCharacter, notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget, out ResourceName item, out BaseMuzzleComponent muzzle)
	{
		item = m_sItemPrefab;
		
		return !SCR_StringHelper.IsEmptyOrWhiteSpace(item);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ResupplyCatalogItemSupportStationData : SCR_ResupplySupportStationData
{
	[Attribute(desc: "This will go through through all the inventory items in the catalog and grab the first one with the given type (needs the SCR_EntityCatalogSupportStationResupplyData)", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ESupportStationResupplyType))]
	protected SCR_ESupportStationResupplyType m_eResupplyType;
	
	[Attribute("0", desc: "If true will use the item associated with the support station default faction. Otherwise will only take defualt if no faction is set")]
	protected bool m_bAlwaysTakeDefaultFaction;
	
	protected ref map<FactionKey, ResourceName> m_mFactionItems = new map<FactionKey, ResourceName>;
		
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity owner)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		
		SCR_Faction scrFaction;
		SCR_EntityCatalog itemCatalog;
		
		array<SCR_EntityCatalogEntry> filteredEntityList = {};
		array<SCR_BaseEntityCatalogData> dataList = {};
		SCR_EntityCatalogSupportStationResupplyData resupplyData;
		int count;
		
		//~ Go over each faction and set which items will be used for the resupply
		foreach (Faction faction : factions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;
			
			itemCatalog = scrFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM);
			if (!itemCatalog)
				continue;
			
			count = itemCatalog.GetEntityListWithData(SCR_EntityCatalogSupportStationResupplyData, filteredEntityList, dataList);
			if (count <= 0)
				continue;
			
			for (int i = 0; i < count; i++)
			{
				resupplyData = SCR_EntityCatalogSupportStationResupplyData.Cast(dataList[i]);
				if (!resupplyData || resupplyData.GetResupplyType() != m_eResupplyType)
					continue;
				
				m_mFactionItems.Insert(faction.GetFactionKey(), filteredEntityList[i].GetPrefab());
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetResupplyItemOrMuzzle(notnull IEntity targetCharacter, notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget, out ResourceName item, out BaseMuzzleComponent muzzle)
	{
		if (m_mFactionItems.IsEmpty())
			return false;
		
		FactionAffiliationComponent factionAffiliationComponent = GetFactionAffiliationGadgetOrOwner(actionOwner, supportStationGadget);
		if (!factionAffiliationComponent)
			return false;
		
		//~ Get faction
		Faction faction;
		if (m_bAlwaysTakeDefaultFaction)
		{
			faction = factionAffiliationComponent.GetDefaultAffiliatedFaction();
			if (!faction)
				return false;
		}
		else 
		{
			faction = factionAffiliationComponent.GetAffiliatedFaction();
			if (!faction)
			{
				faction = factionAffiliationComponent.GetDefaultAffiliatedFaction();
				if (!faction)
					return false;
			}
		}	
	
		if (!m_mFactionItems.Find(faction.GetFactionKey(), item))
			return false;
		
		return !SCR_StringHelper.IsEmptyOrWhiteSpace(item);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ResupplyHeldWeaponSupportStationData : SCR_ResupplySupportStationData
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMuzzleType))]
	protected ref array<EMuzzleType> m_aMuzzleTypes;
	
	[Attribute("0", desc: "If true will check if the found ammo is part of the entity default faction or owner default faction. Only do this check when item has no inventory, else check if item is in storage instead!")]
	protected bool m_bCheckFaction;
	
	//------------------------------------------------------------------------------------------------
	override bool GetResupplyItemOrMuzzle(notnull IEntity targetCharacter, notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget, out ResourceName item, out BaseMuzzleComponent muzzle)
	{
		if (m_aMuzzleTypes.IsEmpty())
			return false;
		
		BaseWeaponManagerComponent weaponsManager = BaseWeaponManagerComponent.Cast(targetCharacter.FindComponent(BaseWeaponManagerComponent));
		if (!weaponsManager)
			return false;		
		
		//~ Get held weapon to resupply
		BaseWeaponComponent baseWeaponComp = weaponsManager.GetCurrentWeapon();
		if (!baseWeaponComp)
			return false;
		
		string weaponSlotType = baseWeaponComp.GetWeaponSlotType();
		if (weaponSlotType != "primary" && weaponSlotType != "secondary")
			return false;

		array<BaseMuzzleComponent> muzzles = {};

		//~ Get base muzzle to only supply magazines
		baseWeaponComp.GetMuzzlesList(muzzles);
		SCR_MuzzleInMagComponent inMagMuzzle;
		
		foreach (BaseMuzzleComponent muzzleComp : muzzles)
		{
			if (m_aMuzzleTypes.Contains(muzzleComp.GetMuzzleType()))
			{
				//~ Cannot be reloaded
				inMagMuzzle = SCR_MuzzleInMagComponent.Cast(muzzleComp);
				if (inMagMuzzle && !inMagMuzzle.CanBeReloaded())
					continue;
				
				//~ Check if has default magazine
				item = muzzleComp.GetDefaultMagazineOrProjectileName();
				if (!item.IsEmpty())
				{
					muzzle = muzzleComp;
					break;
				}
			}
		}
		
		//~ No valid muzzle found
		if (!muzzle)
			return false;
		
		//~ Check if ammo is part of the faction of gadget or action owner
		if (m_bCheckFaction)
		{
			SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
			if (!catalogManager)
				return true;
			
			FactionAffiliationComponent factionAffiliationComponent = GetFactionAffiliationGadgetOrOwner(actionOwner, supportStationGadget);
			if (!factionAffiliationComponent)
				return true;
			
			SCR_Faction faction = SCR_Faction.Cast(factionAffiliationComponent.GetDefaultAffiliatedFaction());
			if (!faction)
				return true;
			
			//~ Not the correct faction so do not allow for resupply
			if (!catalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, item, faction))
				return false;
		}
		
		return true;
	}
}

