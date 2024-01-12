[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_CharacterRole : EditablePrefabsLabel_Base
{	
	[Attribute()]
	private int m_MedicItemCountRequired;
	
	private ref array<EEditableEntityLabel> m_RoleLabels = {};
	
	override bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		if (authoredLabels.Contains(EEditableEntityLabel.ROLE_LEADER))
		{
			return false;
		}
		
		m_RoleLabels.Clear();
		
		CheckWeapons(entitySource);
		CheckInventory(entitySource);
		CheckLoadout(entitySource);
		
		if (GetRoleLabel(label))
		{
			return true;
		}
		
		return false;
	}
	
	void CheckWeapons(IEntitySource entitySource)
	{
		array<ref array<IEntityComponentSource>> weaponSlotComponents = {};
		array<string> componentTypeArray = {"CharacterWeaponSlotComponent"};
		int weaponSlotCount = SCR_BaseContainerTools.FindComponentSources(entitySource, componentTypeArray, weaponSlotComponents);
		
		array<IEntityComponentSource> weaponSlotComponentSources = weaponSlotComponents.Get(0);
		
		if (!weaponSlotComponentSources)
		{
			return;
		}
		
		foreach	(IEntityComponentSource weaponSlotComponent : weaponSlotComponentSources)
		{
			ResourceName weaponPrefab;
			if (weaponSlotComponent.Get("WeaponTemplate", weaponPrefab))
			{
				if (!weaponPrefab)
				{
					continue;
				}
				
				IEntitySource weaponSource = SCR_BaseContainerTools.FindEntitySource(Resource.Load(weaponPrefab));
				if (!weaponSource)
				{
					continue;
				}
				
				IEntityComponentSource weaponComponentSource = SCR_BaseContainerTools.FindComponentSource(weaponSource, "WeaponComponent");
				if (!weaponComponentSource)
				{
					continue;
				}
				
				EWeaponType weaponType;
				if (weaponComponentSource.Get("WeaponType", weaponType))
				{
					AddLabelForWeaponType(weaponType);
				}
				
				CheckWeaponAttachments(weaponComponentSource);
			}
		}
	}
	
	void CheckWeaponAttachments(IEntityComponentSource weaponEntitySource)
	{
		BaseContainer weaponComponent = weaponEntitySource.ToBaseContainer();
		BaseContainerList weaponComponents = weaponComponent.GetObjectArray("components");
		
		// Check WeaponComponent child components
		for (int i=0; i<weaponComponents.Count(); i++)
		{
			BaseContainer weaponComp = weaponComponents.Get(i);
			if (weaponComp && weaponComp.GetClassName() == "AttachmentSlotComponent")
			{
				string attachmentType;
				weaponComp.Get("AttachmentType", attachmentType);
				if (attachmentType == "underbarrel")
				{
					BaseContainer attachmentSlot = weaponComp.GetObject("AttachmentSlot");
					ResourceName attachmentPrefab;
					if (attachmentSlot && attachmentSlot.Get("Prefab", attachmentPrefab))
					{
						// TODO check if underbarrel attachment is actually a grenade launcher.
						m_RoleLabels.Insert(EEditableEntityLabel.ROLE_GRENADIER);
					}	
				}		
			}
		}
	}
	
	void AddLabelForWeaponType(EWeaponType weaponType)
	{
		switch (weaponType)
		{
			case EWeaponType.WT_RIFLE : 
			{
				m_RoleLabels.Insert(EEditableEntityLabel.ROLE_RIFLEMAN);
				break;
			}
			case EWeaponType.WT_MACHINEGUN : 
			{
				m_RoleLabels.Insert(EEditableEntityLabel.ROLE_MACHINEGUNNER);
				break;
			}
			case EWeaponType.WT_ROCKETLAUNCHER : 
			{
				m_RoleLabels.Insert(EEditableEntityLabel.ROLE_ANTITANK);
				break;
			}
			case EWeaponType.WT_GRENADELAUNCHER : 
			{
				m_RoleLabels.Insert(EEditableEntityLabel.ROLE_GRENADIER);
				break;
			}
			case EWeaponType.WT_SNIPERRIFLE : 
			{
				m_RoleLabels.Insert(EEditableEntityLabel.ROLE_SHARPSHOOTER);
				break;
			}
			default: 
			{
				break;
			}
		}
	}
	
	void CheckLoadout(IEntitySource entitySource)
	{
		IEntityComponentSource inventoryManagerComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, BaseLoadoutManagerComponent);
		if (!inventoryManagerComponent)
		{
			return;
		}
		
		BaseContainerList slotList = inventoryManagerComponent.GetObjectArray("Slots");
		if (!slotList)
		{
			return;
		}
		
		for (int i = 0; i < slotList.Count() ; i++)
		{
			BaseContainer slot = slotList.Get(i);
			LoadoutAreaType slotArea;
			if (!slot.Get("AreaType", slotArea))
			{
				continue;
			}
			
			ResourceName slotPrefab;
			if (slotArea.IsInherited(LoadoutBackpackArea) && slot.Get("Prefab", slotPrefab))
			{
				Resource slotResource = Resource.Load(slotPrefab);
				IEntityComponentSource radioComponentSource = SCR_BaseContainerTools.FindComponentSource(slotResource, SCR_RadioComponent);
				ERadioCategory radioCategory;
				if (radioComponentSource && radioComponentSource.Get("m_iRadioCategory", radioCategory) && radioCategory == ERadioCategory.MANPACK)
				{
					m_RoleLabels.Insert(EEditableEntityLabel.ROLE_RADIOOPERATOR);
				}
			}
		}
	}
	
	void CheckInventory(IEntitySource entitySource)
	{
		IEntityComponentSource inventoryManagerComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_InventoryStorageManagerComponent);
		if (!inventoryManagerComponent)
		{
			return;
		}
		
		BaseContainerList inventoryItems = inventoryManagerComponent.GetObjectArray("InitialInventoryItems");
		if (!inventoryItems)
		{
			return;
		}
		
		int medicItemCount = 0;
		for (int i = 0; i < inventoryItems.Count() ; i++)
		{
			BaseContainer inventoryItem = inventoryItems.Get(i);
			
			array<ResourceName> prefabsToSpawn = {};
			inventoryItem.Get("PrefabsToSpawn", prefabsToSpawn);
			
			if (!prefabsToSpawn)
			{
				continue;
			}
			
			for (int j = 0; j < prefabsToSpawn.Count() ; j++)
			{
				Resource inventoryPrefab = Resource.Load(prefabsToSpawn[j]);
				if (!inventoryPrefab)
				{
					continue;
				}
				IEntityComponentSource consumableComponentSource = SCR_BaseContainerTools.FindComponentSource(inventoryPrefab, SCR_ConsumableItemComponent);
				if (consumableComponentSource)
				{
					BaseContainer consumableEffect = consumableComponentSource.GetObject("m_ConsumableEffect");
					if (consumableEffect)
					{
						SCR_EConsumableType consumableType;
						if (consumableEffect.Get("m_eConsumableType", consumableType) && consumableType == SCR_EConsumableType.BANDAGE)
						{
							medicItemCount++;
						}
					}
				}
			}
		}
		
		if (medicItemCount >= m_MedicItemCountRequired)
		{
			m_RoleLabels.Insert(EEditableEntityLabel.ROLE_MEDIC);
		}
	}
	
	bool GetRoleLabel(out EEditableEntityLabel label)
	{
		label = EEditableEntityLabel.NONE;
		
		if(m_RoleLabels.Contains(EEditableEntityLabel.ROLE_RADIOOPERATOR))
		{
			label = EEditableEntityLabel.ROLE_RADIOOPERATOR;
		}
		else if (m_RoleLabels.Contains(EEditableEntityLabel.ROLE_ANTITANK))
		{
			label = EEditableEntityLabel.ROLE_ANTITANK;
		}
		else if(m_RoleLabels.Contains(EEditableEntityLabel.ROLE_MACHINEGUNNER))
		{
			label = EEditableEntityLabel.ROLE_MACHINEGUNNER;
		}
		else if(m_RoleLabels.Contains(EEditableEntityLabel.ROLE_GRENADIER))
		{
			label = EEditableEntityLabel.ROLE_GRENADIER;
		}
		else if(m_RoleLabels.Contains(EEditableEntityLabel.ROLE_SHARPSHOOTER))
		{
			label = EEditableEntityLabel.ROLE_SHARPSHOOTER;
		}
		else if(m_RoleLabels.Contains(EEditableEntityLabel.ROLE_MEDIC))
		{
			label = EEditableEntityLabel.ROLE_MEDIC;
		}
		else if(m_RoleLabels.Contains(EEditableEntityLabel.ROLE_RIFLEMAN))
		{
			label = EEditableEntityLabel.ROLE_RIFLEMAN;
		}
		return label != EEditableEntityLabel.NONE;
	}

	void EditablePrefabsLabel_CharacterRole()
	{
		
	}
};
