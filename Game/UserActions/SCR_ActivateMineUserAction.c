//------------------------------------------------------------------------------------------------
class SCR_ActivateMineUserAction : ScriptedUserAction
{
	[Attribute("30", "Arming delay in seconds.")]
	protected float m_fArmingDelay;
	
	[Attribute("10", "How long the user action will be shown as inactive after starting arming.")]
	protected float m_fArmingProtectionTime;
	
	protected InventoryItemComponent m_Item;
	
	protected bool m_bCanArm = true;
	
	//------------------------------------------------------------------------------------------------
	void OrientToForward(vector forward, vector mat[4])
	{
		vector origin = mat[3];
		vector basis[4];
		forward = -forward;
		Math3D.AnglesToMatrix(Vector(forward.VectorToAngles()[0], 0, 0), basis);
		Math3D.MatrixMultiply3(mat, basis, mat);
		mat[3] = origin;
	}
	
	//------------------------------------------------------------------------------------------------
	void AllowArming()
	{
		m_bCanArm = true;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetGroundNormal(notnull IEntity owner)
	{
		vector pos = owner.GetOrigin();
		vector normal = SCR_TerrainHelper.GetTerrainNormal(pos, owner.GetWorld());
		if (normal != vector.Zero)
			return normal;
		else
			return vector.Up;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CheckAngle(vector up)
	{
		if (vector.Dot(up, vector.Up) < 0.5) // Rject based on the angle of placement (the maximum should be dictated by item settings)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SnapToGround(IEntity user, out vector normal)
	{
		IEntity owner = GetOwner();
		vector mat[4];
		owner.GetTransform(mat);
		
		SCR_PlaceableInventoryItemComponent item = SCR_PlaceableInventoryItemComponent.Cast(owner.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (item)
			item.SnapToGround(normal, {user}, startOffset: mat[1] * 0.1, direction: -mat[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_bCanArm = false;
		GetGame().GetCallqueue().CallLater(AllowArming, 1000 * m_fArmingProtectionTime);
		
		vector matUser[4], mat[4];
		pUserEntity.GetTransform(matUser);
		
		vector normal = vector.Zero;
		SnapToGround(pUserEntity, normal);
		
		pOwnerEntity.GetTransform(mat);
		SCR_EntityHelper.OrientUpToVector(normal, mat);
		OrientToForward(matUser[2], mat);
		pOwnerEntity.SetTransform(mat);
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		
		if (!character)
		{
			Print("Non-ChimeraCharacter user tried using SCR_ActivateMineUserAction!", LogLevel.WARNING);
			return;
		}
		
		SCR_PressureTriggerComponent pressureTrigger = SCR_PressureTriggerComponent.Cast(pOwnerEntity.FindComponent(SCR_PressureTriggerComponent));
		if (!pressureTrigger)
			return;
		
		pressureTrigger.SetUser(pUserEntity);
		
		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");
			vector charWorldMat[4];
			pUserEntity.GetWorldTransform(charWorldMat);
			charWorldMat[3] = pOwnerEntity.GetOrigin();
			PointInfo ptWS = new PointInfo();
			ptWS.Set(null, "", charWorldMat);
			charController.TryUseItemOverrideParams(pOwnerEntity, false, false, itemActionId, 3, 0, 15.0, 0, 0, false, ptWS);
		}
		
		pOwnerEntity.GetTransform(mat);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsUnderWater()
	{
		return SCR_WorldTools.IsObjectUnderwater(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		vector mat[4];
		GetOwner().GetTransform(mat);
		
		if (IsUnderWater() || !CheckAngle(mat[1]))
			return false;
		
		if (Math.AbsFloat(mat[3][1] - user.GetOrigin()[1]) > 0.4)
			return false;
		
		return m_bCanArm;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_Item = InventoryItemComponent.Cast(pOwnerEntity.FindComponent(InventoryItemComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (m_Item == null || m_Item.GetParentSlot() != null || m_Item.IsLocked())
			return false;
		
		SCR_PressureTriggerComponent mineTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!mineTriggerComponent)
			return false;
		
		if (mineTriggerComponent.IsActivated())
			return false;
		
		return super.CanBeShownScript(user);
	}
};
