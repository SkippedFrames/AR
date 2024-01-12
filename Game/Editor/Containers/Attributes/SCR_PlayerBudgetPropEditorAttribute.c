/**
Modes and Budget attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_PlayerBudgetPropEditorAttribute: SCR_BasePlayerBudgetEditorAttribute
{	
	override EEditableEntityBudget GetBudgetType()
	{
		return EEditableEntityBudget.PROPS;
	}	
};
