//------------------------------------------------------------------------------------------------
class SCR_CampaignMobileAssemblyStandaloneComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_CampaignMobileAssemblyStandaloneComponent : ScriptComponent
{
	static ref ScriptInvokerVoid s_OnSpawnPointOwnerChanged = new ScriptInvokerVoid();
	static ref ScriptInvokerVoid s_OnUpdateRespawnCooldown = new ScriptInvokerVoid();

	static const int RESPAWN_COOLDOWN = 60000;

	protected RplComponent m_RplComponent;

	protected MapItem m_MapItem;

	protected IEntity m_Vehicle;

	protected SCR_CampaignFaction m_ParentFaction;
	protected SCR_CampaignFaction m_Faction;

	protected bool m_bIsInRadioRange;
	protected bool m_bCooldownDone = true;
	protected bool m_bIsHovered;

	protected ref array<MapLink> m_aMapLinks = {};
	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aBasesInRadioRange = {};
	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aBasesInRadioRangeOld = {};

	[RplProp(onRplName: "OnFactionChanged")]
	protected int m_iFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;

	[RplProp(onRplName: "OnParentFactionIDSet")]
	protected int m_iParentFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;

	[RplProp()]
	#ifndef AR_CAMPAIGN_TIMESTAMP
	protected float m_fRespawnAvailableSince = float.MAX;
	#else
	protected WorldTimestamp m_fRespawnAvailableSince;
	#endif
	[RplProp()]
	protected int m_iBasesCoveredByMHQOnly;

	[RplProp()]
	protected int m_iRadioRange;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		super.OnPostInit(owner);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		GetGame().GetCallqueue().CallLater(UpdateRadioCoverage, 1000, true);
		GetGame().GetCallqueue().CallLater(UpdateRespawnCooldown, 250, true, null);

		if (IsProxy())
		{
			campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Insert(OnParentFactionIDSet);
		}
		else
		{
			// Only update respawn cooldown if a load state has not been applied
			#ifndef AR_CAMPAIGN_TIMESTAMP
			if (GetGame().GetWorld().GetWorldTime() > 10000)
			{
				m_fRespawnAvailableSince = Replication.Time() + RESPAWN_COOLDOWN;
				m_bCooldownDone = false;
			}
			else
			{
				m_fRespawnAvailableSince = Replication.Time();
				m_bCooldownDone = true;
			}
			#else
			ChimeraWorld world = GetGame().GetWorld();
			if (world.GetWorldTime() > 10000)
			{
				m_fRespawnAvailableSince = world.GetServerTimestamp().PlusMilliseconds(RESPAWN_COOLDOWN);
				m_bCooldownDone = false;
			}
			else
			{
				m_fRespawnAvailableSince = world.GetServerTimestamp();
				m_bCooldownDone = true;
			}
			#endif
			Replication.BumpMe();
		}

		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		// Delay so map links can operate with properly loaded (or deleted) spawnpoint
		GetGame().GetCallqueue().CallLater(HandleMapLinks, 1000, false, false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		GetGame().GetCallqueue().Remove(UpdateRadioCoverage);
		GetGame().GetCallqueue().Remove(UpdateRespawnCooldown);

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			m_aBasesInRadioRangeOld.Copy(m_aBasesInRadioRange);
			m_aBasesInRadioRange.Clear();
			HandleMapLinks(true);
		}

		campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Remove(OnParentFactionIDSet);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateBasesInRadioRange()
	{
		array<SCR_MilitaryBaseComponent> bases = {};
		SCR_MilitaryBaseManager.GetInstance().GetBases(bases);

		int radioRange = m_iRadioRange * m_iRadioRange;	// We're checking square distance
		vector truckPosition = GetOwner().GetOrigin();
		SCR_CampaignMilitaryBaseComponent campaignBase;

		m_aBasesInRadioRange = {};

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase || !campaignBase.IsInitialized())
				continue;

			if (campaignBase.IsHQ() && campaignBase.GetFaction() != m_ParentFaction)
				continue;

			if (vector.DistanceSqXZ(truckPosition, campaignBase.GetOwner().GetOrigin()) < radioRange)
				m_aBasesInRadioRange.Insert(campaignBase);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool CanReachByRadio(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		return m_aBasesInRadioRange.Contains(base);
	}

