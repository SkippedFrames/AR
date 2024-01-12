/*
Panel which shows state of addon.

!!! This component relies on SCR_WorkshopUiCommon to be initialized to work correctly.
*/

class SCR_AddonDetailsPanelComponent : SCR_ContentDetailsPanelBase
{
	protected ref SCR_WorkshopItem m_Item;

	protected bool m_bForceShowVersion = false; // Forces version text to be always shown

	const int MAX_ADDON_TYPE_IMAGES = 8; // Max amount of addon type images will be shown.

	const ResourceName ADDON_TYPE_IMAGE_LAYOUT = "{8D067F8167DB936D}UI/layouts/Menus/Common/DetailsPanel/Prefabs/AddonTypeImage.layout";

	protected ref SCR_AddonDetailsPanelWidgets m_Widgets;

	// -------- Public API -----------
	//------------------------------------------------------------------------------------------------
	void SetWorkshopItem(SCR_WorkshopItem item)
	{
		if (m_Item)
		{
			m_Item.m_OnChanged.Remove(Callback_OnChanged);
		}

		m_Item = item;

		if (m_Item)
			m_Item.m_OnChanged.Insert(Callback_OnChanged);

		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	void SetForceShowVersion(bool showVersion)
	{
		m_bForceShowVersion = showVersion;
		UpdateAllWidgets();
	}

	// -------- Protected / Private -----------
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets = new SCR_AddonDetailsPanelWidgets();
		m_Widgets.Init(w);

		if (!SCR_Global.IsEditMode())
			UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates properties which never change at run time (addon description, size, etc)
	//! This is called only when new addon is assigned or at start
	protected void UpdateAllWidgets()
	{
		// Don't mess up widgets in workbench
		if (SCR_Global.IsEditMode())
			return;

		bool hide;

		if (!m_Item)
			hide = true;
		else
		{
			hide = m_Item.GetRestricted();
		}

		if (hide)
		{
			m_Widgets.m_MainArea.SetVisible(false);
			m_Widgets.m_BackendImageComponent.SetWorkshopItemAndImage(null, null);
			return;
		}

		m_Widgets.m_MainArea.SetVisible(true);

		// Name, description, author name
		string title = m_Item.GetName();
		//title.ToUpper();
		m_Widgets.m_NameText.SetText(title);
		m_Widgets.m_AuthorNameText.SetText(m_Item.GetAuthorName());
		m_Widgets.m_DescriptionText.SetText(m_Item.GetSummary());

		// Rating
		int rating = Math.Ceil(m_Item.GetAverageRating() * 100.0);
		m_Widgets.m_RatingText.SetText(rating.ToString() + " %");

		// Size
		float sizef = m_Item.GetSizeBytes();
		string sizeStr = SCR_ByteFormat.GetReadableSize(sizef);
		m_Widgets.m_AddonSizeText.SetText(sizeStr);

		// Version
		Revision revisionCurrent = m_Item.GetCurrentLocalRevision();
		Revision revisionLatest = m_Item.GetLatestRevision();
		string versionText;
		bool showVersion0, showVersion1;
		bool showVersion = m_bForceShowVersion || m_Item.GetOffline();
		if (revisionCurrent && revisionLatest && !Revision.AreEqual(revisionCurrent, revisionLatest))
		{
			m_Widgets.m_VersionText0.SetText(SCR_WorkshopUiCommon.FormatVersion(revisionCurrent.GetVersion()));
			m_Widgets.m_VersionText1.SetText(SCR_WorkshopUiCommon.FormatVersion(revisionLatest.GetVersion()));
			showVersion0 = true;
			showVersion1 = true;
		}
		else if (revisionCurrent)
		{
			m_Widgets.m_VersionText0.SetText(SCR_WorkshopUiCommon.FormatVersion(revisionCurrent.GetVersion()));
			showVersion0 = true;
		}
		else if (revisionLatest)
		{
			m_Widgets.m_VersionText1.SetText(SCR_WorkshopUiCommon.FormatVersion(revisionLatest.GetVersion()));
			showVersion1 = true;
		}

		m_Widgets.m_VersionText0.SetVisible(showVersion && showVersion0);
		m_Widgets.m_VersionText1.SetVisible(showVersion && showVersion1);
		m_Widgets.m_VersionArrow.SetVisible(showVersion && showVersion0 && showVersion1);

		// Time since last played and downloaded
		int timeSinceLastPlay = m_Item.GetTimeSinceLastPlay();
		m_Widgets.m_LastPlayedOverlay.SetVisible(timeSinceLastPlay >= 0);
		if (timeSinceLastPlay >= 0)
			m_Widgets.m_LastPlayedText.SetText(SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceLastPlay));

		int timeSinceDownload = m_Item.GetTimeSinceFirstDownload();
		m_Widgets.m_DownloadedOverlay.SetVisible(timeSinceDownload >= 0);
		if (timeSinceDownload >= 0)
			m_Widgets.m_DownloadedText.SetText(SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceDownload));

