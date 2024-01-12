//------------------------------------------------------------------------------------------------
//! Error call to be used in scripted BT nodes
ENodeResult NodeError(Node node, AIAgent owner, string msg)
{
	string sOwner = owner.ToString();
	string nodeStack;
	node.GetCallstackStr(nodeStack);
	Debug.Error(sOwner + " : " + nodeStack + "\n" + msg);
	return ENodeResult.FAIL;
};

// used for BT nodes to ensure they runs on AIGroup
//------------------------------------------------------------------------------------------------
void SCR_AgentMustBeAIGroup(Node node, AIAgent owner)
{
	NodeError(node,owner, node.Type().ToString() + " must be run on group AIAgent!");
};

// used for BT nodes to ensure they runs on ChimeraAIAgent
//------------------------------------------------------------------------------------------------
ENodeResult SCR_AgentMustChimera(Node node, AIAgent owner)
{
	string sOwner = owner.ToString();
	string nodeStack;
	node.GetCallstackStr(nodeStack);
	Debug.Error(sOwner + " : " + nodeStack + "\n" + "must be run on ChimeraAIAgent!");
	return ENodeResult.FAIL;
};
