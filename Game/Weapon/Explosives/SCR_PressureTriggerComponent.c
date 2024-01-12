[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_PressureTriggerComponentClass : SCR_BaseTriggerComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PressureTriggerComponent : SCR_BaseTriggerComponent
{	
	protected const float MIN_DELAY = 125; //time between explosion tries in ms
	protected float m_fLastTryTime = 0;
	
	[Attribute("10", desc: "Min. weight that can set off this trigger when applied to it. [kg]")]
	protected float m_fMinWeight;
	
	//------------------------------------------------------------------------------------------------
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (GetGame().GetWorld().GetWorldTime() - m_fLastTryTime < MIN_DELAY)
			return;
		
		Physics otherPhysics = other.GetPhysics();
		if (!otherPhysics)
			return;
		
	 	float otherMass = otherPhysics.GetMass();

		if(GetGame().GetIsClientAuthority())
		{
			VehicleWheeledSimulation vehicleSimulation = VehicleWheeledSimulation.Cast(other.FindComponent(VehicleWheeledSimulation));
			if (vehicleSimulation)
				otherMass /= vehicleSimulation.WheelCount(); // assume it's a vehicle and assume min. weight it lays on the trigger is weight / wheels count
		}
		else
		{
			VehicleWheeledSimulation_SA vehicleSimulation = VehicleWheeledSimulation_SA.Cast(other.FindComponent(VehicleWheeledSimulation_SA));
			if (vehicleSimulation)
				otherMass /= vehicleSimulation.WheelCount(); // assume it's a vehicle and assume min. weight it lays on the trigger is weight / wheels count
		}
		
		m_fLastTryTime = GetGame().GetWorld().GetWorldTime();
		
		if (otherMass < m_fMinWeight)
			return; // Too light, won't set the trigger off
		
		BaseTriggerComponent baseTriggerComponent = BaseTriggerComponent.Cast(GetOwner().FindComponent(BaseTriggerComponent));
		if (!baseTriggerComponent)
			return;
		
		GetGame().GetCallqueue().CallLater(RPC_DoTrigger); // Delay it to next frame, cannot delete entity in EOnContact
		Rpc(RPC_DoTrigger);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Manually trigger the entity (Server Only)
	*/
	void TriggerManuallyServer()
	{
		//~ Delay it to next frame, cannot trigger entity at the same time as Rpc
		GetGame().GetCallqueue().CallLater(RPC_DoTrigger);
		Rpc(RPC_DoTrigger);
	}
	
	//------------------------------------------------------------------------------------------------
	// Only call this on the server
	override void ActivateTrigger()
	{
		super.ActivateTrigger();
		
		GenericEntity owner = GenericEntity.Cast(GetOwner());
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rplComponent || rplComponent.IsProxy())
			return;
		
		SetEventMask(owner, EntityEvent.CONTACT);
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void EOnInit(IEntity owner)
	{
		if (m_bLive)
			ActivateTrigger(); // Using call later to avoid accessing uninitialized components
	}
};
