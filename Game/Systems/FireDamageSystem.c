class FireDamageSystem: GameSystem
{
	protected ref array<SCR_DamageManagerComponent> m_aDamageManagers = {};

	//------------------------------------------------------------------------------------------------
	protected override ESystemPoint GetSystemPoint()
	{
		return ESystemPoint.FixedFrame;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();

		foreach (SCR_DamageManagerComponent component : m_aDamageManagers)
		{
			if (component)
				component.UpdateFireDamage(timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDiag(float timeSlice)
	{
		DbgUI.Begin("FireDamageSystem");

		DbgUI.Text("Damage managers: " + m_aDamageManagers.Count());

		if (DbgUI.Button("Dump active fire damage manager components"))
		{
			foreach (SCR_DamageManagerComponent component : m_aDamageManagers)
			{
				Print(component.GetOwner(), LogLevel.ERROR);
			}
		}

		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	void Register(SCR_DamageManagerComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (!m_aDamageManagers.Contains(component))
			m_aDamageManagers.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_DamageManagerComponent component)
	{
		m_aDamageManagers.RemoveItem(component);

		// Final update
		if (component)
			component.UpdateFireDamage(GetWorld().GetFixedTimeSlice());
	}
}
