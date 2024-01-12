class SCR_DeployInventoryItemRestrictedAction : SCR_DeployInventoryItemBaseAction
{
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		if (!m_DeployableItemComp || !m_RplComp || m_RplComp.IsProxy())
			return;
		
		SCR_RestrictedDeployableSpawnPointComponent restrictedDeployableSpawnPointComp = SCR_RestrictedDeployableSpawnPointComponent.Cast(m_DeployableItemComp);
		if (!restrictedDeployableSpawnPointComp)
			return;
		
		restrictedDeployableSpawnPointComp.TryDeploy(pUserEntity);
	}
}
