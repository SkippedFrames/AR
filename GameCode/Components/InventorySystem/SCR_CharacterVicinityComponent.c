
class SCR_CharacterVicinityComponentClass: CharacterVicinityComponentClass
{
};

class SCR_CharacterVicinityComponent: CharacterVicinityComponent
{

	ref ScriptInvoker OnVicinityUpdateInvoker = new ref ScriptInvoker();
	override protected void OnUpdate()
	{
		OnVicinityUpdateInvoker.Invoke();
	}
};
