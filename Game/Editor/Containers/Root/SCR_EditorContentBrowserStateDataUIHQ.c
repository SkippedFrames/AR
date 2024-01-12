/*!
Editor Content browser State data that includes UI data for tabs, with specific conditin for HQ
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_BrowserStateUIInfo")]
class SCR_EditorContentBrowserSaveStateDataUIHQ : SCR_EditorContentBrowserSaveStateDataUI
{
	//! Tab allowing a supply truck to build a new base is not available for Conflict mode.
	override bool CanBeShown()
	{
		return !SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
	}
}
