[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_CharUnconsciousnessEditorAttribute : SCR_BaseEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		IEntity owner = editableEntity.GetOwner();
		if (!owner) 
			return null;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return null;
		
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!characterDamageManager) 
			return null;
		
		if (characterDamageManager.GetState() == EDamageState.DESTROYED)  
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(characterDamageManager.GetPermitUnconsciousness());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		IEntity owner =  editableEntity.GetOwner();
		if (!owner) 
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return;
		
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!characterDamageManager) 
			return;
		
		if (characterDamageManager.GetState() == EDamageState.DESTROYED)
			return;
		
		if (!characterDamageManager.IsDamageHandlingEnabled())
			return;
		
		//Neutralize character if unconsciousness is disabled and character is already unconscious
		if (characterDamageManager.GetIsUnconscious() && !var.GetBool())
			characterDamageManager.Kill(Instigator.CreateInstigator(null));
		
		characterDamageManager.SetPermitUnconsciousness(var.GetBool(), true);
	}
};
