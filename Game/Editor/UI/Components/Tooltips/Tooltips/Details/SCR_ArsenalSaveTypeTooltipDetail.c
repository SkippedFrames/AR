[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_ArsenalSaveTypeTooltipDetail : SCR_EntityTooltipDetail
{	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		RichTextWidget text = RichTextWidget.Cast(widget);
		if (!text)
			return false;
		
		SCR_ArsenalManagerComponent arsenalManager;
		
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return false;
		
		SCR_ArsenalSaveTypeInfoHolder arsenalSaveTypeInfoHolder = arsenalManager.GetArsenalSaveTypeInfoHolder();
		if (!arsenalSaveTypeInfoHolder)
			return false;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(entity.GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent && entity.GetEntityType() == EEditableEntityType.VEHICLE)
		{
			//~ If vehicle check if arsenal is on children
			arsenalComponent = SCR_ArsenalComponent.GetArsenalComponentFromChildren(entity.GetOwner());
		}
		
		if (!arsenalComponent || !arsenalComponent.HasSaveArsenalAction())
			return false;
		
		SCR_ArsenalSaveTypeUIInfo saveTypeUIInfo = arsenalSaveTypeInfoHolder.GetUIInfoOfType(arsenalComponent.GetArsenalSaveType());
		if (!saveTypeUIInfo)
			return false;
		
		text.SetText(saveTypeUIInfo.GetName());
		
		return true;
	}
};
