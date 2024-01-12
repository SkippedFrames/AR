[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStage104Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStage104 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_26");
		m_fWaypointCompletionRadius = 20;
		m_fWaypointHeightOffset = 0;
		GetGame().GetCallqueue().CallLater(DelayedPopup, 1000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_HintToggle", 12, "", "", "<color rgba='226,168,79,200'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'><action name = 'HintToggle'/></shadow></color>", "");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};
