class VehiclesDustSystem: GameSystem
{
	protected ref array<SCR_VehicleDustPerWheel> m_Components = {};
	
	override protected ESystemPoint GetSystemPoint()
	{
		return ESystemPoint.Frame;
	}
	
	override protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetTimeSlice();
		
		foreach (SCR_VehicleDustPerWheel comp: m_Components)
		{
			comp.Update(timeSlice);
		}
	}
	
	override protected void OnDiag(float timeSlice)
	{
		DbgUI.Begin("VehiclesDustSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_VehicleDustPerWheel comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	void Register(SCR_VehicleDustPerWheel component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	void Unregister(SCR_VehicleDustPerWheel component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		m_Components.Remove(idx);
	}
}
