[ComponentEditorProps(category: "GameScripted/Spectating", description: "Entity that this is attached to becomes selectable by the Spectator Camera.")]
class SCR_SpectateTargetComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_SpectateTargetComponent : ScriptComponent
{
	// List of all spectate targets
	private static ref array <SCR_SpectateTargetComponent> s_aSpectateTargets = null;
	
	private DamageManagerComponent m_DamageManagerComponent = null;
	private IEntity m_Entity = null;
	
	
	//------------------------------------------------------------------------------------------------
	//! Returns entity that this component is attached to.
	IEntity GetEntity()
	{
		return m_Entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if unit is alive or has no damage component attached. Returns false otherwise.
	bool IsAlive()
	{
		if (!m_DamageManagerComponent)
			return true;
		
		return m_DamageManagerComponent.GetState() != EDamageState.DESTROYED;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_SpectateTargetComponent GetSpectateTarget(int index)
	{
		if (!s_aSpectateTargets)
			return null;
		
		int count = s_aSpectateTargets.Count();
		if (index >= 0 && index < count)
		{
			return s_aSpectateTargets[index];
		}
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the amount of possible elements to spectate.
	static int GetSpectateTargetCount()
	{
		if (!s_aSpectateTargets)
			return 0;
		
		return s_aSpectateTargets.Count();
	}	
	
	//------------------------------------------------------------------------------------------------
	private static void RegisterTarget(SCR_SpectateTargetComponent spectateTarget)
	{
		if (!s_aSpectateTargets)
			s_aSpectateTargets = new ref array<SCR_SpectateTargetComponent>();
		
		if (s_aSpectateTargets)
		{
			s_aSpectateTargets.Insert(spectateTarget);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private static void UnregisterTarget(SCR_SpectateTargetComponent spectateTarget)
	{
		if (!s_aSpectateTargets)
			return;
		
		int index = s_aSpectateTargets.Find(spectateTarget);
		if (index != -1)
		{
			s_aSpectateTargets.RemoveItem(spectateTarget);
			
			int cnt = s_aSpectateTargets.Count();
			if (cnt - 1 >= 0)
				s_aSpectateTargets.Resize(cnt-1);
		}
		
		int count = s_aSpectateTargets.Count();
		if (count <= 0)
		{
			s_aSpectateTargets.Clear();
			s_aSpectateTargets = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_SpectateTargetComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!GetGame().GetWorldEntity())
			return;
		
		RegisterTarget(this);		
		m_Entity = ent;		
		GenericEntity genericEntity = GenericEntity.Cast(ent);
		m_DamageManagerComponent = DamageManagerComponent.Cast(genericEntity.FindComponent(DamageManagerComponent));
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SpectateTargetComponent()
	{		
		UnregisterTarget(this);
	}

};
