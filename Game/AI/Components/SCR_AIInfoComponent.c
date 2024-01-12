[ComponentEditorProps(category: "GameScripted/AI", description: "Component for AI checking state of characters")]
class SCR_AIInfoComponentClass: SCR_AIInfoBaseComponentClass
{
};

enum EUnitRole
{
	NONE 			= 0,
	RIFLEMAN 		= 1,
	MEDIC 			= 2,
	MACHINEGUNNER 	= 4,
	AT_SPECIALIST 	= 8,
	GRENADIER 		= 16,
};

enum EUnitState
{
	NONE 			= 0,
	WOUNDED 		= 1,
	IN_TURRET		= 2,
	IN_VEHICLE		= 4,
	UNCONSCIOUS		= 8
};

enum EUnitAIState
{
	AVAILABLE,
	BUSY,
	UNRESPONSIVE,
};

//------------------------------------------------------------------------------------------------
class SCR_AIInfoComponent : SCR_AIInfoBaseComponent
{
	protected EUnitState m_iUnitStates;
	protected EUnitAIState m_iAIStates;	
	protected SCR_InventoryStorageManagerComponent m_inventoryManagerComponent;
	protected BaseWeaponManagerComponent m_weaponManagerComponent;
	protected SCR_CompartmentAccessComponent m_CompartmentAccessComponent;
	protected SCR_AIThreatSystem m_ThreatSystem;
	protected ScriptedDamageManagerComponent m_DamageManager;
	protected SCR_AICombatComponent m_CombatComponent;
	protected EventHandlerManagerComponent m_EventHandlerManagerComponent;
	PerceptionComponent m_Perception;
	
	protected ECharacterStance m_eStance;
	protected EMovementType m_eMovementType;
	protected bool m_bWeaponRaised;
	protected int m_iAttackCount;
	protected int m_unitID;
	
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	void OnVehicleEntered( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID )
	{
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		if (TurretCompartmentSlot.Cast(compSlot))
			AddUnitState(EUnitState.IN_TURRET);
	}
	
	void OnVehicleLeft( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID )
	{
		auto aiworld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiworld)
			return;
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		if (!TurretCompartmentSlot.Cast(compSlot))
			return;
		
		RemoveUnitState(EUnitState.IN_TURRET);
		
		AIAgent owner = AIAgent.Cast(GetOwner());
		if (!owner)
			return;
		AICommunicationComponent mailbox = owner.GetCommunicationComponent();
		if (mailbox)
		{
			SCR_AIMessage_AttackStaticDone msg1 = new SCR_AIMessage_AttackStaticDone();

			msg1.SetText("Get out of turret");
			msg1.SetReceiver(owner);
			mailbox.RequestBroadcast(msg1,owner);
		}
	}
	
	override protected void EOnInit(IEntity owner)
	{
		IEntity ent = owner;
		AIAgent agent = AIAgent.Cast(owner);
		if (agent)
			ent = agent.GetControlledEntity();
		
		if (ent)
		{
			m_inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(ent.FindComponent(SCR_InventoryStorageManagerComponent));
			m_weaponManagerComponent = BaseWeaponManagerComponent.Cast(ent.FindComponent(BaseWeaponManagerComponent));
			m_CompartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(ent.FindComponent(SCR_CompartmentAccessComponent));
			m_DamageManager = ScriptedDamageManagerComponent.Cast(ent.FindComponent(ScriptedDamageManagerComponent));
			m_CombatComponent = SCR_AICombatComponent.Cast(ent.FindComponent(SCR_AICombatComponent));
			m_EventHandlerManagerComponent = EventHandlerManagerComponent.Cast(ent.FindComponent(EventHandlerManagerComponent));
			
			if (m_EventHandlerManagerComponent)
				m_EventHandlerManagerComponent.RegisterScriptHandler("OnConsciousnessChanged", this, this.OnConsciousnessChanged, true);
			
			CharacterControllerComponent characterController = CharacterControllerComponent.Cast(ent.FindComponent(CharacterControllerComponent));
			if (characterController)
				OnConsciousnessChanged(!characterController.IsUnconscious());
			
			m_Perception = PerceptionComponent.Cast(ent.FindComponent(PerceptionComponent));
		}
		
		if (m_CompartmentAccessComponent)
		{
			m_CompartmentAccessComponent.GetOnCompartmentEntered().Insert(OnVehicleEntered);
			m_CompartmentAccessComponent.GetOnCompartmentLeft().Insert(OnVehicleLeft);
		}
		
		if (m_DamageManager)
		{
			m_DamageManager.GetOnDamageOverTimeAdded().Insert(OnDamageOverTimeAdded);
			m_DamageManager.GetOnDamageOverTimeRemoved().Insert(OnDamageOverTimeRemoved);
			EvaluateWoundedState();
		}
	}
	
	void ~SCR_AIInfoComponent()
	{
		if (m_EventHandlerManagerComponent)
			m_EventHandlerManagerComponent.RemoveScriptHandler("OnConsciousnessChanged", this, this.OnConsciousnessChanged, true);
	}
	
	override protected void OnDelete(IEntity owner)
	{
		if (m_CompartmentAccessComponent)
		{
			m_CompartmentAccessComponent.GetOnCompartmentEntered().Remove(OnVehicleEntered);
			m_CompartmentAccessComponent.GetOnCompartmentLeft().Remove(OnVehicleLeft);
		}
		
		if (m_DamageManager)
		{
			m_DamageManager.GetOnDamageOverTimeAdded().Remove(OnDamageOverTimeAdded);
			m_DamageManager.GetOnDamageOverTimeRemoved().Remove(OnDamageOverTimeRemoved);
		}
	}
	
	bool IsOwnerAgent(AIAgent agent)
	{
		return GetOwner() == agent;
	}
	
