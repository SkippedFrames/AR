#include "scripts/Game/config.c"
void OnSignalChangedDelegate(SCR_CampaignMilitaryBaseComponent base);
void OnAllBasesInitializedDelegate();
void OnLocalPlayerEnteredBaseDelegate(SCR_CampaignMilitaryBaseComponent base);
void OnLocalPlayerLeftBaseDelegate(SCR_CampaignMilitaryBaseComponent base);
void OnLocalFactionCapturedBaseDelegate();

typedef func OnSignalChangedDelegate;
typedef func OnAllBasesInitializedDelegate;
typedef func OnLocalPlayerEnteredBaseDelegate;
typedef func OnLocalPlayerLeftBaseDelegate;
typedef func OnLocalFactionCapturedBaseDelegate;

typedef ScriptInvokerBase<OnSignalChangedDelegate> OnSignalChangedInvoker;
typedef ScriptInvokerBase<OnAllBasesInitializedDelegate> OnAllBasesInitializedInvoker;
typedef ScriptInvokerBase<OnLocalPlayerEnteredBaseDelegate> OnLocalPlayerEnteredBaseInvoker;
typedef ScriptInvokerBase<OnLocalPlayerLeftBaseDelegate> OnLocalPlayerLeftBaseInvoker;
typedef ScriptInvokerBase<OnLocalFactionCapturedBaseDelegate> OnLocalFactionCapturedBaseInvoker;

//------------------------------------------------------------------------------------------------
//! Created in SCR_GameModeCampaign
class SCR_CampaignMilitaryBaseManager
{
	protected static const int PARENT_BASE_DISTANCE_THRESHOLD = 300;			//AI patrols closer than this to a base will couterattack
	protected static const int HQ_NO_REMNANTS_RADIUS = 300;						//AI patrols closer than this to main HQs will be removed
	protected static const int HQ_NO_REMNANTS_PATROL_RADIUS = 600;				//AI patrols with a waypoint which is closer than this to main HQs will be removed
	protected static const int MAX_HQ_SELECTION_ITERATIONS = 20;
	protected static const int DEPOT_PLAYER_PRESENCE_CHECK_INTERVAL = 2000;		//ms
	protected static const float CP_AVG_DISTANCE_TOLERANCE = 0.25;				//highest relative distance tolerance to control points when evaluating main HQs
	protected static const string ICON_NAME_SUPPLIES = "Slot_Supplies";
	protected static const float MAX_DIST_TO_BASE = 300;
	protected static const float PARKED_LIFETIME = 3600;

	protected SCR_GameModeCampaign m_Campaign;

	protected SCR_CampaignFaction m_LocalPlayerFaction;

	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aBases = {};
	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aControlPoints = {};
	protected ref array<SCR_CampaignSuppliesComponent> m_aRemnantSupplyDepots = {};

	protected ref OnSignalChangedInvoker m_OnSignalChanged;
	protected ref OnAllBasesInitializedInvoker m_OnAllBasesInitialized;
	protected ref OnLocalPlayerEnteredBaseInvoker m_OnLocalPlayerEnteredBase;
	protected ref OnLocalPlayerLeftBaseInvoker m_OnLocalPlayerLeftBase;

	protected int m_iActiveBases;
	protected int m_iTargetActiveBases;

	protected bool m_bAllBasesInitialized;

	//------------------------------------------------------------------------------------------------
	//! Bases which have been initialized
	int GetActiveBasesCount()
	{
		return m_iActiveBases;
	}

	//------------------------------------------------------------------------------------------------
	//! Total bases expected to be initialized
	int GetTargetActiveBasesCount()
	{
		return m_iTargetActiveBases;
	}

	//------------------------------------------------------------------------------------------------
	void AddActiveBase()
	{
		m_iActiveBases++;

		if (m_iActiveBases == m_iTargetActiveBases)
			OnAllBasesInitialized();
	}

	//------------------------------------------------------------------------------------------------
	void AddTargetActiveBase()
	{
		m_iTargetActiveBases++;
	}

	//------------------------------------------------------------------------------------------------
	void SetTargetActiveBasesCount(int count)
	{
		m_iTargetActiveBases = count;
	}

	//------------------------------------------------------------------------------------------------
	OnLocalPlayerEnteredBaseInvoker GetOnLocalPlayerEnteredBase()
	{
		if (!m_OnLocalPlayerEnteredBase)
			m_OnLocalPlayerEnteredBase = new OnLocalPlayerEnteredBaseInvoker();

		return m_OnLocalPlayerEnteredBase;
	}

