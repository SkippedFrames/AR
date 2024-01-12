/*!
This component is meant for cases when content (expecially text) is too long horisontally and doesn't fit into UI.
The component animates content left-right when activated.

!!! WARNING !!!
The content widget (the one which will be animated) must be in a Frame Slot!
In most basic case you should create a Frame, attach the component to it, and and put a text widget inside the Frame.
*/

class SCR_HorizontalScrollAnimationComponent : ScriptedWidgetComponent
{
	[Attribute("Content", UIWidgets.EditBox, "Name of widget which will be animated")]
	string m_sAnimationContentWidget;
	
	[Attribute("10", UIWidgets.EditBox, "Animation speed")]
	protected float m_fAnimationSpeedPxPerSec;
	
	[Attribute("0.5", UIWidgets.EditBox, "Wait time while animating when content is on the left")]
	protected float m_fWaitTimeLeftSec;
	
	[Attribute("0.5", UIWidgets.EditBox, "Wait time while animating when content is on the right")]
	protected float m_fWaitTimeRightSec;
	
	[Attribute("false", UIWidgets.CheckBox, "When true, content will animate to start after it has animated left. Otherwise it will jump to start instantly.")]
	protected bool m_bAnimateBack;
	
	[Attribute("0", UIWidgets.CheckBox, "Inwards offset from right edge of root widget to be used in calculations")]
	protected float m_fOffsetRight;
	
	[Attribute("0", UIWidgets.CheckBox, "Inwards offset from left edge of root widget to be used in calculations")]
	protected float m_fOffsetLeft;
	
	protected int m_State;
	protected Widget m_wContent;
	protected Widget m_wRoot;
	protected float m_fStartPosX;
	protected float m_fTimer;
	
	// Internal state
	protected static const int STATE_NONE = 0;						// Nothing
	protected static const int STATE_MOVING_LEFT = 1;				// Moving left
	protected static const int STATE_MOVING_LEFT_DONE_WAITING = 2;	// Finished moving left, waiting
	protected static const int STATE_MOVING_RIGHT = 3;				// Moving right
	protected static const int STATE_MOVING_RIGHT_DONE_WAITING = 4;	// Finished moving right, waiting
	
	
	//------------------------------------------------------------------------
	// P U B L I C
	
	//------------------------------------------------------------------------
	void AnimationStop()
	{
		m_State = STATE_NONE;
	}
	
	//------------------------------------------------------------------------
	void AnimationStart()
	{
		// If not animating, start animation
		if (m_State == STATE_NONE)
			m_State = STATE_MOVING_LEFT;
		
		// Otherwise it's already animating in some state, do nothing
	}
	
	//------------------------------------------------------------------------
	//! Sets X position to its initial value when component was initialized
	void ResetPosition()
	{
		if (!m_wContent)
			return;
		
		FrameSlot.SetPosX(m_wContent, m_fStartPosX);
	}
	
	//------------------------------------------------------------------------
	//! Returns true when content fits the parent widget
	bool GetContentFit()
	{
		if (!m_wContent)
			return true;
		
		float rootw, rooth;
		m_wRoot.GetScreenSize(rootw, rooth);
				
		float contentw, contenth;
		m_wContent.GetScreenSize(contentw, contenth);
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		return rootw > contentw + workspace.DPIScale(m_fOffsetLeft + m_fOffsetRight);
	}
	
	
	
	
	//------------------------------------------------------------------------
	// P R O T E C T E D
	
	//------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_wRoot = w;
		m_wContent = w.FindAnyWidget(m_sAnimationContentWidget);
		
		if (!m_wContent)
		{
			return;
			Print(string.Format("[SCR_HorizontalScrollAnimationComponent] Widget %1 was not found", m_sAnimationContentWidget), LogLevel.ERROR);
		}
		
		if (m_wContent)
		{
			m_fStartPosX = FrameSlot.GetPosX(m_wContent);
		}		

		m_State = STATE_MOVING_LEFT; // STATE_NONE;
		auto callQueue = GetGame().GetCallqueue();
		callQueue.CallLater(OnEachFrame, 0, true);
	}
	
	//------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		GetGame().GetCallqueue().Remove(OnEachFrame);
	}
	
	//------------------------------------------------------------------------
	protected void OnEachFrame()
	{
		float tDelta = ftime / 1000.0;
		
		switch (m_State)
		{
			case STATE_NONE:
				// Doing nothing
				break;
			
			case STATE_MOVING_LEFT:
			{
				// Moving left
				AnimatePosition(tDelta, -m_fAnimationSpeedPxPerSec);
				
				WorkspaceWidget workspace = GetGame().GetWorkspace();
				float rootw, rooth, rootx, rooty;
				float contentw, contenth, contentx, contenty;
				m_wRoot.GetScreenSize(rootw, rooth);
				m_wRoot.GetScreenPos(rootx, rooty);				
				m_wContent.GetScreenSize(contentw, contenth);
				m_wContent.GetScreenPos(contentx, contenty);
				
				if (contentx + contentw < rootx + rootw - workspace.DPIScale(m_fOffsetRight))
				{
					m_State = STATE_MOVING_LEFT_DONE_WAITING;
					m_fTimer = m_fWaitTimeLeftSec;
				}
				
				break;
			}
			
			case STATE_MOVING_LEFT_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					if (m_bAnimateBack)
					{
						m_State = STATE_MOVING_RIGHT;
					}
					else
					{
						ResetPosition();
						m_fTimer = m_fWaitTimeRightSec;
						m_State = STATE_MOVING_RIGHT_DONE_WAITING;
					}
				}
				break;
			}
			
			case STATE_MOVING_RIGHT:
			{
				// Moving right
				AnimatePosition(tDelta, m_fAnimationSpeedPxPerSec);
				
				WorkspaceWidget workspace = GetGame().GetWorkspace();
				float rootx, rooty;
				float contentx, contenty;
				m_wRoot.GetScreenPos(rootx, rooty);
				m_wContent.GetScreenPos(contentx, contenty);
				
				if (contentx > rootx + workspace.DPIScale(m_fOffsetLeft))
				{
					m_State = STATE_MOVING_RIGHT_DONE_WAITING;
					m_fTimer = m_fWaitTimeRightSec;
				}
				
				break;
			}
			
			case STATE_MOVING_RIGHT_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					m_State = STATE_MOVING_LEFT;
				}
			}
		}
	}
	
	
	//------------------------------------------------------------------------
	protected void AnimatePosition(float tDelta, float speed)
	{
		float currentPos = FrameSlot.GetPosX(m_wContent);
		float newPos = currentPos + tDelta * speed;
		FrameSlot.SetPosX(m_wContent, newPos);
	}
};
