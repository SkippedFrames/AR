enum EActionMenuScroll
{
	HIDDEN = 0,
	DISABLED = 1,
	ENABLED = 2,
};

enum EActionMenu
{
	UNDEFINED = 0,
	COLLAPSED = 1,
	EXPANDED = 2,
};

class ActionMenuColor
{
	const ref Color ENABLED = Color.FromSRGBA(255, 255, 255, 255);
	const ref Color DISABLED = Color.FromSRGBA(128, 128, 128, 255);
	const ref Color ENABLED_CONTEXT = Color.FromSRGBA(255, 182, 95, 255);
};

//! Temp class until localization is done
class ActionMenuFailReason
{
	const static string DEFAULT = "#AR-UserActionUnavailable";
};

class ActionMenuElementContext
{
	// Global action menu atributes
	static WorkspaceWidget s_wWorkspace;
	static Widget s_wActionMenu;

	// Shared element atributes
	static ResourceName s_sLayout;
	static float s_fFadeInSpeed;
	static int s_iSpacing = 2;
	
	// Selected element atributes
	static float s_fHeight = 32;
	static int s_iTextSize = 30;
	static float s_fAlpha = 1;
	
	// Non-selected element atributes
	static float s_fHeightNonSelected = 20;	
	static int s_iTextSizeNonSelected = 18;
	static float s_fAlphaNonSelected = 0.8;
};

class ActionMenuElement
{
	protected Widget m_wElement;
	protected TextWidget m_wActionText;
	protected TextWidget m_wContextText;
	protected ImageWidget m_wPerformType;
	protected ImageWidget m_wPerformTypeHold;
	protected ImageWidget m_wProgress;
	protected bool m_bIsSelected = false;
	protected int m_iIndex = 0;
	
	void SetEnabled(bool bEnabled)
	{
		if (m_wActionText) 
		{
			if (bEnabled)
			{
				m_wActionText.SetColor(ActionMenuColor.ENABLED);
			}
			else
			{
				m_wActionText.SetColor(ActionMenuColor.DISABLED);
			}
		}
		
		if (m_wContextText) 
		{
			if (bEnabled)
			{
				m_wContextText.SetColor(ActionMenuColor.ENABLED_CONTEXT);
			}
			else
			{
				m_wContextText.SetColor(ActionMenuColor.DISABLED);
			}
		}
	}

