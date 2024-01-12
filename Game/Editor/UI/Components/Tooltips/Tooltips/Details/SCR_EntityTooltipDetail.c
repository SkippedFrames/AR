[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_EntityTooltipDetail
{
	[Attribute()]
	protected LocalizedString m_sDisplayName;
	
	[Attribute(params: "layout")]
	protected ResourceName m_Layout;
	
	protected Widget m_Widget;
	
	[Attribute("1", "If false hides label")]
	protected bool m_bShowLabel;
	
	bool NeedUpdate()
	{
		return false;
	}
	bool CreateDetail(SCR_EditableEntityComponent entity, Widget parent, TextWidget label, bool setFrameslot = true)
	{
		if (label)
			label.SetText(m_sDisplayName);
		
		WorkspaceWidget workspace = parent.GetWorkspace();
		m_Widget = workspace.CreateWidgets(m_Layout, parent);
		
		if (InitDetail(entity, m_Widget))
		{
			return true;
		}
		else
		{
			m_Widget.RemoveFromHierarchy();
			m_Widget = null;
			return false;
		}
	}
	void UpdateDetail(SCR_EditableEntityComponent entity)
	{
	}
	bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		//--- Return true to show the detail
		return false;
	}
	
	//Returns if label should be shown
	bool GetShowLabel()
	{
		return m_bShowLabel;
	}
};
