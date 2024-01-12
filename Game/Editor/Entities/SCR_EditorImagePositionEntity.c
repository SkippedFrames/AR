[EntityEditorProps(category: "GameScripted/Editor", description: "Image generator for pecific subset of entities", color: "255 0 0 255")]
class SCR_EditorImagePositionEntityClass: GenericEntityClass
{
};

/** @ingroup Editor_Entities
*/

/*!

*/
class SCR_EditorImagePositionEntity : GenericEntity
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, "Use this location fo entities with these labels.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Configuration")]
	protected ref array<EEditableEntityLabel> m_Labels;
	
	[Attribute("2", uiwidget:UIWidgets.Slider, "Delay between creating the entity and taking a screenshot.", params: "1 5 0.5", category: "Configuration")]
	protected float m_fDelay;
	
	[Attribute("0", desc: "Order in which the position will be evaluated.\nHigher numbers are processed first.", category: "Configuration")]
	protected int m_iPriority;
	
	[Attribute(uiwidget:UIWidgets.ResourcePickerThumbnail, params: "xob", category: "Configuration")]
	protected ResourceName m_PreviewMesh;
	
	[Attribute(desc: "", category: "Configuration")]
	protected bool m_bEnablePhysics;
	
	[Attribute(desc: "Position the entity by center of its bounding box, not by center of the prefab.", category: "Configuration")]
	protected bool m_bUseBoundingCenter;
	
	[Attribute(desc: "When enabled, the prefab must fit into camera view. When that's not the game, another camera tied to the position will be searched for.", category: "Configuration")]
	protected bool m_bMustFitInView;
	
	[Attribute(desc: "Names of camera entities belonging to this position. Multiple positions could refer to the same camera.", category: "Configuration")]
	protected ref array<string> m_aCameraNames;
	
	[Attribute(uiwidget:UIWidgets.ResourceNamePicker, params: "agr", category: "Animation")]
	protected ResourceName m_PosesGraph;
	
	[Attribute(uiwidget:UIWidgets.ResourceNamePicker, params: "asi", category: "Animation")]
	protected ResourceName m_PosesInstance;
	
	[Attribute(category: "Animation")]
	protected string m_sStartNode;
	
	[Attribute(category: "Animation")]
	protected string m_sPoseVar;
	
	[Attribute(category: "Animation")]
	protected int m_iPoseID;
	
	[Attribute(category: "Animation",  params: "1 7 1")]
	protected int m_iArmIK;
	
	[Attribute(uiwidget:UIWidgets.ResourcePickerThumbnail, params: "anm", category: "Animation")]
	protected ResourceName m_ArmIKResource;
	
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, category: "Animation", enums: SCR_Enum.GetList(EWeaponType, ParamEnum("<Unchanged>", "-1")))]
	protected EWeaponType m_ForceWeaponType;
	
	[Attribute("-4", uiwidget:UIWidgets.Slider, params: "-90 90 1", category: "Environment")]
	protected float m_fLatitude;
	
	[Attribute("71", uiwidget:UIWidgets.Slider, params: "-180 180 1", category: "Environment")]
	protected float m_fLongitude;
	
	[Attribute("0.415", uiwidget:UIWidgets.Slider, params: "0 1 0.01", category: "Environment")]
	protected float m_fTime;
	
	[Attribute("1985", uiwidget: UIWidgets.Slider, params: "1899 2050 1", category: "Environment")]
	protected int m_iYear;
	
	[Attribute("5", uiwidget: UIWidgets.Slider, params: "1 12 1", category: "Environment")]
	protected int m_iMonth;
	
	[Attribute("8", uiwidget: UIWidgets.Slider, params: "1 31 1", category: "Environment")]
	protected int m_iDay;
	
	[Attribute(defvalue: "Clear", uiwidget: UIWidgets.ComboBox, category: "Environment", enums: { ParamEnum("Clear", "Clear"), ParamEnum("Cloudy", "Cloudy"), ParamEnum("Overcast", "Overcast"), ParamEnum("Rainy", "Rainy") }, desc: "Area shape")]
	private string m_sWeatherState;
	
