[ComponentEditorProps(category: "GameScripted/Weapon", description:"Keeps settings for melee weapon")]
class SCR_MeleeWeaponPropertiesClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MeleeWeaponProperties : ScriptComponent
{
	[Attribute("10", UIWidgets.Slider, "Size of damage dealt by the weapon", "0.0 100.0 1.0", category: "Global")]
	private float m_fDamage;	
	[Attribute("1", UIWidgets.Slider, "Range of the weapon [m]", "1 5 0.5", category: "Global")]
	private float m_fRange;

	//------------------------------------------------------------------------------------------------	
	//! Value of damage dealt to the target
	float GetWeaponDamage()
	{
		return m_fDamage;
	}

	//------------------------------------------------------------------------------------------------	
	//! Range in meters that is used as max raycast length
	float GetWeaponRange()
	{
		return m_fRange;
	}
};
