[EntityEditorProps(category: "GameScripted/UI", description: "Helper entity handling processing widget animations. Do not insert into world.", color: "0 0 255 255")]
class AnimateWidgetEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class AnimateWidgetEntity : GenericEntity
{
	protected static AnimateWidgetEntity s_Instance;
	protected ref AnimateWidget m_Animatator;
	
	//------------------------------------------------------------------------------------------------
	static AnimateWidgetEntity GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAnimatingCompleted()
	{
		ClearEventMask(EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnimatingStarted()
	{
		SetEventMask(EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override private void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_Animatator)
			m_Animatator.UpdateAnimations(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	void AnimateWidgetEntity(IEntitySource src, IEntity parent)
	{
		if (s_Instance != null)
			return;

		SetEventMask(EntityEvent.FRAME);

		m_Animatator = AnimateWidget.GetInstance();
		if (!m_Animatator)
			m_Animatator = new AnimateWidget();

		if (m_Animatator.IsActive())
			OnAnimatingStarted();

		m_Animatator.m_OnAnimatingStarted.Insert(OnAnimatingStarted);
		m_Animatator.m_OnAnimatingCompleted.Insert(OnAnimatingCompleted);
	}
};
