[BaseContainerProps(configRoot: true)]
class SCR_HintConditionList
{
	[Attribute()]
	protected ref array<ref SCR_BaseHintCondition> m_aConditions;

	void Init(IEntity owner)
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		for (int i, count = m_aConditions.Count(); i < count; i++)
		{
			m_aConditions[i].InitCondition(owner, hintManager);
		}
	}
	void Exit(IEntity owner)
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		for (int i, count = m_aConditions.Count(); i < count; i++)
		{
			m_aConditions[i].ExitCondition(owner, hintManager);
		}
	}
};
