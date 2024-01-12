class SCR_AIGetSuppressionPosition: AITaskScripted
{
  	static const string PORT_DURATION = "DurationIn";
  	static const string PORT_SHOOT_START = "ShootStartIn";
	static const string PORT_SHOOT_END = "ShootEndIn";
	static const string PORT_AIM_POSITION = "AimPositionOut";
	static const string PORT_AIM_TIME = "AimTimeOut";
	static const string PORT_RESET_TIME = "ResetTime";
	static const string PORT_RESET_TIME_OUT = "ResetTimeOut";
	
	[Attribute("50", UIWidgets.EditBox, desc: "Number of LERP steps" )]
	private int m_fSteps;
	
	private float m_fIterativeStep,m_fCurrentTime,m_fDuration;
	
	private vector m_vStart,m_vEnd,m_vDirection;
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_DURATION,
		PORT_SHOOT_START,
		PORT_SHOOT_END,
		PORT_RESET_TIME
	};
	override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
    
	protected static ref TStringArray s_aVarsOut = {
            PORT_AIM_POSITION,
            PORT_AIM_TIME,
            PORT_RESET_TIME_OUT
	};
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		bool reset;
		GetVariableIn(PORT_DURATION, m_fDuration);
		GetVariableIn(PORT_RESET_TIME, reset);
		if (reset)
		{
			m_fCurrentTime = 0;
			SetVariableOut(PORT_RESET_TIME_OUT, false);	
		};		
		
		if (m_fDuration < 1e-10)
			return ENodeResult.FAIL;
		m_fIterativeStep = 1 / m_fSteps;	
		
		if (!GetVariableIn(PORT_SHOOT_START,m_vStart) || !GetVariableIn(PORT_SHOOT_END,m_vEnd))
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_AIM_TIME,m_fDuration/m_fSteps);
		m_vDirection = (m_vEnd - m_vStart);
		
		m_vDirection *= Math.Clamp(m_fCurrentTime,0,1);
		
		//PrintFormat("Covering current time step: %1", m_fCurrentTime);
		SetVariableOut(PORT_AIM_POSITION,m_vStart + m_vDirection);
		m_fCurrentTime += m_fIterativeStep;
		
		return ENodeResult.SUCCESS;
    }
	
	protected override string GetOnHoverDescription()
	{
		return "Calculate position to shoot at during suppressive fire";
	}
	
	override bool VisibleInPalette() 
	{
		return true;
	}
};

