class VehicleWaterPhysicsSystem: GameSystem
{
	protected bool m_bUpdating = false;
	protected ref array<SCR_VehicleWaterPhysicsComponent> m_Components = {};
	protected ref array<SCR_VehicleWaterPhysicsComponent> m_DeletedComponents = {};
	
	override protected ESystemPoint GetSystemPoint()
	{
		return ESystemPoint.FixedFrame;
	}
	
	override protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();
		
		m_bUpdating = true;
		foreach (SCR_VehicleWaterPhysicsComponent comp: m_Components)
		{
			comp.UpdateFixed(timeSlice);
		}
		m_bUpdating = false;
		
		foreach (SCR_VehicleWaterPhysicsComponent comp: m_DeletedComponents)
		{
			m_Components.RemoveItem(comp);
		}
		m_DeletedComponents.Clear();
	}
	
	override protected void OnDiag(float timeSlice)
	{
		DbgUI.Begin("VehicleWaterPhysicsSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_VehicleWaterPhysicsComponent comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	void Register(SCR_VehicleWaterPhysicsComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	void Unregister(SCR_VehicleWaterPhysicsComponent component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		if (m_bUpdating)
			m_DeletedComponents.Insert(component);
		else
			m_Components.Remove(idx);
	}
}
