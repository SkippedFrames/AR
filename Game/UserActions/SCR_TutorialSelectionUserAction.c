class SCR_TutorialSelectionUserAction: ScriptedUserAction
{
	[Attribute("hello", UIWidgets.EditBox)]
	protected string m_sStageName;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETutorialArlandStageMasters))]
	protected SCR_ETutorialArlandStageMasters m_sStageConfig;
	
	protected SCR_GameModeCampaign m_CampaignGamemode;
	protected SCR_CampaignTutorialArlandComponent m_tutorialComponent;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{

	}
	
	//---------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		
		m_CampaignGamemode = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if(!m_CampaignGamemode)
			return;
		
		m_tutorialComponent = SCR_CampaignTutorialArlandComponent.Cast(m_CampaignGamemode.FindComponent(SCR_CampaignTutorialArlandComponent));
		
		if(!m_tutorialComponent)
			return;
		
		m_tutorialComponent.SetActiveConfig(m_sStageConfig);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = m_sStageName;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user) 
	{ 
		return true; 
	};
}
