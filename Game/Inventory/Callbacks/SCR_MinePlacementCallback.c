//------------------------------------------------------------------------------------------------
class SCR_PlacementCallback : SCR_InvCallBack
{
	vector m_vMat[4];
	
	//------------------------------------------------------------------------------------------------
	void SetMatrix(vector mat[4])
	{
		m_vMat = mat;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Places the object on a specific position after being removed from the inventory
	protected override void OnComplete()
	{
		SCR_ItemPlacementComponent placementComponent = SCR_ItemPlacementComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ItemPlacementComponent));
		if (!placementComponent)
			return;
		
		RplComponent rplComponent = RplComponent.Cast(m_pItem.FindComponent(RplComponent));
		if (!rplComponent)
			return;
		
		//placementComponent.OnLocalPlacementCompleted(m_vMat, rplComponent.Id());
		
		super.OnComplete();
	}
}
