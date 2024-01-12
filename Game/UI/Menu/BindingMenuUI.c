class RowIndexComponent: ScriptedWidgetComponent
{
	int m_Index;
};

// Script File
class BindingMenuUI: ChimeraMenuBase
{
	[MenuBindAttribute()]
	Widget Actions;
	
	[MenuBindAttribute()]
	XComboBoxWidget Contexts;
	
	[MenuBindAttribute()]
	void Back()
	{
		Close();
	}
	
	[MenuBindAttribute()]
	void Apply()
	{
		m_Binding.Save();
		Close();
	}
	
	protected ref array<string> m_ContextNames = new array<string>();
	protected ref array<string> m_ActionNames = new array<string>();
	protected const string ROW_LAYOUT_PATH = "{C76F51C554AF8F01}UI/layouts/Menus/Binding/actionRow.layout";
	protected ref InputBinding m_Binding;
	protected SCR_ConfigurableDialogUi m_BindingDialog;
	
	override void OnMenuInit() 
	{
		m_Binding = GetGame().GetInputManager().CreateUserBinding();
		Contexts.ClearAll();
		
		m_Binding.GetContexts(m_ContextNames);
		
		foreach (string ctx: m_ContextNames)
		{
			Contexts.AddItem(ctx);
		}		

		SetContext(0);		
	}
	
	void SetContext(int index)
	{
		while (Actions.GetChildren())
		{
			delete Actions.GetChildren();
		}
		m_ActionNames.Clear();
		
		if (index >= m_ContextNames.Count()) 
			return;
		
		BaseContainer ctxDef = m_Binding.FindContext(m_ContextNames[index]);
		
		if (ctxDef.Get("ActionRefs", m_ActionNames))
		{
			int cnt = m_ActionNames.Count();
			for (int i = 0; i < cnt; i++)
			{
				string actionName = m_ActionNames[i];
				Widget row = GetGame().GetWorkspace().CreateWidgets(ROW_LAYOUT_PATH, Actions);
				TextWidget nameLabel = TextWidget.Cast(row.FindWidget("rowLayout.nameLabel"));
				nameLabel.SetText(actionName);			
				
				TextWidget bindingLabel = TextWidget.Cast(row.FindWidget("rowLayout.bindingLabel"));
				array<string> binding = {};
				
				if (m_Binding.GetBindings(actionName, binding, EInputDeviceType.KEYBOARD))
				{
					bindingLabel.SetText(binding[0]);	
					if (!m_Binding.IsDefault(actionName))
					{
						bindingLabel.SetColorInt(ARGB(255,206, 245, 66));
						bindingLabel.SetBold(true);
					}
				}
				else
				{
					bindingLabel.SetText("");	
				}
								
				RowIndexComponent rowComponent = RowIndexComponent.Cast(row.FindHandler(RowIndexComponent));
				if (rowComponent)
					rowComponent.m_Index = i;
			
				if (i % 2)
					row.SetColorInt(ARGB(100,150,150,150));		
				else	
					row.SetColorInt(ARGB(100,100,100,100));		
			}
		}
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (w == Contexts)
		{
			SetContext(Contexts.GetCurrentItem());
			return true;
		}
		
		return false;
	}
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w.GetName() == "bindButton")
		{
			RowIndexComponent rowComponent = RowIndexComponent.Cast(w.GetParent().GetParent().FindHandler(RowIndexComponent));
			if (rowComponent.m_Index < m_ActionNames.Count())
			{
				m_Binding.StartCapture(m_ActionNames[rowComponent.m_Index]);
				m_BindingDialog = SCR_CommonDialogs.CreateDialog("bind_key");
				m_BindingDialog.SetTitle("Binding '" + m_ActionNames[rowComponent.m_Index] + "'");
				m_BindingDialog.SetMessage("Press key");
				m_BindingDialog.FindButton("confirm").SetVisible(false);
				GetGame().GetCallqueue().CallLater(CheckBindingStatus, 100, true);
			}
			return true;
		}
		else if (w.GetName() == "defaultButton")
		{
			RowIndexComponent rowComponent = RowIndexComponent.Cast(w.GetParent().GetParent().FindHandler(RowIndexComponent));
			if (rowComponent.m_Index < m_ActionNames.Count())
			{
				m_Binding.ResetDefault(m_ActionNames[rowComponent.m_Index]);
				SetContext(Contexts.GetCurrentItem());
			}
			return true;
		}
		
		return false;
	}
	
	void CheckBindingStatus()
	{
		if (m_Binding.GetCaptureState() == 2)
		{
			GetGame().GetCallqueue().Remove(CheckBindingStatus);
			m_Binding.SaveCapture();
			//m_BindingDialog.Close();
			SetContext(Contexts.GetCurrentItem());
		}
	}
};
