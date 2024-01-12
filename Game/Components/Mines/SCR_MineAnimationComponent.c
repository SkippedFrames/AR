[EntityEditorProps(category: "GameScripted/Mines", description: "Base component for handling animation.")]
class SCR_MineAnimationComponentClass : WeaponAnimationComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MineAnimationComponent : WeaponAnimationComponent
{
	protected AnimationEventID m_iMineDown = -1;
	protected AnimationEventID m_iMineActivated = -1;
	protected IEntity m_Owner;
	
	//------------------------------------------------------------------------------------------------
	void SCR_MineAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Owner = ent;
		m_iMineDown = GameAnimationUtils.RegisterAnimationEvent("MineDown");
		m_iMineActivated = GameAnimationUtils.RegisterAnimationEvent("MineActivated");
	}
	
	//------------------------------------------------------------------------------------------------
	protected override event void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		super.OnAnimationEvent(animEventType, animUserString, intParam, timeFromStart, timeToEnd);
		if (!m_Owner)
			return;
		
		if (animEventType == m_iMineDown)
		{
			SoundComponent soundComponent = SoundComponent.Cast(m_Owner.FindComponent(SoundComponent));
			if (!soundComponent)
				return;
			
			TraceParam param = new TraceParam();
			param.Start = m_Owner.GetOrigin();
			param.End = param.Start - vector.Up;
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			param.Exclude = m_Owner;
			param.LayerMask = EPhysicsLayerPresets.Projectile;
			m_Owner.GetWorld().TraceMove(param, null);
			
			GameMaterial material = param.SurfaceProps;
			if (!material)
				return;
			
			soundComponent.SetSignalValueStr("Surface", material.GetSoundInfo().GetSignalValue());
		}
		
		// Master only
		RplComponent rplComponent = RplComponent.Cast(m_Owner.FindComponent(RplComponent));
		if (!rplComponent || rplComponent.IsProxy())
			return;
		
		if (animEventType == m_iMineActivated)
		{
			SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(m_Owner.FindComponent(SCR_PressureTriggerComponent));
			if (!pressureTriggerComponent)
				return;
			
			pressureTriggerComponent.ActivateTrigger();
		}
	}
}
