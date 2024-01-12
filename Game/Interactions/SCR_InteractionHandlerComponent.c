//------------------------------------------------------------------------------------------------
class SCR_InteractionHandlerComponentClass: InteractionHandlerComponentClass
{
};


//------------------------------------------------------------------------------------------------
/*!
	Provides an identification for SCR_InteractionHandlerComponent which type of input to use.
	This is added for convenience of quick switching of different methods.
*/
enum SCR_NearbyContextDisplayMode
{
	//! Nearby display will never be allowed.
	DISABLED = 0,
	
	//! Nearby display will always be on (when possible).
	ALWAYS_ON = 1,
	
	//! Nearby display will show on provided input action.
	ON_INPUT_ACTION = 2,
	
	//! Nearby display will show when controlled entity is in freelook.
	ON_FREELOOK = 3
};

/*!
	This component allows the player to interact with their environment.
	It collects UserActionContext from ActionsManagerComponent from surrounding entities via queries,
	filters and finds the most appropriate one and provides script API to work with them.
	
	It should always be attached to PlayerController entity and is local only.
*/
class SCR_InteractionHandlerComponent : InteractionHandlerComponent
{
	/*!
		Display (action menu) used to show UI to the player regarding collected actions.
	*/
	protected SCR_BaseInteractionDisplay m_pDisplay;
	
	[Attribute("3", UIWidgets.ComboBox, "Display mode", "", ParamEnumArray.FromEnum(SCR_NearbyContextDisplayMode), category: "Nearby Context Properties" )]
	SCR_NearbyContextDisplayMode m_eDisplayMode;
	
	[Attribute("", UIWidgets.EditBox, "Action to listen for when SCR_NearbyContextDisplayMode is set to ON_INPUT_ACTION", category: "Nearby Context Properties")]
	string m_sActionName;
	[Attribute("", UIWidgets.EditBox, "Context to activate when SCR_NearbyContextDisplayMode is set to ON_INPUT_ACTION. Mustn't be empty to be activated.", category: "Nearby Context Properties")]
	string m_sActionContext;
	
	
	//! Last selected context
	protected UserActionContext m_pLastContext;
	//! Last selected user action
	protected BaseUserAction m_pLastUserAction;
	//! Currently selected action index
	protected int m_iSelectedActionIndex;
	

	private bool m_bIsPerforming;
	private bool m_bLastInput;
	private float m_fCurrentProgress;

	//! List of inspected entities (weapon, ...)
	protected ref array<IEntity> m_aInspectedEntities = {};

