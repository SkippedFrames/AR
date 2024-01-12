class SCR_ResourceSystem : GameSystem
{
	static const int DYNAMIC_COMPONENTS_MAX_FRAME_BUDGET = 10;
    protected ref array<SCR_ResourceInteractor> m_aSubscribedInteractors	= {};
    protected ref array<SCR_ResourceComponent> m_aDynamicComponents			= {};
    protected ref array<SCR_ResourceContainer> m_aContainers				= {};
	protected SCR_ResourceSystemSubscriptionManager m_ResourceSystemSubscriptionManager;
 	protected ref SCR_ResourceGrid m_ResourceGrid;
	protected int m_iSubscribedInteractorPivot;
	protected int m_iDynamicComponentsPivot;
	protected int m_iDynamicComponentsFrameBudget = SCR_ResourceSystem.DYNAMIC_COMPONENTS_MAX_FRAME_BUDGET;
	//------------------------------------------------------------------------------------------------
    override protected ESystemPoint GetSystemPoint()
    {
        return ESystemPoint.FixedFrame;
    }
 	
	//------------------------------------------------------------------------------------------------
	bool IsRegistered(notnull SCR_ResourceComponent component)
	{
		return m_aDynamicComponents.Contains(component);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsRegistered(notnull SCR_ResourceContainer container)
	{
		return m_aContainers.Contains(container);
	}
	
	//------------------------------------------------------------------------------------------------
    override protected void OnUpdate(ESystemPoint point)
    {
 		bool wasGridUpdateIdIncreased;
		float timeSlice = GetWorld().GetFixedTimeSlice();
		
		//!------------------------------------------------------------ Process new items in the grid.
		m_ResourceGrid.ProcessFlaggedItems();
		
		//!----------------------------------------------------------------- Process graceful handles.
		m_ResourceSystemSubscriptionManager.ProcessGracefulHandles();
		
		//!------------------------------------------------------------------- Container self updates.
		int containersCount = m_aContainers.Count();
		SCR_ResourceContainer container;
		
		for (int i = containersCount - 1; i >= 0; --i)
		{
			container = m_aContainers[i];
			
			if (container)
				container.Update(timeSlice);
			else
				m_aContainers.Remove(i);
		}
		
		//!-------------------------------------------------------- Process dynamic items in the grid.
		int dynamicComponentsCount = m_aDynamicComponents.Count();
		m_iDynamicComponentsFrameBudget = SCR_ResourceSystem.DYNAMIC_COMPONENTS_MAX_FRAME_BUDGET;
		SCR_ResourceComponent dynamicComponent;
		
		for (int i = dynamicComponentsCount - 1; i >= 0; --i)
		{
			if(m_iDynamicComponentsPivot >= dynamicComponentsCount)
				m_iDynamicComponentsPivot = 0;
			
			dynamicComponent = m_aDynamicComponents[m_iDynamicComponentsPivot];
			
			if (!dynamicComponent)
			{
				m_aDynamicComponents.Remove(m_iDynamicComponentsPivot);
				
				continue;
			}
			
			if (vector.DistanceSq(dynamicComponent.GetOwner().GetOrigin(), dynamicComponent.GetLastPosition()) <= SCR_ResourceComponent.UPDATE_DISTANCE_TRESHOLD_SQUARE)
				continue;
			
			if (!wasGridUpdateIdIncreased)
			{
				m_ResourceGrid.IncreaseGridUpdateId();
				
				wasGridUpdateIdIncreased = true;
			}
			
			m_ResourceGrid.UpdateResourceDynamicItem(dynamicComponent);
			dynamicComponent.UpdateLastPosition();
			
			--m_iDynamicComponentsFrameBudget;
			++m_iDynamicComponentsPivot;
			
			if (m_iDynamicComponentsFrameBudget <= 0)
				break;
        }
		
		//!------------------------------------------------ Process and update subscribed interactors.
		m_ResourceGrid.ResetFrameBudget();
		
		int subscribedInteractorsCount = m_aSubscribedInteractors.Count();
		SCR_ResourceInteractor subscribedInteractor;
		
		for (int i = subscribedInteractorsCount - 1; i >= 0; --i)
		{
			if(m_iSubscribedInteractorPivot >= subscribedInteractorsCount)
				m_iSubscribedInteractorPivot = 0;
			
			subscribedInteractor = m_aSubscribedInteractors[m_iSubscribedInteractorPivot];
			
			if (!subscribedInteractor)
			{
				m_aSubscribedInteractors.Remove(m_iSubscribedInteractorPivot);
				
				continue;
			}
			
			if (vector.DistanceSq(subscribedInteractor.GetOwnerOrigin(), subscribedInteractor.GetLastPosition()) > SCR_ResourceComponent.UPDATE_DISTANCE_TRESHOLD_SQUARE)
				m_ResourceGrid.UpdateInteractor(m_aSubscribedInteractors[m_iSubscribedInteractorPivot], true);
			
			++m_iSubscribedInteractorPivot;
			
			if (m_ResourceGrid.GetFrameBudget() <= 0)
				break;
		}
		
		//!------------------------------------------------------------- Replicate resource listeners.
		m_ResourceSystemSubscriptionManager.ReplicateListeners();
    }
 	
	//------------------------------------------------------------------------------------------------
    override protected void OnDiag(float timeSlice)
    {
        DbgUI.Begin("SCR_ResourceSystem");
        DbgUI.Text("Containers: " + m_aContainers.Count());
        DbgUI.Text("Dynamic components: " + m_aDynamicComponents.Count());
        DbgUI.Text("Subscribed interactors: " + m_aSubscribedInteractors.Count());
 
        if (DbgUI.Button("Dump active components"))
        {
			foreach (SCR_ResourceContainer container : m_aContainers)
            {
                Print(container.GetOwner(), LogLevel.ERROR);
            }
			
            foreach (SCR_ResourceComponent component : m_aDynamicComponents)
            {
                Print(component.GetOwner(), LogLevel.ERROR);
            }
			
			foreach (SCR_ResourceInteractor interactor : m_aSubscribedInteractors)
            {
                Print(interactor.GetOwner(), LogLevel.ERROR);
            }
        }
 
        DbgUI.End();
    }
	
	//------------------------------------------------------------------------------------------------
    void RegisterDynamicComponent(notnull SCR_ResourceComponent component)
    {
        if (!m_aDynamicComponents.Contains(component))
            m_aDynamicComponents.Insert(component);
    }
	
	//------------------------------------------------------------------------------------------------
    void RegisterContainer(notnull SCR_ResourceContainer container)
    {
        if (!m_aContainers.Contains(container))
            m_aContainers.Insert(container);
    }
	
	//------------------------------------------------------------------------------------------------
    void RegisterSubscribedInteractor(notnull SCR_ResourceInteractor interactor)
    {
        if (!m_aSubscribedInteractors.Contains(interactor))
            m_aSubscribedInteractors.Insert(interactor);
    }
	
	//------------------------------------------------------------------------------------------------
    void UnregisterDynamicComponent(notnull SCR_ResourceComponent component)
    {
        m_aDynamicComponents.RemoveItem(component);
    }
	
	//------------------------------------------------------------------------------------------------
    void UnregisterContainer(notnull SCR_ResourceContainer container)
    {
        m_aContainers.RemoveItem(container);
    }
	
	//------------------------------------------------------------------------------------------------
    void UnregisterSubscribedInteractor(notnull SCR_ResourceInteractor interactor)
    {
        m_aSubscribedInteractors.RemoveItem(interactor);
    }
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnStarted()
	{
		m_ResourceGrid = GetGame().GetResourceGrid();
		m_ResourceSystemSubscriptionManager = GetGame().GetResourceSystemSubscriptionManager();
	}
}
