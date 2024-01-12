[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_BaseDeployableInventoryItemComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Base class which all deployable inventory items inherit from
class SCR_BaseDeployableInventoryItemComponent : ScriptComponent
{
	[RplProp(onRplName: "OnRplDeployed")] 
	protected bool m_bIsDeployed;
	
	protected int m_iItemOwnerID = -1;
	
	//------------------------------------------------------------------------------------------------
	//! Gets called when deploy action is executed by player
	void Deploy(IEntity userEntity = null)
	{						
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp || rplComp.IsProxy())
			return;
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		if (userEntity)
			m_iItemOwnerID = playerManager.GetPlayerIdFromControlledEntity(userEntity);
		
		// Put deploy logic here
		
		m_bIsDeployed = true;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets called when dismantle action is executed by player
	void Dismantle(IEntity userEntity = null)
	{
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp || rplComp.IsProxy())
			return;
		
		m_iItemOwnerID = -1; // Reset owner ID
		
		// Put dismantle logic here
		
		m_bIsDeployed = false;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRplDeployed();
	
	//------------------------------------------------------------------------------------------------
	bool CanDeployBeShown(notnull IEntity userEntity)
	{
		return !m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanDismantleBeShown(notnull IEntity userEntity)
	{
		return m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDeployed()
	{
		return m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetItemOwnerID()
	{
		return m_iItemOwnerID;
	}
};
