[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotTriggerClass : SCR_ScenarioFrameworkSlotBaseClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkSlotTrigger : SCR_ScenarioFrameworkSlotBase
{
	/*
	[Attribute(defvalue: "5.0", UIWidgets.Slider, params: "1.0 1000.0 0.5", desc: "Radius of the trigger if selected", category: "Trigger")];
	protected float							m_fAreaRadius;
	
	
	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(TA_EActivationPresence), category: "Trigger")]
	protected TA_EActivationPresence		m_eActivationPresence;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")];
	protected bool 							m_bOnce;
	
	
	*/
	/*
	//TODO: prepared for dynamic invoker selection
	protected ref ScriptInvoker m_OnActivate = new ScriptInvoker();
	protected ref ScriptInvoker m_OnDeactivate = new ScriptInvoker();
	
	protected ref array<string> m_aInvokers = { "OnActivate", "OnDeactivate" };

	
	//------------------------------------------------------------------------------------------------
	array<string> GetInvokerArray()
	{
		return m_aInvokers;
	}
	*/
	
	[Attribute(desc: "Actions that will be performed after trigger conditions are true and the trigger itself activates (not the slot itself)", category: "OnActivation")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aTriggerActions;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		super.Init(area, activation);
		foreach(SCR_ScenarioFrameworkActionBase triggerAction : m_aTriggerActions)
		{
			triggerAction.Init(m_Entity);
		}
		
		if (!m_ParentLayer)
		{
			IEntity parentEntity = GetOwner().GetParent();
			if (!parentEntity)
				return;
		
			m_ParentLayer = SCR_ScenarioFrameworkLayerBase.Cast(parentEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (m_ParentLayer)
				m_ParentLayer.CheckAllChildrenSpawned(this);
		}
		/*
		if (m_eActivationType != activation)
			return;
		super.Init(area, activation);
		if (!BaseGameTriggerEntity.Cast(m_Entity))
		{
			Print("ScenarioFramework: SlotTrigger - The selected prefab is not trigger!");
			return;
		}
		BaseGameTriggerEntity.Cast(m_Entity).SetSphereRadius(m_fAreaRadius);
		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(m_Entity);
		if (trigger)
		{
			trigger.SetActivationPresence(m_eActivationPresence);
			trigger.SetOwnerFaction(m_sFaction);
			ScriptInvoker invoker = trigger.GetOnActivate();
			invoker.Insert(OnActivate);
		
			
		}
		*/
	}
	/*
	//------------------------------------------------------------------------------------------------
	void OnActivate()
	{
		if (!m_bOnce)
			return;
		
		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(m_Entity);
		if (trigger)
			trigger.Deactivate();
	}
		
	#ifdef WORKBENCH	
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if (m_sObjectToSpawn)
			super._WB_AfterWorldUpdate(owner, timeSlice);
	}
	#endif	
	
		
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkSlotTrigger(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		//m_sObjectToSpawn = "{698D02E7065F159B}Prefabs/Triggers/TriggerExtraction.et";
		#ifdef WORKBENCH
			m_iDebugShapeColor = ARGB(32, 0xFF, 0x00, 0x10);
			m_fDebugShapeRadius = m_fAreaRadius;
		#endif
	}
	*/
};

