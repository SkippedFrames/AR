class SCR_FieldManualUI_WeaponStatsHelper
{
	protected string m_sDisplayName;
	protected string m_sDescription;
	protected ResourceName m_InventoryIcon;

	protected int m_iDefaultZeroing;
	protected ref array<string> m_aFireModes;
	int m_iMuzzleVelocity;
	protected int m_iRateOfFire;
	protected float m_fWeight;
	protected ref array<int> m_aZeroing;

	//------------------------------------------------------------------------------------------------
	string GetDisplayName()
	{
		return m_sDisplayName;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetInventoryIcon()
	{
		return m_InventoryIcon;
	}

	//------------------------------------------------------------------------------------------------
	string GetDescription()
	{
		return m_sDescription;
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetFireModes()
	{
		return m_aFireModes;
	}

	//------------------------------------------------------------------------------------------------
	int GetRateOfFire()
	{
		return m_iRateOfFire;
	}

	//------------------------------------------------------------------------------------------------
	float GetWeight()
	{
		return m_fWeight;
	}

	//------------------------------------------------------------------------------------------------
	array<int> GetZeroing()
	{
		return m_aZeroing;
	}

	//------------------------------------------------------------------------------------------------
	int GetDefaultZeroing()
	{
		return m_iDefaultZeroing;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_FieldManualUI_WeaponStatsHelper(ResourceName weaponResource)
	{
		if (!weaponResource)
		{
			return;
		}

		m_aFireModes = {};
		m_aZeroing = {};

		Resource resource = Resource.Load(weaponResource);
		if (!resource.IsValid())
		{
			return;
		}

		IEntity entity = GetGame().SpawnEntityPrefabLocal(resource, null, null);
		if (!entity)
		{
			return;
		}

		WeaponComponent weaponComponent = WeaponComponent.Cast(entity.FindComponent(WeaponComponent));
		if (!weaponComponent)
		{
			return;
		}

		WeaponAttachmentsStorageComponent weaponAttachmentsStorageComponent = WeaponAttachmentsStorageComponent.Cast(entity.FindComponent(WeaponAttachmentsStorageComponent));

		MuzzleComponent muzzleComponent = MuzzleComponent.Cast(weaponComponent.FindComponent(MuzzleComponent));
		WeaponAttachmentsStorageComponent storageComponent = WeaponAttachmentsStorageComponent.Cast(entity.FindComponent(WeaponAttachmentsStorageComponent));
		BaseSightsComponent sightsComponent = weaponComponent.GetSights();

		// WeaponComponent
		m_iDefaultZeroing = Math.Round(weaponComponent.GetCurrentSightsZeroing());
		m_iMuzzleVelocity = weaponComponent.GetInitialProjectileSpeed();
		UIInfo weaponComponentUIInfo = weaponComponent.GetUIInfo();
		if (weaponComponentUIInfo != null)
		{
			m_sDisplayName = weaponComponentUIInfo.GetName();
			m_sDescription = weaponComponentUIInfo.GetDescription();
		}

		// WeaponAttachmentsStorageComponent
		if (weaponAttachmentsStorageComponent != null)
		{
			ItemAttributeCollection collection = weaponAttachmentsStorageComponent.GetAttributes();
			UIInfo uiInfo = collection.GetUIInfo();
			WeaponUIInfo weaponUIInfo = WeaponUIInfo.Cast(uiInfo);
			if (weaponUIInfo != null)
			{
				//Warka: Hotfix, as the GetWeaponIconPath() was removed from the WeaponUIInfo class
				//m_InventoryIcon = weaponUIInfo.GetWeaponIconPath();
				
				m_InventoryIcon = "";
			}
		}

		// MuzzleComponent
		if (muzzleComponent != null)
		{
			array<BaseFireMode> fireModes = {};
			muzzleComponent.GetFireModesList(fireModes);

			foreach (BaseFireMode fireMode : fireModes)
			{
				if (fireMode && fireMode.GetBurstSize() != 0)
				{
					m_aFireModes.Insert(fireMode.GetUIName());
					if (fireMode.GetShotSpan() > 0)
					{
						int rateOfFire = Math.Round(60 / fireMode.GetShotSpan());
						if (rateOfFire > m_iRateOfFire)
						{
							m_iRateOfFire = rateOfFire;
						}
					}
				}
			}
		}

		// WeaponAttachmentsStorageComponent
		if (storageComponent != null)
		{
			m_fWeight = storageComponent.GetTotalWeight();
		}

		// BaseSightsComponent
		if (sightsComponent != null)
		{
			// TODO when API is available
		}

		delete entity;
	}
};