//----------- BIT operations on Roles

	bool HasRole(EUnitRole role)
	{
		switch (role)
		{
			case EUnitRole.MEDIC:			return m_inventoryManagerComponent.GetHealthComponentCount() > 0;
			case EUnitRole.MACHINEGUNNER:	return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_MACHINEGUN);
			case EUnitRole.RIFLEMAN:		return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_RIFLE);
			case EUnitRole.AT_SPECIALIST:	return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_ROCKETLAUNCHER);
			case EUnitRole.GRENADIER:		return m_CombatComponent.HasWeaponOfType(EWeaponType.WT_GRENADELAUNCHER); // todo right now it will not detect a UGL muzzle, because weapon type is still rifle
		}
		
		return false;
	}
	
	// ! Use only for debugging!
	EUnitRole GetRoles()
	{
		typename t = EUnitRole;
		int tVarCount = t.GetVariableCount();
		EUnitRole roles = 0;
		for (int i = 0; i < tVarCount; i++)
		{
			EUnitRole flag;
			t.GetVariableValue(null, i, flag);
			if (flag && HasRole(flag))
				roles |= flag;
		}
		return roles;
	}
	
//---------- BIT operation on States
	void AddUnitState(EUnitState state)
	{
		m_iUnitStates = m_iUnitStates | state;
	}
	
	void RemoveUnitState(EUnitState state)
	{
		if (HasUnitState(state))
			m_iUnitStates = m_iUnitStates & ~state;
	}
	
	bool HasUnitState(EUnitState state)
	{
		return ( m_iUnitStates & state );
	}
	
	// ! Use only for debugging!
	EUnitState GetUnitStates()
	{
		return m_iUnitStates;
	}
	
//--------- AI states are disjoined - one can be in only one state at the time
	void SetAIState(EUnitAIState state)
	{
		m_iAIStates = state; 
	}
	
	EUnitAIState GetAIState()
	{
		return m_iAIStates;
	}
	
//--------  Info about magazines available to SCR_AIResupplyActivity
	
	int GetMagazineCountByWellType(typename magazinyWellType)
	{
		return m_CombatComponent.GetMagazineCount(magazinyWellType, false);
	}
		
//-------- 	Set or get AI stance, speed, raising weapon etc.
	void SetStance(ECharacterStance stance)
	{
		m_eStance = stance;
	}
	
	ECharacterStance GetStance()
	{
		return m_eStance;
	}
	
	void SetMovementType(EMovementType mode)
	{
		m_eMovementType = mode;
	}
	
	EMovementType GetMovementType()
	{
		return m_eMovementType;
	}
	
	void SetWeaponRaised(bool raised)
	{
		m_bWeaponRaised = raised;
	}
	
	bool GetWeaponRaised()
	{
		return m_bWeaponRaised;
	}
	
	void InitThreatSystem(SCR_AIThreatSystem threatSystem)
	{
		m_ThreatSystem = threatSystem;	
	}
	
	EAIThreatState GetThreatState()
	{
		if (m_ThreatSystem)
			return m_ThreatSystem.GetState();
		else 
			return EAIThreatState.SAFE;
	}
	
	
//-------- 	Evaluation of wounded state of AI
	//! Returns true when state has changed and we must invoke an event
	protected void EvaluateWoundedState()
	{
		bool wounded = m_DamageManager.IsDamagedOverTime(EDamageType.BLEEDING);
		if (wounded)
			AddUnitState(EUnitState.WOUNDED);
		else
			RemoveUnitState(EUnitState.WOUNDED);
	}
	
	protected void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz)
	{
		if (dType != EDamageType.BLEEDING)
			return;
		
		EvaluateWoundedState();
	}
	
	protected void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		if (dType != EDamageType.BLEEDING)
			return;
		
		EvaluateWoundedState();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnConsciousnessChanged(bool conscious)
	{
		if (conscious)
			RemoveUnitState(EUnitState.UNCONSCIOUS);
		else
			AddUnitState(EUnitState.UNCONSCIOUS);
	}
	
//-------- Debugging
	// Used in SCR_AIDebugInfoComponent
	void DebugPrintToWidget(TextWidget w)
	{
		string str;
		str = str + string.Format("\n%1 %2", m_iAIStates, typename.EnumToString(EUnitAIState, m_iAIStates));
		w.SetText(str);
	}
	
	
	
	string GetBehaviorEditorDebugName()
	{
		AIAgent agent = AIAgent.Cast(GetOwner());
		
		SCR_CallsignCharacterComponent callsignComp = SCR_CallsignCharacterComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CallsignCharacterComponent));
		
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(agent.GetControlledEntity().FindComponent(FactionAffiliationComponent));
		
		string str;
		
		if (factionComp)
		{
			string faction = factionComp.GetAffiliatedFaction().GetFactionKey();
			str = str + string.Format("[%1] ", faction);
		}
		
		if (callsignComp)
		{
			string company, platoon, squad, character, format;
			bool setCallsign = callsignComp.GetCallsignNames(company, platoon, squad, character, format);
			if (setCallsign)
			{
				string callsign = WidgetManager.Translate(format, company, platoon, squad, character);
				str = str + string.Format(" %1", callsign);
			}
		}
		return str;
	}
};
