class SCR_FieldManualConfigLoader
{
	//------------------------------------------------------------------------------------------------
	static SCR_FieldManualConfigRoot LoadConfigRoot(ResourceName configPath)
	{
		SCR_FieldManualConfigRoot result = SCR_ConfigHelperT<SCR_FieldManualConfigRoot>.GetConfigObject(configPath);
		if (!result)
			return null;

		if (result.m_aCategories)
		{
			foreach (SCR_FieldManualConfigCategory category : result.m_aCategories)
			{
				FillCategories(category);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected static void FillCategories(notnull SCR_FieldManualConfigCategory category)
	{
		if (category.m_aCategories)
		{
			foreach (SCR_FieldManualConfigCategory subCategory : category.m_aCategories)
			{
				FillCategories(subCategory);
			}
		}

		if (category.m_aEntries)
		{
			FillEntries(category.m_aEntries);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void FillEntries(notnull array<ref SCR_FieldManualConfigEntry> entries)
	{
		foreach (SCR_FieldManualConfigEntry entry : entries)
		{
			SCR_FieldManualConfigEntry_Weapon weaponEntry = SCR_FieldManualConfigEntry_Weapon.Cast(entry);
			if (weaponEntry)
			{
				FillEntries_Weapon(weaponEntry);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void FillEntries_Weapon(notnull SCR_FieldManualConfigEntry_Weapon weaponEntry)
	{
		if (weaponEntry.m_WeaponEntityPath.IsEmpty())
			return;

		SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper = new SCR_FieldManualUI_WeaponStatsHelper(weaponEntry.m_WeaponEntityPath);

		if (weaponEntry.m_sTitle.IsEmpty())
			weaponEntry.m_sTitle = weaponStatsHelper.GetDisplayName();

		if (weaponEntry.m_Image.IsEmpty())
			weaponEntry.m_Image = weaponStatsHelper.GetInventoryIcon();

		if (!weaponEntry.m_aContent)
		{
			weaponEntry.m_aContent = {};
		}

		if (weaponEntry.m_aContent.Count() == 1)
		{
			SCR_FieldManualPiece_Text piece = SCR_FieldManualPiece_Text.Cast(weaponEntry.m_aContent[0]);
			if (piece && piece.m_sText.IsEmpty())
				piece.m_sText = weaponStatsHelper.GetDescription();
		}

		weaponEntry.m_WeaponStatsHelper = weaponStatsHelper;
	}
};
