[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_DeployableInventoryItemReplacementComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Holds Position of where the deployable item will be attached to.
class SCR_DeployableInventoryItemReplacementComponent : ScriptComponent
{
	[Attribute()]
	protected ref PointInfo m_vItemPosition;
	
	protected ref ScriptInvokerVoid m_OnCompositionDestroyed = new ScriptInvokerVoid();
	
	//------------------------------------------------------------------------------------------------
	void GetItemTransform(out vector mat[4])
	{
		if (m_vItemPosition)
			m_vItemPosition.GetTransform(mat);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnCompositionDestroyed()
	{
		return m_OnCompositionDestroyed;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DeployableInventoryItemReplacementComponent()
	{
		m_OnCompositionDestroyed.Invoke();
	}
}
