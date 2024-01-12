[EntityEditorProps(insertable: false)]
class SCR_HeliCourse_stage3Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_HeliCourse_stage3 : SCR_BaseCampaignTutorialArlandStage
{
	protected bool m_bEngineStarted;
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
		
		m_OnEngineStartedInvoker = comp.GetOnEngineStart();
		if (m_OnEngineStartedInvoker)
			comp.GetOnEngineStart().Insert(OnEngineStart);
		
	}
	//------------------------------------------------------------------------------------------------
	protected void OnEngineStart()
	{
		m_bEngineStarted = true;
		
		if (!m_OnEngineStartedInvoker)
			m_OnEngineStartedInvoker.Remove(OnEngineStart);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bEngineStarted;
	}
};
