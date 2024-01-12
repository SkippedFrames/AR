class DeployMenuSystem : GameSystem
{	
	protected SCR_PlayerDeployMenuHandlerComponent m_MenuHandler;
	protected bool m_bReady = false;

	protected override ESystemPoint GetSystemPoint()
	{
		return ESystemPoint.Frame;
	}

	protected override void OnInit()
	{
		SCR_RespawnSystemComponent rsc = SCR_RespawnSystemComponent.GetInstance();
		Enable(
			!System.IsConsoleApp() && 
			(rsc && rsc.CanOpenDeployMenu())
		);
	}

	void Register(SCR_PlayerDeployMenuHandlerComponent handler)
	{
		if (!m_MenuHandler)
			m_MenuHandler = handler;
	}

	void Unregister(SCR_PlayerDeployMenuHandlerComponent handler)
	{
		m_MenuHandler = null;
	}

	void SetReady(bool ready)
	{
		m_bReady = ready;
	}

	override protected void OnUpdate(ESystemPoint point)
	{
		if (!m_MenuHandler)
			return;

		float dt = GetWorld().GetTimeSlice();

		if (!m_MenuHandler.UpdateLoadingPlaceholder(dt))
			return;

		if (!m_bReady)
			return;

		m_MenuHandler.Update(dt);
	}
}
