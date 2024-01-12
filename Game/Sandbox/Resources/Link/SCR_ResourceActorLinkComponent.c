[ComponentEditorProps(category: "GameScripted/Resources", description: "")]
class SCR_ResourceActorLinkComponentClass : ScriptComponentClass
{

};

class SCR_ResourceActorLinkComponent : ScriptComponent
{
	protected SCR_ResourceActor m_Actor;
	
	void SetActor(SCR_ResourceActor actor)
	{
		m_Actor = actor;
	}
	
	SCR_ResourceActor GetActor()
	{
		return m_Actor;
	}
};
