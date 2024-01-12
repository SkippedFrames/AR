#include "scripts/Game/config.c"
//------------------------------------------------------------------------------------------------
class SCR_CampaignMapUIBase : SCR_CampaignMapUIElement
{
	[Attribute("{14E86B20D0ED993F}UI/layouts/Campaign/ServiceContainer.layout")]
	protected ResourceName m_sServiceElement;

	[Attribute("{94F1E2223D7E0588}UI/layouts/Campaign/ServiceHint.layout")]
	protected ResourceName m_sServiceHint;

	[Attribute("{CA8B528037B6831A}UI/Textures/InventoryIcons/Supplies_icon_UI.edds")]
	protected ResourceName m_sSupplyIcon;

	protected static const float OPACITY_DISABLED = 0.5;
	protected static const float OPACITY_UNSELECTED = 0.9;

	/*
		note(koudelkaluk): base name and service container offsets
		are currently hardcoded in SetBaseImage() based on the icon size.
		Once this system starts utilizing SCR_MilitarySymbolUIComponent,
		this whole thing can be redone in a better way.
	*/
	protected int m_iServicesPadding = 0;
	protected float m_fNameOffset = 2;

	protected ref map<EEditableEntityLabel, SCR_ServicePointDelegateComponent> m_mServices = new map<EEditableEntityLabel, SCR_ServicePointDelegateComponent>(); // true if built
	protected ref map<Widget, SCR_MapUITask> m_mTasks = new map<Widget, SCR_MapUITask>();

	protected SCR_CampaignMilitaryBaseComponent m_Base;
	protected string m_sFactionKey;
	protected SCR_CampaignFaction m_PlayerFaction;
	protected SCR_CampaignMobileAssemblyStandaloneComponent m_MobileAssembly;
	protected SCR_ResourceComponent	m_ResourceComponent;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected ref SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;
	protected RplId m_ResourceInventoryPlayerComponentRplId;

	protected SizeLayoutWidget m_wImageOverlay;
	protected Widget m_wBaseFrame;
	protected Widget m_wBaseIcon;
	protected Widget m_wBaseOverlay;
	protected Widget m_wInfoOverlay;
	protected Widget m_wServices;
	protected TextWidget m_w_NameDialog;
	protected static Widget m_wServiceHint;
	protected Widget m_w_ServicesOverlay;

	protected TextWidget m_wBaseName;
	protected TextWidget m_wCallsignName;
	protected TextWidget m_wBaseNameDialog;
	protected TextWidget m_wCallsignNameDialog;
	protected TextWidget m_wSuppliesText
	protected Widget m_wInfoText;
	protected Widget m_wAntennaImg;

	protected int m_iBaseSize = 80; 	// Maximum base icon size - it needs to be divisible by 4, otherwise the texture will look blurred	
	protected int m_iDefBaseSize = 46;	// Default base size
	protected int m_iMaxBaseSize = 80;  // Maximum size of the base, everything is calculated from this value, changing it will change the scale of all the steps
	protected int m_iDefHQSize = 88;	
	protected int m_iDefRelaySize = 64; 
	
	protected ImageWidget m_wLocalTask;
	protected LocalizedString m_sAssembly = "#AR-Vehicle_MobileAssembly_Name";

	static ref ScriptInvoker Event_OnIconUpdated = new ScriptInvoker();

	protected ref ScriptInvoker m_OnBaseSelected = new ScriptInvoker();
	protected ref ScriptInvoker m_OnMapIconEnter;
	protected ref ScriptInvoker m_OnMapIconClick;

	bool m_bHighlighted;
	bool m_bIsAnyElementHovered;
	bool m_bIsAnyElementClicked;
	protected bool m_bServicesShown;
	protected bool m_bCanRespawn;
	protected bool m_bIsRespawnMenu;
	protected bool m_bIsEditor;
	protected bool m_bCanPlaySounds = true;

