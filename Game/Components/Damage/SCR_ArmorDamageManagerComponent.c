//------------------------------------------------------------------------------------------------
class SCR_ArmorDamageManagerComponentClass : ScriptedDamageManagerComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ArmorDamageManagerComponent : ScriptedDamageManagerComponent
{
	[Attribute(defvalue: "0.4", uiwidget: UIWidgets.EditBox, desc: "Multiplies rawdamage to be passed to character from this armor piece")]
	protected float m_fPassedDamageScale;

	//------------------------------------------------------------------------------------------------
	override protected void OnDamage(
				  EDamageType type,
				  float damage,
				  HitZone pHitZone,
				  notnull Instigator instigator, 
				  inout vector hitTransform[3], 
				  float speed,
				  int colliderID, 
				  int nodeID)
	{
		if (type != EDamageType.KINETIC && type != EDamageType.MELEE)
			return;

		IEntity parent = GetOwner();
		while (parent)
		{
			if (ChimeraCharacter.Cast(parent))
				break;

			parent = parent.GetParent();
		}
		
		ChimeraCharacter char = ChimeraCharacter.Cast(parent);
		if (!char)
			return;
		
		SCR_CharacterDamageManagerComponent charDamMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!charDamMan)
			return;

		charDamMan.ArmorHitEventEffects(damage);
		charDamMan.ArmorHitEventDamage(type, damage * m_fPassedDamageScale, instigator.GetInstigatorEntity());
	}
}