	//------------------------------------------------------------------------------------------------
	OnLocalPlayerLeftBaseInvoker GetOnLocalPlayerLeftBase()
	{
		if (!m_OnLocalPlayerLeftBase)
			m_OnLocalPlayerLeftBase = new OnLocalPlayerLeftBaseInvoker();

		return m_OnLocalPlayerLeftBase;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when all bases have been successfully initialized
	OnAllBasesInitializedInvoker GetOnAllBasesInitialized()
	{
		if (!m_OnAllBasesInitialized)
			m_OnAllBasesInitialized = new OnAllBasesInitializedInvoker();

		return m_OnAllBasesInitialized;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when a base's radio coverage changes
	OnSignalChangedInvoker GetOnSignalChanged()
	{
		if (!m_OnSignalChanged)
			m_OnSignalChanged = new OnSignalChangedInvoker();

		return m_OnSignalChanged;
	}

	//------------------------------------------------------------------------------------------------
	void OnAllBasesInitialized()
	{
		m_bAllBasesInitialized = true;

		// On server, this is done in gamemode class Start() method
		if (m_Campaign.IsProxy())
			UpdateBases();

		if (m_OnAllBasesInitialized)
			m_OnAllBasesInitialized.Invoke();

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			InitializeSupplyDepotIcons();
			HideUnusedBaseIcons();
		}

		if (m_Campaign.IsProxy())
			return;

		SCR_MilitaryBaseManager.GetInstance().GetOnLogicRegisteredInBase().Insert(DisableExtraSeizingComponents);
		ProcessRemnantsPresence();
		RecalculateRadioCoverage(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
		RecalculateRadioCoverage(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
	}

	//------------------------------------------------------------------------------------------------
	bool IsBasesInitDone()
	{
		return m_bAllBasesInitialized;
	}

	//------------------------------------------------------------------------------------------------
	protected void DisableExtraSeizingComponents(SCR_MilitaryBaseComponent base, SCR_MilitaryBaseLogicComponent logic)
	{
		if (logic.Type() == SCR_SeizingComponent)
			SCR_SeizingComponent.Cast(logic).Disable();
	}

	//------------------------------------------------------------------------------------------------
	void SetLocalPlayerFaction(notnull SCR_CampaignFaction faction)
	{
		m_LocalPlayerFaction = faction;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetLocalPlayerFaction()
	{
		return m_LocalPlayerFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! Update the list of Conflict bases
	int UpdateBases()
	{
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();
		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);

		m_aBases.Clear();
		m_aControlPoints.Clear();
		int count;

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase)
				continue;

			m_aBases.Insert(campaignBase);
			count++;

			if (campaignBase.IsControlPoint())
				m_aControlPoints.Insert(campaignBase);
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Picks Main Operating Bases from a list of candidates by checking average distance to active control points
	void SelectHQs(notnull array<SCR_CampaignMilitaryBaseComponent> candidates, notnull array<SCR_CampaignMilitaryBaseComponent> controlPoints, out notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		// Pick the same HQs every time when debugging
#ifdef TDM_CLI_SELECTION
		SelectHQsSimple(candidates, selectedHQs);
		return;
#endif

		int candidatesCount = candidates.Count();

		// If only two HQs are set up, don't waste time with processing
		if (candidatesCount == 2)
		{
			SelectHQsSimple(candidates, selectedHQs);
			return;
		}

		int iterations;
		int totalBasesDistance;
		SCR_CampaignMilitaryBaseComponent bluforHQ;
		SCR_CampaignMilitaryBaseComponent opforHQ;
		array<SCR_CampaignMilitaryBaseComponent> eligibleForHQ;
		vector bluforHQPos;
		int averageHQDistance;
		int averageCPDistance;
		int acceptedCPDistanceDiff;

		while (!opforHQ && iterations < MAX_HQ_SELECTION_ITERATIONS)
		{
			iterations++;
			totalBasesDistance = 0;
			eligibleForHQ = {};

			// Pick one of the HQs at random
			Math.Randomize(-1);
			bluforHQ = candidates.GetRandomElement();
			bluforHQPos = bluforHQ.GetOwner().GetOrigin();

			// Calculate average distance between our HQ and others
			foreach (SCR_CampaignMilitaryBaseComponent otherHQ : candidates)
			{
				if (otherHQ == bluforHQ)
					continue;

				totalBasesDistance += vector.DistanceSqXZ(bluforHQPos, otherHQ.GetOwner().GetOrigin());
			}

			averageHQDistance = totalBasesDistance / (candidatesCount - 1);	// Our HQ is subtracted
			averageCPDistance = GetAvgCPDistanceSq(bluforHQ, controlPoints);
			acceptedCPDistanceDiff = averageCPDistance * CP_AVG_DISTANCE_TOLERANCE;

			foreach (SCR_CampaignMilitaryBaseComponent candidate : candidates)
			{
				if (candidate == bluforHQ)
					continue;

				// Ignore HQs closer than the average distance
				if (vector.DistanceSqXZ(bluforHQPos, candidate.GetOwner().GetOrigin()) < averageHQDistance)
					continue;

				// Ignore HQs too far from control points (relative to our HQ)
				if (Math.AbsInt(averageCPDistance - GetAvgCPDistanceSq(candidate, controlPoints)) > acceptedCPDistanceDiff)
					continue;

				eligibleForHQ.Insert(candidate);
			}

			// No HQs fit the condition, restart loop
			if (eligibleForHQ.Count() == 0)
				continue;

			Math.Randomize(-1);
			opforHQ = eligibleForHQ.GetRandomElement();
		}

		// Selection failed, use the simplified but reliable one
		if (!opforHQ)
		{
			SelectHQsSimple(candidates, selectedHQs);
			return;
		}

		// Randomly assign the factions in reverse in case primary selection gets too limited
		if (Math.RandomFloat01() >= 0.5)
			selectedHQs = {bluforHQ, opforHQ};
		else
			selectedHQs = {opforHQ, bluforHQ};
	}

	//------------------------------------------------------------------------------------------------
	//! If there are only two candidates for main HQ or the main process fails, HQs are selected simply and cheaply
	protected void SelectHQsSimple(notnull array<SCR_CampaignMilitaryBaseComponent> candidates, out notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		// Pick the same HQs every time when debugging
#ifdef TDM_CLI_SELECTION
		selectedHQs = {candidates[0], candidates[1]};
		return;
#endif

		// In Tutorial mode, we always want to use the same HQs
		if (m_Campaign.IsTutorial())
		{
			if (candidates[0].GetFaction(true) == m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
				selectedHQs = {candidates[0], candidates[1]};
			else
				selectedHQs = {candidates[1], candidates[0]};

			return;
		}

		Math.Randomize(-1);
		SCR_CampaignMilitaryBaseComponent bluforHQ = candidates.GetRandomElement();
		candidates.RemoveItem(bluforHQ);
		SCR_CampaignMilitaryBaseComponent opforHQ = candidates.GetRandomElement();

		if (Math.RandomFloat01() >= 0.5)
			selectedHQs = {bluforHQ, opforHQ};
		else
			selectedHQs = {opforHQ, bluforHQ};
	}

	//------------------------------------------------------------------------------------------------
	void SetHQFactions(notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		SCR_CampaignFaction factionBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignFaction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);
		SCR_CampaignFaction factionINDFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR);

		if (selectedHQs[0].GetFaction() == selectedHQs[1].GetFaction())
		{
			// Preset owners are the same or null, assign new owners normally
			selectedHQs[0].SetFaction(factionBLUFOR);
			selectedHQs[1].SetFaction(factionOPFOR);
		}
		else
		{
			// Check if one of the preset owners is invalid, if yes, assign a new owner which is not assigned to the other HQ
			if (!selectedHQs[0].GetFaction() || selectedHQs[0].GetFaction() == factionINDFOR)
			{
				if (selectedHQs[1].GetFaction() == factionBLUFOR)
					selectedHQs[0].SetFaction(factionOPFOR);
				else
					selectedHQs[0].SetFaction(factionBLUFOR);
			}
			else if (!selectedHQs[1].GetFaction() || selectedHQs[1].GetFaction() == factionINDFOR)
			{
				if (selectedHQs[0].GetFaction() == factionBLUFOR)
					selectedHQs[1].SetFaction(factionOPFOR);
				else
					selectedHQs[1].SetFaction(factionBLUFOR);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns squared average distance to control points - used for starting HQ location calculations
	protected int GetAvgCPDistanceSq(notnull SCR_CampaignMilitaryBaseComponent HQ, notnull array<SCR_CampaignMilitaryBaseComponent> controlPoints)
	{
		int thresholdCP = m_Campaign.GetControlPointTreshold();

		// Avoid division by zero
		if (thresholdCP == 0)
			return 0;

		array<SCR_CampaignMilitaryBaseComponent> nearestControlPoints = {};

		int distanceToHQ;
		int controlPointsCount;
		int arrayIndex;
		int nearestControlPointsCount;

		vector HQPos = HQ.GetOwner().GetOrigin();

		foreach (SCR_CampaignMilitaryBaseComponent controlPoint : controlPoints)
		{
			if (!controlPoint)
				continue;

			distanceToHQ = vector.DistanceSqXZ(controlPoint.GetOwner().GetOrigin(), HQPos);
			controlPointsCount = nearestControlPoints.Count();
			arrayIndex = controlPointsCount;

			for (int i = 0; i < controlPointsCount; i++)
			{
				if (distanceToHQ < vector.DistanceSqXZ(HQPos, nearestControlPoints[i].GetOwner().GetOrigin()))
				{
					arrayIndex = i;
					break;
				}
			}

			nearestControlPointsCount = nearestControlPoints.InsertAt(controlPoint, arrayIndex);
		}

		// Avoid division by zero
		if (nearestControlPointsCount == 0)
			return 0;

		if (thresholdCP < nearestControlPointsCount)
			nearestControlPoints.Resize(thresholdCP);

		int totalDist;

		foreach (SCR_CampaignMilitaryBaseComponent controlPoint : nearestControlPoints)
		{
			totalDist += vector.DistanceSqXZ(HQPos, controlPoint.GetOwner().GetOrigin());
		}

		return totalDist / nearestControlPointsCount;
	}

	//------------------------------------------------------------------------------------------------
	void InitializeBases(notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs, bool randomizeSupplies)
	{
		array<SCR_CampaignMilitaryBaseComponent> basesSorted = {};
		SCR_CampaignMilitaryBaseComponent baseCheckedAgainst;
		vector originHQ1 = selectedHQs[0].GetOwner().GetOrigin();
		vector originHQ2 = selectedHQs[1].GetOwner().GetOrigin();
		float distanceToHQ;
		bool indexFound;
		int callsignIndex;
		array<int> allCallsignIndexes = {};
		array<int> callsignIndexesBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetBaseCallsignIndexes();
		SCR_CampaignFaction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);

		// Grab all valid base callsign indexes (if both factions have the index)
		foreach (int indexBLUFOR : callsignIndexesBLUFOR)
		{
			if (factionOPFOR.GetBaseCallsignByIndex(indexBLUFOR))
				allCallsignIndexes.Insert(indexBLUFOR);
		}

		int callsignsCount = allCallsignIndexes.Count();
		Math.Randomize(-1);
		Faction defaultFaction;
		BaseRadioComponent radio;
		BaseTransceiver tsv;

		foreach (int iBase, SCR_CampaignMilitaryBaseComponent campaignBase : m_aBases)
		{
			if (!campaignBase.IsInitialized())
				continue;

			defaultFaction = campaignBase.GetFaction(true);

			// Apply default faction set in FactionAffiliationComponent or INDFOR if undefined
			if (!campaignBase.GetFaction())
			{
				if (defaultFaction)
					campaignBase.SetFaction(defaultFaction);
				else
					campaignBase.SetFaction(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR));
			}

			// Register bases in range of each other
			campaignBase.UpdateBasesInRadioRange();

			// Assign callsign
			if (campaignBase.GetType() == SCR_ECampaignBaseType.BASE)
			{
				callsignIndex = allCallsignIndexes.GetRandomIndex();
				campaignBase.SetCallsignIndex(allCallsignIndexes[callsignIndex]);
				allCallsignIndexes.Remove(callsignIndex);
			}
			else
			{
				// Relays use a dummy callsign just so search by callsign is still possible
				campaignBase.SetCallsignIndex(callsignsCount + iBase);
			}

			// Sort bases by distance to a HQ so randomized supplies can be applied fairly (if enabled)
			if (randomizeSupplies && campaignBase.GetType() == SCR_ECampaignBaseType.BASE)
			{
				indexFound = false;
				distanceToHQ = vector.DistanceSqXZ(originHQ1, campaignBase.GetOwner().GetOrigin());

				for (int i = 0, count = basesSorted.Count(); i < count; i++)
				{
					baseCheckedAgainst = basesSorted[i];

					if (distanceToHQ < vector.DistanceSqXZ(originHQ1, baseCheckedAgainst.GetOwner().GetOrigin()))
					{
						basesSorted.InsertAt(campaignBase, i);
						indexFound = true;
						break;
					}
				}

				if (!indexFound)
					basesSorted.Insert(campaignBase);
			}
		}

		if (randomizeSupplies)
			AddRandomSupplies(basesSorted, selectedHQs);
	}

	//------------------------------------------------------------------------------------------------
	//! Add randomized supplies to each base, calculate batches so each side encounters similarly stacked bases
	void AddRandomSupplies(notnull array<SCR_CampaignMilitaryBaseComponent> basesSorted, notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		array<int> suppliesBufferBLUFOR = {};
		array<int> suppliesBufferOPFOR = {};
		int intervalMultiplier = Math.Floor((m_Campaign.GetMaxStartingSupplies() - m_Campaign.GetMinStartingSupplies()) / m_Campaign.GetStartingSuppliesInterval());
		FactionKey factionToProcess;
		vector basePosition;
		float distanceToHQ1;
		float distanceToHQ2;
		int suppliesToAdd;

		foreach (SCR_CampaignMilitaryBaseComponent base : basesSorted)
		{
			if (base.IsHQ())
				continue;

			basePosition = base.GetOwner().GetOrigin();
			distanceToHQ1 = vector.DistanceSq(basePosition, selectedHQs[0].GetOwner().GetOrigin());
			distanceToHQ2 = vector.DistanceSq(basePosition, selectedHQs[1].GetOwner().GetOrigin());

			if (distanceToHQ1 > distanceToHQ2)
				factionToProcess = selectedHQs[1].GetCampaignFaction().GetFactionKey();
			else
				factionToProcess = selectedHQs[0].GetCampaignFaction().GetFactionKey();

			// Check if we have preset supplies stored in buffer
			if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR) && !suppliesBufferBLUFOR.IsEmpty())
			{
				suppliesToAdd = suppliesBufferBLUFOR[0];
				suppliesBufferBLUFOR.RemoveOrdered(0);
			}
			else if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR) && !suppliesBufferOPFOR.IsEmpty())
			{
				suppliesToAdd = suppliesBufferOPFOR[0];
				suppliesBufferOPFOR.RemoveOrdered(0);
			}
			else
			{
				// Supplies from buffer not applied, add random amount, store to opposite faction's buffer
				suppliesToAdd = m_Campaign.GetMinStartingSupplies() + (m_Campaign.GetStartingSuppliesInterval() * Math.RandomIntInclusive(0, intervalMultiplier));

				if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR))
					suppliesBufferOPFOR.Insert(suppliesToAdd);
				else
					suppliesBufferBLUFOR.Insert(suppliesToAdd);
			}

