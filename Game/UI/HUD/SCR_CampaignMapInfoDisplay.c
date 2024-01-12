//------------------------------------------------------------------------------------------------
class SCR_CampaignMapInfoDisplay : SCR_CampaignInfoDisplay
{
	protected bool m_bMapOpen;
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateHUD()
	{
		m_bPeriodicRefresh = false;
		
		if (!m_wRoot || !m_bInitDone)
			return;
		
		if (m_Campaign.IsTutorial() || m_Campaign.GetIsMatchOver())
		{
			Show(false);
			return;
		}
		
		if (m_bMapOpen)
			Show(true);
		
		UpdateHUDValues();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		if (m_Campaign.IsTutorial())
			return;
		
		m_bMapOpen = true;
		
		if (SCR_DeployMenuMain.GetDeployMenu() == null)
		{
			Show(true);
			UpdateHUD();
		}
		else
		{
			Show(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapClose(MapConfiguration config)
	{
		if (m_Campaign.IsTutorial())
			return;
		
		m_bMapOpen = false;
		Show(false);
	}
};