	//------------------------------------------------------------------------------------------------
	void OnIconHovered(SCR_CampaignMapUIBase icon, bool hovering)
	{
		m_bIsHovered = hovering;

		foreach (MapLink link : m_aMapLinks)
		{
			if (!link)
				continue;

			ColorMapLink(link);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ColorMapLink(notnull MapLink link)
	{
		if (!m_ParentFaction)
			return;

		MapLinkProps props = link.GetMapLinkProps();

		if (!props)
			return;

		SCR_CampaignMilitaryBaseComponent hq = m_ParentFaction.GetMainBase();

		if (!hq)
			return;

		props.SetLineWidth(hq.GetLineWidth());

		Color c = props.GetLineColor();

		if (!c)
			return;

		SCR_GraphLinesData linesData = hq.GetGraphLinesData();

		if (m_bIsHovered)
		{
			// Highlight only bases in range
			MapItem otherEnd = link.Target();

			if (otherEnd == m_MapItem)
				otherEnd = link.Owner();

			IEntity otherEndOwner = otherEnd.Entity();

			if (!otherEndOwner)
				return;

			SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(otherEndOwner.FindComponent(SCR_CampaignMilitaryBaseComponent));

			if (!base)
				return;

			if (!m_aBasesInRadioRange.Contains(base))
				return;

			c.SetA(linesData.GetHighlightedAlpha());
		}
		else
		{
			c.SetA(linesData.GetDefaultAlpha());
		}

		props.SetLineColor(c);
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleMapLinks(bool onDelete = false)
	{
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBasesInRadioRangeOld)
		{
			if (!m_aBasesInRadioRange.Contains(base))
				base.GetMapDescriptor().HandleMapLinks(onDelete);
		}

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBasesInRadioRange)
		{
			base.GetMapDescriptor().HandleMapLinks();
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddMapLink(notnull MapLink link)
	{
		m_aMapLinks.Insert(link);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	void SetVehicle(IEntity vehicle)
	{
		m_Vehicle = vehicle;
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetVehicle()
	{
		return m_Vehicle;
	}

	//------------------------------------------------------------------------------------------------
	void SetParentFactionID(int factionID)
	{
		m_iParentFaction = factionID;
		Replication.BumpMe();
		OnParentFactionIDSet();
	}

	//------------------------------------------------------------------------------------------------
	void OnParentFactionIDSet()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		m_ParentFaction = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(m_iParentFaction));

		if (!m_ParentFaction)
			return;

		m_ParentFaction.SetMobileAssembly(this);

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			SCR_MapDescriptorComponent mapDescriptor = SCR_MapDescriptorComponent.Cast(GetOwner().FindComponent(SCR_MapDescriptorComponent));

			if (mapDescriptor)
				m_MapItem = mapDescriptor.Item();

			if (!m_MapItem || m_ParentFaction != playerFaction)
				return;

			m_MapItem.SetVisible(true);

			switch (m_ParentFaction.GetFactionKey())
			{
				case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR):
				{
					m_MapItem.SetFactionIndex(EFactionMapID.WEST);
					break;
				}

				case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR):
				{
					m_MapItem.SetFactionIndex(EFactionMapID.EAST);
					break;
				}

				default:
				{
					m_MapItem.SetFactionIndex(EFactionMapID.UNKNOWN);
				}
			}
		}

		UpdateBasesInRadioRange();
		UpdateRadioCoverage();

		if (m_MapItem)
			HandleMapLinks();

		if (!IsProxy())
			campaign.GetBaseManager().RecalculateRadioCoverage(m_ParentFaction);
	}