#ifdef WORKBENCH
	protected ref array<SCR_CameraBase> m_aCameras = {};
	protected SCR_EditorImagePositionEntity m_Parent;
	protected ref SCR_SortedArray<SCR_EditorImagePositionEntity> m_aSubPositions = new SCR_SortedArray<SCR_EditorImagePositionEntity>();
	protected IEntity m_Entity;
	protected ref array<IEntity> m_aCurrentNearbyEntities = {};
	protected ref array<IEntity> m_aOriginalNearbyEntities = {};
	protected string m_sNewWeaponMesh;
	protected string m_sCurrentWeaponMesh;
	
	protected CharacterAnimationComponent m_CharacterAnimation;
	
	float GetDelay()
	{
		return m_fDelay;
	}
	int GetPriority()
	{
		return m_iPriority;
	}
	bool IsSuitable(array<EEditableEntityLabel> labels)
	{
		foreach (EEditableEntityLabel label: m_Labels)
		{
			//--- All position labels must be compatible with the editable entity. If even one is missing, ignore the position.
			if (!labels.Contains(label))
				return false;
		}
		return true;
	}
	SCR_EditorImagePositionEntity FindSuitableSubPosition(SCR_SortedArray<SCR_EditorImagePositionEntity> subPositions, array<EEditableEntityLabel> labels)
	{
		for (int i = subPositions.Count() - 1; i >= 0; i--)
		{
			if (subPositions.GetValue(i).IsSuitable(labels))
				return subPositions.GetValue(i);
		}
		return null;
	}
	bool ActivatePosition(ResourceName prefab)
	{
		//--- Prevent AI groups from creating members themselves, do it manually here
		SCR_AIGroup.IgnoreSpawning(true);
		
		//--- Create prefab
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		GetTransform(spawnParams.Transform);
		m_Entity = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetWorld(), spawnParams);
		if (!m_Entity)
		{
			Debug.Error2(Type().ToString(), string.Format("Error when creating prefab '%1'!", prefab));
			return false;
		}
		
		//--- Orient composition to terrain
		SCR_SlotCompositionComponent composition = SCR_SlotCompositionComponent.Cast(m_Entity.FindComponent(SCR_SlotCompositionComponent));
		if (composition)
		{
			SCR_EditableEntityComponent editableComposition = SCR_EditableEntityComponent.GetEditableEntity(m_Entity);
			editableComposition.SetTransformWithChildren(spawnParams.Transform);
			//composition.OrientToTerrain();
		}
		
		//--- Set environment
		ChimeraWorld world = GetWorld();
		TimeAndWeatherManagerEntity envManager = world.GetTimeAndWeatherManager();
		if (envManager)
		{
			int h, m, s;
			envManager.SetCurrentLatitude(m_fLatitude);
			envManager.SetCurrentLongitude(m_fLongitude);
			envManager.TimeToHoursMinutesSeconds(m_fTime * 24, h, m, s);
			envManager.SetHoursMinutesSeconds(h, m, s, false);
			envManager.SetDate(m_iYear, m_iMonth, m_iDay, true);
			
			//Set weather
			WeatherStateTransitionManager weatherTransitionManager = envManager.GetTransitionManager();
			if (weatherTransitionManager)
			{
				WeatherStateTransitionNode transitionNode = weatherTransitionManager.CreateStateTransition(m_sWeatherState, 0, 1);
				transitionNode.SetLooping(true);
			
				weatherTransitionManager.EnqueueStateTransition(transitionNode, false);
				weatherTransitionManager.RequestStateTransitionImmediately(transitionNode);
			}

			//int hX, mX, sX;
			//envManager.GetHoursMinutesSeconds(hX, mX, sX);
			//PrintFormat("%1 == %2, %3 == %4, %5 == %6", h, hX, m, mX, s, sX);
		}
			
		//--- Move group members to sub-positions
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Entity);
		if (group)
		{
			//--- Make sure group AI members are spawned instantly, not asynchronously
			group.SetMemberSpawnDelay(0);
			group.SpawnUnits();
			
			array<AIAgent> agents = {};
			int agentCount = group.GetAgents(agents);
			if (agentCount <= m_aSubPositions.Count())
			{
				m_Entity = GetGame().SpawnEntity(GenericEntity, GetWorld(), spawnParams);
				IEntity member;
				vector transform[4];
				SCR_SortedArray<SCR_EditorImagePositionEntity> subPositions = new SCR_SortedArray<SCR_EditorImagePositionEntity>();
				subPositions.CopyFrom(m_aSubPositions);
				SCR_EditorImagePositionEntity subPosition;
				bool failed = false;
				for (int i = 0; i < agentCount; i++)
				{
					member = agents[i].GetControlledEntity();
					
					//--- Get member labels
					SCR_EditableEntityComponent editableMember = SCR_EditableEntityComponent.GetEditableEntity(member);
					SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(editableMember.GetInfo());
					array<EEditableEntityLabel> memberLabels = {};
					if (info)
						info.GetEntityLabels(memberLabels);
					
					//--- Find suitable sub-position, taking member labels into consideration
					subPosition = FindSuitableSubPosition(subPositions, memberLabels);
					if (subPosition)
					{
						subPositions.RemoveValues(subPosition);
						subPosition.GetTransform(transform);
						
						CloneCharacter(member, transform);
						//m_Entity.AddChild(member, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM); //--- Crashes the game! Not needed, area garbage collection removes clones anyway.
						subPosition.SetPose(member);
						subPosition.EOnImagePositonActivate(member);
					}
					else
					{
						Print(string.Format("Cannot capture group member @\"%1\"! Unable to find suitable sub-position for %1!", member.GetPrefabData().GetPrefabName()), LogLevel.WARNING);
						failed = true;
					}
				}
				SCR_EntityHelper.DeleteEntityAndChildren(group);
				
				if (failed)
					return false;
			}
			else
			{
				Print(string.Format("Cannot capture group @\"%1\"! It has %2 members, but the position '%3' has only %4 sub-positions!", prefab, agentCount, GetPositionName(), m_aSubPositions.Count()), LogLevel.WARNING);
				return false;
			}
			
			EOnImagePositonActivate(m_Entity);
		}
		else
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(m_Entity);
			if (character)
			{
				if (m_ForceWeaponType >= 0)
				{
					InventoryStorageManagerComponent inventoryStorage = InventoryStorageManagerComponent.Cast(m_Entity.FindComponent(InventoryStorageManagerComponent));
					if (inventoryStorage)
					{
						array<IEntity> items = {};
						for (int i = inventoryStorage.GetItems(items) - 1; i >= 0; i--)
						{
							BaseWeaponComponent weapon = BaseWeaponComponent.Cast(items[i].FindComponent(BaseWeaponComponent));
							if (weapon)
							{
								if (weapon.GetWeaponType() == m_ForceWeaponType)
								{
									m_sNewWeaponMesh = items[i].GetVObject().GetResourceName();
									break;
								}
							}
						}
					}
					
					BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Entity.FindComponent(BaseWeaponManagerComponent));
					if (weaponManager)
					{
						BaseWeaponComponent currentWeapon = weaponManager.GetCurrentWeapon();
						if (currentWeapon)
							m_sCurrentWeaponMesh = currentWeapon.GetOwner().GetVObject().GetResourceName();
					}
				}
				
				// Set hands position 
				CharacterControllerComponent characterController = character.GetCharacterController();
				 m_CharacterAnimation = characterController.GetAnimationComponent();
				
				vector transform[4];
				m_Entity.GetTransform(transform);
				CloneCharacter(m_Entity, transform);
				SetPose(m_Entity);
			}
			
			//--- Position the entity by its bounding center, not mesh origin
			if (m_bUseBoundingCenter)
			{
				vector min, max;
				SCR_Global.GetWorldBoundsWithChildren(m_Entity, min, max);
				vector bCenter = min + (max - min) / 2;
				m_Entity.SetOrigin(2 * m_Entity.GetOrigin() - bCenter);
				m_Entity.Update();
			}
		}
		
		//--- Activate physics
		if (m_bEnablePhysics)
		{
			Physics phys = m_Entity.GetPhysics();
			if (phys)
				phys.SetActive(true);
		}
		
		//--- Activate camera
		SCR_CameraBase camera;
		if (m_bMustFitInView)
		{
			vector min, max;
			SCR_Global.GetWorldBoundsWithChildren(m_Entity, min, max);
			array<vector> corners = {
				min,
				Vector(min[0], min[1], max[2]),
				Vector(min[0], max[1], min[2]),
				Vector(max[0], min[1], min[2]),
				Vector(max[0], max[1], min[2]),
				Vector(max[0], min[1], max[2]),
				Vector(min[0], max[1], max[2]),
				max
			};
			
			foreach (SCR_CameraBase cameraCandidate: m_aCameras)
			{
				bool isInView = true;
				for (int i = 0; i < 8; i++)
				{
					if (!cameraCandidate.IsInView(corners[i]))
					{
						isInView = false;
						break;
					}
				}
				if (isInView)
				{
					camera = cameraCandidate;
					break;
				}
			}
		}
		else
		{
			camera = m_aCameras[0];
		}
		
		if (camera)
		{
			CameraManager cameraManager = GetGame().GetCameraManager();
			if (cameraManager)
				cameraManager.SetCamera(camera);
		}
		else
			Debug.Error2(Type().ToString(), string.Format("No camera which would fit '%1' found on position '%2'!", prefab, GetPositionName()));
		
		return true;
	}
	void DeactivatePosition()
	{
		UpdateNearbyEntities();
		foreach (IEntity entity: m_aCurrentNearbyEntities)
		{
			if (!m_aOriginalNearbyEntities.Contains(entity))
				delete entity;
		}
	}
	protected void CloneCharacter(out IEntity character, vector transform[4])
	{
		InventoryItemComponent inventoryComponent = InventoryItemComponent.Cast(character.FindComponent(InventoryItemComponent));
		if (!inventoryComponent)
			return;
		
		//--- Clone character using inventory preview function so we can play animations on it
		IEntity clone = inventoryComponent.CreatePreviewEntity(GetWorld(), GetWorld().GetCurrentCameraId());
		if (!clone)
			return;
		
		//--- Swap weapons
		if (m_sNewWeaponMesh != m_sCurrentWeaponMesh)
		{
			IEntity currentWeapon, newWeapon;
			TNodeId currentPivot, newPivot;
			vector currentLocalTransform[4], newLocalTransform[4];
			
			IEntity child = clone.GetChildren();
			while (child)
			{
				VObject mesh = child.GetVObject();
				if (mesh)
				{
					if (mesh.GetResourceName() == m_sCurrentWeaponMesh)
					{
						currentWeapon = child;
						currentPivot = child.GetPivot();
						child.GetLocalTransform(currentLocalTransform);
					}
					if (mesh.GetResourceName() == m_sNewWeaponMesh)
					{
						newWeapon = child;
						newPivot = child.GetPivot();
						child.GetLocalTransform(newLocalTransform);
					}
				}
				child = child.GetSibling();
			}
			
			clone.AddChild(newWeapon, currentPivot);
			newWeapon.SetLocalTransform(currentLocalTransform);
			
			clone.AddChild(currentWeapon, newPivot);
			currentWeapon.SetLocalTransform(newLocalTransform);
		}
		
		
		SCR_EntityHelper.DeleteEntityAndChildren(character);
		character = clone;
		character.SetTransform(transform);
	}
	protected void SetPose(IEntity entity)
	{
		if (m_PosesGraph.IsEmpty() && m_PosesInstance.IsEmpty() && m_sStartNode.IsEmpty())
			return;
		
		PreviewAnimationComponent animComponent = PreviewAnimationComponent.Cast(entity.FindComponent(PreviewAnimationComponent));
		animComponent.SetGraphResource(entity, m_PosesGraph, m_PosesInstance, m_sStartNode);
		
		//animComponent.SetIkState(true, true);
		//animComponent.SetHandsIKPose(entity, m_PosesGraph);
		
		// Perform one frame step to apply animations immediately
		animComponent.UpdateFrameStep(entity, 1.0 / 30.0);
				
		// Find variable that we want to change
		int poseVar = animComponent.BindIntVariable(m_sPoseVar);
		if (poseVar != -1)
		{
			// change pose
			animComponent.SetIntVariable(poseVar, m_iPoseID);
			
			// Set hands IK 
			int armIkVar = animComponent.BindIntVariable("ArmIK");
			if (armIkVar != -1)
			{
				animComponent.SetIntVariable(armIkVar, m_iArmIK);
				animComponent.SetHandsIKPose(entity, m_ArmIKResource);
			}
		
			// Perform one frame step to submit graph variable change
			animComponent.UpdateFrameStep(entity, 1.0 / 30.0);
		}
		else
		{
			Debug.Error2(Type().ToString(), string.Format("Unable to set character pose at positon '%1'!", GetPositionName()));
		}
	}
	protected void AddSubPosition(SCR_EditorImagePositionEntity subPosition)
	{
		m_aSubPositions.Insert(subPosition.GetPriority(), subPosition);
	}
	protected void UpdateNearbyEntities()
	{
		m_aCurrentNearbyEntities.Clear();
		GetWorld().QueryEntitiesByAABB(GetOrigin() + vector.One * -128, GetOrigin() + vector.One * 128, QueryEntitiesCallback);
	}
	protected bool QueryEntitiesCallback(IEntity e)
	{
		m_aCurrentNearbyEntities.Insert(e);
		return true;
	}
	protected string GetPositionName()
	{
		if (GetName().IsEmpty())
			return GetOrigin().ToString();
		else
			return GetName();
	}
	event void EOnImagePositonActivate(IEntity entity)
	{
	}
	
	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		SCR_EditorImageGeneratorEntity manager = SCR_EditorImageGeneratorEntity.GetInstance();
		if (!manager)
		{
			Debug.Error2(Type().ToString(), "SCR_EditorImageGeneratorEntity is missing in the world!");
			return;
		}
		
		if (m_Parent)
		{
			//--- Sub-position
			m_Parent.AddSubPosition(this);
		}
		else
		{
			//--- Main position
			SCR_CameraBase camera;
			
			//--- Find referenced camera entities
			for (int i = 0, count = m_aCameraNames.Count(); i < count; i++)
			{
				camera = SCR_CameraBase.Cast(GetWorld().FindEntityByName(m_aCameraNames[i]));
				if (camera && !m_aCameras.Contains(camera))
					m_aCameras.Insert(camera);
			}
			
			//--- Find child camera entities
			IEntity child = GetChildren();
			while (child)
			{
				camera = SCR_CameraBase.Cast(child);
				if (camera && !m_aCameras.Contains(camera))
					m_aCameras.Insert(camera);
				
				child = child.GetSibling();
			}
			
			if (m_aCameras.IsEmpty())
			{
				Print("SCR_EditorImagePositionEntity is missing child entity of type SCR_CameraBase!", LogLevel.WARNING);
				return;
			}
			
			manager.AddPosition(this);
		}
		
		//--- Get nearby entities
		UpdateNearbyEntities();
		m_aOriginalNearbyEntities.Copy(m_aCurrentNearbyEntities);
	}
	void SCR_EditorImagePositionEntity(IEntitySource src, IEntity parent)
	{	
		m_Parent = SCR_EditorImagePositionEntity.Cast(parent);
			
		if (SCR_Global.IsEditMode(this))
		{
			if (m_PreviewMesh)
				SetObject(Resource.Load(m_PreviewMesh).GetResource().ToVObject(), "");
			
			return;
		}
		
		SetEventMask(EntityEvent.INIT);
	}
	void ~SCR_EditorImagePositionEntity()
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api && api.IsEntitySelected(this))
			SCR_EditorImageGeneratorEntity.AddSelectedPosition(this);
	}
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (m_Labels.IsEmpty())
			return;
		
		string name = "";
		for (int i = 0, count = m_Labels.Count(); i < count; i++)
		{
			if (i != 0)
			{
				if (i % 2 == 0)
					name += ",\n";
				else
					name += ", ";
			}
			
			name += typename.EnumToString(EEditableEntityLabel, m_Labels[i]);
		}
		
		float fontSize = 12;
		if (m_Parent)
			fontSize = 8;
		
		vector pos = GetOrigin();
		DebugTextWorldSpace.Create(GetWorld(), name, DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA | DebugTextFlags.ONCE, pos[0], pos[1], pos[2], fontSize, Color.WHITE, ARGBF(1, 0.5, 0, 1));
	}
#endif
};