	protected SCR_SpawnPoint m_SpawnPoint;

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMapIconEnter()
	{
		if (!m_OnMapIconEnter)
			m_OnMapIconEnter = new ScriptInvoker();

		return m_OnMapIconEnter;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMapIconClick()
	{
		if (!m_OnMapIconClick)
			m_OnMapIconClick = new ScriptInvoker();

		return m_OnMapIconClick;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_OnMapIconClick)
			m_OnMapIconClick.Invoke();

		CheckIfCanRespawn();

		if (!m_bIsAnyElementHovered && m_bCanRespawn && m_bIsRespawnMenu)
		{
			m_bIsAnyElementClicked = true;
			SelectIcon();
			m_bIsAnyElementClicked = false;
			m_wHighlightImg.SetVisible(false);
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void SelectIcon(bool invoke = true)
	{
		if (!m_wSelectImg)
			return;

		if (s_SelectedElement && s_SelectedElement != this)
		{
			s_SelectedElement.Select(false);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_CLICK_POINT_ON);
		}

		Select(invoke);

		m_wSelectImg.SetVisible(true);
		if (m_wGradient)
			m_wGradient.SetVisible(true);

		if (m_bIsSelected && invoke)
			m_Parent.OnSpawnPointSelected(m_SpawnPoint.GetRplId());
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_wImageOverlay && w == m_wImageOverlay && !m_wBaseOverlay.IsEnabled())
			m_wBaseOverlay.SetEnabled(true);	// enable the underlying button, allowing click

		SCR_UITaskManagerComponent tm = SCR_UITaskManagerComponent.GetInstance();
		if (tm && !tm.IsTaskListOpen())
		{
			GetGame().GetWorkspace().SetFocusedWidget(w);
		}

		if (m_OnMapIconEnter)
			m_OnMapIconEnter.Invoke();

		super.OnMouseEnter(w, x, y);

		if (w.Type() == ButtonWidget)
			AnimExpand();
		
		if (m_Base)
		{
			m_Base.GetMapDescriptor().OnIconHovered(this, true);

		if (m_wServices && m_Base && m_Base.GetType() == SCR_ECampaignBaseType.BASE && m_Base.IsHQRadioTrafficPossible(SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction())))
			{
				m_wServices.SetVisible(!m_mServices.IsEmpty());
				m_wServices.SetEnabled(!m_mServices.IsEmpty());
			}
		}
		
		if (m_wInfoText)
		{
			m_wInfoText.SetVisible(true);
			m_wRoot.SetZOrder(1);
		}
		
		if (m_MobileAssembly){
			m_MobileAssembly.OnIconHovered(this, true);
			m_wServices.SetVisible(true);
			m_w_ServicesOverlay.SetVisible(false);
			m_wRoot.SetZOrder(1);
		}
		
