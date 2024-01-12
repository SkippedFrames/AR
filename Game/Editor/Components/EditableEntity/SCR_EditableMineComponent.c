[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableMineComponentClass: SCR_EditableSystemComponentClass
{
};

/** @ingroup Editable_Entities
*/

/*!
Editable Mine
*/
class SCR_EditableMineComponent: SCR_EditableSystemComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		//~ Activate Mine on server one frame later so the SCR_PressureTriggerComponent can initialize
		if (IsServer())	
			GetGame().GetCallqueue().CallLater(ActivateMine);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Make sure the mine is activated so it explodes when a vehicle drives over it
	protected void ActivateMine()
	{
		SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (pressureTriggerComponent)
			pressureTriggerComponent.ActivateTrigger();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsDestroyed()
	{
		//~ If this component exists it is not destroyed
		return false;		
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanDestroy()
	{
		//~ If this component exists it can be destroyed
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Destroy()
	{		
		if (!IsServer())
			return false;
		
		SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (pressureTriggerComponent)
		{
			pressureTriggerComponent.TriggerManuallyServer();
			return true;
		}

		return false;
	}
}
