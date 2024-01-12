class SCR_AIGetMyGroup: AITaskScripted
{
	static const string PORT_GROUP_OUT	= "GroupOut";
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette()
    {
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_GROUP_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (GetVariableType(false, PORT_GROUP_OUT) != AIGroup)
		{
			NodeError(this, owner, PORT_GROUP_OUT+" should be AIGroup");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIGroup group = AIGroup.Cast(owner.GetParentGroup());
		if ( group )
		{
			SetVariableOut(PORT_GROUP_OUT,group);
			return ENodeResult.SUCCESS;
		}
		else
			return ENodeResult.FAIL;

	}
};
