//------------------------------------------------------------------------------------------------
class SCR_TaskDefendClass: SCR_ScenarioFrameworkTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TaskDefend : SCR_ScenarioFrameworkTask
{	
	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Asset || !m_SupportEntity)
			return;
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(ScriptedDamageManagerComponent.GetDamageManager(m_Asset));
		if (objectDmgManager)
		{
			objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
			GetGame().GetCallqueue().Remove(CheckEngineDrowned);
			
			VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Asset.FindComponent(VehicleControllerComponent_SA));
			if (vehicleController)
				vehicleController.GetOnEngineStop().Remove(CheckEngineDrowned);
		}
				
		m_SupportEntity.FailTask(this);				
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckEngineDrowned()
	{
		if (!m_Asset || !m_SupportEntity)
			return;
		
		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Asset.FindComponent(VehicleControllerComponent_SA));
		if (vehicleController && vehicleController.GetEngineDrowned())
		{
			vehicleController.GetOnEngineStop().Remove(CheckEngineDrowned);
			GetGame().GetCallqueue().Remove(CheckEngineDrowned);
			
			ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(ScriptedDamageManagerComponent.GetDamageManager(m_Asset));
			if (objectDmgManager)
		 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
			
			m_SupportEntity.FailTask(this);	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void RehookTaskSubject(IEntity object)
	{
		if (!object)
			return;
		
		m_Asset = object;
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(ScriptedDamageManagerComponent.GetDamageManager(m_Asset));
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		
		if (Vehicle.Cast(m_Asset))
		{
			VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Asset.FindComponent(VehicleControllerComponent_SA));
			if (vehicleController)
				vehicleController.GetOnEngineStop().Insert(CheckEngineDrowned);
			
			// Since there is no invoker and no reliable way how to tackle drowned vehicles, in order to make it reliable,
			// We cannot solely rely on GetOnEngineStop because vehicle could have been pushed/moved into the water without started engine.
			GetGame().GetCallqueue().CallLater(CheckEngineDrowned, 5000, true);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		m_SupportEntity = SCR_ScenarioFrameworkTaskDefendSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDefendSupportEntity));
		
		if (!m_SupportEntity)
		{
			Print("ScenarioFramework: Task Defend support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
					
		if (!m_Asset)
			return;	
				
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(ScriptedDamageManagerComponent.GetDamageManager(m_Asset));
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		
		if (Vehicle.Cast(m_Asset))
		{
			VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(m_Asset.FindComponent(VehicleControllerComponent_SA));
			if (vehicleController)
				vehicleController.GetOnEngineStop().Insert(CheckEngineDrowned);
			
			// Since there is no invoker and no reliable way how to tackle drowned vehicles, in order to make it reliable,
			// We cannot solely rely on GetOnEngineStop because vehicle could have been pushed/moved into the water without started engine.
			GetGame().GetCallqueue().CallLater(CheckEngineDrowned, 5000, true);
		}
	}
}