	bool Set(BaseUserAction pAction, int iIndex, bool bIsSelected, bool bIsAvailable)
	{
		if (!m_wElement)
			return false;
			
		if (!pAction)
			return false;

		m_bIsSelected = bIsSelected;
		m_iIndex = iIndex;		
				
		string sActionName = pAction.GetActionName();
		
		// TODO: Make a special widget?
		// Handle differently?
		if (m_bIsSelected && !bIsAvailable)
		{
			string failReason = pAction.GetCannotPerformReason();
			if (failReason == string.Empty)
				failReason = ActionMenuFailReason.DEFAULT;
			
			// failReason.ToUpper() wouldn't cut it :\
			sActionName = string.Format("%1 (%2)", sActionName, failReason);
		}
		
		array<string> aActionStrings = new array<string>();
		sActionName.Split("%CTX_HACK%", aActionStrings, true);

		int iActionStrings = aActionStrings.Count();
		
		if (iActionStrings == 0 || !aActionStrings[0])
			return false;
		
		// Set string in widget and format it with provided ActionNameParams (if any)
		m_wActionText.SetTextFormat(
			aActionStrings[0], 
			pAction.ActionNameParams[0], 
			pAction.ActionNameParams[1], 
			pAction.ActionNameParams[2], 
			pAction.ActionNameParams[3], 
			pAction.ActionNameParams[4], 
			pAction.ActionNameParams[5], 
			pAction.ActionNameParams[6], 
			pAction.ActionNameParams[7], 
			pAction.ActionNameParams[8]
		);
		
		if (iActionStrings > 1 && aActionStrings[1])
		{
			m_wContextText.SetText(aActionStrings[1]);
			m_wContextText.SetVisible(true);
		}
		else
		{
			m_wContextText.SetText("");
			m_wContextText.SetVisible(false);
		}

		if (bIsSelected)
		{
			m_wActionText.SetExactFontSize(ActionMenuElementContext.s_iTextSize);
			m_wActionText.SetDesiredFontSize(ActionMenuElementContext.s_iTextSize);
			
			m_wContextText.SetExactFontSize(ActionMenuElementContext.s_iTextSize);
			m_wContextText.SetDesiredFontSize(ActionMenuElementContext.s_iTextSize);
			
			m_wPerformType.SetSize(2, ActionMenuElementContext.s_fHeight);
		}
		else
		{
			m_wActionText.SetExactFontSize(ActionMenuElementContext.s_iTextSizeNonSelected);
			m_wActionText.SetDesiredFontSize(ActionMenuElementContext.s_iTextSizeNonSelected);
						
			m_wContextText.SetExactFontSize(ActionMenuElementContext.s_iTextSizeNonSelected);
			m_wContextText.SetDesiredFontSize(ActionMenuElementContext.s_iTextSizeNonSelected);			
			
			m_wPerformType.SetSize(2, ActionMenuElementContext.s_fHeightNonSelected);
		}		
				
		// Instant & hold actions indicator
		float fDuration = pAction.GetActionDuration();
		
		if (fDuration != 0)
		{
			m_wPerformType.SetColor(ActionMenuColor.ENABLED);
			m_wPerformTypeHold.SetOpacity(1);
		}
		else
		{
			m_wPerformType.SetColor(ActionMenuColor.DISABLED);
			m_wPerformTypeHold.SetOpacity(0);
		}
		
		return true;		
	}

	void Show(float fFadeSpeed, float fTimeSlice = 0)
	{
		if (!m_wElement)
			return;

		float fAlpha;		
		float fTargetAlpha;
		
		if (m_bIsSelected)
			fTargetAlpha = ActionMenuElementContext.s_fAlpha;
		else
			fTargetAlpha = ActionMenuElementContext.s_fAlphaNonSelected;
		
		if (fTimeSlice == 0) 
		{
			fAlpha = fTargetAlpha;
		}
		else
		{
			fAlpha = m_wElement.GetOpacity();
			fAlpha = Math.Lerp(fAlpha, fTargetAlpha, fFadeSpeed * fTimeSlice);	
		}
		
		m_wElement.SetOpacity(fAlpha);
		m_wElement.SetVisible(true);
	}	
	
	void Hide()
	{
		if (!m_wElement)
			return;

		m_wElement.SetOpacity(0);
		m_wElement.SetVisible(false);
	}
	
	void SetProgress(float p)
	{		
		if (!m_wProgress)
			return;
		
		m_wProgress.SetMaskProgress(p);
	}
	
	void ActionMenuElement()
	{
		m_wElement = ActionMenuElementContext.s_wWorkspace.CreateWidgets(ActionMenuElementContext.s_sLayout, ActionMenuElementContext.s_wActionMenu);
		LayoutSlot.SetHorizontalAlign(m_wElement, LayoutHorizontalAlign.Left);
		LayoutSlot.SetPadding(m_wElement, 0, 0, 0, ActionMenuElementContext.s_iSpacing);

		m_wActionText = TextWidget.Cast(m_wElement.FindAnyWidget("ActionText"));
		m_wContextText = TextWidget.Cast(m_wElement.FindAnyWidget("ActionContext"));		
		m_wPerformType = ImageWidget.Cast(m_wElement.FindAnyWidget("ActionType"));
		m_wPerformTypeHold = ImageWidget.Cast(m_wElement.FindAnyWidget("ActionTypeHold"));
		m_wProgress = ImageWidget.Cast(m_wElement.FindAnyWidget("ActionProgress"));
	}
	
