[EntityEditorProps(insertable: false)]
class SCR_HeliCourse_stage12Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HeliCourse_stage12 : SCR_BaseCampaignTutorialArlandStage
{
	protected bool m_bEngineOff;
	protected ScriptInvokerVoid m_OnEngineStartedInvoker;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		SCR_HelicopterControllerComponent comp = SCR_HelicopterControllerComponent.Cast(helicopter.FindComponent(SCR_HelicopterControllerComponent));
		if (!comp)
			return;
		
		m_OnEngineStartedInvoker = comp.GetOnEngineStop();
		if (m_OnEngineStartedInvoker)
			comp.GetOnEngineStop().Insert(GetOnEngineStop);
		
	}
	//------------------------------------------------------------------------------------------------
	protected void GetOnEngineStop()
	{
		m_bEngineOff = true;
		
		if (!m_OnEngineStartedInvoker)
			m_OnEngineStartedInvoker.Remove(GetOnEngineStop);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bEngineOff;
	}
};
