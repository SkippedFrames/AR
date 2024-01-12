[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced22Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced22 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		RegisterWaypoint("Hummer2");	
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		m_TutorialComponent.SetWaypointMiscImage("CUSTOM", true);
		
		array<IEntity> rootItems = {};
		
		IEntity entity = GetGame().GetPlayerManager().GetPlayerControlledEntity(GetGame().GetPlayerController().GetPlayerId());
		
		if (!entity)
			return;
				
		SCR_CharacterInventoryStorageComponent component = SCR_CharacterInventoryStorageComponent.Cast(entity.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		if (!component)
			return;
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!storageManComp)
			return;
		
		storageManComp.GetAllRootItems(rootItems);
		
		foreach (IEntity item : rootItems)
		{
			if(!item)
				continue;
			
			SCR_RepairSupportStationComponent repairComp = SCR_RepairSupportStationComponent.Cast(item.FindComponent(SCR_RepairSupportStationComponent));
		
			if (repairComp)
				component.StoreItemToQuickSlot(item, 4, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		Vehicle hmw = m_TutorialComponent.GetHummer();
		if (hmw)
		{
			SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(hmw.GetDamageManager());
			if (damageManager)
			{
				HitZone engineHitZone = damageManager.GetHitZoneByName("Engine_01");
				if (engineHitZone)
					return engineHitZone.GetHealth() > 598;
			}
			return false;
		}
		return false;
	}
};
