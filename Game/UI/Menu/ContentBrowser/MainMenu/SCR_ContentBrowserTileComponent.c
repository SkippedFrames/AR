//#define WORKSHOP_DEBUG

//------------------------------------------------------------------------------------------------
//! Component for a tile in the content browser
//! It is designed to be fairly autonomous, for instance it doesn't depend on other code or callbacks if something else starts download
//! of the addon without notifying this tile.
//! You must call SetWorkshopItem() after tile creation to activate it
class SCR_ContentBrowserTileComponent : SCR_ScriptedWidgetComponent
{
	// Event Handlers
	ref ScriptInvoker m_OnFocus = new ScriptInvoker(); // (SCR_ContentBrowserTileComponent tile, SCR_WorkshopItem item)

	// Constants
	protected const float MAIN_BUTTON_FADE_SPEED = 1 / 0.07;

	protected const vector TOOLTIP_OFFSET = "15 0 0";

	// Main button modes
	// It's not perfect that it's a string, but it directly matches to the tags of button effects in the layout.
	protected const string MAIN_BUTTON_MODE_DOWNLOAD			= "download";
	protected const string MAIN_BUTTON_MODE_DOWNLOADING			= "downloading";
	protected const string MAIN_BUTTON_MODE_REPAIR				= "repair";
	protected const string MAIN_BUTTON_MODE_UPDATE				= "update";
	protected const string MAIN_BUTTON_ALL						= "all";

	protected const string MAIN_BUTTON_COLOR_MODE_NEUTRAL 		= "neutral";
	protected const string MAIN_BUTTON_COLOR_MODE_MODERATE		= "moderate";
	protected const string MAIN_BUTTON_COLOR_MODE_CRITICAL		= "critical";
	protected const string MAIN_BUTTON_COLOR_MODE_DOWNLOADING	= "downloading";

	// Other
	protected ResourceName ICON_IMAGE_SET = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	protected ref SCR_WorkshopItem m_Item;
	protected bool m_bShowRestricted;
	protected bool m_bRatingVisible = true;
	protected bool m_bFocus = false;
	protected bool m_bMouseOver = false;
	protected bool m_bDownloading = false;
	protected bool m_bAnyIssue = false;			// Any issue at all

	protected ref SCR_ContentBrowserTileWidgets widgets = new SCR_ContentBrowserTileWidgets;

	// Animators
	ref SCR_FadeInOutAnimator m_AnimatorFadeIn;
	ref SCR_FadeInOutAnimator m_AnimatorFadeOut;

	// Flag to prevent checking image all the time, because there is some bug when image is first shown as loaded by WorkshopItem, but later isn't
	protected bool m_bImageLoaded;

	[Attribute()]
	protected ref SCR_ContentBrowser_ColorScheme m_ColorScheme;

	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	// ------------------- Public -------------------
	//------------------------------------------------------------------------------------------------
	// If a null pointer is passed, tile becomes hidden (invisible)
	// - showRestricted - when false and if item is reported, the tile will switch to restricted appearance (image, name, author name will be hidden).
	void SetWorkshopItem(SCR_WorkshopItem workshopItem, bool showRestricted = false)
	{
		m_bShowRestricted = showRestricted;

		// Uregister from previous item
		if (m_Item)
		{
			m_Item.m_OnChanged.Remove(Callback_OnItemChanged);
			m_Item = null; // Release the old item
		}

		if (workshopItem)
		{
			m_Item = workshopItem;
			m_Item.m_OnChanged.Insert(Callback_OnItemChanged);
			m_bImageLoaded = false;
		}

		this.UpdateAllWidgets();

		// Since we reuse those tiles, when we assign a new workshop item to this tile, we should not animate the effects,
		// but we must force them into final value
		// so far it only matters for this button
		widgets.m_EnableButtonComponent.InvokeAllEnabledEffects(instant: true);
	}

