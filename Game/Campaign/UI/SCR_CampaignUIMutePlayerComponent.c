//------------------------------------------------------------------------------------------------
class SCR_CampaignMutePlayerComponent : ScriptedWidgetComponent
{
	protected int m_iPlayerID = -1;
	protected bool m_bMuted = false;

	//------------------------------------------------------------------------------------------------
	//! Returns player ID
	int GetPlayerID()
	{
		return m_iPlayerID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets player ID
	void SetPlayerID(int playerID)
	{
		m_iPlayerID = playerID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when the button, this component is attached to, is clicked
	override bool OnClick(Widget w, int x, int y, int button)
	{
		PlayerController pController = GetGame().GetPlayerController();
		
		if (!pController)
			return false;
		
		if (pController.GetPlayerBlockedState(m_iPlayerID) != PermissionState.ALLOWED)
		{
			pController.SetPlayerBlockedState(m_iPlayerID, false);
			TextWidget.Cast(w.GetParent().FindAnyWidget("Text")).SetTextFormat("#AR-ButtonMutePlayer");
		}
		else
		{
			pController.SetPlayerBlockedState(m_iPlayerID, true);
			TextWidget.Cast(w.GetParent().FindAnyWidget("Text")).SetTextFormat("#AR-ButtonUnmutePlayer");
		}
		
		return false;
	}
};