	void ~ActionMenuElement()
	{
		if (m_wElement)
			m_wElement.RemoveFromHierarchy();
		
		m_wElement = null;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_ActionMenuInteractionDisplay : SCR_BaseInteractionDisplay
{
	[Attribute("{C80DEB58AA948E59}UI/layouts/HUD/InteractionSystem/ActionMenuElement.layout", UIWidgets.ResourceNamePicker, params: "layout")]
	ResourceName m_sElementLayout;

	[Attribute("0", UIWidgets.CheckBox, "Fixed placement")]
	protected bool m_bFixedPlacement;
		
	[Attribute("10.0", UIWidgets.Slider, params: "0 100 0.1")]
	protected float m_fScrollAnimationSpeed;
	
	[Attribute("1.0", UIWidgets.Slider, params: "0 10 0.05")]
	protected float m_fAutoExpandTime;

	[Attribute("10.0", UIWidgets.Slider, params: "0 100 0.1")]
	protected float m_fFadeInSpeed;	

	[Attribute("0.05", UIWidgets.Slider, "Action list element fade-in expand delay (in seconds).", params: "0 1 0.001")]
	protected float m_fFadeInOffset;	
			
	protected EActionMenu m_eState = EActionMenu.UNDEFINED;

	protected Widget m_wActionMenu;
	protected Widget m_wActionButton;
	protected Widget m_wArrowUp;
	protected Widget m_wArrowDown;
	protected Widget m_wHoldText;
	protected ImageWidget m_wArrowIcon;
	
	protected ref map<BaseUserAction,ActionMenuElement> m_mActionWidget = new ref map<BaseUserAction,ActionMenuElement>;
	protected ref array<ref ActionMenuElement> m_aActionMenuElements = new ref array<ref ActionMenuElement>;	
	
	protected int m_iCurrentScroll = 0;
	protected float m_fCurrentScrollAnimation = 0.0;

	protected float m_fExpandTimer = 0.0;			// Timer for auto-expand feature
	protected float m_fExpandedTimer = 0.0;			// Time elapsed from menu expand, used for sequential fade in of elements

	protected UserActionContext m_pPrevContext = null;
	
	// Data containing info from interaction handler
	protected ref ActionDisplayData m_pLastData;
	
	//~ In order for the continues action process not to instantly go from 100% to 0 it holds the 100% for x seconds. This const is how long it holds (In seconds)
	protected const float CONTINUES_ACTION_RESET_HOLD = 0.25;
	
	//------------------------------------------------------------------------------------------------
	//! Creates and initializes root widget to be slotted with actions
	protected void Create() 
	{
		if (!m_wRoot)
		{
			Print("ActionsMenuDisplay is missing root layout!", LogLevel.ERROR);
			return;
		}
		
		if (!m_bFixedPlacement)
		{
			FrameSlot.SetAnchorMin(m_wRoot, 0, 0);
			FrameSlot.SetAnchorMax(m_wRoot, 0, 0);
			FrameSlot.SetAlignment(m_wRoot, 0, 0);		
		}
		else
		{
			FrameSlot.SetAnchorMin(m_wRoot, 0.48, 0.94);
			FrameSlot.SetAnchorMax(m_wRoot, 0.48, 0.94);
			FrameSlot.SetAlignment(m_wRoot, 0, 0);		
		}

		m_wActionMenu = m_wRoot.FindAnyWidget("ActionMenu");
		m_wActionButton = m_wRoot.FindAnyWidget("ActionButton");
		m_wArrowUp = m_wRoot.FindAnyWidget("ArrowUp");
		m_wArrowDown = m_wRoot.FindAnyWidget("ArrowDown");
		m_wHoldText = m_wRoot.FindAnyWidget("HoldText");
		m_wArrowIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("DirectionArrow"));

		if (!m_wActionMenu)
		{
			// TODO: Log error?
			return;
		}			
		
		// Store constant data in structure	for ActionMenuElement(s) to access
		ActionMenuElementContext.s_wWorkspace = m_wRoot.GetWorkspace();
		ActionMenuElementContext.s_wActionMenu = m_wActionMenu;
		ActionMenuElementContext.s_sLayout = m_sElementLayout;
		ActionMenuElementContext.s_fFadeInSpeed = m_fFadeInSpeed;		
		
		// Initialize action menu state and data
		Collapse();
	}	

	//------------------------------------------------------------------------------------------------
	protected void Destroy()
	{
		if (m_aActionMenuElements)
			m_aActionMenuElements.Clear();
	}	
	
	//------------------------------------------------------------------------------------------------
	protected bool Update(ActionsTuple pActionsData, BaseUserAction pSelectedAction, UserActionContext pCurrentContext, float fTimeSlice)
	{
		if (!pCurrentContext)
			return false;	

		// Detect context change
		if (pCurrentContext != m_pPrevContext)
		{
			m_pPrevContext = pCurrentContext;
			Collapse();
		}				
		
		int iActionsCount = pActionsData.param1.Count();
		
		if (!pActionsData.param1 || iActionsCount == 0)
			return false;
		
		if (iActionsCount > 1)
		{
			RunExpandTimer(fTimeSlice);
			RunExpandedTimer(fTimeSlice);
		}
		else
			Collapse();
		
		int iActiveWidgetsCount = m_aActionMenuElements.Count();
		int iActionsDelta = iActiveWidgetsCount - iActionsCount;
		int iAbsDelta = Math.AbsInt(iActionsDelta);

		// Add or remove widgets
		if (iActionsDelta < 0)
		{			
			// Elements missing
			for (int i = 0; i < iAbsDelta; i++)
			{
				ref ActionMenuElement pActionMenuElement = new ActionMenuElement();
				m_aActionMenuElements.Insert(pActionMenuElement);
			}
		}
		else if (iActionsDelta > 0)
		{
			// Elements over
			for (int i = iActiveWidgetsCount-1; i >= iActiveWidgetsCount-iAbsDelta; i--)
			{
				ActionMenuElement pActionMenuElement = m_aActionMenuElements[i];	
				pActionMenuElement.Hide();
			}
		}

		int iCurrentAction = -1;
		for (int i = 0; i < pActionsData.param1.Count(); i++)
		{
			if (pActionsData.param1[i] == pSelectedAction)
			{
				iCurrentAction = i;
				break;
			}
		}
		
		if (iCurrentAction == -1)
			return false;		

		// Expand menu upon interaction
		if (iCurrentAction > 0)
			Expand();		
				
		// Clear action map before it is re-filled by actual data
		m_mActionWidget.Clear();
		
		// Set widget data
		for (int i = 0; i < iActionsCount; i++)
		{
			BaseUserAction pAction = pActionsData.param1[i];
			ActionMenuElement pActionMenuElement = m_aActionMenuElements[i];

			// Setup menu element 
			bool succeeded = pActionMenuElement.Set(pAction, i, pAction == pSelectedAction, pActionsData.param2[i]);
			
			if (!succeeded)
			{
				pActionMenuElement.Hide();
				continue;
			}
			
			// Add new entry to action map
			m_mActionWidget.Insert(pAction, pActionMenuElement);
			
			// Set element visibility based on menu state			
			if ((i == 0 || m_eState == EActionMenu.EXPANDED) && (m_fExpandedTimer >= i * m_fFadeInOffset))
				pActionMenuElement.Show(m_fFadeInSpeed, fTimeSlice);
			else
				pActionMenuElement.Hide();
			
			// Set enabled/disabled based on whether action can be performed
			pActionMenuElement.SetEnabled(pActionsData.param2[i]);
		}
		
		// Update visibility of the HOLD button prefix
		if (pSelectedAction.GetActionDuration() != 0)
			m_wHoldText.SetVisible(true);
		else
			m_wHoldText.SetVisible(false);
		
		// Toggle selection / adjustment widgets
		bool isInProgress = pSelectedAction && pSelectedAction.IsInProgress();
		if (isInProgress && SCR_AdjustSignalAction.Cast(pSelectedAction))
		{
			UpdateArrow(m_wArrowUp, EActionMenuScroll.ENABLED);
			UpdateArrow(m_wArrowDown, EActionMenuScroll.ENABLED);
		}
		else if (isInProgress || iActionsCount <= 1)
		{
			UpdateArrow(m_wArrowUp, EActionMenuScroll.HIDDEN);
			UpdateArrow(m_wArrowDown, EActionMenuScroll.HIDDEN);
		}
		else
		{
			if (pSelectedAction == pActionsData.param1[0])
				UpdateArrow(m_wArrowUp, EActionMenuScroll.DISABLED);
			else
				UpdateArrow(m_wArrowUp, EActionMenuScroll.ENABLED);
			
			if (pSelectedAction == pActionsData.param1[iActionsCount-1])
				UpdateArrow(m_wArrowDown, EActionMenuScroll.DISABLED);
			else
				UpdateArrow(m_wArrowDown, EActionMenuScroll.ENABLED);
		}
		
		// Set and animate scroll		
		SetScroll(iCurrentAction, fTimeSlice);
		
		return true;	
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetSafeZone(vector worldPosition, vector screenPosition, vector min01, vector max01, out vector clampedPosition)
	{
		WorkspaceWidget workspace = m_wActionMenu.GetWorkspace();
		int screenw = workspace.DPIUnscale(workspace.GetWidth());
		int screenh = workspace.DPIUnscale(workspace.GetHeight());
		
		
		float padding = 15.0;
		
		vector min = Vector(
			screenw * min01[0] + padding,
			screenh * min01[1] + padding,
			0.0);
		
		float menuw, menuh;
		m_wActionMenu.GetScreenSize(menuw, menuh);
		menuw = workspace.DPIUnscale(menuw);
		menuh = workspace.DPIUnscale(menuh);
		
		// Maxs are offset by the menu width, so the menu never overflows
		vector max = Vector(
			screenw * max01[0] - menuw - padding,
			screenh * max01[1] - menuh + padding,
			0.0);
		
		
		bool clamped = false;
		clampedPosition = screenPosition;
		
		// Bold assumption that can be done is that if this display is shown,
		// the main camera is used for collecting the actions
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase currentCamera = cameraManager.CurrentCamera();
			if (currentCamera)
			{
				vector camTM[4];
				currentCamera.GetWorldCameraTransform(camTM);
				vector toActionDir = (worldPosition - camTM[3]).Normalized();	
				float d = vector.Dot(toActionDir, camTM[2]);
				const float threshold = 0.5;
				const float invThreshold = 1.0 / 0.5;
				// Action is "behind view"
				if (d < threshold)
				{
					float stickiness = 1.0;
					// Use this to blend towards "full backwards" smoothly
					if (d > 0.0)
						stickiness = Math.Clamp(1.0 - (d * invThreshold), 0.0, 1.0);
					
					clampedPosition[1] = Math.Lerp(clampedPosition[1], max[1], stickiness);
					clamped = true;
				}
			}
		}
		
		
		for (int i = 0; i < 2; ++i)
		{
			if (clampedPosition[i] < min[i]) 
			{
				clampedPosition[i] = min[i];
				clamped = true;
			}
			else if (clampedPosition[i] > max[i])
			{
				clampedPosition[i] = max[i];
				clamped = true;
			}
		}
		
		return clamped;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPosition(vector vWorldPosition)
	{
		if (!m_wRoot)
			return;

		if (!m_bFixedPlacement)
		{
			vector vScreenPosition = GetGame().GetWorkspace().ProjWorldToScreen(vWorldPosition, GetGame().GetWorld());
			vector vClampedPosition;
			if (GetSafeZone(vWorldPosition, vScreenPosition, "0.15 0.15 0", "0.85 0.85 0", vClampedPosition))
			{
				
				vector dir = (vScreenPosition - vClampedPosition).Normalized();
				
				float rads = 0;
				float d = vector.Dot(dir, vector.Right);
				if (vector.Dot(dir, vector.Up) > 0.0)
					rads = Math.Acos(d);
				else
					rads = -Math.Acos(d);
				
				const float size = 25.0;
				vector point = size * dir;
				FrameSlot.SetPos(m_wArrowIcon, 3.0 + point[0], point[1]);
				m_wArrowIcon.SetRotation(rads * Math.RAD2DEG);
				m_wArrowIcon.SetVisible(true);
			}
			else
			{
				m_wArrowIcon.SetVisible(false);
			}
						
			FrameSlot.SetPos(m_wRoot, vClampedPosition[0], vClampedPosition[1]);
			
			// If action menu is drawn over picture in picture, fade out its opacity
			// so it is not as intrusive and so it promotes the idea of it being "behind" the sights
			SCR_2DPIPSightsComponent pipSights = ArmaReforgerScripted.GetCurrentPIPSights();
			bool isPointInPIP = pipSights && pipSights.IsScreenPositionInSights(vClampedPosition);
			
			float alpha = 1.0;
			if (isPointInPIP)
				alpha = 0.75;
			
			m_wActionButton.SetOpacity(alpha);
			m_wActionMenu.SetOpacity(alpha);
		}
	}
	
	protected void SetScreenPosition(vector screenPosition)
	{
		if (!m_wRoot)
			return;
		
		FrameSlot.SetPos(m_wRoot, screenPosition[0], screenPosition[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetScroll(int iCurrentScroll, float fTimeSlice = 0)
	{
		if (!m_wActionMenu)
			return;

		float fCurrentScroll = iCurrentScroll;

		// Reset action list to initial position
		if (fTimeSlice == 0)
		{
			m_fCurrentScrollAnimation = 0.0;
			
			float fVerticalPos = - 0.5 * ActionMenuElementContext.s_fHeight;
			
			FrameSlot.SetPosY(m_wActionMenu, fVerticalPos);
		}
		// Scroll the action list towards the target position (iCurrentScroll)
		else if (Math.AbsFloat(m_fCurrentScrollAnimation - fCurrentScroll) > 0.001)
		{
			m_fCurrentScrollAnimation = Math.Lerp(m_fCurrentScrollAnimation, fCurrentScroll, m_fScrollAnimationSpeed * fTimeSlice);		

			float fVerticalPos = - 0.5 * ActionMenuElementContext.s_fHeight;
			fVerticalPos -= m_fCurrentScrollAnimation * (ActionMenuElementContext.s_fHeightNonSelected + ActionMenuElementContext.s_iSpacing);

			FrameSlot.SetPosY(m_wActionMenu, fVerticalPos);
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdateArrow(Widget w, EActionMenuScroll state)
	{
		if (!w) 
			return;
		
		if (state == EActionMenuScroll.HIDDEN)
		{
			w.SetVisible(false);
		}
		else
		{
			w.SetVisible(true);
			
			if (state == EActionMenuScroll.ENABLED)
				w.SetColor(ActionMenuColor.ENABLED);
			else			
				w.SetColor(ActionMenuColor.DISABLED);
		}
	}		

	//------------------------------------------------------------------------------------------------
	void RunExpandTimer(float fTimeSlice)
	{
		if (m_eState != EActionMenu.COLLAPSED)
		{
			m_fExpandTimer = 0.0;
			return;
		}
		
		// Expansion of additional actions
		m_fExpandTimer += fTimeSlice;

		if (m_fExpandTimer > m_fAutoExpandTime)
			Expand();
	}	

	//------------------------------------------------------------------------------------------------
	void RunExpandedTimer(float fTimeSlice)
	{
		if (m_eState != EActionMenu.EXPANDED)
		{
			m_fExpandedTimer = 0.0;
			return;
		}
		
		// Expansion of additional actions
		m_fExpandedTimer += fTimeSlice;
	}		
	
	//------------------------------------------------------------------------------------------------
	void Expand()
	{
		if (m_eState == EActionMenu.EXPANDED)
			return;
		
		m_eState = EActionMenu.EXPANDED;
	}	

	//------------------------------------------------------------------------------------------------
	void Collapse()
	{
		m_eState = EActionMenu.COLLAPSED;
		
		SetScroll(0);
	}		
		
	#ifndef DISABLE_INTERACTIONS	
	//------------------------------------------------------------------------------------------------
	//! Called when action starts being performed
	override void OnActionStart(IEntity pUser, BaseUserAction pPerformedAction)
	{
		ActionMenuElement pActionMenuElement = m_mActionWidget.Get(pPerformedAction);
		
		if (!pActionMenuElement)
			return;

		pActionMenuElement.SetProgress(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when action is being performed
	override void OnActionProgress(IEntity pUser, BaseUserAction pPerformedAction, float fProgress, float fDuration)
	{
		 ActionMenuElement pActionMenuElement = m_mActionWidget.Get(pPerformedAction);
        if (!pActionMenuElement)
            return;
        if (fDuration == 0.0)
            pActionMenuElement.SetProgress(0.0);
        //~ Continues action
        else
            pActionMenuElement.SetProgress(fProgress / fDuration);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when an action is finished
	override void OnActionFinish(IEntity pUser, BaseUserAction pFinishedAction, ActionFinishReason eFinishReason)
	{
		ActionMenuElement pActionMenuElement = m_mActionWidget.Get(pFinishedAction);
		
		if (!pActionMenuElement)
			return;

		pActionMenuElement.SetProgress(0);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// Need data to update
		if (!m_pLastData || !m_pLastData.pSelectedAction) 
		{
			if (m_bShown)
				Show(false);
			
			return;
		}

		// Update ui
		bool succeeded = Update(m_pLastData.pActionsData, m_pLastData.pSelectedAction, m_pLastData.pCurrentContext, timeSlice);
		
		// Toggle visibility
		if (!succeeded)
		{
			if (m_bShown)
				Show(false);

			return;
		}
		
		if (!m_bShown)
			Show(true);
		
		// Update current widget context position
		UserActionContext lastContext = m_pLastData.pCurrentContext;
		if (lastContext)
		{
			vector worldPosition = lastContext.GetOrigin();
			SetPosition(worldPosition);
		}
		
		// Clear last data
		m_pLastData = null;
		
		// Be careful: pUser can be passed in as NULL if player has no controlled entity!
		// Can be used to progress animations
		// Will be called every frame from the interaction handler as long as we have a valid player controller
		
		
		// PlayerController pc = PlayerController.Cast(owner);
		// if (pc)
		// auto user = pc.GetControlledEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		Create();
		
		Show(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		Destroy();
		m_pLastData = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when display was hidden and is supposed to be shown. Called before first ShowActionsDisplay call
	override void ShowDisplay() 
	{
		super.ShowDisplay();
		
		Show(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when display was open and is supposed to hide. Called after last ShowActionsDisplay call
	override void HideDisplay()
	{
		m_pLastData = null;
		
		Show(false);
		
		super.HideDisplay();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Data provided by the interaction handler.
	override void SetDisplayData(ActionDisplayData data)
	{
		m_pLastData = data;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ActionMenuInteractionDisplay()
	{
		Destroy();
	}
	#endif
};
