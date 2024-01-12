//-----------------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Mines", description: "Damage manager for land mines.")]
class SCR_MineDamageManagerClass : SCR_DamageManagerComponentClass
{
	
};

//-----------------------------------------------------------------------------------------------------------
class SCR_MineDamageManager : SCR_DamageManagerComponent
{
	//-----------------------------------------------------------------------------------------------------------
	override event protected void OnDamageStateChanged(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		IEntity owner = GetOwner();
		
		bool isProxy = false; 
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && rplComponent.IsProxy())
			isProxy = true;
		
		if (!isProxy)
			GetGame().GetCallqueue().CallLater(ExplodeWrapper, 1);
		else
			ExplodeWrapper();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void ExplodeWrapper()
	{
		IEntity owner = GetOwner();
		BaseTriggerComponent triggerComponent = BaseTriggerComponent.Cast(owner.FindComponent(BaseTriggerComponent));
		if (!triggerComponent)
			return;
		
		triggerComponent.SetLive();
		triggerComponent.OnUserTriggerOverrideInstigator(owner, GetInstigator());
	}
}
