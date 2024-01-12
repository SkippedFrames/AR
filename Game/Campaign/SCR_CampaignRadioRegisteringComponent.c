//------------------------------------------------------------------------------------------------
class SCR_CampaignRadioRegisteringComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignRadioRegisteringComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	protected bool IsParentBase(IEntity ent)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(ent.FindComponent(SCR_CampaignMilitaryBaseComponent));

		// Base was found, stop query
		if (base)
		{
			// Delay so user actions can properly initialize first
			GetGame().GetCallqueue().CallLater(base.RegisterHQRadio, 1000, false, GetOwner());
			return false;
		}
		
		// Keep looking
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		BaseWorld world = GetGame().GetWorld();
		
		if (!world)
			return;
		
		world.QueryEntitiesBySphere(owner.GetOrigin(), SCR_CampaignReconfigureHQRadioUserAction.MAX_BASE_DISTANCE, IsParentBase, null, EQueryEntitiesFlags.ALL);
	}
};
