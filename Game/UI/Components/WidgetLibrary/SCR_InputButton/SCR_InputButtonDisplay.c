class SCR_InputButtonDisplay : ScriptedWidgetComponent
{
	//Key data
	protected BaseContainer m_KeyData;
	protected string m_sButtonText;
	protected ref array<ref SCR_ButtonTexture> m_aButtonTextures = {};
	protected SCR_EButtonSize m_eButtonType;
	protected ResourceName m_sButtonLayoutConfig;
	protected SCR_InputButtonStyle m_ButtonLayout;

	protected ref array<ImageWidget> m_aAdditionalWidgets = {};

	protected int m_iWidth;
	protected float m_fMediumButtonWidhtModifier = 1.5;
	protected float m_fLargeButtonWidhtModifier = 1.78;
	protected float m_fButtonTextSizeModifier = 3;

	float m_fAnimationRate;
	float m_fAnimationTime;
	protected float m_fMaxHoldtime;
	protected float m_fDoubleTapTime;

	protected bool m_bIsHoldAction;
	protected bool m_bIsDoubleTapAction;
	protected bool m_bPressedInput;
	protected bool m_bCanBeToggled;
	protected bool m_bIsToggled;
	protected bool m_bIsOverwritten;
	
	protected SCR_InputButtonComponent m_InputButtonComp;
	protected WidgetAnimationAlphaMask m_HoldingAnimation;

	protected Widget m_wRoot;
	protected Widget m_wParent;

	protected OverlayWidget m_wOverlay;
	protected ImageWidget m_wGlow;
	protected ImageWidget m_wOutlineClear;
	protected ImageWidget m_wOutline;
	protected ImageWidget m_wKeyBG;
	protected ImageWidget m_wDoubleTabIndicator;
	protected ImageWidget m_wButtonImgGlow;
	protected ImageWidget m_wButtonImg;
	protected RichTextWidget m_wButtonText;

	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wRoot = w;

		m_wOverlay = OverlayWidget.Cast(m_wRoot.FindAnyWidget("m_Overlay"));
		m_wGlow = ImageWidget.Cast(m_wRoot.FindAnyWidget("m_Glow"));
		m_wOutlineClear = ImageWidget.Cast(m_wRoot.FindAnyWidget("m_OutlineClear"));
		m_wOutline = ImageWidget.Cast(m_wRoot.FindAnyWidget("m_Outline"));
		m_wKeyBG = ImageWidget.Cast(m_wRoot.FindAnyWidget("m_KeyBG"));
		m_wDoubleTabIndicator = ImageWidget.Cast(m_wRoot.FindAnyWidget("m_DoubleTabIndicator"));
		m_wButtonImgGlow = ImageWidget.Cast(m_wRoot.FindAnyWidget("m_ButtonImgGlow"));
		m_wButtonImg = ImageWidget.Cast(m_wRoot.FindAnyWidget("m_ButtonImg"));
		m_wButtonText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("m_ButtonText"));
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Initialize Widget and subscribe to needed ScriptInvokers
	\param Parent widget
	!*/
	void Init(Widget parent)
	{
		m_wParent = parent;
		if (!m_wParent)
			return;

		m_InputButtonComp = SCR_InputButtonComponent.Cast(m_wParent.FindHandler(SCR_InputButtonComponent));
		if (!m_InputButtonComp)
			return;

		m_fAnimationRate = m_InputButtonComp.GetAnimationRate();
		m_fAnimationTime = m_InputButtonComp.GetAnimationTime();

		UpdateEnableColor();

		m_InputButtonComp.GetOnAnimateHover().Insert(AnimateHover);
		m_InputButtonComp.GetOnUpdateEnableColor().Insert(UpdateEnableColor);
		m_InputButtonComp.GetOnHoldAnimComplete().Insert(AnimateClick);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get all visual informations about the button and apply them
	!*/
	bool SetAction(BaseContainer data, BaseContainer filter)
	{
		GetFilter(filter);

		data.Get("m_sText", m_sButtonText);
		data.Get("m_aTextures", m_aButtonTextures);
		data.Get("m_eType", m_eButtonType);
		
		GameProject.GetModuleConfig("ChimeraGlobalConfig").Get("InputButtonLayoutConfig", m_sButtonLayoutConfig);
		
		SCR_InputButtonLayoutConfig m_cInput = SCR_InputButtonLayoutConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(BaseContainerTools.LoadContainer(m_sButtonLayoutConfig).GetResource().ToBaseContainer()));

		m_ButtonLayout = m_cInput.GetButtonSize(m_eButtonType);

		if (!m_ButtonLayout || m_bIsOverwritten)
			return false;

		//! Lets apply all gathered textures from the config to the correct ImageWidgets
		ApplyTextureToButton();

		m_wOutline.SetVisible(m_bIsHoldAction);
		m_wOutlineClear.SetVisible(m_bIsHoldAction);
		m_wDoubleTabIndicator.SetVisible(m_bIsDoubleTapAction);

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Overrides Texture of the Button
	//! USE SCR_InputButtonComponent.SetTexture() to call
	void OverrideTexture(string imagePath, string image = string.Empty, Color color = Color.White)
	{
		m_bIsOverwritten = true;
		
		GameProject.GetModuleConfig("ChimeraGlobalConfig").Get("InputButtonLayoutConfig", m_sButtonLayoutConfig);
		
		SCR_InputButtonLayoutConfig m_cInput = SCR_InputButtonLayoutConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(BaseContainerTools.LoadContainer(m_sButtonLayoutConfig).GetResource().ToBaseContainer()));

		m_eButtonType = SCR_EButtonSize.KEYBOARD_MEDIUM;
		m_ButtonLayout = m_cInput.GetButtonSize(m_eButtonType);

		if (!m_ButtonLayout)
			return;
		
		DeleteAdditionalWidgets();
		
		m_wKeyBG.LoadImageFromSet(0, m_ButtonLayout.m_sImageSet, m_ButtonLayout.m_sKeyBackground);
		m_wOutline.LoadImageFromSet(0, m_ButtonLayout.m_sImageSet, m_ButtonLayout.m_sKeyOutline);
		m_wOutlineClear.LoadImageFromSet(0, m_ButtonLayout.m_sImageSet, m_ButtonLayout.m_sKeyOutline);
		m_wGlow.LoadImageFromSet(0, m_ButtonLayout.m_sGlowImageSet, m_ButtonLayout.m_sGlow);
		
		GetWidth();
		
		m_wKeyBG.SetSize(m_iWidth, m_InputButtonComp.m_iHeightInPixel);
		m_wOutline.SetSize(m_iWidth, m_InputButtonComp.m_iHeightInPixel);
		m_wOutlineClear.SetSize(m_iWidth, m_InputButtonComp.m_iHeightInPixel);
		m_wGlow.SetSize(m_iWidth, m_InputButtonComp.m_iHeightInPixel);
		
		m_wButtonText.SetVisible(false);
		
		if (image)
			m_wButtonImg.LoadImageFromSet(0, imagePath, image);
		else
			m_wButtonImg.LoadImageTexture(0, image);
		
		m_wButtonImg.SetColor(color);		
		m_wButtonImg.SetVisible(true);
		m_wButtonImg.SetSize(m_InputButtonComp.m_iHeightInPixel * 0.5, m_InputButtonComp.m_iHeightInPixel * 0.5);
	}

	//------------------------------------------------------------------------------------------------
	protected void GetFilter(BaseContainer filter)
	{
		if (!filter)
		{
			m_bIsHoldAction = false;
			return;
		}

		switch (filter.GetClassName())
		{
			case "InputFilterHold":
				m_bIsHoldAction = true;
				break;
			case "InputFilterHoldOnce":
				m_bIsHoldAction = true;
				break;
			case "InputFilterDoubleClick":
				m_bIsDoubleTapAction = true;
				break;
			case "InputFilterPressed":
				m_bPressedInput = true;
				break;
			case "InputFilterToggle":
				m_bCanBeToggled = true;
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Apply textures from set SCR_InputButtonLayout config to ImageWidgets
	//! Set the correct size for every widget by using set Height
	protected bool ApplyTextureToButton()
	{
		m_wKeyBG.LoadImageFromSet(0, m_ButtonLayout.m_sImageSet, m_ButtonLayout.m_sKeyBackground);
		m_wOutline.LoadImageFromSet(0, m_ButtonLayout.m_sImageSet, m_ButtonLayout.m_sKeyOutline);
		m_wOutlineClear.LoadImageFromSet(0, m_ButtonLayout.m_sImageSet, m_ButtonLayout.m_sKeyOutline);

		if (m_bIsHoldAction)
		{
			m_wGlow.LoadImageFromSet(0, m_ButtonLayout.m_sGlowImageSet, m_ButtonLayout.m_sGlowOutline);
			m_wOutline.LoadMaskFromSet(m_ButtonLayout.m_sImageSet, m_ButtonLayout.m_sAlphaMask);
		}
		else
		{
			m_wGlow.LoadImageFromSet(0, m_ButtonLayout.m_sGlowImageSet, m_ButtonLayout.m_sGlow);
		}

		m_wDoubleTabIndicator.LoadImageFromSet(0, m_ButtonLayout.m_sImageSet, m_ButtonLayout.m_sDoubleTabText);

		GetWidth();

		m_wKeyBG.SetSize(m_iWidth, m_InputButtonComp.m_iHeightInPixel);
		m_wOutline.SetSize(m_iWidth, m_InputButtonComp.m_iHeightInPixel);
		m_wOutlineClear.SetSize(m_iWidth, m_InputButtonComp.m_iHeightInPixel);
		m_wGlow.SetSize(m_iWidth, m_InputButtonComp.m_iHeightInPixel);
		m_wDoubleTabIndicator.SetSize(m_InputButtonComp.m_iHeightInPixel, m_InputButtonComp.m_iHeightInPixel);

		//! Checks if button has a Image defined in config otherwise it uses the text//
		if (m_aButtonTextures && !m_aButtonTextures.IsEmpty())
		{
			m_wButtonImg.LoadImageFromSet(0, m_ButtonLayout.m_sImageSet, m_aButtonTextures[0].m_sTexture);
			m_wButtonImg.SetColor(m_aButtonTextures[0].m_Color);
			m_wButtonImg.SetVisible(true);

			if (m_aButtonTextures[0].m_bHasShadow)
			{
				m_wButtonImgGlow.LoadImageFromSet(0, m_ButtonLayout.m_sGlowImageSet, m_aButtonTextures[0].m_sTexture);
				m_wButtonImgGlow.SetVisible(true);
			}
			else
			{
				m_wButtonImgGlow.SetVisible(false);
			}

			m_wButtonImg.SetSize(m_InputButtonComp.m_iHeightInPixel, m_InputButtonComp.m_iHeightInPixel);
			m_wButtonImgGlow.SetSize(m_InputButtonComp.m_iHeightInPixel, m_InputButtonComp.m_iHeightInPixel);
			//! If the button has more than 1 image assigned, we create as many ImageWidgets as we need
			if (m_aButtonTextures.Count() > 1)
				CreateAdditionalWidgets();
			else if (!m_aAdditionalWidgets.IsEmpty())
				DeleteAdditionalWidgets();
		}
		else
		{
			m_wButtonText.SetVisible(true);
			m_wButtonImg.SetVisible(false);
			m_wButtonImgGlow.SetVisible(false);
			//! Lets just clean up the Images if we dont need them anymore
			if (!m_aAdditionalWidgets.IsEmpty())
				DeleteAdditionalWidgets();
		}

		m_wButtonText.SetText(m_sButtonText);
		m_wButtonText.SetDesiredFontSize((int)m_InputButtonComp.m_iHeightInPixel / m_fButtonTextSizeModifier);
		
		m_wOutlineClear.SetColor(m_ButtonLayout.m_OutlineColor);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void GetWidth()
	{
		//! Apply correct size to all images
		int sx, sy;
		switch (m_eButtonType)
		{
			case SCR_EButtonSize.KEYBOARD_MEDIUM:
				m_iWidth = m_InputButtonComp.m_iHeightInPixel * m_fMediumButtonWidhtModifier;
			break;
			case SCR_EButtonSize.KEYBOARD_BIG:
				m_iWidth = m_InputButtonComp.m_iHeightInPixel * m_fLargeButtonWidhtModifier;
			break;
			//Square keyboard button + Gamepad / Mouse buttons
			default:
				m_iWidth = m_InputButtonComp.m_iHeightInPixel;
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create temporary ImageWidgets if the Button has more than one texture applied in the config.
	//! For each entry in the "inputButton.m_aTextures" array a new ImageWidget will be created to save a lot of space in the widget.
	protected bool CreateAdditionalWidgets()
	{
		for (int i = 1, count = m_aButtonTextures.Count(); i < count; i++)
		{
			//! Is the texture has a glow/shadow create that first so it's behind the actual texture
			if (m_aButtonTextures[i].m_bHasShadow)
			{
				Widget w_additionalShadow = GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.BLEND | WidgetFlags.STRETCH | WidgetFlags.IGNORE_CURSOR | WidgetFlags.INHERIT_CLIPPING, new Color(0.000000, 0.000000, 0.000000, 0.502007), 0, m_wOverlay);
				ImageWidget additionalWidgetshadow = ImageWidget.Cast(w_additionalShadow);
				additionalWidgetshadow.LoadImageFromSet(0, m_ButtonLayout.m_sGlowImageSet, m_aButtonTextures[i].m_sTexture);
				additionalWidgetshadow.SetSize(m_InputButtonComp.m_iHeightInPixel, m_InputButtonComp.m_iHeightInPixel);
				m_aAdditionalWidgets.Insert(additionalWidgetshadow);
			}

			Widget w_additional = GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.BLEND | WidgetFlags.STRETCH | WidgetFlags.IGNORE_CURSOR | WidgetFlags.INHERIT_CLIPPING, m_aButtonTextures[i].m_Color, 0, m_wOverlay);
			ImageWidget additionalWidget = ImageWidget.Cast(w_additional);
			additionalWidget.LoadImageFromSet(0, m_ButtonLayout.m_sImageSet, m_aButtonTextures[i].m_sTexture);
			additionalWidget.SetSize(m_InputButtonComp.m_iHeightInPixel, m_InputButtonComp.m_iHeightInPixel);
			m_aAdditionalWidgets.Insert(additionalWidget);
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Go through all previosly created Widgets and delete them, to keep the widget clean.
	protected bool DeleteAdditionalWidgets()
	{
		foreach (ImageWidget img : m_aAdditionalWidgets)
		{
			if (img)
				img.RemoveFromHierarchy();
		}

		m_aAdditionalWidgets.Clear();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void ActionPressed()
	{
		if (m_bIsHoldAction)
		{
			AnimateHold();
			return;
		}

		if (m_bCanBeToggled)
		{
			SetToggled(!m_bIsToggled);
			return;
		}

		if (m_bIsDoubleTapAction)
		{
			OnDoubleTap();
			return;
		}

		AnimateClick();
	}

	//------------------------------------------------------------------------------------------------
	void ActionReleased()
	{
		if (m_bIsHoldAction)
		{
			if (m_HoldingAnimation && m_HoldingAnimation.GetValue() < 1)
				m_HoldingAnimation.SetSpeed(-1);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetToggled(bool toggled, bool animate = true, bool invokeChange = true)
	{
		if (!m_bCanBeToggled)
			return;

		m_bIsToggled = toggled;
		m_InputButtonComp.PlaySoundClicked();

		AnimateClick();

		if (m_bIsToggled)
			AnimateToggle();
		else
			ResetColor();
	}

	//------------------------------------------------------------------------------------------------
	void OnDoubleTap()
	{
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp replicationTime = world.GetServerTimestamp();

		if (!m_InputButtonComp.m_bIsDoubleTapStated)
		{
			AnimateDoubleTab();
			m_fDoubleTapTime = replicationTime.DiffMilliseconds(replicationTime);
			!m_InputButtonComp.m_bIsDoubleTapStated = true;
		}
		else		
		{
			if (replicationTime.DiffMilliseconds(replicationTime) - m_fDoubleTapTime <= m_InputButtonComp.m_iDoubleTapThreshold)
			{
				AnimateDoubleTab();
			}
			else
			{
				ResetColor();
			}

			!m_InputButtonComp.m_bIsDoubleTapStated = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	ImageWidget GetOutlineWidget()
	{
		return m_wOutline;
	}

	//------------------------------------------------------------------------------------------------
	ImageWidget GetBackgroundWidget()
	{
		return m_wKeyBG;
	}

	//------------------------------------------------------------------------------------------------
	//! Animate Widget when mouse hovers over it
	protected void AnimateHover()
	{
		if (AnimateWidget.IsAnimating(m_wKeyBG))
			return;
		
		if (m_InputButtonComp.m_bIsHovered)
		{
			AnimateWidget.Color(m_wKeyBG, m_InputButtonComp.m_ActionHovered, m_fAnimationRate);
		}
		else
		{
			if (m_bIsToggled)
				AnimateToggle();
			else
				ResetColor();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Animate Widget when action is performed
	protected void AnimateClick()
	{
		if (m_bCanBeToggled)
			return;

		if (!m_bIsDoubleTapAction && !m_bPressedInput)
		{
			AnimateWidget.StopAnimation(m_wKeyBG, WidgetAnimationColor);
			AnimateWidget.Color(m_wKeyBG, Color.White, m_fAnimationRate);
			
			//! Using CallLater to give the animation time to complete before it's being reset to default state
			GetGame().GetCallqueue().CallLater(ResetColor, m_fAnimationTime * 600 + 1, false);
		}		
	}

	//------------------------------------------------------------------------------------------------
	void AnimateHold()
	{
		m_wOutline.SetMaskProgress(0);
		m_HoldingAnimation = SCR_InputButtonAnimations.ButtonAlphaMask(m_wOutline, 1, (1 / m_InputButtonComp.m_fMaxHoldtime), true);
	}

	//------------------------------------------------------------------------------------------------
	void AnimateHoldComplete()
	{
		m_InputButtonComp.GetOnHoldAnimComplete().Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void AnimateDoubleTab()
	{
		if (!m_InputButtonComp.m_bIsDoubleTapStated)
		{
			AnimateWidget.StopAnimation(m_wKeyBG, WidgetAnimationColor);

			m_wKeyBG.SetColor(m_InputButtonComp.m_ActionHovered);
			//! Using CallLater to give the animation time to complete before it's being reset to default state
			GetGame().GetCallqueue().CallLater(ResetColor, m_fAnimationTime * 300 + 1, false);
		}
		else
		{
			AnimateWidget.StopAnimation(m_wKeyBG, WidgetAnimationColor);
			m_wKeyBG.SetColor(Color.White);
			//! Using CallLater to give the animation time to complete before it's being reset to default state
			GetGame().GetCallqueue().CallLater(ResetColor, m_fAnimationTime * 300 + 1, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AnimateToggle()
	{
		AnimateWidget.Color(m_wKeyBG, m_InputButtonComp.m_ActionToggled, m_fAnimationRate);
	}

	//------------------------------------------------------------------------------------------------
	void ResetColor()
	{		
		AnimateWidget.StopAnimation(m_wKeyBG, WidgetAnimationColor);
		if (m_InputButtonComp.m_bIsHovered)
			AnimateHover();
		else
			AnimateWidget.Color(m_wKeyBG, m_InputButtonComp.m_ActionDefault, m_fAnimationRate);
		
		GetGame().GetCallqueue().Remove(ResetColor);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the correct color when Widget is initialized. (Active or disabled)
	protected void UpdateEnableColor()
	{
		Color color;
		if (!m_wParent.IsEnabled())
			color = m_InputButtonComp.m_ActionDisabled;
		else
			color = m_InputButtonComp.m_ActionDefault;

		m_wKeyBG.SetColor(color);

		if (m_bIsDoubleTapAction)
		{
			if (m_wParent.IsEnabled())
				m_wDoubleTabIndicator.SetColor(Color.White);
			else
				m_wDoubleTabIndicator.SetColor(m_InputButtonComp.m_ActionDisabled);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	float GetTextSizeModifier()
	{
		return m_fButtonTextSizeModifier;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsOverwritten()
	{
		return m_bIsOverwritten;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsOverwritten(bool setOverride)
	{
		m_bIsOverwritten = setOverride;
	}
}
