[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_FactionControlTriggerEntityClass: SCR_BaseFactionTriggerEntityClass
{
};
class SCR_FactionControlTriggerEntity: SCR_BaseFactionTriggerEntity
{
	[Attribute("1", UIWidgets.ComboBox, "How should friendly ratio limit be evaluated.", category: "Faction Control Trigger", enums: { ParamEnum("More than", "0"), ParamEnum("Equals to", "1"), ParamEnum("Less than", "2") })]
	protected int m_iRatioMethod;
	
	[Attribute("0.5", UIWidgets.Slider, "Limit for how large portion of trigger entities are friendly.\n\nExamples:\n1 = Only friendlies are present\n0 = Only enemies are present\n0.5 = Equal number of friendlies and enemies\n\nEvaluated only when at least some friendlies or enemies are inside.\nWhen the trigger is empty, condition for ratio = 0 won't activate the trigger.", category: "Faction Control Trigger", params: "0 1 0.1")]
	protected float m_fFriendlyRatioLimit;
	
	protected int m_iFriendlyCount, m_iEnemyCount;
	protected bool m_bResult;
	protected bool m_bEvaluateResult;
	
	/*!
	Get number of entities inside trigger for each side
	\param[out] outFriendlyCount Number of friendlies
	\param[out] outEnemyCount Number of enemies
	*/
	void GetSideCounts(out int outFriendlyCount, out int outEnemyCount)
	{
		outFriendlyCount = m_iFriendlyCount;
		outEnemyCount = m_iEnemyCount;
	}
	/*
	Single entity evaluation in ScriptedEntityFilterForQuery cannot produce a result yet.
	All entities have to be scanned first and counted.
	At the end of each cycle, evaluation begins.
	However, because trigger outcome is detemined by returned value of ScriptedEntityFilterForQuery, another cycle the simply returnes the result.
	
	Here's how it goes step by step:
	1) ScriptedEntityFilterForQuery is called on each entity inside and counts them per faction.
	2) OnQueryFinished is called at the end of the cycle and evaluates the result.
	3) ScriptedEntityFilterForQuery is called on each entity inside, but instantly returns the result.
	4) OnQueryFinished is called at the end of the cycle and resets the variables for new calculation.
	Repeat from step 1)
	*/
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		//--- Evaluation round, return the result instantly
		if (m_bEvaluateResult)
		{
			return m_bResult;
		}
		
		//--- No faction defined
		if (!m_OwnerFaction)
		{
			m_iFriendlyCount = 0;
			m_iEnemyCount = 0;
			return false;
		}
		
		//--- Evaluate engine-driven conditions, e.g., entity class
		if (!IsAlive(ent))
			return false;
		
		//--- Increase faction counters
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
		{
			Faction entFaction = factionAffiliation.GetAffiliatedFaction();
			if (entFaction)
			{
				if (m_OwnerFaction.IsFactionEnemy(entFaction))
					m_iEnemyCount++;
				else
					m_iFriendlyCount++;
			}
		}
		
		return false;
	}
	override protected void OnQueryFinished(bool bIsEmpty)
	{
		//--- Finished evaluation round. Reset variables and start again.
		if (m_bEvaluateResult)
		{
			m_iFriendlyCount = 0;
			m_iEnemyCount = 0;
			m_bEvaluateResult = false;
			return;
		}
		
		m_bResult = false;
		m_bEvaluateResult = true;
		
		//--- Nobody is in the trigger, skip evaluation
		if (m_iFriendlyCount == 0 && m_iEnemyCount == 0)
			return;
		
		float friendlyRatio = m_iFriendlyCount / Math.Max(m_iFriendlyCount + m_iEnemyCount, 1);
		switch (m_iRatioMethod)
		{
			case 0:
			{
				m_bResult = friendlyRatio > m_fFriendlyRatioLimit;
				break;
			}
			case 1:
			{
				m_bResult = float.AlmostEqual(friendlyRatio, m_fFriendlyRatioLimit);
				break;
			}
			case 2:
			{
				m_bResult = friendlyRatio < m_fFriendlyRatioLimit;
				break;
			}
		}
		
		super.OnQueryFinished(bIsEmpty);
	}
};
