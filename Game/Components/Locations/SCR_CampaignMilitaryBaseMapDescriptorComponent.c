//------------------------------------------------------------------------------------------------
class SCR_CampaignMilitaryBaseMapDescriptorComponentClass : SCR_MilitaryBaseMapDescriptorComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignMilitaryBaseMapDescriptorComponent : SCR_MilitaryBaseMapDescriptorComponent
{
	protected ref array<MapLink> m_aMapLinks = {};
	
	SCR_CampaignMilitaryBaseComponent m_Base;
	
	protected bool m_bIsHovered;
	
	//------------------------------------------------------------------------------------------------
	void SetParentBase(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		m_Base = base;
	}
	
	//------------------------------------------------------------------------------------------------
	void MapSetup(notnull Faction faction)
	{
		MapItem item = Item();
		Faction baseFaction = m_Base.GetFaction();
		
		if (m_Base.IsHQ() && faction != baseFaction)
		{
			item.SetVisible(false);
			return;
		}
		else
		{
			item.SetVisible(true);
			item.SetDisplayName(m_Base.GetBaseName());
			
			MapDescriptorProps props = item.GetProps();
			props.SetDetail(96);
			
			Color rangeColor;
			
			if (baseFaction)
				rangeColor = baseFaction.GetFactionColor();
			else
				rangeColor = Color(1, 1, 1, 1);
			
			props.SetOutlineColor(rangeColor);
			rangeColor.SetA(0.1);
			props.SetBackgroundColor(rangeColor);
			
			item.SetProps(props);
			item.SetRange(0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnIconHovered(SCR_CampaignMapUIBase icon, bool hovering)
	{
		m_bIsHovered = hovering;
		
		for (int i = m_aMapLinks.Count() - 1; i >= 0; i--)
		{
			if (!m_aMapLinks[i])
				continue;
			
			ColorMapLink(m_aMapLinks[i]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterMapLink(notnull MapLink link)
	{
		m_aMapLinks.RemoveItem(link);
	}
	
	//------------------------------------------------------------------------------------------------
	bool FindMapLink(MapDescriptorComponent owner, MapDescriptorComponent target)
	{
		for (int i = m_aMapLinks.Count() - 1; i >= 0; i--)
		{
			if(!m_aMapLinks[i])
				return false;
			if ((m_aMapLinks[i].Owner().Descriptor() == owner && m_aMapLinks[i].Target().Descriptor() == target) || (m_aMapLinks[i].Owner().Descriptor() == target && m_aMapLinks[i].Target().Descriptor() == owner))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterMapLink(notnull MapLink link)
	{
		m_aMapLinks.Insert(link);
	}
	
	//------------------------------------------------------------------------------------------------
	void ColorMapLink(notnull MapLink link)
	{
		MapLinkProps props = link.GetMapLinkProps();
		
		if (!props)
			return;
		
		props.SetLineWidth(m_Base.GetLineWidth());
		
		Color c = props.GetLineColor();
		
		if (!c)
			return;
		
		SCR_GraphLinesData linesData = m_Base.GetGraphLinesData();
		
		if (!linesData)
			return;
		
		if (m_bIsHovered)
		{
			// Highlight only bases in range
			MapItem otherEnd = link.Target();
			
			if (otherEnd == Item())
				otherEnd = link.Owner();
			
			IEntity otherEndOwner = otherEnd.Entity();
			
			if (!otherEndOwner)
				return;
			
			SCR_CampaignMilitaryBaseComponent otherBase = SCR_CampaignMilitaryBaseComponent.Cast(otherEndOwner.FindComponent(SCR_CampaignMilitaryBaseComponent));
			SCR_CampaignMobileAssemblyStandaloneComponent mobileHQ;
			
			if (!otherBase)
			{
				SCR_SpawnPoint spawnpoint = SCR_SpawnPoint.Cast(otherEndOwner);
				
				if (spawnpoint)
				{
					SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetPlayerFaction(GetGame().GetPlayerController().GetPlayerId()));
					SCR_CampaignMobileAssemblyStandaloneComponent factionMHQ = faction.GetMobileAssembly();
					
					if (factionMHQ && factionMHQ.GetOwner() == spawnpoint)
						mobileHQ = factionMHQ;
				}
				
				if (!mobileHQ)
					return;
			}
			
			if ((mobileHQ && m_Base.CanReachByRadio(mobileHQ)) || (otherBase && m_Base.CanReachByRadio(otherBase)))
				c.SetA(linesData.GetHighlightedAlpha());
			else
				return;
		}
		else
		{
			c.SetA(linesData.GetDefaultAlpha());
		}
		
		props.SetLineColor(c);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterMyMapLinks(bool unlinkHQ = false)
	{
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		
		for (int i = m_aMapLinks.Count() - 1; i >= 0; i--)
		{
			if (!m_aMapLinks.IsIndexValid(i))
				continue;
			
			MapLink link = m_aMapLinks[i];
			
			if (!link)
				continue;
			
			IEntity otherBaseOwner;
			
			if (link.Owner().Entity() == m_Base.GetOwner())
				otherBaseOwner = link.Target().Entity();
			else
				otherBaseOwner = link.Owner().Entity();
		
			if (!otherBaseOwner || (unlinkHQ && SCR_SpawnPoint.Cast(otherBaseOwner) != null))
			{
				link.Target().UnLink(Item());
				Item().UnLink(link.Target());
				UnregisterMapLink(link);
				continue;
			}
			
			SCR_CampaignMilitaryBaseComponent otherBase = SCR_CampaignMilitaryBaseComponent.Cast(otherBaseOwner.FindComponent(SCR_CampaignMilitaryBaseComponent));
			
			if (otherBase && (!otherBase.CanReachByRadio(m_Base) || m_Base.GetHQRadioCoverage(playerFaction) == SCR_ECampaignHQRadioComms.NONE))
			{
				otherBase.GetMapDescriptor().UnregisterMapLink(link);
				UnregisterMapLink(link);
				Item().UnLink(otherBase.GetMapDescriptor().Item());
				otherBase.GetMapDescriptor().Item().UnLink(Item());
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void HandleMapLinks(bool unlinkHQ = false)
	{
		if (m_Base.IsHQ() && m_Base.GetFaction() != SCR_FactionManager.SGetLocalPlayerFaction())
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;

		UnregisterMyMapLinks(unlinkHQ);
		SCR_CampaignFaction localPlayerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		
		if (!localPlayerFaction)
			return;
		
		SCR_CampaignMobileAssemblyStandaloneComponent mobileHQ = localPlayerFaction.GetMobileAssembly();
		MapItem mobilehqMapItem;
		
		if (mobileHQ)
		{
			SCR_SpawnPoint spawnpoint = SCR_SpawnPoint.Cast(mobileHQ.GetOwner());
			
			if (spawnpoint)
			{
				SCR_MapDescriptorComponent desc = SCR_MapDescriptorComponent.Cast(spawnpoint.FindComponent(SCR_MapDescriptorComponent));
				
				if (desc && mobileHQ.IsInRadioRange())
				{
					array<SCR_CampaignMilitaryBaseComponent> basesInRangeOfMobileHQ = {};
					mobileHQ.GetBasesInRange(basesInRangeOfMobileHQ);
					
					if (basesInRangeOfMobileHQ.Contains(m_Base))
						mobilehqMapItem = desc.Item();
				}
			}
		}
		
		SCR_CampaignFaction faction = m_Base.GetCampaignFaction();
		
		if (!mobilehqMapItem)
		{
			if (!m_Base.IsHQRadioTrafficPossible(localPlayerFaction) || localPlayerFaction != faction)
				return;
		}
		
		CreateLinks(faction, localPlayerFaction, mobilehqMapItem);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateLinks(Faction myFaction, Faction localPlayerFaction, MapItem mobilehq = null)
	{
		if (mobilehq)
		{
			MapLink link = Item().LinkTo(mobilehq);
			ColorMapLink(link);
			RegisterMapLink(link);
			SCR_CampaignMobileAssemblyStandaloneComponent comp = SCR_CampaignFaction.Cast(localPlayerFaction).GetMobileAssembly();
			
			if (comp)
				comp.AddMapLink(link);
		}
		
		array<SCR_CampaignMilitaryBaseComponent> bases = {};
		m_Base.GetBasesInRange(SCR_ECampaignBaseType.BASE | SCR_ECampaignBaseType.RELAY, bases);
		
		for (int i = bases.Count() - 1; i >= 0; i--)
		{
			if (bases[i].IsHQ() && bases[i].GetFaction() != localPlayerFaction)
				continue;
			
			if (FindMapLink(this, bases[i].GetMapDescriptor()))
				continue;
			
			if (myFaction != localPlayerFaction)
				continue;
			
			MapItem otherMapItem = bases[i].GetMapDescriptor().Item();
			
			if (!otherMapItem)
				continue;
			
			MapLink link = Item().LinkTo(otherMapItem);
			ColorMapLink(link);
			RegisterMapLink(link);
			bases[i].GetMapDescriptor().RegisterMapLink(link);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Shows info about the base in the map
	void HandleMapInfo(SCR_CampaignFaction playerFactionCampaign = null)
	{
		SCR_GameModeCampaign campaignGameMode = SCR_GameModeCampaign.GetInstance();

		if (!campaignGameMode)
			return;
		
		if (!playerFactionCampaign)
			playerFactionCampaign = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		
		if (!playerFactionCampaign)
			playerFactionCampaign = campaignGameMode.GetBaseManager().GetLocalPlayerFaction();

		if (!playerFactionCampaign)
			return;

		if (m_Base.IsHQ() && m_Base.GetFaction() != playerFactionCampaign)
			return;
		
		// Set callsign based on player's faction
		if (m_Base.GetType() != SCR_ECampaignBaseType.RELAY && m_Base.GetCallsignDisplayName().IsEmpty())
			m_Base.SetCallsign(playerFactionCampaign);
		
		SCR_CampaignMapUIBase mapUI = m_Base.GetMapUI();
		
		if (mapUI)
			mapUI.SetIconInfoText();

		// Update base icon color
		EFactionMapID factionMapID = EFactionMapID.UNKNOWN;
		bool isInRange = m_Base.IsHQRadioTrafficPossible(playerFactionCampaign);

		// Show proper faction color only for HQs or bases within radio signal
		if (m_Base.GetFaction() && (m_Base.IsHQ() || isInRange))
		{
			switch (m_Base.GetFaction().GetFactionKey())
			{
				case campaignGameMode.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR): {factionMapID = EFactionMapID.EAST; break;};
				case campaignGameMode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR): {factionMapID = EFactionMapID.WEST; break;};
				case campaignGameMode.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR): {factionMapID = EFactionMapID.FIA; break;};
			}
		}
		
		Item().SetFactionIndex(factionMapID);

		array<SCR_ServicePointDelegateComponent> delegates = {};
		m_Base.GetServiceDelegates(delegates);
		
		foreach (SCR_ServicePointDelegateComponent delegate: delegates)
		{
			IEntity owner = delegate.GetOwner();
			
			if (!owner)
				continue;
			
			SCR_ServicePointMapDescriptorComponent comp = SCR_ServicePointMapDescriptorComponent.Cast(owner.FindComponent(SCR_ServicePointMapDescriptorComponent));

			if (comp)
			{
				if (isInRange)
					comp.SetServiceMarker(m_Base.GetCampaignFaction());
				else
					comp.SetServiceMarker(visible: false);
			}
		}

		if (mapUI)
			mapUI.UpdateBaseIcon(factionMapID);
		
		HandleMapLinks();
	}
};
