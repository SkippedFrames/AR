/*
Componenet for displaying progress of budget
*/

class SCR_BudgetUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected EEditableEntityBudget m_BudgetType;
	
	[Attribute("")]
	protected string m_sBudgetProgressRoot;
	
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected SCR_BudgetEditorComponent m_BudgetManager;
	protected SCR_EditableEntityCoreBudgetSetting m_Budget;
	
	protected SCR_BudgetProgressEditorUIComponent m_BudgetProgress;
	
	protected int m_iCurrentBudget;
	protected int m_iMaxBudget;
	
	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{	
		super.HandlerAttachedScripted(w);
		
		// Widget
		Widget budgetRoot = w.FindAnyWidget(m_sBudgetProgressRoot);
		if (!budgetRoot)
			budgetRoot = w;
		
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		m_BudgetProgress = SCR_BudgetProgressEditorUIComponent.Cast(budgetRoot.FindHandler(SCR_BudgetProgressEditorUIComponent));
		
		// Setup budget 
		m_BudgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent, false, true));
		if (m_BudgetManager)
		{
			InitializeBudgets();
			m_BudgetManager.Event_OnBudgetUpdated.Insert(OnBudgetUpdate);
			m_BudgetManager.Event_OnBudgetMaxUpdated.Insert(OnBudgetMaxUpdate);
			m_BudgetManager.Event_OnBudgetPreviewUpdated.Insert(OnBudgetPreviewUpdate);
			m_BudgetManager.Event_OnBudgetPreviewReset.Insert(ResetWidgetPreviewData);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void InitializeBudgets()
	{
		// Setup 
		array<ref SCR_EditableEntityCoreBudgetSetting> budgets = {};
		m_BudgetManager.GetBudgets(budgets);
		
		foreach (SCR_EditableEntityCoreBudgetSetting budget : budgets)
		{
			if (budget.GetBudgetType() == m_BudgetType)
				m_Budget = budget;
		} 
		
		// Setup values 
		m_iCurrentBudget = m_BudgetManager.GetCurrentBudgetValue(m_BudgetType);
		m_BudgetManager.GetMaxBudgetValue(m_BudgetType, m_iMaxBudget);
		
		// Update UI
		bool base = m_CampaignBuildingComponent.IsProviderBase(); 
		m_BudgetProgress.GetRootWidget().SetVisible(base);
		
		m_BudgetProgress.HideBudgetChange();
		m_BudgetProgress.ShowBudget(m_iCurrentBudget / m_iMaxBudget);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetUpdate(EEditableEntityBudget budgetType, int originalBudgetValue, int updatedBudgetValue, int maxBudgetValue)
	{
		m_iCurrentBudget = m_BudgetManager.GetCurrentBudgetValue(m_BudgetType);
		m_BudgetManager.GetMaxBudgetValue(m_BudgetType, m_iMaxBudget);
		
		m_BudgetProgress.HideBudgetChange();
		m_BudgetProgress.ShowBudget(originalBudgetValue / maxBudgetValue);
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetMaxUpdate(EEditableEntityBudget budgetType, int currentBudgetValue, int maxBudgetValue)
	{
		m_iMaxBudget = maxBudgetValue;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetPreviewUpdate(EEditableEntityBudget budgetType, float previewBudgetValue, float budgetChange)
	{
		m_BudgetProgress.ShowBudgetChange(budgetChange, previewBudgetValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetWidgetPreviewData()
	{
		m_BudgetProgress.HideBudgetChange();
	}
}