	//------------------------------------------------------------------------------------------------
	int GetParentFactionID()
	{
		return m_iParentFaction;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetParentFaction()
	{
		return m_ParentFaction;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetFaction()
	{
		return m_Faction;
	}

	//------------------------------------------------------------------------------------------------
	int GetBasesInRange(notnull out array<SCR_CampaignMilitaryBaseComponent> basesInRange)
	{
		return basesInRange.Copy(m_aBasesInRadioRange);
	}

	//------------------------------------------------------------------------------------------------
	void SetRadioRange(int range)
	{
		m_iRadioRange = range;
	}

	//------------------------------------------------------------------------------------------------
	int GetRadioRange()
	{
		return m_iRadioRange;
	}

	//------------------------------------------------------------------------------------------------
	void OnFactionChanged()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!fManager)
			return;

		FactionKey fKey;
		Faction f = fManager.GetFactionByIndex(m_iFaction);

		if (f)
		{
			fKey = f.GetFactionKey();
			m_Faction = SCR_CampaignFaction.Cast(f);
		}

		SCR_SpawnPoint.Cast(GetOwner()).SetFaction(f);
		s_OnSpawnPointOwnerChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	bool IsInRadioRange()
	{
		return m_bIsInRadioRange;
	}

	//------------------------------------------------------------------------------------------------
	MapItem GetMapItem()
	{
		return m_MapItem;
	}

	//------------------------------------------------------------------------------------------------
	void SetCountOfExclusivelyLinkedBases(int cnt)
	{
		m_iBasesCoveredByMHQOnly = cnt;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	int GetCountOfExclusivelyLinkedBases()
	{
		return m_iBasesCoveredByMHQOnly;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateRadioCoverage()
	{
		if (!m_ParentFaction)
			return;

		bool inRangeNow = SCR_GameModeCampaign.GetInstance().GetBaseManager().IsEntityInFactionRadioSignal(GetOwner(), m_ParentFaction);
		bool refreshLinks = inRangeNow != m_bIsInRadioRange;
		m_bIsInRadioRange = inRangeNow;

		if (refreshLinks && RplSession.Mode() != RplMode.Dedicated)
		{
			foreach (SCR_CampaignMilitaryBaseComponent base : m_aBasesInRadioRange)
			{
				base.GetMapDescriptor().HandleMapLinks();
			}
		}

		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));

		if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
		{
			if (m_MapItem)
				m_MapItem.SetVisible(false);

			if (!IsProxy())
			{
				IEntity vehicle = GetOwner().GetParent();

				if (vehicle)
				{
					BaseRadioComponent radioComponent = BaseRadioComponent.Cast(vehicle.FindComponent(BaseRadioComponent));

					if (radioComponent)
						radioComponent.SetPower(false);
				}
			}
		}

		if (IsProxy())
			return;

		if (!m_bIsInRadioRange && m_iFaction != SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX)
		{
			m_iFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;
			Replication.BumpMe();
			OnFactionChanged();
		}

		if (m_bIsInRadioRange && m_iFaction == SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX && m_bCooldownDone)
		{
			m_iFaction = m_iParentFaction;
			Replication.BumpMe();
			OnFactionChanged();
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdateRespawnCooldown(Widget w = null)
	{
		if (!m_ParentFaction)
			return;

		SCR_CampaignMilitaryBaseComponent hq = m_ParentFaction.GetMainBase();

		TextWidget respawn;
		ImageWidget respawnImg;

		ResourceName imageset = hq.GetBuildingIconImageset();

		if (w && imageset)
		{
			respawn = TextWidget.Cast(w.FindAnyWidget("Respawn"));
			respawnImg = ImageWidget.Cast(w.FindAnyWidget("RespawnIMG"));
			respawnImg.LoadImageFromSet(0, imageset, "RespawnBig");
		}

		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (m_fRespawnAvailableSince > Replication.Time())
		#else
		ChimeraWorld world = GetOwner().GetWorld();
		if (m_fRespawnAvailableSince.Greater(world.GetServerTimestamp()))
		#endif
		{
			if (m_bCooldownDone)
				m_bCooldownDone = false;

			if (RplSession.Mode() != RplMode.Dedicated)
			{
				if (!w)
					s_OnUpdateRespawnCooldown.Invoke();
				else
				{
					respawn.SetVisible(true);
					respawnImg.SetVisible(true);

					#ifndef AR_CAMPAIGN_TIMESTAMP
					float respawnCooldown = Math.Ceil((m_fRespawnAvailableSince - Replication.Time()) * 0.001);
					#else
					float respawnCooldown = Math.Ceil(m_fRespawnAvailableSince.DiffMilliseconds(world.GetServerTimestamp()) * 0.001);
					#endif
					int d, h, m, s;
					string sStr;
					SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(respawnCooldown, d, h, m, s);
					sStr = s.ToString();

					if (s < 10)
						sStr = "0" + sStr;

					respawn.SetTextFormat("#AR-Campaign_MobileAssemblyCooldown", m, sStr);
				}
			}
		}
		else
		{
			GetGame().GetCallqueue().Remove(UpdateRespawnCooldown);

			if (w)
			{
				if (RplSession.Mode() != RplMode.Dedicated)
				{
					respawn.SetVisible(false);
					respawnImg.SetVisible(false);
				}
			}
			else
			{
				if (!m_bCooldownDone)
				{
					m_bCooldownDone = true;
					s_OnUpdateRespawnCooldown.Invoke();

					if (!IsProxy() && IsInRadioRange())
					{
						m_iFaction = m_iParentFaction;
						Replication.BumpMe();
						OnFactionChanged();
					}
				}
			}
		}
	}
}