		// Type - show a list of images associated with tags
		array<WorkshopTag> tagObjects = {};						// Get tag objects from workshop item
		m_Item.GetWorkshopItem().GetTags(tagObjects);
		array<string> tags = {};

		// Convert array of tag objects to array of strings
		foreach (WorkshopTag tag : tagObjects)
		{
			tags.Insert(tag.Name());
		}
		
		if (!tags.IsEmpty())
		{
			int nRecognizedTags = 0;

			// Delete old images
			// todo try to reuse image widgets instead of deleting them
			while (m_Widgets.m_TypeImages.GetChildren())
			{
				m_Widgets.m_TypeImages.RemoveChild(m_Widgets.m_TypeImages.GetChildren());
			}

			for (int i = 0; i < tags.Count() && i < MAX_ADDON_TYPE_IMAGES; i++)
			{
				string tag = tags[i];
				string image;
				ResourceName imageSet;
				SCR_WorkshopUiCommon.GetTagImage(tag, imageSet, image);
				if (!image.IsEmpty())
				{
					Widget w = GetGame().GetWorkspace().CreateWidgets(ADDON_TYPE_IMAGE_LAYOUT, m_Widgets.m_TypeImages);
					ImageWidget wimg = ImageWidget.Cast(w.FindAnyWidget("Image"));
					wimg.LoadImageFromSet(0, imageSet, image);
					nRecognizedTags++;
				}
			}

			// Show the overlay if we have find an image for at least any tag
			m_Widgets.m_TypeOverlay.SetVisible(nRecognizedTags != 0);
		}
		else
		{
			m_Widgets.m_TypeOverlay.SetVisible(false);
		}

		// Image
		m_Widgets.m_BackendImageComponent.SetWorkshopItemAndImage(m_Item, m_Item.GetThumbnail());

		// State text / icon
		SCR_WorkshopUiCommon.UpdateAddonStateIconAndText(m_Item, m_ColorScheme, m_Widgets.m_AddonStateIcon, m_Widgets.m_AddonStateText);

		// Dependencies and dependent
		array<ref SCR_WorkshopItem> dependentAddons = SCR_AddonManager.SelectItemsBasic(m_Item.GetDependentAddons(), EWorkshopItemQuery.OFFLINE);
		array<ref SCR_WorkshopItem> dependencies = m_Item.GetLatestDependencies();

		m_Widgets.m_DependenciesOverlay.SetVisible(!dependencies.IsEmpty());
		m_Widgets.m_DependentOverlay.SetVisible(!dependentAddons.IsEmpty());

		if (!dependencies.IsEmpty())
		{
			if (dependencies.Count() == 1)
				m_Widgets.m_DependenciesText.SetText("#AR-Workshop_Details_ModDependencies_Short_One_LC");
			else
				m_Widgets.m_DependenciesText.SetTextFormat("#AR-Workshop_Details_ModDependencies_Short_LC", dependencies.Count());
		}

		if (!dependentAddons.IsEmpty())
		{
			if (dependentAddons.Count() == 1)
				m_Widgets.m_DependentText.SetText("#AR-Workshop_Details_DependentMods_Short_One_LC");
			else
				m_Widgets.m_DependentText.SetTextFormat("#AR-Workshop_Details_DependentMods_Short_LC", dependentAddons.Count());
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		if (m_Item)
		{
			m_Item.m_OnChanged.Remove(Callback_OnChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnChanged()
	{
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetItem()
	{
		return m_Item;
	}
}
