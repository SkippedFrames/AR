// When enabled, the component will visualize debug information
//#define BACKEND_IMAGE_DEBUG

//! This abstract component handles loading of backend images and also manages the loading overlay while image is loading.
class SCR_BackendImageComponentBase : ScriptedWidgetComponent
{	
	// Attributes
	// We must rely on this coefficient since Widget.GetScreenSize() is not reliable in several cases and might return 0.
	[Attribute("1.0", UIWidgets.EditBox, "Coefficient which will be used to estimate target widget size. To calculate it, use ration of image width and screen width in Full HD resolution.")]
	protected float m_iScreenSizeCoefficient;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Image used as fallback for addon image", params: "edds")]
	protected ResourceName m_sFallbackImage;
	
	// Other
	protected ref BackendImage m_BackendImage;
	protected ref SCR_WorkshopItemCallback_DownloadImage m_DownloadImageCallback;
	protected int m_iPreferedWidth; // Used only for debugging
	protected int m_iScreenWidth; // Used only for debugging
	
	protected ref ScriptInvoker<> Event_OnImageSelected;

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnImageSelected()
	{
		if (Event_OnImageSelected)
			Event_OnImageSelected.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnImageSelected()
	{
		if (!Event_OnImageSelected)
			Event_OnImageSelected = new ScriptInvoker();

		return Event_OnImageSelected;
	}
	
	//----------------------------------------------------------------------------------
	//! image can be null
	void SetImage(BackendImage image)
	{
		Widget wImageSize = GetImageSizeWidget();
		if (!wImageSize) // Makes no sense
		{
			#ifdef BACKEND_IMAGE_DEBUG
			ShowDebugText("Error");
			#endif
			return;
		}
			
		// Get widget size instantly, related variables might be needed for debug text
		float fPreferedWidth, fHeight;
		wImageSize.GetScreenSize(fPreferedWidth, fHeight);
		m_iScreenWidth = Math.Round(fPreferedWidth); // Store for debugging
		
		bool showDefaultImage = true;
		if (image)
		{
			if (image.GetScale(0))
				showDefaultImage = false;
		}
		
		if (showDefaultImage)
		{
			ShowDefaultImage();
			m_iPreferedWidth = 0;
			#ifdef BACKEND_IMAGE_DEBUG
			ShowDebugText("Image unavailable\nShowing default image");
			#endif
			return;
		}
		
		// Ignore if called many times with same image
		//if (image == m_BackendImage)
		//	return;
		
		m_BackendImage = image;
		
		// Sometimes widget can report its size as 0, ensure that we request non-zero size image.
		int preferedWidth = Math.Round(fPreferedWidth);
		if (preferedWidth == 0)
		{			
			int referenceScreenWidth, referenceScreenHeight;
			WidgetManager.GetReferenceScreenSize(referenceScreenWidth, referenceScreenHeight);
			float referenceScreenRatio = referenceScreenWidth / referenceScreenHeight;
			
			float fScreenWidth, fScreenHeight;
			GetGame().GetWorkspace().GetScreenSize(fScreenWidth, fScreenHeight);
			if (fScreenWidth == 0 || fScreenHeight == 0) // This is next to impossible, but who knows . . .
			{  
				fScreenWidth = referenceScreenWidth;
				fScreenHeight = referenceScreenHeight;
			}
			
			if (fScreenWidth/fScreenHeight > referenceScreenRatio)
				fScreenWidth = referenceScreenRatio * fScreenHeight;
			
			preferedWidth = Math.Ceil(m_iScreenSizeCoefficient * fScreenWidth);
		}
		
		m_iPreferedWidth = preferedWidth; // Store for debug text
		auto backend = GetGame().GetBackendApi();
		bool connected = backend.IsActive() && backend.IsAuthenticated();		
		if (connected)
		{
			// We are connected, download image
			
			ImageScale imageScale = m_BackendImage.GetScale(preferedWidth);
			
			// Discard the old callback
			// We must unsubscribe from events of previous callback, because it still might get called
			// When previous image is downloaded
			if (m_DownloadImageCallback)
			{
				m_DownloadImageCallback.m_OnError.Remove(Callback_DownloadImage_OnTimeoutError);
				m_DownloadImageCallback.m_OnTimeout.Remove(Callback_DownloadImage_OnTimeoutError);
				m_DownloadImageCallback.m_OnSuccess.Remove(Callback_DownloadImage_OnSuccess);
			}
			
			m_DownloadImageCallback = new SCR_WorkshopItemCallback_DownloadImage();
			m_DownloadImageCallback.m_OnError.Insert(Callback_DownloadImage_OnTimeoutError);
			m_DownloadImageCallback.m_OnTimeout.Insert(Callback_DownloadImage_OnTimeoutError);
			m_DownloadImageCallback.m_OnSuccess.Insert(Callback_DownloadImage_OnSuccess);
			
			m_DownloadImageCallback.m_Scale = imageScale;
			bool downloadStarted = imageScale.Download(m_DownloadImageCallback);
			if (downloadStarted)
			{
				// While we are downloading, try to get a smaller local scale
				ImageScale localImageScale = m_BackendImage.GetLocalScale(preferedWidth);
				if (localImageScale)
				{
					if (!localImageScale.Path().IsEmpty())
						ShowLoadingImage(localImageScale.Path());
					else
						ShowLoadingImage(string.Empty);
				}
				else
					ShowLoadingImage(string.Empty);
				
				#ifdef BACKEND_IMAGE_DEBUG
				ShowDebugText("Waiting");
				#endif
			}
			else
			{
				// Most likely we have already downloaded this
				string imagePath = imageScale.Path();
				if (!imagePath.IsEmpty())
				{
					#ifdef BACKEND_IMAGE_DEBUG
					string dbgtxt = string.Format("Local Image: %1x%2", imageScale.Width(), imageScale.Height());
					ShowDebugText(dbgtxt);
					#endif
					
					ShowImage(imagePath);
				}
				else
					ShowDefaultImage();
			}
		}
		else
		{
			// No connection, try to use local image
			ImageScale localImageScale = m_BackendImage.GetLocalScale(preferedWidth);
			if (localImageScale)
			{
				string imagePath = localImageScale.Path();
				if (!imagePath.IsEmpty())
					ShowImage(localImageScale.Path());
				else
					ShowDefaultImage();
			}
			else
				ShowDefaultImage();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_DownloadImage_OnTimeoutError(SCR_WorkshopItemCallback_DownloadImage callback)
	{		
		ShowDefaultImage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_DownloadImage_OnSuccess(SCR_WorkshopItemCallback_DownloadImage callback)
	{		
		if (callback.m_Scale)
		{
			string imgPath = callback.m_Scale.Path();
			if (!imgPath.IsEmpty())
			{
				ShowImage(imgPath);
				
				#ifdef BACKEND_IMAGE_DEBUG
				string dbgtxt = string.Format("Received %1x%2", callback.m_Scale.Width(), callback.m_Scale.Height());
				ShowDebugText(dbgtxt);
				#endif
			}
			else
				ShowDefaultImage();
		}
		else
			ShowDefaultImage();
	}
	
	// Implement handling of widgets here
	protected void ShowLoadingImage(string fallbackImage);	// Must visualize loading of image. When !fallbackImage.IsEmpty(), there is a local different size of this image.
	protected void ShowDefaultImage();						// Must display something when image is not available
	protected void ShowImage(string imagePath);				// Must display provided image
	protected Widget GetImageSizeWidget();					// Must return widget which wraps image widget. It will be used for size measurement.
	protected void ShowDebugText(string txt);				// Should implement visualization of debug text
};

//! This component implements basic image and loading overlay handling
//! m_sImageWidgetName must be an Image Widget
//! m_sLoadingOverlayName must have an SCR_LoadingOverlay component attached
class SCR_BackendImageComponent : SCR_BackendImageComponentBase
{
	[Attribute()]
	protected string m_sImageWidgetName;
	
	[Attribute("", UIWidgets.EditBox, "Widget from which we will be reading size of image widget. It can be detached from image widget, since its reported size can be different when wrapped into a scale widget.")]
	protected string m_sImageSizeWidgetName;
	
	[Attribute("1")]
	protected bool m_bShowLoadingImage;
	
	[Attribute("")]
	protected string m_sLoadingOverlayName;
	
	[Attribute("")]
	protected string m_sDebugTextName;
	
	// Widgets
	protected ImageWidget m_wImage;
	protected Widget m_wImageSize;
	protected SCR_LoadingOverlay m_LoadingOverlay;
	protected Widget m_wRoot;
	protected TextWidget m_wDebugText;
	
	//----------------------------------------------------------------------------------
	Widget GetRootWidget() 
	{
		return m_wRoot; 
	}
	
	//----------------------------------------------------------------------------------
	protected override void ShowLoadingImage(string fallbackImage)
	{
		if (!m_wImage)
			return;
		
		if (fallbackImage.IsEmpty())
			m_wImage.SetVisible(false);
		else
			ShowImage(fallbackImage);
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(true);
	}
	
	//----------------------------------------------------------------------------------
	//! By default it hides image widget
	protected override void ShowDefaultImage()
	{
		if (!m_wImage)
			return;
		
		m_wImage.SetVisible(false);
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(false);
		
		InvokeEventOnImageSelected();
	}
	
	//----------------------------------------------------------------------------------
	protected override void ShowImage(string imagePath)
	{
		if (!m_wImage)
			return;
		
		if (imagePath.IsEmpty())
		{
			m_wImage.LoadImageTexture(0, m_sFallbackImage, false, true);
			return;
		}
		
		m_wImage.LoadImageTexture(0, imagePath, false, true);
		
		int sx, sy;
		m_wImage.GetImageSize(0, sx, sy);
		m_wImage.SetSize(sx, sy);
		
		m_wImage.SetVisible(true);
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(false);
		
		InvokeEventOnImageSelected();
	}
	
	//----------------------------------------------------------------------------------
	protected override Widget GetImageSizeWidget()
	{
		return m_wImageSize;
	}
	
	//----------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wRoot = w;
		
		if (!m_sImageWidgetName.IsEmpty())
			m_wImage = ImageWidget.Cast(w.FindAnyWidget(m_sImageWidgetName));
		
		if (!m_sImageSizeWidgetName.IsEmpty())
			m_wImageSize = w.FindAnyWidget(m_sImageSizeWidgetName);
		
		if (!m_sLoadingOverlayName.IsEmpty())
		{
			Widget wLoading = w.FindAnyWidget(m_sLoadingOverlayName);
			if (wLoading)
			{
				m_LoadingOverlay = SCR_LoadingOverlay.Cast(wLoading.FindHandler(SCR_LoadingOverlay));
			}
		}
		
		if (!m_sDebugTextName.IsEmpty())
		{
			m_wDebugText = TextWidget.Cast(w.FindAnyWidget(m_sDebugTextName));
			
			
			if (m_wDebugText)
			{
				#ifdef BACKEND_IMAGE_DEBUG
					m_wDebugText.SetVisible(true);
					ShowDebugText("SCR_BackendImageComponent\nInitialized");
				#else
					m_wDebugText.SetVisible(false);
				#endif
			}
		}	
	}
	
	//----------------------------------------------------------------------------------
	override protected void ShowDebugText(string txt)
	{
		if (!m_wDebugText)
			return;
		
		string commotDebugData;
		
		if (m_iPreferedWidth != 0)
			commotDebugData = string.Format("Widget Screen Width: %1\nPrefered Width: %2\nScales in Backend:\n%3",
				m_iScreenWidth, m_iPreferedWidth, FormatAvailableScales());
		
		m_wDebugText.SetText(commotDebugData + "\n" + txt);
	}
	
	//----------------------------------------------------------------------------------
	protected string FormatAvailableScales()
	{
		if (!m_BackendImage)
			return string.Empty;
		
		array<ImageScale> scales = {};
		m_BackendImage.GetScales(scales);
		
		string str;
		int j = 0;
		for (int i = 0; i < scales.Count(); i++)
		{
			string s;
			
			if (scales[i])
				s = string.Format("%1x%2 ", scales[i].Width(), scales[i].Height());
			else
				s = "null ";
			
			j = j+1;
			if (j == 3)
			{
				s = s + "\n";
				j = 0;
			}
			
			str = str + s;
		}
		
		return str;
	}
};