	//------------------------------------------------------------------------------------------------
	void SetRatingVisible(bool visible)
	{
		m_bRatingVisible = visible;
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem()
	{
		return m_Item;
	}

	// ------------- Protected -----------------------
	//------------------------------------------------------------------------------------------------
	protected void UpdateDependencyCountWidgets()
	{
		if (!m_Item)
			return;

		bool offline = m_Item.GetOffline();
		//widgets.m_DependencyCountText.SetVisible(offline);
		widgets.m_DependencyCountText.SetVisible(false); // For now disabled

		if (!offline)
		{
			widgets.m_DependentHover.SetVisible(false);
			widgets.m_DependencyHover.SetVisible(false);
			return;
		}

		int nDependencies = SCR_AddonManager.CountItemsBasic(m_Item.GetLatestDependencies(), EWorkshopItemQuery.OFFLINE);
		int nDependent = SCR_AddonManager.CountItemsBasic(m_Item.GetDependentAddons(), EWorkshopItemQuery.OFFLINE);

		if (nDependencies == 0 && nDependent == 0)
			widgets.m_DependencyCountText.SetText(string.Empty);
		else
		{
			// "99 > [  ] > 99"
			string strLeft, strRight;
			string strCenter = "[  ]";
			if (nDependent > 0)
				strLeft = string.Format("%1 > ", nDependent);
			if (nDependencies > 0)
				strRight = string.Format(" > %1", nDependencies);
			widgets.m_DependencyCountText.SetText(strLeft + strCenter + strRight);
		}

		// Images
		widgets.m_DependentHover.SetVisible(nDependent > 0);
		widgets.m_DependencyHover.SetVisible(nDependencies > 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDownloadProgressWidgets()
	{
		if (m_bDownloading)
		{
			float progress = GetDownloadProgress();
			widgets.m_DownloadProgressBar.SetCurrent(progress);
		}
		else
		{
			widgets.m_DownloadProgressBar.SetCurrent(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateEnableButtonVisible()
	{
		// Visible only if offline
		bool visible = m_Item.GetOffline() && m_bFocus;
		widgets.m_EnableButton.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateEnableButtonToggled()
	{
		bool toggled = m_Item.GetEnabled();
		widgets.m_EnableButtonComponent.SetToggled(toggled, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFavouriteButtonToggled()
	{
		bool toggled = m_Item.GetFavourite();
		widgets.m_FavouriteButtonComponent.SetToggled(toggled, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFavouriteButtonVisible()
	{
		bool visible = m_bFocus || m_Item.GetFavourite();
		widgets.m_FavouriteButton.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	// Updates the state of widgets which depend on addon state much
	protected void UpdateStateWidgets()
	{
		if (!m_Item)
			return;

		bool mainButtonAlwaysVisible = false;
		string mainButtonMode;
		string mainButtonColorMode;
		string hintText;
		bool criticalProblem = false;
		string problemDescription;
		bool anyProblem = SCR_WorkshopUiCommon.GetHighestPriorityProblemDescription(m_Item, problemDescription, criticalProblem);

		if (m_bDownloading)
		{
			// Downloading

			mainButtonAlwaysVisible = true;
			mainButtonMode = MAIN_BUTTON_MODE_DOWNLOADING;
			mainButtonColorMode = MAIN_BUTTON_COLOR_MODE_DOWNLOADING;

			hintText = string.Format("%1 %2%%",
				WidgetManager.Translate("#AR-Workshop_ButtonDownloading"),
				Math.Round(100.0 * GetDownloadProgress()));
		}
		else
		{
			// Not downloading

			hintText = SCR_WorkshopUiCommon.GetPrimaryActionName(m_Item);

			if (m_Item.GetOffline())
			{
				// Offline
				EWorkshopItemProblem problem = m_Item.GetHighestPriorityProblem();

				switch (problem)
				{
					case EWorkshopItemProblem.DEPENDENCY_MISSING:
					case EWorkshopItemProblem.DEPENDENCY_DISABLED:
					{
						mainButtonAlwaysVisible = true;
						mainButtonMode = MAIN_BUTTON_MODE_REPAIR;
						mainButtonColorMode = MAIN_BUTTON_COLOR_MODE_CRITICAL;
						break;
					}

					case EWorkshopItemProblem.UPDATE_AVAILABLE:
					case EWorkshopItemProblem.DEPENDENCY_OUTDATED:
					{
						mainButtonAlwaysVisible = true;
						mainButtonMode = MAIN_BUTTON_MODE_UPDATE;
						mainButtonColorMode = MAIN_BUTTON_COLOR_MODE_MODERATE;
						break;
					}

					case EWorkshopItemProblem.NO_PROBLEM:
					{
						mainButtonAlwaysVisible = false;
					}
				}
			}
			else
			{
				// Not downloaded
				mainButtonMode = MAIN_BUTTON_MODE_DOWNLOAD;
				mainButtonColorMode = MAIN_BUTTON_COLOR_MODE_NEUTRAL;
			}
		}

		// Main button
		// When item is restricted, hide the button anyway, even if addon has issues (which should not be possible since it should not be downloaded).
		bool mainButtonVisible = !mainButtonMode.IsEmpty() && (mainButtonAlwaysVisible || m_bFocus) && !m_Item.GetRestricted();
		widgets.m_MainButton.SetVisible(mainButtonVisible);
		widgets.m_HintText.SetText(hintText);

		if (mainButtonVisible)
		{
			widgets.m_MainButtonComponent.SetEffectsWithAnyTagEnabled({MAIN_BUTTON_ALL, mainButtonMode, mainButtonColorMode});
		}

		// Enable button
		widgets.m_EnableButton.SetVisible(m_Item.GetOffline());
		if (m_Item.GetOffline())
		{

			string enableButtonMode = "no_problems";
			if (criticalProblem)
				enableButtonMode = "problems";

			widgets.m_EnableButtonComponent.SetEnabled(!m_bDownloading);
			widgets.m_EnableButtonComponent.SetEffectsWithAnyTagEnabled({"all", enableButtonMode});
		}


		// Visibility of addon size
		bool addonSizeVisible = m_Item.GetOffline() || m_bFocus || m_Item.GetSizeBytes() > ContentBrowserUI.GetSmallDownloadThreshold();
		widgets.m_AddonSizeOverlay.SetVisible(addonSizeVisible);

		// Visibility of addon rating
		widgets.m_RatingOverlay.SetVisible(m_bRatingVisible);

		ShowBackendEnv();

		return;
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowBackendEnv()
	{
		string gameEnv = GetGame().GetBackendApi().GetBackendEnv();
		string modEnv = m_Item.GetWorkshopItem().GetBackendEnv();

		bool display = (modEnv != "local") && (modEnv != "ask") && (modEnv != "invalid");
		bool envMatch = gameEnv == modEnv;

		widgets.m_BackendSource.SetVisible(display && !envMatch);
		if (!display)
			return;

		widgets.m_BackendSourceText.SetText(modEnv);

		if (envMatch)
			widgets.m_BackendSourceIcon.SetColor(UIColors.CONFIRM);
		else
			widgets.m_BackendSourceIcon.SetColor(UIColors.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFrame(float tDelta)
	{
		bool mouseOverMainButton = widgets.m_MainButtonComponent.GetMouseOver();
		m_AnimatorFadeIn.ForceVisible(mouseOverMainButton);
		m_AnimatorFadeOut.ForceVisible(!mouseOverMainButton);

		m_AnimatorFadeIn.Update(tDelta);
		m_AnimatorFadeOut.Update(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates widgets which represent some state of the item
	protected void UpdateAllWidgets()
	{
		//Print("UpdateAllWidgets()");

		if (!m_Item)
		{
			// This tile is disabled
			m_wRoot.SetVisible(false);
			return;
		}
		else
		{
			m_wRoot.SetVisible(true);
		}

		// If the item is blocked for a whatever reason
		if (m_Item.GetRestricted() && !m_bShowRestricted)
		{
			widgets.m_BlockedMode.SetVisible(true);
			widgets.m_NormalMode.SetVisible(false);
			return;
		}
		else
		{
			widgets.m_BlockedMode.SetVisible(false);
			widgets.m_NormalMode.SetVisible(true);
		}

		// Update state of downloads
		UpdateStateFlags();

		SCR_WorkshopItem item = m_Item;

		// Name and its animation
		widgets.m_NameText.SetText(item.GetName());
		if (m_bFocus && !widgets.m_NamesHorizontalAnimationComponent.GetContentFit())
		{
			widgets.m_NamesHorizontalAnimationComponent.AnimationStart();
		}
		else
		{
			widgets.m_NamesHorizontalAnimationComponent.AnimationStop();
			widgets.m_NamesHorizontalAnimationComponent.ResetPosition(); // Move text back to default pos.
		}

		// Author name
		widgets.m_AuthorNameText.SetText(item.GetAuthorName());

		// Rating - Text
		// Rating overlay visibility is updated separately!
		int rating = Math.Ceil(item.GetAverageRating() * 100.0); // Must be more positive (:
		widgets.m_RatingText.SetText(rating.ToString() + " %");

		// Image
		widgets.m_BackendImageComponent.SetWorkshopItemAndImage(m_Item, m_Item.GetThumbnail());

		// Addon size - Text
		// Addon size visibility is updated separately!
		float sizef = item.GetSizeBytes();
		string sizeStr = SCR_ByteFormat.GetReadableSizeMb(sizef);
		widgets.m_AddonSizeText.SetText(sizeStr);

		// Other UI elements
		UpdateEnableButtonVisible();
		UpdateEnableButtonToggled();
		UpdateFavouriteButtonToggled();
		UpdateFavouriteButtonVisible();
		UpdateDownloadProgressWidgets();
		UpdateDependencyCountWidgets();

		UpdateStateWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateStateFlags()
	{
		if (!m_Item)
			return;

		// Downloading is true if we are downloading anything for this addon or
		// if we have started a download for any of its dependencies through this item
		SCR_WorkshopItemActionDownload actionThisItem = m_Item.GetDownloadAction();
		SCR_WorkshopItemActionComposite actionDependencies = m_Item.GetDependencyCompositeAction();

		m_bDownloading = actionThisItem || actionDependencies;

		//m_Item.LogState();
		EWorkshopItemProblem problem = m_Item.GetHighestPriorityProblem();

		m_bAnyIssue = problem != EWorkshopItemProblem.NO_PROBLEM;
	}

	//------------------------------------------------------------------------------------------------
	void InitWidgets()
	{
		widgets.Init(m_wRoot);

		widgets.m_EnableButtonComponent.m_OnToggled.Insert(OnEnableButtonToggled);
		widgets.m_FavouriteButtonComponent.m_OnToggled.Insert(OnFavouriteButtonToggled);
		widgets.m_MainButtonComponent.m_OnClicked.Insert(OnMainButton);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns download progress of current action or of current composite action
	protected float GetDownloadProgress()
	{
		return SCR_DownloadManager.GetItemDownloadActionsProgress(m_Item);
	}

	// ---------------- Event Handlers --------------------
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!GetGame().InPlayMode())
			return;

		InitWidgets();

		// Create animators
		m_AnimatorFadeIn = new SCR_FadeInOutAnimator(widgets.m_ShowOnMainButtonHover, MAIN_BUTTON_FADE_SPEED, MAIN_BUTTON_FADE_SPEED, 0);
		m_AnimatorFadeOut = new SCR_FadeInOutAnimator(widgets.m_HideOnMainButtonHover, MAIN_BUTTON_FADE_SPEED, MAIN_BUTTON_FADE_SPEED, 0);

		ChimeraMenuBase parentMenu = ChimeraMenuBase.GetOwnerMenu(w);
		if (parentMenu)
		{
			parentMenu.m_OnUpdate.Insert(OnFrame);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		ChimeraMenuBase parentMenu = ChimeraMenuBase.GetOwnerMenu(w);
		if (parentMenu)
		{
			parentMenu.m_OnUpdate.Remove(OnFrame);
		}

		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		// This will also get called on updates of children, ignore them
		if (w != m_wRoot)
			return true;

		GetGame().GetCallqueue().CallLater(UpdateSize, 0);

		return super.OnUpdate(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSize()
	{
		// Bail if it's called after all widgets are destroyed.
		if (!m_wRoot || !widgets.m_MainAreaSizeRatio)
			return;

		// Resize the height of the main area, it must keep a fixed aspect ratio
		float sizex, sizey;
		m_wRoot.GetScreenSize(sizex, sizey);
		float sizexUnscaled = GetGame().GetWorkspace().DPIUnscale(sizex);
		widgets.m_MainAreaSizeRatio.EnableHeightOverride(true);
		widgets.m_MainAreaSizeRatio.SetHeightOverride(sizexUnscaled / SCR_WorkshopUiCommon.IMAGE_SIZE_RATIO);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		m_OnFocus.Invoke(this, m_Item);
		m_bFocus = true;

		AnimateWidget.Opacity(widgets.m_BorderPanel, 1, UIConstants.FADE_RATE_DEFAULT);
		UpdateAllWidgets();

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);

		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_bFocus = false;
		AnimateWidget.Opacity(widgets.m_BorderPanel, 0, UIConstants.FADE_RATE_DEFAULT);
		UpdateAllWidgets();

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		return super.OnFocusLost(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	// Focus on this widget when mouse enters it
	// By default a widget is focused after we click on it
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
			GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);
		return super.OnMouseEnter(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEnableButtonToggled(SCR_ModularButtonComponent comp)
	{
		SCR_WorkshopUiCommon.OnEnableAddonToggleButton(m_Item, comp);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMouseEnter_SetFocus()
	{
		GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFavouriteButtonToggled(SCR_ModularButtonComponent comp, bool toggled)
	{
		if (!m_Item)
			return;

		m_Item.SetFavourite(toggled);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMainButton()
	{
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_WorkshopItem.m_OnChanged
	protected void Callback_OnItemChanged(SCR_WorkshopItem item)
	{
		UpdateAllWidgets();
	}

	// -------------- Hovering over dependency icons ------------------
	//------------------------------------------------------------------------------------------------
	void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		if (!m_Item || !tooltipWidget)
			return;

		array<ref SCR_WorkshopItem> addons = {};

		switch (tag)
		{
			case "Dependent":
				addons = SCR_AddonManager.SelectItemsBasic(m_Item.GetDependentAddons(), EWorkshopItemQuery.OFFLINE);
				break;

			case "Dependencies":
				addons = SCR_AddonManager.SelectItemsBasic(m_Item.GetLatestDependencies(), EWorkshopItemQuery.OFFLINE);
				break;
		}

		SCR_ContentBrowser_AddonsTooltipComponent comp = SCR_ContentBrowser_AddonsTooltipComponent.FindComponent(tooltipWidget);
		if (comp)
			comp.Init(addons);
	}

	//------------------------------------------------------------------------------------------------
	SCR_ContentBrowserTileWidgets GetWidgets()
	{
		return widgets;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ContentBrowserTileComponent FindComponent(notnull Widget w)
	{
		return SCR_ContentBrowserTileComponent.Cast(w.FindHandler(SCR_ContentBrowserTileComponent));
	}
}
