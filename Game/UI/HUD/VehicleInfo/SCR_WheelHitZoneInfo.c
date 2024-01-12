class SCR_WheelHitZoneInfo : SCR_HitZoneInfo
{
	protected CarControllerComponent m_pCarController;
	protected CarControllerComponent_SA m_pCarController_SA;

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		EVehicleInfoState state = super.GetState();

		if(GetGame().GetIsClientAuthority())
		{
			if (!m_pCarController)
				return state;
	
			if (state != EVehicleInfoState.ERROR)
				return state;
	
			VehicleWheeledSimulation simulation = m_pCarController.GetSimulation();
			if (simulation && simulation.GetSpeedKmh() > 1)
				m_bIsBlinking = true;
	
			return state;
		}
		else
		{
			if (!m_pCarController_SA)
				return state;
	
			if (state != EVehicleInfoState.ERROR)
				return state;
	
			VehicleWheeledSimulation_SA simulation = m_pCarController_SA.GetSimulation();
			if (simulation && simulation.GetSpeedKmh() > 1)
				m_bIsBlinking = true;
	
			return state;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);
		if(GetGame().GetIsClientAuthority())
			m_pCarController = CarControllerComponent.Cast(owner.FindComponent(CarControllerComponent));
		else
			m_pCarController_SA = CarControllerComponent_SA.Cast(owner.FindComponent(CarControllerComponent_SA));
	}
};