	private IEntity m_pControlledEntity;

	
	//------------------------------------------------------------------------------------------------
	protected SCR_BaseInteractionDisplay FindDisplay(IEntity owner)
	{
		PlayerController playerController = PlayerController.Cast(owner);
		if (!playerController) 
		{
			Print("InteractionHandler must be attached to a PlayerController!", LogLevel.ERROR);
			return null;
		}
		HUDManagerComponent hudManager = HUDManagerComponent.Cast(playerController.FindComponent(HUDManagerComponent));
		array<BaseInfoDisplay> displayInfos = {};
		int count = hudManager.GetInfoDisplays(displayInfos);
		for (int i = 0; i < count; i++)
		{
			SCR_BaseInteractionDisplay current = SCR_BaseInteractionDisplay.Cast(displayInfos[i]);
			if (current)
				return current;
		}

		Print("InteractionDisplay not found! InteractionDisplay must be stored in HUDManagerComponent of a PlayerController!", LogLevel.WARNING);
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Checks input, compares it to previous states and evaluates what the handler should do next.
	*/
	protected void DoProcessInteraction(
		ChimeraCharacter user, 
		UserActionContext context, 
		BaseUserAction action, 
		bool canPerform, 
		bool performInput, 
		float timeSlice,
		SCR_BaseInteractionDisplay display)
	{
		if (action)
			action.SetActiveContext(context);

		// Can action be performed?
		bool isOk = action && canPerform && action == m_pLastUserAction;
		// We want to perform and action is OK
		if (performInput && isOk)
		{
			// We want to be performing, but we're not yet.
			// Start the action and dispatch events.
			if (!m_bIsPerforming && !m_bLastInput)
			{
				if (!GetCanInteractScript(user))
					return;

				// UI
				if (display)
					display.OnActionStart(user, action);
				
				// Start the action. Calls action.OnActionStart
				user.DoStartObjectAction(action);
				
				// Set state
				m_bIsPerforming = true;
				m_fCurrentProgress = 0.0;
			}
			// We want to perform and we already started performing,
			// update continuous handler state until we're finished
			else if (m_bIsPerforming)
			{
				// Tick action
				if (action.ShouldPerformPerFrame())
					user.DoPerformContinuousObjectAction(action, timeSlice);
				
				// Update elapsed time
				ScriptedSignalUserAction signalUserAction = ScriptedSignalUserAction.Cast(action);
				SCR_ScriptedUserAction scriptedUserAction = SCR_ScriptedUserAction.Cast(action);
				if (signalUserAction)
					m_fCurrentProgress = signalUserAction.GetActionProgress();
				else if (scriptedUserAction)
					m_fCurrentProgress = scriptedUserAction.GetActionProgress(m_fCurrentProgress, timeSlice);
				else
					m_fCurrentProgress += timeSlice;
				
				// Get action duration
				float duration = action.GetActionDuration();
				if (duration == 0 && signalUserAction)
					duration = signalUserAction.GetMaximumValue() - signalUserAction.GetMinimumValue();
				
				// Update UI
				if (display)
					display.OnActionProgress(user, action, m_fCurrentProgress, Math.AbsFloat(duration));
				
				// We are finished, dispatch events and reset state
				if (m_fCurrentProgress >= duration && duration >= 0)
				{
					// Update UI
					if (display)
						display.OnActionFinish(user, action, ActionFinishReason.FINISHED);
					
					// Finally perform action
					if (!action.ShouldPerformPerFrame())
						user.DoPerformObjectAction(action);

					// Reset state
					m_fCurrentProgress = 0.0;
					m_bIsPerforming = false;
				}
			}
		}
		else
		{
			// Input was released, we were performing previously,
			// stop performing and dispatch necessary events.
			if (m_bIsPerforming)
			{
				// Update UI
				if (display)
					display.OnActionFinish(user, action, ActionFinishReason.INTERRUPTED);
				
				// Cancel the action. Calls action.OnActionCanceled
				user.DoCancelObjectAction(action);
				
				// Reset state
				m_bIsPerforming = false;
				m_fCurrentProgress = 0.0;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool CollectInputs(out bool bPerform, out float fScroll)
	{
		InputManager pInputManager = GetGame().GetInputManager();
		if (!pInputManager)
			return false;

		pInputManager.ActivateContext("ActionMenuContext");
		bPerform = pInputManager.GetActionTriggered("PerformAction");
		fScroll = pInputManager.GetActionValue("SelectAction");
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetCanInteractScript(IEntity controlledEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return false;
		
		// No interactions when menu is open
		MenuManager menuManager = GetGame().GetMenuManager();
		if (menuManager && menuManager.IsAnyMenuOpen())
			return false;
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (characterController && !characterController.CanInteract())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetIsInteractionAvailableScript()
	{
		return IsContextAvailable();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override BaseUserAction GetSelectedActionScript()
	{
		return m_pLastUserAction;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool DoIntersectCheck(IEntity controlledEntity)
	{
		if (!controlledEntity)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return false;
		
		if (character.IsInVehicle())
			return true;

		if (character.GetCharacterController().GetInspect())
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnContextChanged(UserActionContext previousContext, UserActionContext newContext)
	{
		// Changed, so hide previous
		UserActionContext currentContext = GetCurrentContext();
		if (!newContext || newContext != currentContext)
		{
			m_iSelectedActionIndex = 0;
			if (m_pDisplay)
				m_pDisplay.HideDisplay();
		}
		// Changed, so show new
		if (m_pDisplay && newContext)
			m_pDisplay.ShowDisplay();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override event bool CanContextChange(UserActionContext currentContext, UserActionContext newContext)
	{
		// Setting null context might be desirable in certain cases when state should be cleared, see below:
		if (!newContext)
		{
			// Allow clearing if no entity is controlled, to prevent leaking of contexts
			if (!m_pControlledEntity)
				return true;
		
			// Allow clearing of context if controlled entity is destroyed, at this point interaction should begone
			DamageManagerComponent dmg = DamageManagerComponent.Cast(m_pControlledEntity.FindComponent(DamageManagerComponent));
			if (dmg && dmg.IsDestroyed())
				return true;
			
			// Otherwise continue with the usual
		}
		
		
		// Check whether we are still in range
		if (currentContext && m_pControlledEntity)
		{
			// We will leave a small error threshold
			const float threshold = 1.1;
			// Global action visibility range in meters
			float visRange = GetVisibilityRange();
			// Maximum sq distance we can interact at
			float maxSqDistance = (visRange * visRange) * 1.1;
			// Sq distance to controlled entity
			float sqDistance = vector.DistanceSq(currentContext.GetOrigin(), m_pControlledEntity.GetOrigin());
			
			if (sqDistance > maxSqDistance)
			{
				// We are out of range, context can change safely
				return true;
			}
		}
		
		// Suppress context changing when we are interacting with one already
		if (currentContext && m_bIsPerforming)
		{
			// TODO: Validate distance to ctx
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns true if nearby action contexts should be shown when
		display mode is set to automatic freelook mode.
	*/
	protected bool ShouldBeEnabledInFreelook(ChimeraCharacter character)
	{
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
		
		// When freelook is enabled manually, enable the display
		if (!controller.IsFreeLookEnforced() && controller.IsFreeLookEnabled())
			return true;
		
		
		// Inspection is priority
		if (controller.GetInspect())
			return true;
		
		// When forced, avoid displaying in certain cases
		
		// Suppress display when getting in our out
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (compartmentAccess)
		{
			if (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut())
				return false;
		}
		
		// Supress display when in a vehicle while in 3rd person
		if (character.IsInVehicle())
		{
			if (controller.IsInThirdPersonView())
				return false;
		}
		
		// Supress display when falling
		if (controller.IsFalling())
			return false;
		
		// Or climbing
		if (controller.IsClimbing())
			return false;
		
		// Or unconscious
		if (controller.IsUnconscious())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ShouldBeEnabled(SCR_NearbyContextDisplayMode displayMode, ChimeraCharacter character, bool playerCameraOnly = true)
	{
		// Disallow when character is none
		if (!character)
			return false;
		
		// Disallow out of player camera when true
		if (playerCameraOnly)
		{
			CameraManager cameraManager = GetGame().GetCameraManager();
			if (cameraManager && !PlayerCamera.Cast(cameraManager.CurrentCamera()))
				return false;
		}
		
		// Handle different display mode cases
		switch (displayMode)
		{
			// Always off
			case SCR_NearbyContextDisplayMode.DISABLED:
				return false;
			
			// Always on
			case SCR_NearbyContextDisplayMode.ALWAYS_ON:
				return true;
			
			// On action
			case SCR_NearbyContextDisplayMode.ON_INPUT_ACTION:
			{
				if (m_sActionName.IsEmpty())
					return false;
				
				InputManager inputManager = GetGame().GetInputManager();
				if (!m_sActionContext.IsEmpty())
					inputManager.ActivateContext(m_sActionContext);
				
				return inputManager.GetActionValue(m_sActionName) > 0;
			}
			
			// When in freelook
			case SCR_NearbyContextDisplayMode.ON_FREELOOK:
			{
				if (!ShouldBeEnabledInFreelook(character))
					return false;
				
				return character.GetCharacterController().IsFreeLookEnabled();
			}
		}
		
		// Nope, sorry.
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override array<IEntity> GetManualOverrideList(IEntity owner, out vector referencePoint)
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase camera = cameraManager.CurrentCamera();
			vector rayDir = camera.GetWorldTransformAxis(2);
			vector rayStart = camera.GetOrigin();
			referencePoint = rayStart + rayDir;
			
			// Inspection correction
			ChimeraCharacter character = ChimeraCharacter.Cast(m_pControlledEntity);
			if (character)
			{
				// During inspection (of a weapon)
				CharacterControllerComponent controller = character.GetCharacterController();
				if (controller.GetInspect() && controller.GetInspectCurrentWeapon())
				{
					// Assume that while in inspection, weapon is tilted and its
					// left side is pointed towards the player camera
					IEntity inspectedEntity = controller.GetInspectEntity();
					
					vector origin = inspectedEntity.GetOrigin();
					vector normal = -inspectedEntity.GetWorldTransformAxis(0);
					
					referencePoint = SCR_Math3D.IntersectPlane(rayStart, rayDir, origin, normal);
					// Shape.CreateSphere(COLOR_RED, ShapeFlags.ONCE, referencePoint, 0.01);
				}
			}
		}
		else
			referencePoint = vector.Zero;

		return m_aInspectedEntities;
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleInspection(notnull ChimeraCharacter character, float timeSlice)
	{
		if (character.GetCharacterController().GetInspect())
		{
			SetManualCollectionOverride(true);
			m_aInspectedEntities.Clear();
			
			// Weapon is the priority if inspected, including all attachements
			CharacterControllerComponent ctrlComp = character.GetCharacterController();
			if (ctrlComp.GetInspectCurrentWeapon())
			{
				// Insert all items we can be interested in
				BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
				if (weaponManager)
				{
					BaseWeaponComponent weapon = weaponManager.GetCurrentWeapon();
					if (weapon)
					{
						m_aInspectedEntities.Insert(weapon.GetOwner());
						
						array<AttachmentSlotComponent> attachments = {};
						weapon.GetAttachments(attachments);
						
						foreach (AttachmentSlotComponent attachment : attachments)
						{
							IEntity attachedEntity = attachment.GetAttachedEntity();
							if (attachedEntity)
								m_aInspectedEntities.Insert(attachedEntity);
						}
						
						BaseMagazineComponent magazineComp = weapon.GetCurrentMagazine();
						if (magazineComp)
							m_aInspectedEntities.Insert(magazineComp.GetOwner());
					}
				}
			}
			else
			{
				// Whatever else is inspected kicks in
				IEntity inspectedItem = ctrlComp.GetInspectEntity();
				if (inspectedItem)
				{
					m_aInspectedEntities.Insert(inspectedItem);
				}
			}
		}
		else
		{
			SetManualCollectionOverride(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostFrame(IEntity owner, IEntity controlledEntity, float timeSlice)
	{
		// TODO@AS: Add a reliable init method and get rid of this monstrosity
		if (!m_pDisplay)
			m_pDisplay = FindDisplay(owner);
		
		m_pControlledEntity = controlledEntity;
		// Make sure we have a valid character
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		
		// Nearby context collection?
		bool enableNearbyCollection = ShouldBeEnabled(m_eDisplayMode, character, true);
		SetNearbyCollectionEnabled(enableNearbyCollection);
		
		// Make sure we have a valid character
		if (!character)
			return;
		
		HandleInspection(character, timeSlice);
		
		// Fetch current inputs
		bool bPerform; float fScroll;
		
		UserActionContext currentContext = GetCurrentContext();
		if (currentContext)
		{
			auto cEntity = GetGame().GetPlayerController().GetControlledEntity();
			array<BaseUserAction> actions = {};
			array<bool> canPerform = {};
			int count = GetFilteredActions(actions, canPerform);
			if (count > 0)
				CollectInputs(bPerform, fScroll);
			
			foreach (BaseUserAction action : actions)
			{
				if (m_pDisplay)
					m_pDisplay.OnActionProgress( character, action, action.GetActionProgress(), action.GetActionDuration() );
			}
			
			AggregateActions(actions, canPerform);
			
			// First of all, prior to doing any destructive changes,
			// find the previous selected action (if any) and 
			// update the index, in case it has been shuffled.
			if (m_pLastUserAction)
			{
				BaseUserAction action;
				for (int i = 0, ac = actions.Count(); i < ac; i++)
				{
					action = actions[i];
					if (action && action == m_pLastUserAction)
					{
						m_iSelectedActionIndex = i;
						break;
					}
				}
			}
			
			// Update selection
			int iScrollAmount = 0;
			int prevActionIndex = m_iSelectedActionIndex;
			// But only if player is not performing an action already
			if (!m_bIsPerforming)
			{
				if ( Math.AbsFloat(fScroll) > 0.5 )
					iScrollAmount = Math.Clamp( fScroll, -1.0, 1.0 );
	
				
				if (iScrollAmount != 0)
					m_iSelectedActionIndex = m_iSelectedActionIndex - iScrollAmount;
			}

			// Make sure that selected action is always within bounds
			int actionsCount = actions.Count();
			m_iSelectedActionIndex = Math.Clamp(m_iSelectedActionIndex, 0, actionsCount - 1);

			BaseUserAction selectedAction = null;
			bool canPerformSelectedAction = false;

			if (m_bIsPerforming)
			{
				selectedAction = m_pLastUserAction;
				if (actions.Count() > prevActionIndex && actions[prevActionIndex] == m_pLastUserAction)
					canPerformSelectedAction = canPerform[prevActionIndex];
				else if (m_pLastUserAction)
					canPerformSelectedAction = m_pLastUserAction.CanBeShown(character) && m_pLastUserAction.CanBePerformed(character);
			}
			else if (actionsCount > 0)
			{
				selectedAction = actions[m_iSelectedActionIndex];
				canPerformSelectedAction = canPerform[m_iSelectedActionIndex];
			}
			
			// Process interaction
			DoProcessInteraction(character, currentContext, selectedAction, canPerformSelectedAction, bPerform, timeSlice, m_pDisplay);			
			m_pLastUserAction = selectedAction;
			SetSelectedAction(selectedAction);

			// Pass data to display
			if (m_pDisplay)
			{
				ref ActionsTuple pData = new ref ActionsTuple();
				bool canInteract = GetCanInteractScript(character);

				if (canInteract || m_bIsPerforming)
				{
					pData.param1 = actions;
					pData.param2 = canPerform;
				}
				else
				{
					pData.Init();
				}
				
				ActionDisplayData pDisplayData = new ActionDisplayData();
				pDisplayData.pUser = cEntity;
				pDisplayData.pActionsData = pData;
				pDisplayData.pSelectedAction = selectedAction;
				pDisplayData.pCurrentContext = currentContext;
				
				m_pDisplay.SetDisplayData(pDisplayData);
			}
		}
		// We don't have a context, but we possibly had, thus reset
		// our current action and make sure to dispatch events.
		else if (m_pLastContext != currentContext)
		{
			// We had valid action
			if (m_pLastUserAction)
			{
				// And we were performing it
				if (m_bIsPerforming)
				{
					// Reset state
					m_bIsPerforming = false;
					m_fCurrentProgress = 0.0;
					
					// Interruption event
					character.DoCancelObjectAction(m_pLastUserAction);
				}
				
				// Reset state
				m_pLastUserAction = null;
				SetSelectedAction(m_pLastUserAction);
			}
		}
		
		
		// Store last input
		m_bLastInput = bPerform;
		// Update last context
		m_pLastContext = currentContext;
	}
	
	private ref array<BaseUserAction> m_ActionsBuffer = {};
	private ref array<bool> m_PerformBuffer = {};
	private ref map<string, ref array<int>> m_IndicesBuffer = new map<string, ref array<int>>();
	
	/*!
		Modifies the input lists (which need to be 1:1 in length and logical sense) so
		each action which can be aggregated (BaseUserAction.CanAggregate() returns true)
		only displays the first available (or any first if none is available to perform)
		action, filtered by BaseUserAction.GetActionName(). 
	*/
	protected void AggregateActions(array<BaseUserAction> actionsList, array<bool> canPerformList)
	{
		m_ActionsBuffer.Copy(actionsList);
		m_PerformBuffer.Copy(canPerformList);
		m_IndicesBuffer.Clear();
		actionsList.Clear();
		canPerformList.Clear();
		
		// First pass, filter&gather
		for (int i = 0; i < m_ActionsBuffer.Count(); i++)
		{
			BaseUserAction action = m_ActionsBuffer[i];
			if (!action)
				continue;
			
			// For non-aggregated actions, skip this process
			bool canPerform = m_PerformBuffer[i];
			if (!action.CanAggregate())
				continue;
			
			// Group actions of same name for aggregation
			string actionName = action.GetActionName();
			if (!m_IndicesBuffer.Contains(actionName))
				m_IndicesBuffer.Insert(actionName, new array<int>());
			
			m_IndicesBuffer[actionName].Insert(i);
		}
		
		// Second pass, resolve&output
		for (int i = 0; i < m_ActionsBuffer.Count(); i++)
		{
			BaseUserAction action = m_ActionsBuffer[i];
			if (!action)
				continue;
			
			// For non-aggregated actions, append the action straight away
			bool canPerform = m_PerformBuffer[i];
			if (!action.CanAggregate())
			{
				actionsList.Insert(action);
				canPerformList.Insert(canPerform);
				continue;
			}
			
			// For aggregated actions, find group of given actions
			string actionName = action.GetActionName();
			// If group was sorted, it will be removed from the map,
			// and no longer present, therefore we can ommit rechecking
			if (!m_IndicesBuffer.Contains(actionName))
				continue;
			
			// If not resolved yet, resolve by finding available action
			int availableIndex = m_IndicesBuffer[actionName][0]; // By default the first action
			foreach (int index : m_IndicesBuffer[actionName])
			{
				// First performable hit
				if (m_PerformBuffer[index])
				{
					availableIndex = index;
					break;
				}
			}
			
			BaseUserAction aggregatedAction = m_ActionsBuffer[availableIndex];
			bool aggregatedState = m_PerformBuffer[availableIndex];
			actionsList.Insert(aggregatedAction);
			canPerformList.Insert(aggregatedState);
			// And remove the action from the group map
			m_IndicesBuffer.Remove(actionName);
		}
	}
};
