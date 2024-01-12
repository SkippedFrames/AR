// Script File//------------------------------------------------------------------------------------------------
class SCR_AIIsTargetVisible : DecoratorScripted
{
	static const string BASE_TARGET_PORT = "BaseTargetIn";
	static const string LAST_POSITION_PORT = "LastPositionOut";
	
	[Attribute("0.5", UIWidgets.EditBox, "How long in past target becomes invisibile", "")]
	float m_lastSeenMax;
	
	PerceptionComponent m_PerceptionComp;
	
	//-----------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_PerceptionComp)
			m_PerceptionComp = PerceptionComponent.Cast(owner.GetControlledEntity().FindComponent(PerceptionComponent));
		
		BaseTarget target;
		GetVariableIn(BASE_TARGET_PORT,target);
		
		if (!target || !m_PerceptionComp)
		{
			ClearVariable(LAST_POSITION_PORT);
			return false;
		}
			
		SetVariableOut(LAST_POSITION_PORT,target.GetLastSeenPosition());
		
		// When m_lastSeenMax value is lower than perception component update interval,
		// which might be large at higher LOD levels, the decorator might periodically return false,
		// even when target is still visible.
		// Therefore we must prevent the threshold from being smaller than update interval of perception component.
		float lastSeenMax = Math.Max(m_lastSeenMax, m_PerceptionComp.GetUpdateInterval()) + 0.02;
		
		return target.GetTimeSinceSeen() < lastSeenMax;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//-----------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "IsTargetVisible: checks timeout of visibility of provided base target. Returns true if it is below lastSeenMax parameter.";
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		BASE_TARGET_PORT
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		LAST_POSITION_PORT
	};
	protected override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};
