class SCR_CampaignPackMobileAssemblyUserAction : SCR_CampaignDeployMobileAssemblyUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_AssemblyComponent)
			return false;
		
		return m_AssemblyComponent.IsDeployed();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{	
		PlayerController playerController = GetGame().GetPlayerController();
		
		if (!playerController)
			return;
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		
		if (!campaignNetworkComponent)
			return;
		
		campaignNetworkComponent.DeployMobileAsembly(m_AssemblyComponent, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_AssemblyComponent)
			return false;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return false;

		if (SCR_FactionManager.SGetLocalPlayerFaction() != m_AssemblyComponent.GetParentFaction())
			return false;
		
		int basesCovered;
		SCR_CampaignMobileAssemblyStandaloneComponent standaloneComponent = m_AssemblyComponent.GetStandaloneComponent();
		
		if (standaloneComponent)
			basesCovered = standaloneComponent.GetCountOfExclusivelyLinkedBases();

		if (basesCovered == 0)
			return false;
		
		ActionNameParams[0] = basesCovered.ToString();
		outName = "#AR-Campaign_Action_Dismantle_BasesInfo-UC";
		return true;
	}
};