		if (!m_bCanRespawn && m_bIsRespawnMenu)
			return false;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);

		AnimCollapse();
		
		if(m_wInfoText)
		{
		m_wInfoText.SetVisible(false);
		m_wRoot.SetZOrder(0);
		}
		
		if (m_Base)
			m_Base.GetMapDescriptor().OnIconHovered(this, false);

		if (m_MobileAssembly)
			m_MobileAssembly.OnIconHovered(this, false);
			m_wInfoText.SetVisible(false);

		if (!m_bCanRespawn && m_bIsRespawnMenu)
			return false;

		if (m_wServices)
			m_wServices.SetEnabled(false);

		if (m_wLocalTask.IsEnabled())
			m_wLocalTask.SetVisible(true);

		if (enterW)
			m_bCanPlaySounds = ScriptedWidgetEventHandler.Cast(w.FindHandler(SCR_CampaignMapUIService)) == null;
		else
			m_bCanPlaySounds = true;

		if (RenderTargetWidget.Cast(enterW) && m_wBaseOverlay.IsEnabled())
		{
			m_wBaseOverlay.SetEnabled(false); // disable the base widget when not hovered, deactivating the button
			m_bCanPlaySounds = true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectBase()
	{
		if (s_SelectedElement && s_SelectedElement != this)
			s_SelectedElement.Select(false);

		Select();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLeave()
	{
	}

	//------------------------------------------------------------------------------------------------
	override void AnimExpand()
	{
		if (!m_bIsAnyElementHovered && m_bCanPlaySounds)
		{
			switch (m_eIconType)
			{
				case SCR_EIconType.NONE:
				{
				} break;

				case SCR_EIconType.BASE:
				{
					if (!m_bIsAnyElementClicked)
						PlayHoverSound(m_sSoundBase);
				} break;

				case SCR_EIconType.ENEMY_BASE:
				{
					PlayHoverSound(m_sSoundEnemyBase);
				} break;

				case SCR_EIconType.RELAY:
				{
					PlayHoverSound(m_sSoundRelay);
				} break;
			}
		}

		int paddingLeft = 0;
		int paddingRight = 0;
		int paddingBottom = 0;
		int expand = -5;
		if (m_mTasks.IsEmpty())
			paddingRight = 0;
		if (m_mServices.IsEmpty())
			paddingLeft = 0;

		if (m_wServices)
		{
			AnimateWidget.Opacity(m_wServices, 1, ANIM_SPEED);
		}

		m_wHighlightImg.SetVisible(true);
		if (m_wGradient)
			m_wGradient.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	override void AnimCollapse()
	{
		m_wHighlightImg.SetVisible(false);
		if (m_wGradient && !m_bIsSelected)
			m_wGradient.SetVisible(false);

		if (m_wServices)
		{
			AnimateWidget.Opacity(m_wServices, 0, ANIM_SPEED);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowServiceHint(string name, string text, bool show, int suppliesAmount = -1, int suppliesMax = -1)
	{
		int mx;
		int my;

		WidgetManager.GetMousePos(mx, my);

		mx = GetGame().GetWorkspace().DPIUnscale(mx);
		my = GetGame().GetWorkspace().DPIUnscale(my);

		if (!m_wServiceHint)
		{
			SCR_MapEntity entity = SCR_MapEntity.GetMapInstance();
			m_wServiceHint = entity.GetMapMenuRoot().FindAnyWidget("ServiceHintRoot");
		}

		if (show)
		{
			FrameSlot.SetPos(m_wServiceHint, mx, my);
			TextWidget.Cast(m_wServiceHint.FindAnyWidget("ServiceName")).SetTextFormat(name);
			Widget resupplyText = m_wServiceHint.FindAnyWidget("ResupplyText");
			if (resupplyText)
				resupplyText.SetVisible(false);

			if (suppliesAmount > -1 && suppliesMax > -1)
			{
				TextWidget serviceText = TextWidget.Cast(m_wServiceHint.FindAnyWidget("ServiceText"));
				if (serviceText)
					serviceText.SetTextFormat(text, suppliesAmount, suppliesMax);

				if (resupplyText)
					resupplyText.SetVisible(suppliesAmount < suppliesMax);
			}
			else
			{
				TextWidget serviceText = TextWidget.Cast(m_wServiceHint.FindAnyWidget("ServiceText"));
				if (serviceText)
					serviceText.SetTextFormat(text);
			}
		}

		m_wServiceHint.SetVisible(show)
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

//		SCR_SelectSpawnPointSubMenu.Event_OnSpawnPointChanged.Insert(OnSelected);

		m_wImageOverlay = SizeLayoutWidget.Cast(w.FindAnyWidget("SizeBaseIcon"));
		m_wBaseOverlay = w.FindAnyWidget("ImageOverlay");
		m_wInfoOverlay = w.FindAnyWidget("InfoOverlay");
		m_wServices = w.FindAnyWidget("ServicesFrame");
		m_w_ServicesOverlay = w.FindAnyWidget("ServicesSizeOverlay");

		m_wBaseFrame = w.FindAnyWidget("BaseFrame");
		m_wBaseIcon = Widget.Cast(w.FindAnyWidget("SideSymbol"));

		m_wBaseName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wCallsignName = TextWidget.Cast(w.FindAnyWidget("Callsign"));
		
		m_wBaseNameDialog = TextWidget.Cast(w.FindAnyWidget("m_w_NameDialog"));
		m_wCallsignNameDialog = TextWidget.Cast(w.FindAnyWidget("Callsign-Dialog"));


		m_wInfoText = Widget.Cast(w.FindAnyWidget("Info"));
		m_wAntennaImg = w.FindAnyWidget("AntenaOff");

		m_wLocalTask = ImageWidget.Cast(w.FindAnyWidget("LocalTask"));

		SCR_GameModeCampaign gameMode = SCR_GameModeCampaign.GetInstance();
		if (gameMode)
		{
			SCR_CampaignMobileAssemblyStandaloneComponent.s_OnUpdateRespawnCooldown.Insert(SetIconInfoText);
			SCR_CampaignMobileAssemblyStandaloneComponent.s_OnSpawnPointOwnerChanged.Insert(UpdateAssemblyIcon);
		}

		m_bIsRespawnMenu = SCR_DeployMenuMain.GetDeployMenu() != null;
		m_bIsEditor = SCR_EditorManagerEntity.IsOpenedInstance(false);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapCloseInvoker);
		m_PlayerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (m_wInfoText)
			m_wSuppliesText = TextWidget.Cast(m_wInfoText.FindAnyWidget("Supplies"));
	}

	//------------------------------------------------------------------------------------------------
	void OnMapCloseInvoker(MapConfiguration config)
	{
		RemoveHint();
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveHint()
	{
		if (m_wServiceHint)
			m_wServiceHint.RemoveFromHierarchy();
		m_wServiceHint = null;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		RemoveHint();

		if (SCR_GameModeCampaign.GetInstance())
		{
			SCR_CampaignMobileAssemblyStandaloneComponent.s_OnUpdateRespawnCooldown.Remove(SetIconInfoText);
			SCR_CampaignMobileAssemblyStandaloneComponent.s_OnSpawnPointOwnerChanged.Remove(UpdateAssemblyIcon);
		}

		SCR_MapEntity.GetOnMapClose().Remove(OnMapCloseInvoker);

		if (m_ResourceComponent)
			m_ResourceComponent.TEMP_GetOnInteractorReplicated().Remove(SetIconInfoText);
		
		m_ResourceSubscriptionHandleConsumer = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusOnBase(SCR_SpawnPoint sp)
	{
		OnLeave();

		if (!sp)
			return;

		IEntity parent = sp.GetParent();
		if (!parent)
			return;

		if (m_MobileAssembly)
		{
			SelectBase();
		}
		else
		{
			SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(parent.FindComponent(SCR_CampaignMilitaryBaseComponent));

			if (base && base == m_Base)
				SelectBase();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelected(SCR_SpawnPoint sp)
	{
		if (!sp)
			return;
		if (sp == m_SpawnPoint)
		{
			m_wRoot.SetZOrder(1);
			m_wSelectImg.SetVisible(true);
			if (m_wGradient)
				m_wGradient.SetVisible(true);
		}
		else
		{
			m_wRoot.SetZOrder(0);
			m_wSelectImg.SetVisible(false);
			if (m_wGradient)
				m_wGradient.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowServices(bool show)
	{
		m_wServices.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowBase(bool show)
	{
		m_wBaseFrame.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void SetIconInfoText()
	{
		if (!m_wInfoText)
			return;

		if (m_MobileAssembly)
			m_MobileAssembly.UpdateRespawnCooldown(m_wInfoText);

		if (!m_Base)
			return;

		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (!playerFaction)
			return;

		Faction baseFaction = m_Base.GetFaction();
		#ifndef AR_CAMPAIGN_TIMESTAMP
		float respawnCooldown = Math.Ceil((m_Base.GetRespawnTimestamp() - Replication.Time()) * 0.001);
		#else
		ChimeraWorld world = GetGame().GetWorld();
		float respawnCooldown = Math.Ceil(m_Base.GetRespawnTimestamp().DiffMilliseconds(world.GetServerTimestamp()) * 0.001);
		#endif
		string shownRespawnCooldown;
		int d, h, m, s;
		string sStr;

		TextWidget respawn = TextWidget.Cast(m_wInfoText.FindAnyWidget("Respawn"));
		RichTextWidget freq = RichTextWidget.Cast(m_wInfoText.FindAnyWidget("Freq"));
		TextWidget supplies = TextWidget.Cast(m_wInfoText.FindAnyWidget("Supplies"));
		ImageWidget respawnImg = ImageWidget.Cast(m_wInfoText.FindAnyWidget("RespawnIMG"));
		ImageWidget freqImg = ImageWidget.Cast(m_wInfoText.FindAnyWidget("FreqIMG"));
		ImageWidget suppliesImg = ImageWidget.Cast(m_wInfoText.FindAnyWidget("SuppliesIMG"));

		// Cooldown in progress, update UI timer
		if (m_Base.GetSupplies() >= m_Base.GetBaseSpawnCost() && respawnCooldown > 0 && baseFaction && baseFaction == playerFaction)
		{
			shownRespawnCooldown = "#AR-Campaign_MobileAssemblyCooldown";
			SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(respawnCooldown, d, h, m, s);
			sStr = string.ToString(s);

			if (s < 10)
				sStr = "0" + sStr;
		}
		else
		{
			respawnImg.SetVisible(false);
		}

		// Compose proper map info based on the base type and ownership
		LocalizedString suppliesString;
		string suppliesInfo;

		if (m_Base.GetType() == SCR_ECampaignBaseType.BASE)
		{
			supplies.SetVisible(true);
			suppliesImg.SetVisible(true);

			if (baseFaction && baseFaction == playerFaction)
			{
				suppliesString = "#AR-Campaign_BaseSuppliesAmount";
				suppliesInfo = m_Base.GetSupplies().ToString();

				if (!m_Base.IsHQ() && !shownRespawnCooldown.IsEmpty())
				{
					respawn.SetVisible(true);
					respawnImg.SetVisible(true);
				}
			}
			else
			{
				suppliesString = "#AR-Campaign_BaseSuppliesUnknown";
			}
		}

		ResourceName imageset = m_Base.GetBuildingIconImageset();

		if (!imageset.IsEmpty())
		{
			respawnImg.LoadImageFromSet(0, imageset, "RespawnBig");
			suppliesImg.LoadImageFromSet(0, imageset, "SuppliesBig");
		}

		supplies.SetTextFormat(suppliesString, suppliesInfo, m_Base.GetSuppliesMax());
		respawn.SetTextFormat(shownRespawnCooldown, m, sStr);

		/*if (m_wSuppliesText)
			UpdateResources();*/
		
		if (suppliesImg)
			suppliesImg.LoadImageTexture(0, m_sSupplyIcon);
	}

	//------------------------------------------------------------------------------------------------
	void InitServices()
	{
		Widget w = m_wServices.FindAnyWidget("ServicesSizeOverlay"); //GetGame().GetWorkspace().CreateWidgets(m_sServiceElement, m_wServices);
		if (!w)
			return;

		m_wServices.SetVisible(false);
		m_wServices.SetOpacity(0);

		int cnt = m_mServices.Count();
		for (int serviceId = 0; serviceId < 8; ++serviceId)
		{
			string btnName = "Service" + serviceId.ToString();
			Widget serviceBtn = w.FindAnyWidget(btnName);

			if (serviceId >= cnt)
			{
				serviceBtn.SetVisible(false);
				continue;
			}

			SCR_CampaignMapUIService handler = SCR_CampaignMapUIService.Cast(serviceBtn.FindHandler(SCR_CampaignMapUIService));
			if (handler)
			{
				handler.SetParent(this);
				handler.SetService(m_mServices.GetKey(serviceId), m_mServices.GetElement(serviceId));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void InitBaseIcon()
	{
		if (!m_Base)
			return;

		SCR_Faction f;
		if (m_bIsEditor || m_Base.IsHQ() || (m_PlayerFaction && m_Base.IsHQRadioTrafficPossible(m_PlayerFaction)))
		{
			f = m_Base.GetCampaignFaction();
		}
		
		Widget w = m_wRoot.FindAnyWidget("rootWidget");
		if (w)
			FrameSlot.SetAlignment(w, 0.5, m_fNameOffset);

		SetIconFaction(f);
		SetIconName(m_Base.GetBaseName());
		SetBaseType(m_Base.GetType());
		SetBaseImage();
		ChangeBaseIconSize();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateResources()
	{
		if (!m_ResourceComponent || !m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES, m_ResourceConsumer))
			return;
		
		m_wSuppliesText.SetText(string.ToString(m_ResourceConsumer.GetAggregatedResourceValue()));
	}

	//------------------------------------------------------------------------------------------------
	void UpdateBaseIcon(int id)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		string f = m_sFactionNeutral;

		if (id == 1)
			f = campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR);
		else if (id == 2)
			f = campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR);
		else if (id == 3)
			f = campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR);

		m_sFactionKey = f;
		SetBaseImage();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAssemblyIcon()
	{
		if (!m_MobileAssembly)
			return;

		SetIconName(m_sAssembly);
		SetIconFaction(m_MobileAssembly.GetParentFaction());

		string img = string.Format("%1_%2", m_sFactionKey, m_sMobileAssembly);
		string selectionImg = string.Format("%1_%2_%3", m_sFactionKey, m_sRelay, m_sSelection);
		SetImage(img);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetIconFaction(Faction faction)
	{
		if (faction)
			m_sFactionKey = faction.GetFactionKey();
		else
			m_sFactionKey = m_sFactionNeutral;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetBaseImage()
	{
		string img;

		if (m_Base.GetType() == SCR_ECampaignBaseType.RELAY)
		{
			img = string.Format("%1_%2", m_sFactionKey, m_sRelay);
			m_fNameOffset = 0.9;
		}
		else if (m_Base.IsHQ())
		{
			img = string.Format("%1_%2_%3", m_sFactionKey, m_sBase, "Main");
			m_iServicesPadding = -10;
			m_fNameOffset = 0.8;
		}
		else if (m_Base.IsControlPoint())
		{
			img = string.Format("%1_%2_%3", m_sFactionKey, m_sBase, "Major");
			m_iServicesPadding = -5;
			m_fNameOffset = 0.8;
		}
		else
		{
			img = string.Format("%1_%2_%3", m_sFactionKey, m_sBase, "Small");
			m_fNameOffset = 0.3;
		}

		if (img != string.Empty)
			SetImage(img);
	}

	//------------------------------------------------------------------------------------------------
	void InitBase(SCR_CampaignMilitaryBaseComponent base)
	{
		m_Base = base;

		SetBaseServices(m_Base);
		InitBaseIcon();
		SetIconInfoText();
		InitServices();
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);

		if (m_Base.GetType() != SCR_ECampaignBaseType.RELAY && !m_Base.IsHQ() && !m_Base.IsControlPoint())
			m_wCallsignName.SetExactFontSize(18);
		
		SCR_CampaignMilitaryBaseMapDescriptorComponent descr = m_Base.GetMapDescriptor();

		if (!descr)
			return;

		m_MapItem = descr.Item();
		MapDescriptorProps props = m_MapItem.GetProps();
		props.SetIconVisible(false);
		props.SetTextVisible(false);
		props.Activate(true);
		props.SetGroupType(EMapDescriptorGroup.MDG_SEPARATE);

		if (!m_wSuppliesText)
			return;
		
		IEntity owner = m_Base.GetOwner();

		if (!owner)
			return;

		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
		
		if (!m_ResourceComponent)
			return;

		m_ResourceConsumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!m_ResourceConsumer)
			return;
		
		m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));
		m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandle(m_ResourceConsumer, m_ResourceInventoryPlayerComponentRplId);
		
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Insert(SetIconInfoText);
	}

	//------------------------------------------------------------------------------------------------
	void InitMobile(SCR_CampaignMobileAssemblyStandaloneComponent assembly)
	{
		m_MobileAssembly = assembly;
		m_eIconType = SCR_EIconType.BASE;
		UpdateAssemblyIcon();
		SetIconInfoText();

		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
		SCR_SpawnPoint spawnpoint = SCR_SpawnPoint.Cast(m_MobileAssembly.GetOwner());
		m_w_ServicesOverlay.SetVisible(false);
		m_wServices.SetVisible(false);

		if (!spawnpoint)
		{
			m_w_ServicesOverlay.SetVisible(false);
			return;
		}

		SCR_MapDescriptorComponent descr = SCR_MapDescriptorComponent.Cast(spawnpoint.FindComponent(SCR_MapDescriptorComponent));
		if (descr)
		{
			m_MapItem = descr.Item();
			MapDescriptorProps props = m_MapItem.GetProps();
			props.SetIconVisible(false);
			props.SetTextVisible(false);
			props.Activate(true);
			props.SetGroupType(EMapDescriptorGroup.MDG_SEPARATE);
			m_w_ServicesOverlay.SetVisible(false);
			m_wServices.SetVisible(false);
			
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapClose(MapConfiguration config)
	{
		if (m_Base)
			m_Base.GetMapDescriptor().OnIconHovered(this, false);
		else if (m_MobileAssembly)
			m_MobileAssembly.OnIconHovered(this, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetIconName(string name)
	{
		if (m_wBaseName)
		{
			m_wBaseName.SetText(name);
			m_wBaseNameDialog.SetText(name);
			m_sName = name;
		}
		if (m_wCallsignName)
		{
			string callsign;
			if (m_Base)
				callsign = m_Base.GetCallsignDisplayNameOnlyUC();

			m_wCallsignName.SetText(callsign);
			m_wCallsignNameDialog.SetText(callsign);

			if (m_Base && m_Base.IsControlPoint())
			{
				m_wCallsignName.SetColorInt(Color.DARK_MAGENTA);
				m_wCallsignNameDialog.SetColorInt(Color.DARK_MAGENTA);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckIfCanRespawn()
	{
		SCR_SpawnPoint spawnPoint;

		if (m_Base)
			spawnPoint = m_Base.GetSpawnPoint();
		if (m_MobileAssembly)
			spawnPoint = SCR_SpawnPoint.Cast(m_MobileAssembly.GetOwner());

		if (!spawnPoint)
			return;

		m_bCanRespawn = (m_PlayerFaction && m_PlayerFaction.GetFactionKey() == spawnPoint.GetFactionKey());// && SCR_SelectSpawnPointSubMenu.GetInstance() != null); @lk: why
	}

	//------------------------------------------------------------------------------------------------
	protected void SetBaseType(SCR_ECampaignBaseType type)
	{
		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		int curLayer;
		if (mapEnt)
				curLayer = mapEnt.GetLayerIndex();
		ChangeNameSizeOnLayerChange(curLayer);
		SCR_MapEntity.GetOnLayerChanged().Insert(ChangeNameSizeOnLayerChange);

		if (type == SCR_ECampaignBaseType.RELAY)
			m_eIconType = SCR_EIconType.RELAY;
		else
		{
			if (m_PlayerFaction && m_PlayerFaction.GetFactionKey() != m_sFactionKey)
				m_eIconType = SCR_EIconType.ENEMY_BASE;
			else
				m_eIconType = SCR_EIconType.BASE;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetBaseServices(SCR_CampaignMilitaryBaseComponent base)
	{
		array<SCR_ServicePointComponent> available = {};
		array<SCR_ServicePointDelegateComponent> built = {};
		array<int> allServices = {};

		SCR_Enum.GetEnumValues(SCR_EServicePointType, allServices);

		if (base)
			base.GetServiceDelegates(built);

		foreach (SCR_EServicePointType type : allServices)
		{
			m_mServices.Set(type, null);
		}

		foreach (SCR_ServicePointDelegateComponent delegate : built)
		{
			m_mServices.Set(delegate.GetType(), delegate);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	void ChangeBaseIconSize() // Function for getting base services count and setting base size according to the value
	{
		protected int m_iServicesLimit = 8;	// Maximum amount of services - used as a max size for base icon resize.
		protected int m_iServicesCount = 1; // Amount of services - it cannot be 0 
		
		array<SCR_EServicePointType> checkedTypes = 
		{
			SCR_EServicePointType.ARMORY,
			SCR_EServicePointType.HELIPAD,
			SCR_EServicePointType.BARRACKS,
			SCR_EServicePointType.FUEL_DEPOT,
			SCR_EServicePointType.RADIO_ANTENNA,
			SCR_EServicePointType.FIELD_HOSPITAL,
			SCR_EServicePointType.LIGHT_VEHICLE_DEPOT,
			SCR_EServicePointType.HEAVY_VEHICLE_DEPOT
		};
		
		foreach (SCR_EServicePointType type : checkedTypes)
		{
			if (m_Base.GetServiceDelegateByType(type))
				m_iServicesCount++;
		}
	
		if (m_iServicesCount > m_iServicesLimit)
			return;
			
		if (m_Base.GetFaction() == m_PlayerFaction || m_wServices && m_Base.GetType() == SCR_ECampaignBaseType.BASE && m_Base.IsHQRadioTrafficPossible(SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction())))
		{
			if (m_Base.IsHQ()) 
			{
				m_iBaseSize = m_iDefHQSize;
				m_wImageOverlay.SetWidthOverride(m_iBaseSize);
				m_wImageOverlay.SetHeightOverride(m_iBaseSize);
				return;
			}
			
			if (m_Base.GetType() == SCR_ECampaignBaseType.RELAY)
			{
				m_iBaseSize = m_iDefRelaySize;
				m_wImageOverlay.SetWidthOverride(m_iBaseSize);
				m_wImageOverlay.SetHeightOverride(m_iBaseSize);
				return;
			}
			
			m_iBaseSize = Math.Round(m_iDefBaseSize + ((m_iMaxBaseSize - m_iDefBaseSize) / m_iServicesLimit) * m_iServicesCount); //Calculation of new base size
			m_wImageOverlay.SetWidthOverride(m_iBaseSize);
			m_wImageOverlay.SetHeightOverride(m_iBaseSize);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetLocalTaskIcon(SCR_BaseTask task = null)
	{
		m_wLocalTask.SetEnabled(task != null);
		m_wLocalTask.SetVisible(task != null);
		if (!task)
			return;
		task.SetWidgetIcon(m_wLocalTask);
	}

	//------------------------------------------------------------------------------------------------
	override void ShowName(bool visible)
	{
		m_wBaseName.SetVisible(visible);
		m_wCallsignName.SetVisible(visible);
		m_wBaseNameDialog.SetVisible(visible);
	//	m_wCallsignNameDialog.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	override vector GetPos()
	{
		if (m_Base)
			return m_Base.GetOwner().GetOrigin();
		
		if (m_MobileAssembly)
			return m_MobileAssembly.GetOwner().GetOrigin();
		
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeNameSize(bool visible)
	{
		Widget w = m_wRoot.FindAnyWidget("BaseNameFrame");

		if (visible){
			m_wCallsignName.SetExactFontSize(24);
			if (m_Base.GetType() != SCR_ECampaignBaseType.RELAY && !m_Base.IsHQ() && !m_Base.IsControlPoint())
			{
				m_fNameOffset = 0.3;
				m_wBaseName.SetVisible(true);
				m_wCallsignName.SetExactFontSize(18);
			}

		}
		else
		{
			m_wCallsignName.SetExactFontSize(18);
			if (m_Base.GetType() != SCR_ECampaignBaseType.RELAY && !m_Base.IsHQ() && !m_Base.IsControlPoint())
			{
				m_fNameOffset = 0.7;
				m_wBaseName.SetVisible(false);
				m_wCallsignName.SetExactFontSize(12);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void ChangeNameSizeOnLayerChange(int layer)
	{
		ChangeNameSize(layer <= 3);
	}

	//------------------------------------------------------------------------------------------------
	string GetFactionKey()
	{
		return m_sFactionKey;
	}
	//------------------------------------------------------------------------------------------------
	Color GetFactionColor()
	{
		return GetColorForFaction(m_sFactionKey);
	}
	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetBase()
	{
		return m_Base;
	}

	//------------------------------------------------------------------------------------------------
	void SetBaseIconFactionColor(Faction faction)
	{
		if (!m_wBaseIcon)
			return;

		Color color;
		
		if (faction)
			color = faction.GetFactionColor();
		else
			color = GetColorForFaction("");

		m_wBaseIcon.SetColor(color);
		if (m_wGradient)
			m_wGradient.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	override void SetImage(string image)
	{
		if (!m_wImage)
			return;

		if (!m_wBaseIcon)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		TStringArray strs = new TStringArray;

		image.Split("_", strs, true);

		Color factionColor = GetColorForFaction(m_sFactionKey);
		m_wBaseIcon.SetColor(factionColor);
		if (m_wGradient)
			m_wGradient.SetColor(factionColor);

		SCR_MilitarySymbolUIComponent m_SymbolUI = SCR_MilitarySymbolUIComponent.Cast(m_wBaseIcon.FindHandler(SCR_MilitarySymbolUIComponent));
		SCR_MilitarySymbol baseIcon = new SCR_MilitarySymbol();
		string selection;
		string highlight;
		SCR_SpawnPoint spawnPoint;

		switch (strs.Get(0))
		{
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR):
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.INDFOR);
				selection = "Neutral_Select";
				highlight = "Neutral_Installation_Focus_Land";
				m_wAntennaImg.SetVisible(false);
				m_w_NameDialog = TextWidget.Cast(m_wRoot.FindAnyWidget("m_w_NameDialog"));
				m_w_NameDialog.SetColor(Color.FromRGBA(0,177,79,255));
				break;
			}
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR):
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.OPFOR);
				selection = "Hostile_Select";
				highlight = "Hostile_Installation_Focus_Land";
				m_w_NameDialog = TextWidget.Cast(m_wRoot.FindAnyWidget("m_w_NameDialog"));
				m_w_NameDialog.SetColor(Color.FromRGBA(238,49,47,255));
				break;
			}
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR):
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.BLUFOR);
				selection = "Friend_Select";
				highlight = "Friend_Installation_Focus_Land";
				image.Replace("US", "USSR");
				m_w_NameDialog = TextWidget.Cast(m_wRoot.FindAnyWidget("m_w_NameDialog"));
				m_w_NameDialog.SetColor(Color.FromRGBA(31,195,243,255));
				break;
			}
			case "Unknown":
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.UNKNOWN);
				selection = "Unknown_Select";
				highlight = "Unknown_Installation_Focus_Land";
				m_wAntennaImg.SetVisible(false);		
				m_w_NameDialog = TextWidget.Cast(m_wRoot.FindAnyWidget("m_w_NameDialog"));
				m_w_NameDialog.SetColor(Color.FromRGBA(249,210,103,255));
				break;
			}
		}

		switch (strs.Get(1))
		{
			case "Relay":
			{
				baseIcon.SetIcons(EMilitarySymbolIcon.RELAY);
				m_wImageOverlay.SetWidthOverride(m_iDefRelaySize);
				m_wImageOverlay.SetHeightOverride(m_iDefRelaySize);
				m_wAntennaImg.SetVisible(false);
				break;
			}
			case "Mobile":
			{
				baseIcon.SetIcons(EMilitarySymbolIcon.MOBILEHQ | EMilitarySymbolIcon.RELAY);
				spawnPoint = SCR_SpawnPoint.Cast(m_MobileAssembly.GetOwner());
				m_wAntennaImg.SetVisible(false);
				m_wImageOverlay.SetWidthOverride(m_iDefRelaySize);
				m_wImageOverlay.SetHeightOverride(m_iDefRelaySize);
				break;
			}
			default:
			{
				spawnPoint = m_Base.GetSpawnPoint();
				if (strs.Get(0) != "Unknown")
				{
					if (!m_Base.IsHQRadioTrafficPossible(m_Base.GetFaction(), SCR_ECampaignHQRadioComms.BOTH_WAYS) && m_Base.GetFaction() == m_PlayerFaction)
					{
						m_wAntennaImg.SetVisible(true);
					}
					else
					{
						m_wAntennaImg.SetVisible(false);
					}
				}
				if (strs.Get(2) == "Small")
				{
					m_wImageOverlay.SetWidthOverride(38);
					m_wImageOverlay.SetHeightOverride(38);
				}
			}
		}

		baseIcon.SetDimension(2);
		m_SymbolUI.Update(baseIcon);

		m_wHighlightImg.LoadImageFromSet(0, m_sImageSetARO, highlight);
		m_wSelectImg.LoadImageFromSet(0, m_sImageSetARO, selection);

		if (!spawnPoint)
			return;

		m_SpawnPoint = spawnPoint;
		m_bCanRespawn = (m_PlayerFaction && m_PlayerFaction.GetFactionKey() == spawnPoint.GetFactionKey());// && SCR_SelectSpawnPointSubMenu.GetInstance() != null); note@lk: why

		if (m_bCanRespawn)
		{
			if (strs.Get(1) == "Mobile")
				baseIcon.SetIcons(EMilitarySymbolIcon.RESPAWN | EMilitarySymbolIcon.MOBILEHQ);
			else
				baseIcon.SetIcons(EMilitarySymbolIcon.RESPAWN);
		}

		m_SymbolUI.Update(baseIcon);
	}

	override RplId GetSpawnPointId()
	{
		if (m_SpawnPoint)
			return m_SpawnPoint.GetRplId();
		return RplId.Invalid();
	}

	//------------------------------------------------------------------------------	
	void SetAntennaIconVisible(bool visible)
	{
		m_wAntennaImg.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignMapUIBase()
	{
		s_SelectedElement = null;
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
	}
};
