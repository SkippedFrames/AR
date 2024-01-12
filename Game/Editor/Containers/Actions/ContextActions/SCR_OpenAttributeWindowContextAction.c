[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_OpenAttributeWindowContextAction : SCR_DoubleClickAction
{
	override bool IsServer()
	{
		return false;
	}
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{	
		return hoveredEntity && !hoveredEntity.HasEntityFlag(EEditableEntityFlag.NON_INTERACTIVE) && !selectedEntities.IsEmpty();
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return hoveredEntity && !hoveredEntity.HasEntityFlag(EEditableEntityFlag.NON_INTERACTIVE) && !selectedEntities.IsEmpty();
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{	
		if (DidCursorMoveDuringDoubleClick())
			return;
		
		SCR_AttributesManagerEditorComponent attributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (attributesManager)
		{
			array<Managed> editedEntities = new array<Managed>; //--- Must be 'array', not 'set', otherwise it will confuse the attribute system
			foreach (SCR_EditableEntityComponent entity: selectedEntities) editedEntities.Insert(entity);
			attributesManager.StartEditing(editedEntities);
		}
	}
};
