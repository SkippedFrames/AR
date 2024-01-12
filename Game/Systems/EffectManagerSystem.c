class EffectManagerSystem: GameSystem
{
	protected ref array<SCR_BaseEffectManagerComponent> m_Components = {};
	
	protected override ESystemPoint GetSystemPoint()
	{
		return ESystemPoint.Frame;
	}
	
	override protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();
		
		foreach (SCR_BaseEffectManagerComponent comp: m_Components)
		{
			comp.UpdateModules(timeSlice);
		}
	}
	
	override void OnDiag(float timeSlice)
	{
		DbgUI.Begin("EffectManagerSystem");
		
		DbgUI.Text("Components: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_BaseEffectManagerComponent comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	void Register(SCR_BaseEffectManagerComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	void Unregister(SCR_BaseEffectManagerComponent component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		m_Components.Remove(idx);
	}
}
