[ComponentEditorProps(category: "GameScripted/Trigger", description: "Gets the BaseTriggerComponent on the entity and triggers it on EOnInit")]
class SCR_InstantTriggerComponentClass: ScriptComponentClass
{
	//Needs BaseTriggerComponent
	static override array<typename> Requires(IEntityComponentSource src)
	{
		array<typename> requires = new array<typename>;
		
		requires.Insert(BaseTriggerComponent);
		
		return requires;
	}
};
class SCR_InstantTriggerComponent: ScriptComponent
{
	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		
		BaseTriggerComponent trigger = BaseTriggerComponent.Cast(owner.FindComponent(BaseTriggerComponent));
		if (trigger)
			trigger.OnUserTrigger(owner);
		else
			Print("'SCR_InstantTriggerComponent' could not find 'BaseTriggerComponent'!", LogLevel.ERROR);
	}
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

};
