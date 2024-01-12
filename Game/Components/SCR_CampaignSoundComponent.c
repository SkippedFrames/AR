[ComponentEditorProps(category: "GameScripted/Sound", description: "Component is active only in conflict mode")]
class SCR_CampaignSoundComponentClass: SoundComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignSoundComponent : SoundComponent
{	
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		// Disable dynamic simulation if not conflict game mode
		if (!SCR_GameModeCampaign.GetInstance())
			EnableDynamicSimulation(false);		
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CampaignSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignSoundComponent()
	{
	}
};
