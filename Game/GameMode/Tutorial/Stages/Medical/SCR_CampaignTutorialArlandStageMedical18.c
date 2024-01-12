[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical18Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical18: SCR_BaseCampaignTutorialArlandStage
{
	protected IEntity m_Figurant;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		m_fWaypointHeightOffset = 0.5;
		RegisterWaypoint("Victim");
		
		m_Figurant = GetGame().GetWorld().FindEntityByName("Victim");
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		GetGame().GetCallqueue().Remove(m_TutorialComponent.RefreshVictimResilience);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		SCR_CharacterDamageManagerComponent damManager = m_TutorialComponent.GetVictimDamageManager();
		if (!damManager)
			return false;
		
		int reason;
		SCR_ConsumableMorphine consumableEffect = new SCR_ConsumableMorphine;
		consumableEffect.CanApplyEffect(m_Figurant, m_Player, reason); 
		
		return reason == SCR_EConsumableFailReason.ALREADY_APPLIED;
	}
};