			base.SetStartingSupplies(suppliesToAdd);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! On clients, the tasks are not properly synced upon deserialization due to delay in callsign assignment
	//! This method will update the task list with the proper bases
	void UpdateTaskBases(Faction assignedFaction)
	{
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (base.IsInitialized() && base.GetCallsignDisplayName().IsEmpty())
				base.GetMapDescriptor().HandleMapInfo();
		}

		SCR_BaseTaskManager taskManager = GetTaskManager();

		if (!taskManager)
			return;

		array<SCR_BaseTask> tasks = {};
		taskManager.GetFilteredTasks(tasks, assignedFaction);

		foreach (SCR_BaseTask task : tasks)
		{
			SCR_CampaignBaseTask conflictTask = SCR_CampaignBaseTask.Cast(task);

			if (!conflictTask)
				continue;

			int baseId = conflictTask.GetTargetBaseId();

			if (baseId == -1)
				continue;

			SCR_CampaignMilitaryBaseComponent base = FindBaseByCallsign(baseId);

			if (!base || base.GetFaction() == conflictTask.GetTargetFaction())
				continue;

			conflictTask.SetTargetBase(base);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Show icons only for supply depots close enough to an active base
	void InitializeSupplyDepotIcons()
	{
		IEntity depot;
		vector origin;
		MapItem item;
		SCR_CampaignMilitaryBaseComponent closestBase;
		MapDescriptorProps props;
		SCR_MapDescriptorComponent mapDescriptorComponent;
		int threshold = m_Campaign.GetSupplyDepotIconThreshold();
		Color colorFIA = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR).GetFactionColor();

		foreach (SCR_CampaignSuppliesComponent comp : m_aRemnantSupplyDepots)
		{
			depot = comp.GetOwner();

			if (!depot)
				continue;

			mapDescriptorComponent = SCR_MapDescriptorComponent.Cast(depot.FindComponent(SCR_MapDescriptorComponent));

			if (!mapDescriptorComponent)
				continue;

			item = mapDescriptorComponent.Item();
			origin = depot.GetOrigin();
			closestBase = FindClosestBase(origin);

			if (!closestBase)
				continue;

			if (vector.Distance(origin, closestBase.GetOwner().GetOrigin()) <= threshold)
			{
				item.SetVisible(true);
				item.SetImageDef(ICON_NAME_SUPPLIES);

				props = item.GetProps();
				props.SetIconSize(32, 0.25, 0.25);
				props.SetFrontColor(colorFIA);
				props.SetTextVisible(false);
				props.Activate(true);
			}
			else
			{
				item.SetVisible(false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void RegisterRemnantSupplyDepot(notnull SCR_CampaignSuppliesComponent comp)
	{
		m_aRemnantSupplyDepots.Insert(comp);
	}

	//------------------------------------------------------------------------------------------------
	void HideUnusedBaseIcons()
	{
		SCR_MapDescriptorComponent mapDescriptorComponent;
		MapItem item;

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (base.IsInitialized())
				continue;

			mapDescriptorComponent = SCR_MapDescriptorComponent.Cast(base.GetOwner().FindComponent(SCR_MapDescriptorComponent));

			if (!mapDescriptorComponent)
				continue;

			item = mapDescriptorComponent.Item();

			if (!item)
				continue;

			item.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Determine the radio coverage of all bases (no coverage / can be reached / can respond / both ways)
	void RecalculateRadioCoverage(notnull SCR_CampaignFaction faction)
	{
		SCR_CampaignMilitaryBaseComponent hq = faction.GetMainBase();

		// No HQ is currently operational, switch all bases to out of signal state
		if (!hq)
		{
			foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
			{
				if (!base.IsInitialized())
					continue;

				if (base.GetFaction() == faction)
					base.SetHQRadioConverage(faction, SCR_ECampaignHQRadioComms.NONE);
			}

			return;
		}

		int basesCount = m_aBases.Count();
		SCR_CampaignMilitaryBaseComponent base;
		map<SCR_CampaignMilitaryBaseComponent, SCR_ECampaignHQRadioComms> newSettings = new map<SCR_CampaignMilitaryBaseComponent, SCR_ECampaignHQRadioComms>();
		array<SCR_CampaignMilitaryBaseComponent> canReceiveHQ = {};
		array<SCR_CampaignMilitaryBaseComponent> canReachHQ = {};
		SCR_CampaignMobileAssemblyStandaloneComponent mobileHQComponent = faction.GetMobileAssembly();
		SCR_ECampaignHQRadioComms connectionMHQ = SCR_ECampaignHQRadioComms.NONE;
		bool processedMHQ;
		int noMHQNeeded;

		// This loop is restarted every time a new link has been found to make sure all new possible links are also handled
		for (int i = 0; i < basesCount; i++)
		{
			base = SCR_CampaignMilitaryBaseComponent.Cast(m_aBases[i]);

			if (base.IsInitialized() && (!base.IsHQ() || base.GetFaction() == faction))
				CheckBaseRadioCoverage(i, base, hq, mobileHQComponent, connectionMHQ, faction, canReceiveHQ, canReachHQ, newSettings);

			// All bases have been processed, check MHQ connection
			if (i == basesCount - 1 && !processedMHQ && connectionMHQ != SCR_ECampaignHQRadioComms.NONE)
			{
				processedMHQ = true;

				if (noMHQNeeded == 0)
					noMHQNeeded = canReceiveHQ.Count();

				array<SCR_CampaignMilitaryBaseComponent> connectedBases = {};
				mobileHQComponent.GetBasesInRange(connectedBases);
				vector posMHQ = mobileHQComponent.GetOwner().GetOrigin();

				foreach (SCR_CampaignMilitaryBaseComponent connectedBase : connectedBases)
				{
					if (connectedBase.IsHQ() && connectedBase.GetFaction() != faction)
						continue;

					CheckBaseMHQConnection(i, connectedBase, connectionMHQ, newSettings, canReceiveHQ, canReachHQ, vector.DistanceXZ(posMHQ, connectedBase.GetOwner().GetOrigin()));
				}
			}
		}

		// We want to know how many bases are linked only by MHQ (shown in a user action)
		if (mobileHQComponent)
			mobileHQComponent.SetCountOfExclusivelyLinkedBases(canReceiveHQ.Count() - noMHQNeeded);

		int settingsCount = newSettings.Count();
		SCR_ECampaignHQRadioComms newCoverage;

		// Trace finished, apply radio status to all bases where the coverage has actually changed
		for (int i = 0; i < settingsCount; i++)
		{
			base = newSettings.GetKey(i);
			newCoverage = newSettings.Get(base);

			if (base.GetHQRadioCoverage(faction) != newCoverage)
				base.SetHQRadioConverage(faction, newCoverage);
		}

		if (settingsCount != 0)
		{
			EvaluateControlPoints();
			SelectPrimaryTarget(faction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void CheckBaseRadioCoverage(inout int i, notnull SCR_CampaignMilitaryBaseComponent base, notnull SCR_CampaignMilitaryBaseComponent hq, SCR_CampaignMobileAssemblyStandaloneComponent mobileHQComponent, inout SCR_ECampaignHQRadioComms connectionMHQ, notnull SCR_CampaignFaction faction, inout notnull array<SCR_CampaignMilitaryBaseComponent> canReceiveHQ, inout notnull array<SCR_CampaignMilitaryBaseComponent> canReachHQ, inout notnull map<SCR_CampaignMilitaryBaseComponent, SCR_ECampaignHQRadioComms> newSettings)
	{
		bool processedReceive = canReceiveHQ.Contains(base);
		bool processedReach = canReachHQ.Contains(base);

		if (processedReceive && processedReach)
			return;

		bool canReceive;
		bool canReach;
		SCR_ECampaignHQRadioComms newCoverage;
		SCR_ECampaignHQRadioComms savedCoverage;

		if (base == hq)
		{
			canReceive = true;
			canReach = true;
		}
		else
		{
			savedCoverage = newSettings.Get(base);

			if (processedReceive)
			{
				canReceive = (savedCoverage == SCR_ECampaignHQRadioComms.RECEIVE);
			}
			else
			{
				// Checked base can receive broadcast from a base which can receive broadcast from HQ
				foreach (SCR_CampaignMilitaryBaseComponent receiving : canReceiveHQ)
				{
					if (receiving.GetFaction() == faction && receiving.CanReachByRadio(base))
					{
						canReceive = true;
						break;
					}
				}
			}

			if (processedReach)
			{
				canReach = (savedCoverage == SCR_ECampaignHQRadioComms.SEND);
			}
			else
			{
				// Checked base can broadcast to a base which can broadcast to HQ
				foreach (SCR_CampaignMilitaryBaseComponent reaching : canReachHQ)
				{
					if (reaching.GetFaction() == faction && base.CanReachByRadio(reaching))
					{
						canReach = true;
						break;
					}
				}
			}
		}

		if (newSettings.Contains(base))
			newCoverage = savedCoverage;
		else
			newCoverage = base.GetHQRadioCoverage(faction);

		if (!canReceive && !canReach)
		{
			newCoverage = SCR_ECampaignHQRadioComms.NONE;
		}
		else
		{
			if (canReceive && !processedReceive)
			{
				if (canReach || processedReach)
					newCoverage = SCR_ECampaignHQRadioComms.BOTH_WAYS;
				else
					newCoverage = SCR_ECampaignHQRadioComms.RECEIVE;

				canReceiveHQ.Insert(base);
				i = -1;		// New link has been found - restart loop
			}

			if (canReach && !processedReach)
			{
				if (canReceive || processedReceive)
					newCoverage = SCR_ECampaignHQRadioComms.BOTH_WAYS;
				else
					newCoverage = SCR_ECampaignHQRadioComms.SEND;

				canReachHQ.Insert(base);
				i = -1;		// New link has been found - restart loop
			}
		}

		newSettings.Set(base, newCoverage);

		// Check if MHQ is connected to the radio network and its traffic capabilities
		if (mobileHQComponent && connectionMHQ != SCR_ECampaignHQRadioComms.BOTH_WAYS)
		{
			if ((newCoverage == SCR_ECampaignHQRadioComms.BOTH_WAYS || newCoverage == SCR_ECampaignHQRadioComms.RECEIVE) && mobileHQComponent.CanReachByRadio(base))
				connectionMHQ = newCoverage;
		}
	}

	//------------------------------------------------------------------------------------------------
	void CheckBaseMHQConnection(inout int i, notnull SCR_CampaignMilitaryBaseComponent connectedBase, SCR_ECampaignHQRadioComms connectionMHQ, inout notnull map<SCR_CampaignMilitaryBaseComponent, SCR_ECampaignHQRadioComms> newSettings, inout notnull array<SCR_CampaignMilitaryBaseComponent> canReceiveHQ, inout notnull array<SCR_CampaignMilitaryBaseComponent> canReachHQ, float distance)
	{
		SCR_ECampaignHQRadioComms coverage = newSettings.Get(connectedBase);

		if (coverage == SCR_ECampaignHQRadioComms.BOTH_WAYS || coverage == connectionMHQ)
			return;

		i = -1;		// We found a new linked base, restart the loop

		if (!canReceiveHQ.Contains(connectedBase))
			canReceiveHQ.Insert(connectedBase);

		bool canReach;

		if (connectionMHQ == SCR_ECampaignHQRadioComms.BOTH_WAYS && connectedBase.GetRadioRange() > distance)
		{
			if (!canReachHQ.Contains(connectedBase))
				canReachHQ.Insert(connectedBase);

			canReach = true;
		}

		if (coverage == SCR_ECampaignHQRadioComms.NONE)
		{
			if (canReach)
				newSettings.Set(connectedBase, connectionMHQ);
			else
				newSettings.Set(connectedBase, SCR_ECampaignHQRadioComms.RECEIVE);
		}
		else if (canReach || coverage == SCR_ECampaignHQRadioComms.SEND)
		{
			newSettings.Set(connectedBase, SCR_ECampaignHQRadioComms.BOTH_WAYS);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Find a base which is most profitable for a faction to capture next
	void SelectPrimaryTarget(notnull SCR_CampaignFaction faction)
	{
		array<SCR_CampaignMilitaryBaseComponent> controlPointsInRange = {};

		// Get all Control Points which are now available for capture
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aControlPoints)
		{
			if (!base.IsInitialized() || base.IsHQ())
				continue;

			if (base.GetFaction() == faction)
				continue;

			if (!base.IsHQRadioTrafficPossible(faction))
				continue;

			controlPointsInRange.Insert(base);
		}

		SCR_CampaignMilitaryBaseComponent target;
		int minDistance = int.MAX;

		// If there are some Control Points in radio range, find the closest one
		if (!controlPointsInRange.IsEmpty())
		{
			array<SCR_CampaignMilitaryBaseComponent> ownedBases = {};

			// Get all bases the given faction currently holds
			foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
			{
				if (!base.IsInitialized())
					continue;

				if (base.GetFaction() != faction)
					continue;

				if (!base.IsHQRadioTrafficPossible(faction))
					continue;

				ownedBases.Insert(base);
			}

			foreach (SCR_CampaignMilitaryBaseComponent controlPoint : controlPointsInRange)
			{
				vector positionCP = controlPoint.GetOwner().GetOrigin();

				foreach (SCR_CampaignMilitaryBaseComponent base : ownedBases)
				{
					int distance = vector.DistanceSqXZ(base.GetOwner().GetOrigin(), positionCP);

					if (distance > minDistance)
						continue;

					minDistance = distance;
					target = controlPoint;
				}
			}
		}
		else	// Otherwise, find the Control Point closest to one of the capturable bases
		{
			array<SCR_CampaignMilitaryBaseComponent> basesInRange = {};

			// Get all bases which are now available for capture
			foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
			{
				if (!base.IsInitialized() || base.IsHQ())
					continue;

				if (base.GetFaction() == faction)
					continue;

				if (!base.IsHQRadioTrafficPossible(faction))
					continue;

				basesInRange.Insert(base);
			}

			bool targetCoversControlPoint;

			foreach (SCR_CampaignMilitaryBaseComponent controlPoint : m_aControlPoints)
			{
				if (!controlPoint.IsInitialized() || controlPoint.IsHQ())
					continue;

				if (controlPoint.GetFaction() == faction)
					continue;

				vector positionCP = controlPoint.GetOwner().GetOrigin();

				foreach (SCR_CampaignMilitaryBaseComponent base : basesInRange)
				{
					int distance = vector.DistanceSqXZ(base.GetOwner().GetOrigin(), positionCP);
					bool closer = distance < minDistance;
					bool coversControlPoint = base.CanReachByRadio(controlPoint);

					// Also check if some of the bases in range can reach the Control Point with radio
					if (coversControlPoint)
					{
						if (targetCoversControlPoint && !closer)
							continue;
					}
					else
					{
						if (targetCoversControlPoint || !closer)
							continue;
					}

					minDistance = distance;
					target = base;
				}
			}
		}

		m_Campaign.SetPrimaryTarget(faction, target);
	}

	//------------------------------------------------------------------------------------------------
	//! Bump supply truck lifetime in garbage manager if it's parked near a base
	void OnSupplyTruckLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		ChimeraWorld world = vehicle.GetWorld();
		GarbageSystem garbageSystem = world.GetGarbageSystem();
		if (!garbageSystem || !garbageSystem.IsInserted(vehicle))
			return;

		float curLifetime = garbageSystem.GetRemainingLifetime(vehicle);
		if (curLifetime >= PARKED_LIFETIME)
			return;

		int baseDistanceSq = Math.Pow(MAX_DIST_TO_BASE, 2);
		vector vehPos = vehicle.GetOrigin();

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base.IsInitialized())
				continue;

			if (vector.DistanceSqXZ(vehPos, base.GetOwner().GetOrigin()) <= baseDistanceSq)
			{
				garbageSystem.Bump(vehicle, PARKED_LIFETIME - curLifetime);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Checks whether some faction is winning the game
	void EvaluateControlPoints()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);

		int controlPointsHeld;
		int controlPointsContested;
		#ifndef AR_CAMPAIGN_TIMESTAMP
		float currentTime = Replication.Time();
		float victoryTimestamp;
		float blockPauseTimestamp;
		#else
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		WorldTimestamp victoryTimestamp;
		WorldTimestamp blockPauseTimestamp;
		#endif

		foreach (Faction faction : factions)
		{
			SCR_CampaignFaction fCast = SCR_CampaignFaction.Cast(faction);

			if (!fCast || !fCast.IsPlayable())
				continue;

			controlPointsHeld = 0;
			controlPointsContested = 0;

			// Update amount of control points currently held by this faction
			foreach (SCR_CampaignMilitaryBaseComponent controlPoint : m_aControlPoints)
			{
				if (controlPoint.IsInitialized() && controlPoint.GetFaction() == fCast && controlPoint.IsHQRadioTrafficPossible(fCast, SCR_ECampaignHQRadioComms.RECEIVE))
				{
					controlPointsHeld++;

					if (controlPoint.GetCapturingFaction() && controlPoint.GetCapturingFaction() != fCast)
						controlPointsContested++
				}
			}

			m_Campaign.SetControlPointsHeld(fCast, controlPointsHeld);

			victoryTimestamp = fCast.GetVictoryTimestamp();
			blockPauseTimestamp = fCast.GetPauseByBlockTimestamp();
			int controlPointsThreshold = m_Campaign.GetControlPointTreshold();

			// Update timers (if a faction starts winning or a point is contested)
			if (controlPointsHeld >= controlPointsThreshold)
			{
				if ((controlPointsHeld - controlPointsContested) < controlPointsThreshold)
				{
					if (blockPauseTimestamp == 0)
						fCast.SetPauseByBlockTimestamp(currentTime);
				}
				else if (blockPauseTimestamp != 0)
				{
					#ifndef AR_CAMPAIGN_TIMESTAMP
					fCast.SetVictoryTimestamp(currentTime + victoryTimestamp - blockPauseTimestamp);
					fCast.SetPauseByBlockTimestamp(0);
					#else
					fCast.SetVictoryTimestamp(currentTime.PlusMilliseconds(victoryTimestamp.DiffMilliseconds(blockPauseTimestamp)));
					fCast.SetPauseByBlockTimestamp(null);
					#endif
				}

				if (victoryTimestamp == 0)
					#ifndef AR_CAMPAIGN_TIMESTAMP
					fCast.SetVictoryTimestamp(currentTime + (m_Campaign.GetVictoryTimer() * 1000));
					#else
					fCast.SetVictoryTimestamp(currentTime.PlusSeconds(m_Campaign.GetVictoryTimer()));
					#endif
			}
			else
			{
				#ifndef AR_CAMPAIGN_TIMESTAMP
				fCast.SetVictoryTimestamp(0);
				fCast.SetPauseByBlockTimestamp(0);
				#else
				fCast.SetVictoryTimestamp(null);
				fCast.SetPauseByBlockTimestamp(null);
				#endif
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnEnemyDetectedByDefenders(SCR_AIGroup group, SCR_AITargetInfo target, AIAgent reporter)
	{
		// Identify the base under attack, notify about it
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base.IsInitialized())
				continue;

			if (base.GetDefendersGroup() == group)
			{
				base.NotifyAboutEnemyAttack(target.m_Faction);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent FindClosestBase(vector position)
	{
		SCR_CampaignMilitaryBaseComponent closestBase;
		float closestBaseDistance = float.MAX;

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base.IsInitialized())
				continue;

			float distance = vector.DistanceSq(base.GetOwner().GetOrigin(), position);

			if (distance < closestBaseDistance)
			{
				closestBaseDistance = distance;
				closestBase = base;
			}
		}

		return closestBase;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent FindBaseByCallsign(int callsign)
	{
		if (callsign == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
			return null;

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (base.GetCallsign() == callsign)
				return base;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent FindBaseByPosition(vector position)
	{
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (base.GetOwner().GetOrigin() == position)
				return base;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEntityInFactionRadioSignal(notnull IEntity entity, notnull Faction faction)
	{
		SCR_CampaignFaction factionC = SCR_CampaignFaction.Cast(faction);

		if (!factionC)
			return false;

		// Check if the entity is within range of deployed mobile HQ which is able to relay the signal
		SCR_CampaignMobileAssemblyStandaloneComponent mobileHQ = factionC.GetMobileAssembly();

		if (mobileHQ && mobileHQ.GetOwner() != entity && mobileHQ.IsInRadioRange())
		{
			if (vector.DistanceSq(entity.GetOrigin(), mobileHQ.GetOwner().GetOrigin()) < Math.Pow(mobileHQ.GetRadioRange(), 2))
				return true;
		}

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base)
				continue;

			if (faction != base.GetFaction())
				continue;

			if (base.GetIsEntityInMyRange(entity) && base.IsHQRadioTrafficPossible(factionC))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void StoreBasesStates(out notnull array<ref SCR_CampaignBaseStruct> outEntries)
	{
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base.IsInitialized())
				continue;

			SCR_CampaignBaseStruct struct = new SCR_CampaignBaseStruct();
			base.StoreState(struct);
			outEntries.Insert(struct);
		}
	}

	//------------------------------------------------------------------------------------------------
	void LoadBasesStates(notnull array<ref SCR_CampaignBaseStruct> entries)
	{
		foreach (SCR_CampaignBaseStruct struct : entries)
		{
			SCR_CampaignMilitaryBaseComponent base = FindBaseByPosition(struct.GetPosition());

			if (!base)
				continue;

			base.LoadState(struct);
		}

		UpdateBases();

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			base.UpdateBasesInRadioRange();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Clean up ambient patrols around Main Operating Bases, assign parent bases where applicable
	void ProcessRemnantsPresence()
	{
		SCR_AmbientPatrolManager manager = SCR_AmbientPatrolManager.GetInstance();

		if (!manager)
			return;

		array<SCR_AmbientPatrolSpawnPointComponent> patrols = {};
		manager.GetPatrols(patrols);

		int distLimit = Math.Pow(PARENT_BASE_DISTANCE_THRESHOLD, 2);
		float minDistance;
		SCR_CampaignMilitaryBaseComponent nearestBase;
		bool register = true;
		float dist;
		vector center;

		int distLimitHQ = Math.Pow(HQ_NO_REMNANTS_RADIUS, 2);
		int distLimitHQPatrol = Math.Pow(HQ_NO_REMNANTS_PATROL_RADIUS, 2);

		foreach (SCR_AmbientPatrolSpawnPointComponent patrol : patrols)
		{
			minDistance = float.MAX;
			register = true;
			center = patrol.GetOwner().GetOrigin();
			nearestBase = null;

			foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
			{
				if (!base.IsInitialized() || base.GetType() == SCR_ECampaignBaseType.RELAY)
					continue;

				dist = vector.DistanceSqXZ(center, base.GetOwner().GetOrigin());

				// Don't clear Remnants patrols around HQs if their state was already loaded
				if (base.IsHQ() && !m_Campaign.WasRemnantsStateLoaded())
				{
					if (dist < distLimitHQ)
					{
						patrol.SetMembersAlive(0);
						register = false;
						break;

					}
					else if (dist < distLimitHQPatrol)
					{
						AIWaypointCycle waypoint = AIWaypointCycle.Cast(patrol.GetWaypoint());

						if (waypoint)
						{
							patrol.SetMembersAlive(0);
							register = false;
							break;
						}
					}
				}

				if (dist > distLimit || dist > minDistance)
					continue;

				if (!base.IsHQ())
				{
					nearestBase = base;
					minDistance = dist;
				}
				else
				{
					register = false;
					break;
				}
			}

			if (register && nearestBase)
				nearestBase.RegisterRemnants(patrol);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerId)
	{
		// If the disconnecting player is currently capturing a base; handle it
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base.IsInitialized())
				continue;

			if (base.GetCapturingFaction() && base.GetReconfiguredByID() == playerId)
			{
				base.EndCapture();
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnLocalPlayerPresenceChanged(notnull SCR_CampaignMilitaryBaseComponent base, bool present)
	{
		if (present)
		{
			if (m_OnLocalPlayerEnteredBase)
				m_OnLocalPlayerEnteredBase.Invoke(base);
		}
		else if (m_OnLocalPlayerLeftBase)
		{
			m_OnLocalPlayerLeftBase.Invoke(base);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseFactionChanged(SCR_MilitaryBaseComponent base, Faction newFaction)
	{
		if (!m_Campaign.IsProxy())
			EvaluateControlPoints();
	}

	//------------------------------------------------------------------------------------------------
	void OnServiceBuilt(SCR_EServicePointStatus state, notnull SCR_ServicePointComponent serviceComponent)
	{
		switch (state)
		{
			case SCR_EServicePointStatus.ONLINE:
			{
				if (m_Campaign.IsProxy())
					return;

				SCR_CatalogEntitySpawnerComponent spawner = SCR_CatalogEntitySpawnerComponent.Cast(serviceComponent);

				if (spawner)
					spawner.GetOnEntitySpawned().Insert(m_Campaign.OnEntityRequested);

				SCR_DefenderSpawnerComponent defenderSpawner = SCR_DefenderSpawnerComponent.Cast(serviceComponent);

				if (defenderSpawner)
					defenderSpawner.GetOnDefenderGroupSpawned().Insert(OnDefenderGroupSpawned);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnServiceRemoved(notnull SCR_MilitaryBaseComponent base, notnull SCR_ServicePointComponent service)
	{
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

		if (!campaignBase)
			return;

		campaignBase.OnServiceRemoved(service);

		SCR_CatalogEntitySpawnerComponent spawner = SCR_CatalogEntitySpawnerComponent.Cast(service);

		if (!spawner)
			return;

		spawner.GetOnEntitySpawned().Remove(m_Campaign.OnEntityRequested);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDefenderGroupSpawned(notnull SCR_DefenderSpawnerComponent spawner, notnull SCR_AIGroup group)
	{
		SCR_AIGroupUtilityComponent comp = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));

		if (!comp)
			return;

		ScriptInvokerBase<SCR_AIGroupPerceptionOnEnemyDetectedFiltered> onEnemyDetected = comp.m_Perception.GetOnEnemyDetectedFiltered();

		if (!onEnemyDetected)
			return;

		onEnemyDetected.Insert(OnEnemyDetectedByDefenders);

		array<SCR_MilitaryBaseComponent> bases = {};
		spawner.GetBases(bases);

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase)
				continue;

			campaignBase.SetDefendersGroup(group);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnConflictStarted()
	{
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CampaignMilitaryBaseManager(notnull SCR_GameModeCampaign campaign)
	{
		m_Campaign = campaign;
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();

		if (!baseManager)
			return;

		baseManager.GetOnServiceUnregisteredInBase().Insert(OnServiceRemoved);
		baseManager.GetOnBaseFactionChanged().Insert(OnBaseFactionChanged);

		m_Campaign.GetOnStarted().Insert(OnConflictStarted);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignMilitaryBaseManager()
	{
		//Unregister from script invokers
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance(false);

		if (baseManager)
		{
			baseManager.GetOnServiceUnregisteredInBase().Remove(OnServiceRemoved);
			baseManager.GetOnBaseFactionChanged().Remove(OnBaseFactionChanged);
		}

		if (m_Campaign)
			m_Campaign.GetOnStarted().Remove(OnConflictStarted);
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_ECampaignHQRadioComms
{
	NONE,
	RECEIVE,
	SEND,
	BOTH_WAYS
}
