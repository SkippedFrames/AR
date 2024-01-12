class SCR_DeployInventoryItemBaseAction : ScriptedUserAction
{
	protected SCR_BaseDeployableInventoryItemComponent m_DeployableItemComp;
	
	protected RplComponent m_RplComp;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DeployableItemComp)
			return false;
		
		return m_DeployableItemComp.CanDeployBeShown(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		return true;
 	}
	
 	//------------------------------------------------------------------------------------------------
 	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		if (!m_DeployableItemComp || !m_RplComp || m_RplComp.IsProxy())
			return;
		
		m_DeployableItemComp.Deploy(pUserEntity);
 	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{	
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		m_DeployableItemComp = SCR_BaseDeployableInventoryItemComponent.Cast(owner.FindComponent(SCR_BaseDeployableInventoryItemComponent));
		m_RplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
}
