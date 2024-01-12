[EntityEditorProps(category: "GameScripted/Components", description: "Registers characters and holds them in an array.", color: "0 0 255 255")]
class SCR_CharacterRegistrationComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CharacterRegistrationComponent : ScriptComponent
{
	static ref array<SCR_ChimeraCharacter> s_aChimeraCharacters = new ref array<SCR_ChimeraCharacter>();
	
	IEntity m_owner;
	
	//------------------------------------------------------------------------------------------------
	static array<SCR_ChimeraCharacter> GetChimeraCharacters()
	{
		return s_aChimeraCharacters;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (!s_aChimeraCharacters)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner);
		if (character && s_aChimeraCharacters.Find(character) != -1)
			s_aChimeraCharacters.RemoveItem(character);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_owner = owner;
		
		if (!s_aChimeraCharacters)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner);
		if (character)
			s_aChimeraCharacters.Insert(character);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CharacterRegistrationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CharacterRegistrationComponent()
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_owner);
		if (character && s_aChimeraCharacters.Find(character) != -1)
			s_aChimeraCharacters.RemoveItem(character);
	}

};
