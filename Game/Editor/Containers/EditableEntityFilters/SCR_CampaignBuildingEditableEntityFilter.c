//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
class SCR_CampaignBuildingEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	//------------------------------------------------------------------------------------------------
	protected bool GetComposition(IEntity owner, out SCR_EditableEntityComponent entity, out SCR_CampaignBuildingCompositionComponent composition)
	{
		composition = SCR_CampaignBuildingCompositionComponent.Cast(owner.FindComponent(SCR_CampaignBuildingCompositionComponent));
		entity = SCR_EditableEntityComponent.Cast(owner.FindComponent(SCR_EditableEntityComponent));
		if (!composition || !entity)
			return false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsCompositionOwned(IEntity owner)
	{
		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(owner.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return false;

		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(owner.FindComponent(SCR_EditableEntityComponent));
		if (!editableComponent)
			return false;

		if (editableComponent.GetParentEntity())
			return false;

		SCR_EditorModeEntity mode = SCR_EditorModeEntity.Cast(GetManager().GetOwner());
		if (!mode)
			return false;

		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(mode.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return false;

		IEntity compositionOwner = compositionComponent.GetProviderEntity();
		if (!compositionOwner)
		{
			SCR_FreeRoamBuildingClientTriggerEntity trigger = buildingComponent.GetTrigger();
			if (!trigger)
				return false;

			if (vector.DistanceXZ(trigger.GetOrigin(), owner.GetOrigin()) <= trigger.GetSphereRadius())
				return true;
		}

		if (compositionOwner != buildingComponent.GetProviderEntity())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		IEntity entityOwner = entity.GetOwnerScripted();
		if (!entityOwner)
			return false;

		return (IsCompositionOwned(entityOwner));
	}
}
