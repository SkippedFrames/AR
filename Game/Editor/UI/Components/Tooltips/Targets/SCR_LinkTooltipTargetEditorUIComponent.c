/** @ingroup Editor_UI Editor_UI_Components
*/
class SCR_LinkTooltipTargetEditorUIComponent: SCR_BaseTooltipTargetEditorUIComponent
{
	protected SCR_UIInfo m_Info;
	protected Managed m_Target;
	
	void SetInfo(SCR_UIInfo info, Managed target = null)
	{
		m_Info = info;
		m_Target = target;
		RefreshTooltip();
	}
	override SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
	override Managed GetTarget()
	{
		return m_Target;
	}
	
	static void SetInfo(Widget widget, SCR_UIInfo info, Managed target = null)
	{
		if (!widget || !info) return;
		
		SCR_LinkTooltipTargetEditorUIComponent tooltipTarget = SCR_LinkTooltipTargetEditorUIComponent.Cast(widget.FindHandler(SCR_LinkTooltipTargetEditorUIComponent));
		if (!tooltipTarget) return;

		tooltipTarget.SetInfo(info, target);
	}
};
