[BaseContainerProps()]
class SCR_EditableGroupUIInfo: SCR_EditableEntityUIInfo
{
	[Attribute()]
	protected ref SCR_MilitarySymbol m_MilitarySymbol;
	
	protected SCR_MilitarySymbol m_MilitarySymbolInstance; //--- No 'ref'
	
	/*!
	Get military symbol.
	\return Military symbol
	*/
	SCR_MilitarySymbol GetMilitarySymbol()
	{
		if (m_MilitarySymbolInstance)
			return m_MilitarySymbolInstance;
		else
			return m_MilitarySymbol;
	}
	/*!
	Set info instance.
	Use only when the UI info is on entity instance, not prefab!
	\param symbol Military symbol
	*/
	void SetInstance(SCR_MilitarySymbol symbol, LocalizedString name)
	{
		Name = name;
		m_MilitarySymbolInstance = symbol;
	}
	
	override string GetName()
	{
		if (!m_MilitarySymbolInstance)
		{
			SCR_GroupIdentityCore core = SCR_GroupIdentityCore.Cast(SCR_GroupIdentityCore.GetInstance(SCR_GroupIdentityCore));
			if (core)
			{
				SCR_GroupNameConfig nameManager = core.GetNames();
				if (nameManager)
				{
					return nameManager.GetGroupName(GetMilitarySymbol());
				}
			}
		}
		return super.GetName();
	}
			
	/*!
	Get Entity budget costs
	*/
	override bool GetEntityBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets)
	{
		return true;
	}
	
	//--- Override without 'protected' keyword
	override override void CopyFrom(SCR_UIName source)
	{
		SCR_EditableGroupUIInfo editableGroupSource = SCR_EditableGroupUIInfo.Cast(source);
		if (editableGroupSource)
		{
			m_MilitarySymbol = editableGroupSource.m_MilitarySymbol;
		}
		
		super.CopyFrom(source);
	}
};
