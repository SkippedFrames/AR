enum ECharacterLifeState
{
	ALIVE = 0,
	INCAPACITATED = 1,
	DEAD = 10
}

[ComponentEditorProps(category: "GameScripted/Character", description: "Scripted character controller", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterControllerComponentClass : CharacterControllerComponentClass
{
};

//------------------------------------------------------------------------------------------------
void OnPlayerDeathWithParam(SCR_CharacterControllerComponent characterController, IEntity killerEntity, notnull Instigator killer);
typedef func OnPlayerDeathWithParam;
typedef ScriptInvokerBase<OnPlayerDeathWithParam> OnPlayerDeathWithParamInvoker;

//------------------------------------------------------------------------------------------------
void OnControlledByPlayer(IEntity ownerEntity, bool controlled);
typedef func OnControlledByPlayer;
typedef ScriptInvokerBase<OnControlledByPlayer> OnControlledByPlayerInvoker;

//------------------------------------------------------------------------------------------------
void OnLifeStateChanged(ECharacterLifeState lifeState);
typedef func OnLifeStateChanged;
typedef ScriptInvokerBase<OnLifeStateChanged> OnLifeStateChangedInvoker;

//------------------------------------------------------------------------------------------------
void OnItemUseBegan(IEntity item, SCR_ConsumableEffectAnimationParameters animParams);
typedef func OnItemUseBegan;
typedef ScriptInvokerBase<OnItemUseBegan> OnItemUseBeganInvoker;

//------------------------------------------------------------------------------------------------
void OnItemUseEnded(IEntity item, bool successful, SCR_ConsumableEffectAnimationParameters animParams);
typedef func OnItemUseEnded;
typedef ScriptInvokerBase<OnItemUseEnded> OnItemUseEndedInvoker;

//------------------------------------------------------------------------------------------------
void OnItemUseFinished(IEntity item, bool successful, SCR_ConsumableEffectAnimationParameters animParams);
typedef func OnItemUseFinished;
typedef ScriptInvokerBase<OnItemUseFinished> OnItemUseFinishedInvoker;

//------------------------------------------------------------------------------------------------
class SCR_CharacterControllerComponent : CharacterControllerComponent
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.EditBox, params:"1 inf 0.1", desc: "Maximum duration it takes for character to drown\n[s]")]
	protected float m_fDrowningDuration;
	
	// Private members
	protected SCR_CharacterCameraHandlerComponent m_CameraHandler; // Set from the camera handler itself
	protected SCR_MeleeComponent m_MeleeComponent;
	protected bool m_bOverrideActions = true;
	protected bool m_bInspectionFocus;
	protected bool m_bCharacterIsDrowning;
	protected float m_fDrowningTime;
	protected ref SCR_ScriptedCharacterInputContext m_pScrInputContext;

	// Character event invokers
	ref ScriptInvokerVoid m_OnPlayerDeath = new ScriptInvokerVoid();
	ref OnPlayerDeathWithParamInvoker m_OnPlayerDeathWithParam = new OnPlayerDeathWithParamInvoker();
	ref OnLifeStateChangedInvoker m_OnLifeStateChanged = new OnLifeStateChangedInvoker();
	ref OnControlledByPlayerInvoker m_OnControlledByPlayer = new OnControlledByPlayerInvoker();
	ref ScriptInvokerFloat2<float> m_OnPlayerDrowning = new ScriptInvokerFloat2();
	ref ScriptInvokerVoid m_OnPlayerStopDrowning = new ScriptInvokerVoid();
	
	// Gadget event invokers
	ref ScriptInvoker<IEntity, bool, bool> m_OnGadgetStateChangedInvoker = new ref ScriptInvoker<IEntity, bool, bool>();
	ref ScriptInvoker<IEntity, bool> m_OnGadgetFocusStateChangedInvoker = new ref ScriptInvoker<IEntity, bool>();

	// Item event invokers
	ref OnItemUseBeganInvoker m_OnItemUseBeganInvoker = new ref OnItemUseBeganInvoker();
	ref OnItemUseEndedInvoker m_OnItemUseEndedInvoker = new ref OnItemUseEndedInvoker();
	// called when all listeners reacted on ItemUseEnded invoker - may delete the item now thus listerners to ItemUseFinished may get first parameter null! 
	ref OnItemUseFinishedInvoker m_OnItemUseFinishedInvoker = new ref OnItemUseFinishedInvoker(); 
	protected ref ScriptInvoker<AnimationEventID, AnimationEventID, int, float, float> m_OnAnimationEvent;

	// Diagnostics, debugging
	#ifdef ENABLE_DIAG
	private static bool s_bDiagRegistered;
	private static bool m_bEnableDebugUI;
	private CharacterAnimationComponent m_AnimComponent;
	private Widget m_wDebugRootWidget;
	private int m_bDebugLastStance = ECharacterStance.STAND;
	#endif

	//------------------------------------------------------------------------------------------------
	bool CanInteract()
	{
		if (IsDead() || IsUnconscious() || IsClimbing())
			return false;
		
		// No interactions when character is dead or in ADS
		ChimeraCharacter character = GetCharacter();
		if (character && character.IsInVehicleADS())
			return false;
		
		// Disable in vehicle 3pp
		if (character.IsInVehicle() && IsInThirdPersonView())
			return false;
		
		if (IsUsingItem())
			return false;
		
		if (GetIsWeaponDeployed())
			return false;
		
		return true;
	}
	
	protected void OnConsciousnessChanged(bool conscious)
	{
		OnLifeStateChanged(GetLifeState());
		
		if (conscious)
			return;

		if (IsDead())
			return;

		AIControlComponent aiControl = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
		if (!aiControl || !aiControl.IsAIActivated())
			return;
		
		IEntity currentWeapon;
		BaseWeaponManagerComponent wpnMan = GetWeaponManagerComponent();
		if (wpnMan && wpnMan.GetCurrentWeapon())
			currentWeapon = wpnMan.GetCurrentWeapon().GetOwner();

		if (currentWeapon)
			TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedContextual, true);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAnimationEvent()
	{
		if (!m_OnAnimationEvent)
			m_OnAnimationEvent = new ScriptInvoker();
		return m_OnAnimationEvent;
	}

	//------------------------------------------------------------------------------------------------
	//! Will be called when gadget taken/removed from hand
	override void OnGadgetStateChanged(IEntity gadget, bool isInHand, bool isOnGround) { m_OnGadgetStateChangedInvoker.Invoke(gadget, isInHand, isOnGround); };
	//! Will be called when gadget fully transitioned to or canceled focus mode
	override void OnGadgetFocusStateChanged(IEntity gadget, bool isFocused) { m_OnGadgetFocusStateChangedInvoker.Invoke(gadget, isFocused); };

	//------------------------------------------------------------------------------------------------
	//! Will be called when item use action is started
	override void OnItemUseBegan(IEntity item, int cmdID, int cmdIntArg, float cmdFloatArg, int intParam, float floatParam, bool boolParam) 
	{
		// Animation duration is not returned.
		SCR_ConsumableEffectAnimationParameters animParams = new SCR_ConsumableEffectAnimationParameters(cmdID, cmdIntArg, cmdFloatArg, -1.0, intParam, floatParam, boolParam);
		m_OnItemUseBeganInvoker.Invoke(item, animParams);
	};
	//! Will be called when item use action is complete
	override void OnItemUseEnded(IEntity item, bool successful, int cmdID, int cmdIntArg, float cmdFloatArg, int intParam, float floatParam, bool boolParam)
	{
		// Animation duration is not returned.
		SCR_ConsumableEffectAnimationParameters animParams = new SCR_ConsumableEffectAnimationParameters(cmdID, cmdIntArg, cmdFloatArg, -1.0, intParam, floatParam, boolParam);
		m_OnItemUseEndedInvoker.Invoke(item, successful, animParams);
		// now all interested in non-null item have listened, we can call Finished to delete the item
		m_OnItemUseFinishedInvoker.Invoke(item, successful, animParams);
	};

	//------------------------------------------------------------------------------------------------
	protected override void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		if (!m_OnAnimationEvent)
			return;

		m_OnAnimationEvent.Invoke(animEventType, animUserString, intParam, timeFromStart, timeToEnd);
	}

	//------------------------------------------------------------------------------------------------
	// handling of melee events. Sends true if melee started, false, when melee ends
	override void OnMeleeDamage(bool started)
	{
		m_MeleeComponent.SetMeleeAttackStarted(started);
	}

	//------------------------------------------------------------------------------------------------
	// Get life state of character, including scripted states
	ECharacterLifeState GetLifeState()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (char)
		{
			if (char.GetDamageManager().GetState() == EDamageState.DESTROYED)
				return ECharacterLifeState.DEAD;
		}

		if (char.GetCharacterController().GetInputContext().IsUnconscious())
			return ECharacterLifeState.INCAPACITATED;

		return ECharacterLifeState.ALIVE;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnPlayerDeath()
	{
		return m_OnPlayerDeath;
	}
	
	//------------------------------------------------------------------------------------------------
	OnPlayerDeathWithParamInvoker GetOnPlayerDeathWithParam()
	{
		return m_OnPlayerDeathWithParam;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeath(IEntity instigatorEntity, notnull Instigator instigator)
	{
		m_OnPlayerDeath.Invoke();
		m_OnPlayerDeathWithParam.Invoke(this, instigatorEntity, instigator);

		OnLifeStateChanged(GetLifeState());
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (pc && m_CameraHandler && m_CameraHandler.IsInThirdPerson())
			pc.m_bRetain3PV = true;

		// Insert the character and see if it held a weapon, if so, try adding that as well
		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if (!world)
			return;

		GarbageSystem garbageSystem = world.GetGarbageSystem();
		if (garbageSystem)
			garbageSystem.Insert(GetCharacter());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLifeStateChanged(ECharacterLifeState lifeState)
	{
		if (m_OnLifeStateChanged)
			m_OnLifeStateChanged.Invoke(lifeState);
		
		ChimeraCharacter char = GetCharacter();
		if (!char)
			return;
		
		IEntity vehicle = CompartmentAccessComponent.GetVehicleIn(char);
		if (!vehicle)
			return;
		
		SCR_VehicleFactionAffiliationComponent vehicleFactionAff = SCR_VehicleFactionAffiliationComponent.Cast(vehicle.FindComponent(SCR_VehicleFactionAffiliationComponent));
		if (!vehicleFactionAff)
			return;
		
		vehicleFactionAff.OnOccupantLifeStateChanged(lifeState);	
	}

	//------------------------------------------------------------------------------------------------
	override bool GetCanMeleeAttack()
	{
		if (!m_MeleeComponent)
			return false;

		if (IsFalling())
			return false;

		if (GetStance() == ECharacterStance.PRONE)
			return false;

		// TODO: Gadget melee weapon properties in case we want to be able to have melee component, like a shovel?
		if (IsGadgetInHands())
			return false;

		//! check presence of MeleeWeaponProperties component to ensure it is an Melee weapon or not
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (weaponManager && !SCR_WeaponLib.CurrentWeaponHasComponent(weaponManager, SCR_MeleeWeaponProperties))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Return true to override default behaviour.
	//! Returns false to use default behaviour - Perform Action key will immediately perform the action
	override bool OnPerformAction()
	{
		return m_bOverrideActions;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		ChimeraCharacter character = GetCharacter();
		if (!character)
			return;

		if (!m_MeleeComponent)
			m_MeleeComponent = SCR_MeleeComponent.Cast(character.FindComponent(SCR_MeleeComponent));
		if (!m_CameraHandler)
			m_CameraHandler = SCR_CharacterCameraHandlerComponent.Cast(character.FindComponent(SCR_CharacterCameraHandlerComponent));

		#ifdef ENABLE_DIAG
		if (!m_AnimComponent)
			m_AnimComponent = CharacterAnimationComponent.Cast(character.FindComponent(CharacterAnimationComponent));
		#endif
		
		m_pScrInputContext = new SCR_ScriptedCharacterInputContext();

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
		{
			eventHandlerManager.RegisterScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------ 
	protected override void OnApplyControls(IEntity owner, float timeSlice)
	{
		if (GetScrInputContext().m_iLoiteringType >= 0)
		{
			SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
			if (!handler.IsLoitering())
			{
				if (TryStartLoiteringInternal())
					return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------ 
	protected override void UpdateDrowning(float timeSlice, vector waterLevel)
	{
		ChimeraCharacter char = GetCharacter();
		if (!char)
			return;
		
		CompartmentAccessComponent accesComp = char.GetCompartmentAccessComponent();
		bool isInWatertightCompartment = accesComp && accesComp.GetCompartment() && accesComp.GetCompartment().GetIsWaterTight();

		float drowningTimeStartFX = 4;
		if (waterLevel[2] > 0 && !isInWatertightCompartment)
		{
			if (m_fDrowningTime < drowningTimeStartFX && (m_fDrowningTime + timeSlice) > drowningTimeStartFX && m_OnPlayerDrowning)
			{
				m_OnPlayerDrowning.Invoke(m_fDrowningDuration, drowningTimeStartFX);
				m_bCharacterIsDrowning = true;
			}
			
			m_fDrowningTime += timeSlice;
		}
		else
		{
			if (m_fDrowningTime && m_OnPlayerStopDrowning)
			{
				m_OnPlayerStopDrowning.Invoke();
				m_bCharacterIsDrowning = false;
			}
			m_fDrowningTime = 0;
			return;
		}

		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMan || !damageMan.GetResilienceHitZone())
			return;
		
		if (m_fDrowningTime > m_fDrowningDuration)
		{
			damageMan.GetResilienceHitZone().HandleDamage(1000, EDamageType.TRUE, GetOwner());
			damageMan.Kill(Instigator.CreateInstigator(char));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDrowningTime()
	{
		return m_fDrowningTime;
	}	
	
	//------------------------------------------------------------------------------------------------
	bool IsCharacterDrowning()
	{
		return m_bCharacterIsDrowning;
	}

	//------------------------------------------------------------------------------------------------ 
	override bool ShouldAligningAdjustAimingAngles()
	{
		return IsAligningBeforeLoiter();
	}
	
	//------------------------------------------------------------------------------------------------ 
	bool IsAligningBeforeLoiter()
	{
		return GetScrInputContext().m_iLoiteringType != -1 && GetScrInputContext().m_bLoiteringShouldAlignCharacter && GetAnimationComponent().GetHeadingComponent().IsAligning();
	}
	
	//------------------------------------------------------------------------------------------------ 
	override bool SCR_GetDisableMovementControls()
	{
		SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
		if (!handler)
			return false;
		
		if (GetScrInputContext().m_iLoiteringType > 0)
		{
			bool isAligningBeforeLoiter = IsAligningBeforeLoiter();
			bool isHolsteringBeforeLoiter = GetScrInputContext().m_iLoiteringShouldHolsterWeapon && IsChangingItem();
			
			if (isAligningBeforeLoiter || isHolsteringBeforeLoiter)
				return true;
		}
		
		return handler.IsLoitering();
	}

	//------------------------------------------------------------------------------------------------ 
	override void SCR_OnDisabledJumpAction()
	{
		SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
		if (!handler)
			return;
		
		if (handler.IsLoitering())
			StopLoitering(false);
		
		if (IsAligningBeforeLoiter())
		{
			CharacterHeadingAnimComponent headingComponent = GetAnimationComponent().GetHeadingComponent();
			if (headingComponent)
			{
				headingComponent.ResetAligning();
				GetScrInputContext().m_iLoiteringType = -1;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// #ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------
		override void OnDiag(IEntity owner, float timeslice)
		{
			ChimeraCharacter character = GetCharacter();
			if (!character)
				return;

			if (IsDead())
				return;

			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_NOBANKING))
				SetBanking(0);

			bool diagTransform = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_TRANSFORMATION);
			bool bIsLocalCharacter = SCR_PlayerController.GetLocalControlledEntity() == character;

			// Character Transformation Diag
			if (bIsLocalCharacter && diagTransform)
			{
				DbgUI.Begin("Character Transformation");
				const string CHAR_POS = "X:\t%1\nY:\t%2\nZ:\t%3";
				const string CHAR_ROT = "PITCH:\t%1\nYAW:\t%2\nROLL:\t%3";
				const string CHAR_SCALE = "%1";
				const string CHAR_AIM = "X:\t%1\nY:\t%2\nZ:\t%3";
				vector pos = character.GetOrigin();
				vector rot = character.GetYawPitchRoll();
				float scale = character.GetScale();
				vector aimRot = GetInputContext().GetAimingAngles();
				string strPosition = string.Format(CHAR_POS, pos[0].ToString(), pos[1].ToString(), pos[2].ToString());
				string strRotation = string.Format(CHAR_ROT, rot[1].ToString(), rot[0].ToString(), rot[2].ToString());
				string strScale = string.Format(CHAR_SCALE, scale.ToString());
				string strAiming = string.Format(CHAR_AIM, aimRot[0].ToString(), aimRot[1].ToString(), aimRot[2].ToString());
				DbgUI.Text("POSITION:\n" + strPosition);
				DbgUI.Text("ROTATION:\n" + strRotation);
				DbgUI.Text("SCALE:" + strScale);
				DbgUI.Text("AIMING ANGLES:\n" + strAiming);

				if (DbgUI.Button("Print plaintext to console"))
					Print("TransformInfo:\nPOSITION:\n"+strPosition + "\nROTATION:\n"+strRotation + "\nSCALE:"+strScale + "\nAIMING ANGLES:\n"+strAiming);

				DbgUI.End();
			}

			// Character Controller diag (inputs...)
			m_bEnableDebugUI = bIsLocalCharacter && DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_MENU);
			if (m_bEnableDebugUI && !m_wDebugRootWidget) // Currently controlled player
				CreateDebugUI();
			else if (!m_bEnableDebugUI && m_wDebugRootWidget)
				DeleteDebugUI();

			if (m_bEnableDebugUI)
				UpdateDebugUI();
		}

		//------------------------------------------------------------------------------------------------
		private void ReloadDebugUI()
		{
			if (m_wDebugRootWidget)
				DeleteDebugUI();

			CreateDebugUI();
		}

		//------------------------------------------------------------------------------------------------
		private void DeleteDebugUI()
		{
			m_wDebugRootWidget.RemoveFromHierarchy();
			m_wDebugRootWidget = null;
		}

		//------------------------------------------------------------------------------------------------
		private void UpdateInputCircles()
		{
			if (!m_wDebugRootWidget || !m_AnimComponent)
				return;

			Widget wInput = m_wDebugRootWidget.FindAnyWidget("Input");
			if (!wInput)
				return;

			WorkspaceWidget workspaceWidget = GetGame().GetWorkspace();

			float topSpeed = m_AnimComponent.GetTopSpeed(-1, false);

			TextWidget wMaxSpeed = TextWidget.Cast(wInput.FindAnyWidget("MaxSpeed"));
			if (wMaxSpeed)
			{
				float speed = Math.Ceil(topSpeed * 10) * 0.1;
				wMaxSpeed.SetText(speed.ToString() + "m/s");
			}

			float ringSize = FrameSlot.GetSizeX(wInput);
			float degSizeDivider = topSpeed * ringSize;
			if (degSizeDivider > 0)
			for (int spd = 0; spd < 3; spd++)
			{
				int spdType;
				if (spd == 0) spdType = EMovementType.WALK;
				if (spd == 1) spdType = EMovementType.RUN;
				if (spd == 2) spdType = EMovementType.SPRINT;

				int color;
				if (spd == 0) color = ARGB(255, 150, 255, 150);
				if (spd == 1) color = ARGB(255, 200, 200, 150);
				if (spd == 2) color = ARGB(255, 255, 150, 150);
				for (int deg = 0; deg < 360; deg++)
				{
					vector velInput = GetMovementVelocity();
					float inputForward = velInput[2];
					float inputRight = velInput[0];

					float degSize = m_AnimComponent.GetMaxSpeed(inputForward, inputRight, spdType) / degSizeDivider;
					if (spdType == EMovementType.SPRINT && !IsSprinting())
						degSize = 0;

					float degOff = (ringSize - degSize) * 0.5;

					string iRingname = "iRing_" + spd.ToString() + "_" + deg.ToString();

					ImageWidget wImg = ImageWidget.Cast(wInput.FindAnyWidget(iRingname));
					if (!wImg)
					{
						wImg = ImageWidget.Cast(workspaceWidget.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.BLEND | WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.NOWRAP, Color.FromInt(color), spd + 2, wInput));
						wImg.LoadImageTexture(0, "{90DF4F065D08EF4E}UI/Textures/HUD_obsolete/character/input_360ringslice.edds");
						wImg.SetRotation(deg);
						wImg.SetName(iRingname);
					}
					FrameSlot.SetAnchorMax(wImg, 0, 0);
					FrameSlot.SetAnchorMin(wImg, 0, 0);
					FrameSlot.SetSize(wImg, degSize, degSize);
					FrameSlot.SetPos(wImg, degOff, degOff);
				}
			}
		}

		//------------------------------------------------------------------------------------------------
		private void CreateDebugUI()
		{
			m_wDebugRootWidget = GetGame().GetWorkspace().CreateWidgets("{C70DA11469BBAF67}UI/layouts/Debug/HUD_Debug_Character.layout", null);
			UpdateInputCircles();
		}

		//------------------------------------------------------------------------------------------------
		private void UpdateDebugBoolWidget(string textWidgetName, bool isTrue, float time = 0)
		{
			TextWidget wTxt = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget(textWidgetName));
			if (!wTxt)
				return;

			time = Math.Ceil(time * 10) * 0.1;
			if (isTrue)
			{
				wTxt.SetColorInt(ARGB(255, 160, 210, 255));
				if (time != 0)
					wTxt.SetText("YES (" + time.ToString() + ")");
				else
					wTxt.SetText("YES");
			}
			else
			{
				wTxt.SetColorInt(ARGB(255, 255, 160, 160));
				if (time != 0)
					wTxt.SetText("NO (" + time.ToString() + ")");
				else
					wTxt.SetText("NO");
			}
		}

		//------------------------------------------------------------------------------------------------
		private void UpdateDebugUI()
		{
			ChimeraCharacter character = GetCharacter();

			// Reload debug UI for stance change
			if (m_bDebugLastStance != GetStance())
			{
				m_bDebugLastStance = GetStance();
				UpdateInputCircles();
			}

			float topSpeed = 0;
			if (m_AnimComponent)
				topSpeed = m_AnimComponent.GetTopSpeed(-1, false);

			Widget wCenter = m_wDebugRootWidget.FindAnyWidget("Center");
			Widget wCenter2 = m_wDebugRootWidget.FindAnyWidget("Center2");
			Widget wCenter3 = m_wDebugRootWidget.FindAnyWidget("Center3");

			vector movementInput = GetMovementInput();
			if (movementInput != vector.Zero)
			{
				float inputLength = movementInput.Length();
				if (inputLength > 1)
					movementInput *= 1 / inputLength;
			}

			if (wCenter && wCenter2 && wCenter3 && topSpeed > 0)
			{
				float inputForward = movementInput[0];
				float inputRight = movementInput[2];
				float maxSpeed = m_AnimComponent.GetMaxSpeed(inputForward, inputRight, GetCurrentMovementPhase());
				float moveScale = maxSpeed / topSpeed;
				float x = movementInput[0] * moveScale * 0.5 + 0.5;
				float y = -movementInput[2] * moveScale * 0.5 + 0.5;
				FrameSlot.SetAnchorMin(wCenter, x, y);
				FrameSlot.SetAnchorMax(wCenter, x, y);

				vector moveLocal = m_AnimComponent.GetInertiaSpeed();
				x = (moveLocal[0] / topSpeed) * 0.5 + 0.5;
				y = (-moveLocal[2] / topSpeed) * 0.5 + 0.5;
				FrameSlot.SetAnchorMin(wCenter2, x, y);
				FrameSlot.SetAnchorMax(wCenter2, x, y);

				moveLocal = character.VectorToLocal(GetVelocity());
				x = (moveLocal[0] / topSpeed) * 0.5 + 0.5;
				y = (-moveLocal[2] / topSpeed) * 0.5 + 0.5;
				FrameSlot.SetAnchorMin(wCenter3, x, y);
				FrameSlot.SetAnchorMax(wCenter3, x, y);
			}

			TextWidget wSpeed = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("Speed"));
			TextWidget wSpeed2 = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("Speed2"));
			if (wSpeed && wSpeed2)
			{
				float speed = Math.Ceil(m_AnimComponent.GetInertiaSpeed().Length() * 10) * 0.1;
				wSpeed.SetText(speed.ToString() + "m/s");

				speed = Math.Ceil(GetVelocity().Length() * 10) * 0.1;
				wSpeed2.SetText(speed.ToString() + "m/s");
			}

			TextWidget wAdjustedSpeed = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("AdjustedSpeed"));
			if (wAdjustedSpeed)
				wAdjustedSpeed.SetText(GetDynamicSpeed().ToString());

			TextWidget wAdjustedStance = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("AdjustedStance"));
			if (wAdjustedStance)
				wAdjustedStance.SetText(GetDynamicStance().ToString());


			UpdateDebugBoolWidget("ToggleSprint", GetIsSprintingToggle());
			UpdateDebugBoolWidget("IsInADS", IsWeaponADS());
			UpdateDebugBoolWidget("IsWeaponHolstered", !IsWeaponRaised());
			UpdateDebugBoolWidget("CanFireWeapon", GetCanFireWeapon());
			UpdateDebugBoolWidget("CanThrow", GetCanThrow());
			UpdateDebugBoolWidget("InFreeLook", IsFreeLookEnabled());

			TextWidget wMovementAngle = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("MovementAngle"));
			CharacterCommandHandlerComponent handler = m_AnimComponent.GetCommandHandler();
			if (!wMovementAngle || !handler)
				return;
			CharacterCommandMove moveCmd = handler.GetCommandMove();
			if (moveCmd)
			{
				float currAngle = moveCmd.GetMovementSlopeAngle();
				wMovementAngle.SetText(currAngle.ToString() + "deg");
			}
		}

	//------------------------------------------------------------------------------------------------
	// #endif ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	#endif

	override void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player)
	{
		if (am.GetActionTriggered("JumpOut") && CanJumpOutVehicleScript())
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(owner);
			CompartmentAccessComponent compAccess = character.GetCompartmentAccessComponent();
			
			BaseCompartmentSlot compartment = compAccess.GetCompartment();
			if (compartment)
			{
				CompartmentUserAction action = compartment.GetJumpOutAction();
				if (action && action.CanBePerformed(GetOwner()))
				{
					action.PerformAction(compartment.GetOwner(), GetOwner());
					return;
				}

				// Add the following once the correct action exists
				//if (!action && compAccess.CanGetOutVehicle())
				//{
				//	compAccess.GetOutVehicle(-1);
				//}
			}			
		}
		
		if (am.GetActionTriggered("GetOut") && CanGetOutVehicleScript())
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(owner);
			CompartmentAccessComponent compAccess = character.GetCompartmentAccessComponent();
			
			BaseCompartmentSlot compartment = compAccess.GetCompartment();
			if (compartment)
			{
				CompartmentUserAction action = compartment.GetGetOutAction();
				if (action && action.CanBePerformed(GetOwner()))
				{
					action.PerformAction(compartment.GetOwner(), GetOwner());
					return;
				}

				if (!action && compAccess.CanGetOutVehicle())
				{
					compAccess.GetOutVehicle(-1, false);
				}
			}
		}

		if (GetStance() == ECharacterStance.PRONE)
		{
			float value = am.GetActionValue("CharacterRoll");
			int rollValue = 0;
			if (value < -0.5)
				rollValue = 1;
			else if (value > 0.5)
				rollValue = 2;

			// If one wants to use hold action - it needs too be allowed on CharacterControllerComponent at character prefab or by calling EnableHoldInputForRoll(true) during construction/initialization
			if (ShouldHoldInputForRoll())
				SetRoll(rollValue);
			else if (rollValue != 0)
				SetRoll(rollValue);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CharacterControllerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef ENABLE_DIAG
		if (System.IsCLIParam("CharacterDebugUI")) // If the startup parameter is set, enable the debug UI
			m_bEnableDebugUI = true;

		if (!s_bDiagRegistered)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_MENU, "", "Enable Debug UI", "Character");
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_MENU, m_bEnableDebugUI);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_NOBANKING, "", "Disable Banking", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_TRANSFORMATION, "", "Enable transform info", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_RECOIL_CAMERASHAKE_DISABLE, "", "Disable recoil cam shake", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_ADDITIONAL_CAMERASHAKE_DISABLE, "", "Disable additional cam shake", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_CAMERASHAKE_TEST_WINDOW, "", "Show shake diag", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_ALLOW_INSPECTION_LOOKAT, "", "Inspection LookAt", "Character");
			s_bDiagRegistered = true;
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CharacterControllerComponent()
	{
		#ifdef ENABLE_DIAG
		if (m_wDebugRootWidget)
			DeleteDebugUI();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	/*!
		If a weapon with sights is equipped, advances to desired sights FOV info.
		\param allowCycling If enabled, selection will cycle from end to start and from start to end, otherwise it will be clamped.
		\param direction If above zero, advances to next info. If below zero, advances to previous info.
	*/
	void SetNextSightsFOVInfo(int direction = 1, bool allowCycling = false)
	{
		if (direction == 0)
			return;

		SightsFOVInfo fovInfo = GetSightsFOVInfo();
		if (!fovInfo)
			return;

		SCR_BaseVariableSightsFOVInfo variableFovInfo = SCR_BaseVariableSightsFOVInfo.Cast(fovInfo);
		if (!variableFovInfo)
			return;

		if (direction > 0)
			variableFovInfo.SetNext(allowCycling);
		else
			variableFovInfo.SetPrevious(allowCycling);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		If a weapon with multiple sights is equipped, switch the next or previous sights on the weapon (if any)
		\param direction If above zero, advances to next sights. If below zero, advances to previous sights.
	*/
	void SetNextSights(int direction = 1)
	{
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (!weaponManager)
			return;

		BaseWeaponComponent weaponComponent = weaponManager.GetCurrentWeapon();
		if (!weaponComponent)
			return;

		if (direction > 0)
			weaponComponent.SwitchNextSights();
		else
			weaponComponent.SwitchPrevSights();
	}


	//------------------------------------------------------------------------------------------------
	/*!
		Returns currently used SightsFOVInfo if any, null otherwise.
	*/
	SightsFOVInfo GetSightsFOVInfo()
	{
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (!weaponManager)
			return null;

		BaseWeaponComponent weaponComponent = weaponManager.GetCurrentWeapon();
		if (!weaponComponent)
			return null;

		BaseSightsComponent sightsComponent = weaponComponent.GetSights();
		if (!sightsComponent)
			return null;

		SightsFOVInfo fovInfo = sightsComponent.GetFOVInfo();
		if (!fovInfo)
			return null;

		return fovInfo;
	}

	protected override void OnControlledByPlayer(IEntity owner, bool controlled)
	{
		// Do initialization/deinitialization of character that was given/lost control by plyer here
		if (controlled)
		{
			GetGame().GetInputManager().AddActionListener("CharacterUnequipItem", EActionTrigger.DOWN, ActionUnequipItem);
			GetGame().GetInputManager().AddActionListener("CharacterDropItem", EActionTrigger.DOWN, ActionDropItem);
			GetGame().GetInputManager().AddActionListener("CharacterWeaponLowReady", EActionTrigger.DOWN, ActionWeaponLowReady);
			GetGame().GetInputManager().AddActionListener("CharacterWeaponRaised", EActionTrigger.DOWN, ActionWeaponRaised);
			GetGame().GetInputManager().AddActionListener("CharacterWeaponBipod", EActionTrigger.DOWN, ActionWeaponBipod);

			// TODO: This should be handled by camera handler itself
			if (m_CameraHandler)
				GetGame().GetInputManager().AddActionListener("SwitchCameraType", EActionTrigger.DOWN, m_CameraHandler.OnCameraSwitchPressed);
		}
		else
		{
			GetGame().GetInputManager().RemoveActionListener("CharacterUnequipItem", EActionTrigger.DOWN, ActionUnequipItem);
			GetGame().GetInputManager().RemoveActionListener("CharacterDropItem", EActionTrigger.DOWN, ActionDropItem);
			GetGame().GetInputManager().RemoveActionListener("CharacterWeaponLowReady", EActionTrigger.DOWN, ActionWeaponLowReady);
			GetGame().GetInputManager().RemoveActionListener("CharacterWeaponRaised", EActionTrigger.DOWN, ActionWeaponRaised);
			GetGame().GetInputManager().RemoveActionListener("CharacterWeaponBipod", EActionTrigger.DOWN, ActionWeaponBipod);

			// TODO: This should be handled by camera handler itself
			if (m_CameraHandler)
				GetGame().GetInputManager().RemoveActionListener("SwitchCameraType", EActionTrigger.DOWN, m_CameraHandler.OnCameraSwitchPressed);
		}

		m_OnControlledByPlayer.Invoke(owner, controlled);

		// diiferentiate the inventory setup for player and AI
		auto pCharInvComponent = SCR_CharacterInventoryStorageComponent.Cast( owner.FindComponent( SCR_CharacterInventoryStorageComponent ) );
		if (pCharInvComponent)
			pCharInvComponent.InitAsPlayer(owner, controlled);
	}

	//------------------------------------------------------------------------------------------------
	void ActionUnequipItem(float value = 0.0, EActionTrigger trigger = 0)
	{
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(GetInventoryStorageManager());
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = storageManager.GetCharacterStorage();
		if (storage)
			storage.UnequipCurrentItem();
	}

	//------------------------------------------------------------------------------------------------
	void ActionDropItem(float value = 0.0, EActionTrigger trigger = 0)
	{
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(GetInventoryStorageManager());
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = storageManager.GetCharacterStorage();
		if (storage)
			storage.DropCurrentItem();
	}

	//------------------------------------------------------------------------------------------------
	void ActionWeaponLowReady(float value = 0.0, EActionTrigger trigger = 0)
	{
		if (GetIsWeaponDeployed())
			return;

		if (GetIsWeaponDeployedBipod())
			return;

		if (CanPartialLower() && !IsPartiallyLowered())
			SetPartialLower(true);
	}

	//------------------------------------------------------------------------------------------------
	void ActionWeaponRaised(float value = 0.0, EActionTrigger trigger = 0)
	{
		if (!IsWeaponRaised())
			SetWeaponRaised(true);

		if (IsPartiallyLowered())
			SetPartialLower(false);
	}

	//------------------------------------------------------------------------------------------------
	void ActionWeaponBipod(float value = 0.0, EActionTrigger trigger = 0)
	{
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (!weaponManager)
			return;

		BaseWeaponComponent weapon = weaponManager.GetCurrentWeapon();
		if (!weapon || !weapon.HasBipod())
			return;

		if (GetIsWeaponDeployed() || GetIsWeaponDeployedBipod())
			StopDeployment();

		weapon.SetBipod(!weapon.GetBipod());
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnInspectionModeChanged(bool newState)
	{
		if (newState)
			m_bInspectionFocus = true;
		else
			m_bInspectionFocus = false;
	}

	protected int m_iTargetContext;

	//------------------------------------------------------------------------------------------------
	protected override float GetInspectTargetLookAt(out vector targetAngles)
	{
		#ifdef ENABLE_DIAG
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_ALLOW_INSPECTION_LOOKAT))
			return 0;
		#else
			return 0; // Disabled for now
		#endif

		// Just for testing
		if (Debug.KeyState(KeyCode.KC_ADD))
		{
			++m_iTargetContext;
			Debug.ClearKey(KeyCode.KC_ADD);
			m_bInspectionFocus = true;
		}
		if (Debug.KeyState(KeyCode.KC_SUBTRACT))
		{
			--m_iTargetContext;
			Debug.ClearKey(KeyCode.KC_SUBTRACT);
			m_bInspectionFocus = true;
		}

		if (!m_bInspectionFocus)
			return 0;

		vector aimChange = GetInputContext().GetAimChange();
		const float threshold = 0.001;
		if (Math.AbsFloat(aimChange[0]) > threshold || Math.AbsFloat(aimChange[1]) > threshold)
		{
			m_bInspectionFocus = false;
			return 0;
		} // not now

		IEntity ent = GetInspectEntity();
		if (!ent)
			return 0;


		SCR_InteractionHandlerComponent handler = SCR_InteractionHandlerComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_InteractionHandlerComponent));

		// Update target
		array<UserActionContext> contexts = {};
		vector _;
		array<IEntity> ents = handler.GetManualOverrideList(null, _);
		foreach (IEntity e : ents)
		{
			ActionsManagerComponent ac = ActionsManagerComponent.Cast(e.FindComponent(ActionsManagerComponent));
			if (!ac)
				continue;

			array<UserActionContext> buff = {};
			int bc = ac.GetContextList(buff);
			for (int i = 0; i < bc; ++i)
			{
				UserActionContext ctx = buff[i];
				if (ctx.GetActionsCount() == 0)
					continue;

				array<BaseUserAction> actions = {};
				int actionsCount = ctx.GetActionsList(actions);
				foreach (BaseUserAction action : actions)
				{
					if (action.CanBeShown(GetCharacter()))
					{
						contexts.Insert(ctx);
						break;
					}
				}
			}
		}

		int nonEmptyCount = contexts.Count();
		m_iTargetContext = Math.Repeat(m_iTargetContext, nonEmptyCount);


		if (nonEmptyCount == 0)
			return 0.0;

		UserActionContext ctx = contexts[m_iTargetContext];
		vector contextPos = ctx.GetOrigin();

		vector localAimDirection = GetHeadAimingComponent().GetAimingDirection();
		vector targetDirection = contextPos - GetCharacter().EyePosition();
		vector localTargetDirection = GetCharacter().VectorToLocal( targetDirection );
		localTargetDirection.Normalize();


		float lookYaw = Math.Sin(localTargetDirection[0]) * Math.RAD2DEG;
		float lookPitch = Math.Sin(localTargetDirection[1]) * Math.RAD2DEG;


		//Shape shape = Shape.CreateSphere(COLOR_RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, contextPos, 0.01);

		lookPitch -= GetAimingComponent().GetAimingDirection()[1];
		targetAngles = Vector(lookYaw, lookPitch, 0) * Math.DEG2RAD;

		return 4.30 * Math.RAD2DEG ; // speed, approx deg/s
	}
	
	SCR_ScriptedCharacterInputContext GetScrInputContext()
	{
		return m_pScrInputContext;
	}
	
	//------------------------------------------------------------------------------------------------ 
	void StartLoitering(int loiteringType, bool holsterWeapon, bool allowRootMotion, bool alignToPosition, vector targetPosition[4] = {"1 0 0", "0 1 0", "0 0 1", "0 0 0"})
	{
		if (GetCharacter().GetRplComponent() && !GetCharacter().GetRplComponent().IsOwner())
			return;
		
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(GetCharacter().GetAnimationComponent().GetCommandHandler());
		if (!scrCmdHandler)
			return;
		if (scrCmdHandler.IsLoitering())
			return;
		
		m_pScrInputContext.m_iLoiteringType = loiteringType;
		m_pScrInputContext.m_iLoiteringShouldHolsterWeapon = holsterWeapon;
		m_pScrInputContext.m_bLoiteringShouldAlignCharacter = alignToPosition;
		m_pScrInputContext.m_mLoiteringPosition = targetPosition;
		m_pScrInputContext.m_bLoiteringRootMotion = allowRootMotion;
		
		if (IsGadgetInHands())
			RemoveGadgetFromHand(true);
		
		if (alignToPosition)
			AlignToPositionFromCurrentPosition(targetPosition);
		
		TryStartLoiteringInternal();
	}
	
	//------------------------------------------------------------------------------------------------ 
	protected bool AlignToPositionFromCurrentPosition(vector targetPosition[4], float toleranceXZ = 0.01, float toleranceY = 0.01)
	{
		if (GetCharacter().GetRplComponent() && !GetCharacter().GetRplComponent().IsOwner())
			return false;
		
		vector currentTransform[4];
		GetCharacter().GetWorldTransform(currentTransform);
		CharacterHeadingAnimComponent headingComponent = GetAnimationComponent().GetHeadingComponent();
		if (headingComponent)
			headingComponent.AlignPosDirWS(currentTransform[3], currentTransform[2], targetPosition[3], targetPosition[2]); 

		return vector.DistanceSqXZ(currentTransform[3], targetPosition[3]) < (toleranceXZ * toleranceXZ)
			&& Math.AbsFloat(currentTransform[3][2] - targetPosition[3][2]) < toleranceY;
	}
	
	//------------------------------------------------------------------------------------------------ 
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_StartLoitering_S(int loiteringType, bool holsterWeapon, bool allowRootMotion, bool alignToPosition, vector targetPosition[4])
	{
		m_pScrInputContext.m_iLoiteringType = loiteringType;
		m_pScrInputContext.m_iLoiteringShouldHolsterWeapon = holsterWeapon;
		m_pScrInputContext.m_bLoiteringShouldAlignCharacter = alignToPosition;
		m_pScrInputContext.m_mLoiteringPosition = targetPosition;
		m_pScrInputContext.m_bLoiteringRootMotion = allowRootMotion;
		
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(GetCharacter().GetAnimationComponent().GetCommandHandler());
		scrCmdHandler.StartCommandLoitering();
	}
	
	//------------------------------------------------------------------------------------------------ 
	protected bool TryStartLoiteringInternal()
	{
		CharacterHeadingAnimComponent headingComponent = GetAnimationComponent().GetHeadingComponent();
		if (IsChangingItem())
			return false;
		
		if (GetScrInputContext().m_iLoiteringShouldHolsterWeapon && GetCurrentItemInHands() != null)
		{
			TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedContextual);
			return false;
		}
		
		if (headingComponent && headingComponent.IsAligning())
		{
			if (AlignToPositionFromCurrentPosition(GetScrInputContext().m_mLoiteringPosition, 0.02, 0.3)) // distance smaller than 2cm
				return false;
		}
		
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(GetCharacter().GetAnimationComponent().GetCommandHandler());
		scrCmdHandler.StartCommandLoitering();
		
		if (GetCharacter().GetRplComponent() && GetCharacter().GetRplComponent().IsProxy())
			Rpc(Rpc_StartLoitering_S,
				m_pScrInputContext.m_iLoiteringType,
				m_pScrInputContext.m_iLoiteringShouldHolsterWeapon,
				m_pScrInputContext.m_bLoiteringRootMotion,
				m_pScrInputContext.m_bLoiteringShouldAlignCharacter,
				m_pScrInputContext.m_mLoiteringPosition);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------ 
	//terminateFast should be true when we are going into alerted or combat state.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_StopLoitering_S(bool terminateFast)
	{
		if (GetCharacter().GetRplComponent() && GetCharacter().GetRplComponent().IsProxy())
			Print("Called RPC_StopLoitering_S on a proxy.", LogLevel.ERROR);
		
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(GetCharacter().GetAnimationComponent().GetCommandHandler());
		scrCmdHandler.StopLoitering(terminateFast);
	}
	
	//------------------------------------------------------------------------------------------------ 
	//terminateFast should be true when we are going into alerted or combat state.
	void StopLoitering(bool terminateFast)
	{
		if (GetCharacter().GetRplComponent() && !GetCharacter().GetRplComponent().IsOwner())
			return;
		
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(GetCharacter().GetAnimationComponent().GetCommandHandler());
		scrCmdHandler.StopLoitering(terminateFast);
		
		if (GetCharacter().GetRplComponent() && GetCharacter().GetRplComponent().IsProxy())
			Rpc(RPC_StopLoitering_S, terminateFast);
	}
};

enum ECharacterGestures
{
	NONE = 0,
	POINT_WITH_FINGER = 1
};
