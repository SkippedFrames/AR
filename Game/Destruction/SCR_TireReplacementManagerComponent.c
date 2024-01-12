[ComponentEditorProps(category: "GameScripted/Destruction", description: "Tire destruction manager component, manages replacing destructible tires with a present spare tire.")]
class SCR_TireReplacementManagerComponentClass: ScriptComponentClass
{
};

class SCR_TireReplacementManagerComponent : ScriptComponent
{
	#ifdef DEBUG_VEHICLES
	[Attribute("0", UIWidgets.CheckBox, "If true, gizmos will be shown indicating whether or not the raycast hit any entities", category: "Debug")]
	protected bool m_bDebugCanReplace;
	#endif
	
	[Attribute("0", UIWidgets.CheckBox, "If true, the user can replace an active wheel by a more damaged wheel", category: "Replacement Conditions")]
	protected bool m_bAllowMoreDamagedTire;
	[Attribute("1", UIWidgets.Auto, "The radius of the tracesphere, used to check whether or not the area is clear for replacing", category: "Replacement Conditions")]
	protected float m_WheelTraceRadius;
	[Attribute("", UIWidgets.Auto, "The radius of the tracesphere, used to check whether or not the area is clear for replacing", category: "Replacement Conditions")]
	protected vector m_WheelTraceOffset;
	
	#ifdef ENABLE_DESTRUCTION
	protected ref Shape m_DebugShape;
	protected ref array<SCR_DestructionTireComponent> 	m_aDestructionTires 		= new array<SCR_DestructionTireComponent>;
	protected ref array<GenericEntity> 					m_aDestructionTireEntities 	= new array<GenericEntity>;
		
	protected SCR_SpareTireComponent					m_SpareTireComponent;
	protected SCR_DestructionTireComponent				m_SpareDestructionTire;
	protected GenericEntity 							m_SpareTireEntity;
	
	protected IEntity 									m_Owner;
	
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner) 
	{
		m_Owner = owner;
		auto ownerEntity = GenericEntity.Cast(owner);
		if(!ownerEntity)
			return;
		
		int wheelCount = 0;
		if(GetGame().GetIsClientAuthority())
		{
			auto simulation = VehicleWheeledSimulation.Cast(ownerEntity.FindComponent(VehicleWheeledSimulation));
			if(!simulation)
				return;
			
			 wheelCount = simulation.WheelCount();
		}
		else
		{
			auto simulation = VehicleWheeledSimulation_SA.Cast(ownerEntity.FindComponent(VehicleWheeledSimulation_SA));
			if(!simulation)
				return;
			
			 wheelCount = simulation.WheelCount();
		}
		
		m_aDestructionTires.Resize(wheelCount);
		m_aDestructionTireEntities.Resize(wheelCount);
		
		array<Managed> slots = new array<Managed>();
		ownerEntity.FindComponents(BaseSlotComponent, slots);
		
		foreach (Managed component : slots)
		{
			BaseSlotComponent slot = BaseSlotComponent.Cast(component);
			if (!slot)
				continue;

			GenericEntity slotEntity = GenericEntity.Cast(slot.GetAttachedEntity());
			if (!slotEntity)
				continue;
			
			SCR_WheelSlotComponent wheelSlotComponent = SCR_WheelSlotComponent.Cast(slot);
			if(wheelSlotComponent)
			{
				if(wheelSlotComponent.m_iWheelIndex < 0 || wheelSlotComponent.m_iWheelIndex >= wheelCount)
				{
					Print("Wheel Index out of bounds. Index: " + wheelSlotComponent.m_iWheelIndex.ToString() + ". WheelCount: " + wheelCount);
					continue;
				}
				
				m_aDestructionTires.Set(wheelSlotComponent.m_iWheelIndex, SCR_DestructionTireComponent.Cast(slotEntity.FindComponent(SCR_DestructionTireComponent)));
				m_aDestructionTireEntities.Set(wheelSlotComponent.m_iWheelIndex, slotEntity);
			}
			
			if(m_SpareTireComponent)
				continue;
			
			SCR_SpareTireComponent spareTire = SCR_SpareTireComponent.Cast(slotEntity.FindComponent(SCR_SpareTireComponent));
			if(spareTire)
			{
				m_SpareTireComponent = spareTire;
				m_SpareTireEntity = slotEntity;
				m_SpareDestructionTire = SCR_DestructionTireComponent.Cast(slotEntity.FindComponent(SCR_DestructionTireComponent));
				continue;
			}		

		}
	}
	
	bool CanTireBeReplaced(int pWheelIndex)
	{
		auto destructionTire = m_aDestructionTires.Get(pWheelIndex);
		if(!destructionTire)
			return false;
		
		bool canBeShown = destructionTire.GetDamagePhase() > m_SpareDestructionTire.GetDamagePhase();
		canBeShown |= (destructionTire.GetDamagePhase() == m_SpareDestructionTire.GetDamagePhase() && destructionTire.GetHealth() < m_SpareDestructionTire.GetHealth());

		return canBeShown;
	}
	
	bool IsWheelAreaClear(int pWheelIndex, notnull IEntity pActionUser)
	{
		IEntity wheelEntity = m_aDestructionTireEntities.Get(pWheelIndex);
		vector mins, maxs, mat[4];
		pActionUser.GetWorldTransform(mat);
		wheelEntity.GetBounds(mins, maxs);

		autoptr TraceSphere param = new TraceSphere;
		param.Radius = m_WheelTraceRadius;
		param.Start = wheelEntity.CoordToParent(m_WheelTraceOffset);
		param.Flags = TraceFlags.ENTS;
		array<IEntity> excludeArray = {pActionUser, m_Owner};
		param.ExcludeArray = excludeArray;
		
		float traceResult = wheelEntity.GetWorld().TracePosition(param, null);
		
		#ifdef DEBUG_VEHICLES
		int color;
		if(traceResult >= 0)
			color = Color.GREEN;
		else
			color = Color.RED;
		
		m_DebugShape = Shape.CreateSphere(color, ShapeFlags.VISIBLE|ShapeFlags.NOOUTLINE, wheelEntity.CoordToParent(-vector.Right * 1), 1);
		#endif
		
		return traceResult >=0;
	}
	
	void InitReplace(int pWheelIndex)
	{
		auto destructionTireComponent = m_aDestructionTires.Get(pWheelIndex);
		if(!destructionTireComponent)
			return;
		
		int myDamagePhase = destructionTireComponent.GetDamagePhase();
		float myHealth = destructionTireComponent.GetHealth();
		
		//TODO @Zguba Level1: Properly implement damage here. This makes no sense.
		destructionTireComponent.GoToDamagePhase(m_SpareDestructionTire.GetDamagePhase());
		destructionTireComponent.SetHitZoneDamage(0);
		destructionTireComponent.GetDefaultHitZone().HandleDamage(m_SpareDestructionTire.GetHealth());
		
		m_SpareDestructionTire.GoToDamagePhase(myDamagePhase);
		m_SpareDestructionTire.SetHitZoneDamage(0);
		m_SpareDestructionTire.GetDefaultHitZone().HandleDamage(myDamage);
	}
	#endif
};
