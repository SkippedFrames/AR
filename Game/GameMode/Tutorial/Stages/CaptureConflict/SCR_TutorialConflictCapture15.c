[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture15Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture15 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		GetGame().GetCallqueue().CallLater(DelayedPopup, 10000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Radios", 10, "", "", "", "");
		SCR_InventoryStorageManagerComponent comp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (comp)
		{
			comp.m_OnItemAddedInvoker.Remove(m_TutorialComponent.CheckRadioPickup);
			comp.m_OnItemAddedInvoker.Insert(m_TutorialComponent.CheckRadioPickup);
		}
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		while (campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetActiveRespawnRadios() > 0)
			campaign.RemoveActiveRespawnRadio(campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (m_TutorialComponent.GetPlayerRadio() != null);
	}
};
