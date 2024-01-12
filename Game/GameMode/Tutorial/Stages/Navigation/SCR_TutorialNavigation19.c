[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation19Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation19 : SCR_BaseCampaignTutorialArlandStage
{	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 10;
		
		GetGame().GetCallqueue().CallLater(TeleportFromVehicle, m_fDuration * 1000, false);
		
		IEntity waypoint = GetGame().GetWorld().FindEntityByName("WP_GREENHOUSE");
		if (!waypoint)
			return;
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TeleportFromVehicle()
	{
		IEntity vehicle = GetGame().GetWorld().FindEntityByName("Navigation_car");
		if (!vehicle)
			return;
		
		CompartmentAccessComponent compartmentAccessComp = CompartmentAccessComponent.Cast(vehicle.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccessComp)
			return;
		
		IEntity tpPos = GetGame().GetWorld().FindEntityByName("PP_START");
		if (!tpPos)		
			return;
		
		vector mat[4];
		tpPos.GetTransform(mat);
		compartmentAccessComp.MoveOutVehicle(-1, mat);
	}
};
