class MotorExhaustSystem: GameSystem
{
	protected ref array<SCR_MotorExhaustEffectGeneralComponent> m_MotorsComponents = {};
	
	protected override ESystemPoint GetSystemPoint()
	{
		return ESystemPoint.Frame;
	}
	
	override protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();
		
		foreach (SCR_MotorExhaustEffectGeneralComponent comp: m_MotorsComponents)
		{
			comp.Update(timeSlice);
		}
	}
	
	override void OnDiag(float timeSlice)
	{
		DbgUI.Begin("MotorExhaustSystem");
		
		DbgUI.Text("Motors: " + m_MotorsComponents.Count());
		
		if (DbgUI.Button("Dump active motors components"))
		{
			foreach (SCR_MotorExhaustEffectGeneralComponent comp: m_MotorsComponents)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	void Register(SCR_MotorExhaustEffectGeneralComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_MotorsComponents.Find(component) != -1)
			return;
		
		m_MotorsComponents.Insert(component);
	}
	
	void Unregister(SCR_MotorExhaustEffectGeneralComponent component)
	{
		int idx = m_MotorsComponents.Find(component);
		if (idx == -1)
			return;
		
		m_MotorsComponents.Remove(idx);
	}
}
