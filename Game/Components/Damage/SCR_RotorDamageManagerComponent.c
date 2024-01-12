//------------------------------------------------------------------------------------------------
class SCR_RotorDamageManagerComponentClass : ScriptedDamageManagerComponentClass
{
}

class SCR_RotorDamageManagerComponent : ScriptedDamageManagerComponent
{
	protected VehicleHelicopterSimulation m_HelicopterSimulation;
	protected SCR_VehicleDamageManagerComponent m_RootDamageManager;
	protected float m_fMinImpulse;

	[Attribute(defvalue: "0.1", desc: "Minimum contact with ChimeraCharacter from which rotor will take damage", category: "Rotor Contact")]
	protected float m_fMinImpulseMultiplierCharacters;

	[Attribute(defvalue: "0.3", desc: "Minimum contact with object from which rotor will take damage", category: "Rotor Contact")]
	protected float m_fMinImpulseMultiplierOther;	
	
	[Attribute(defvalue: "0.5", desc: "Multiplies impulse value to pass as damage", category: "Rotor Contact")]
	protected float m_fRotorDamageMultiplier;

	//------------------------------------------------------------------------------------------------
	protected void DamageRotors(notnull IEntity owner, notnull Contact contact, notnull SCR_RotorHitZone rotorHitZone)
	{
		vector hitPosDirNorm[3] = {contact.Position, contact.VelocityBefore1.Normalized(), contact.Normal};
		HandleDamage(EDamageType.COLLISION, contact.Impulse * m_fRotorDamageMultiplier, hitPosDirNorm, owner, rotorHitZone, Instigator.CreateInstigator(null), null, -1, -1);
	}

	//------------------------------------------------------------------------------------------------
	protected void DamageOther(notnull IEntity other, notnull Contact contact, notnull SCR_RotorHitZone rotorHitZone)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(other);
		SCR_DamageManagerComponent characterDamageManager;
		if (character)
			characterDamageManager = character.GetDamageManager();

		if (characterDamageManager)
		{
			//Physics otherPhysics = other.GetPhysics();
			vector hitPosDirNorm[3] = {contact.Position, contact.VelocityBefore1.Normalized(), contact.Normal};
			vector rotorTransform[4];
			rotorHitZone.GetPointInfo().GetTransform(rotorTransform);

			array<HitZone> characterHitZones = {};
			characterDamageManager.GetAllHitZones(characterHitZones);

			foreach (HitZone characterHitZone : characterHitZones)
			{
				float damage = m_HelicopterSimulation.RotorGetRPM(rotorHitZone.GetRotorIndex()) * Math.AbsFloat(vector.Distance(rotorTransform[3], contact.Position)) + contact.Impulse;
				characterDamageManager.HandleDamage(EDamageType.KINETIC, damage / characterHitZones.Count(), hitPosDirNorm, other, characterHitZone, Instigator.CreateInstigator(null), null, -1, -1);

				/*if (!characterHitZone.HasColliderNodes())
					continue;

				int colliderCount = characterHitZone.GetNumColliderDescriptors();

				for (int i = 0; i < colliderCount; i++)
				{
					vector colliderTransform[4];
					int boneIndex, nodeID;
					characterHitZone.TryGetColliderDescription(other, i, colliderTransform, boneIndex, nodeID);

					if (rotorHitZone.HasCollision(
						otherPhysics.GetGeomWorldPosition(boneIndex)) &&
						characterHitZone.GetHealth() > 0 &&
						characterDamageManager.GetHealth() > 0)
					{
						vector hitPosDirNorm[3] = {contact.Position, contact.VelocityBefore1.Normalized(), contact.Normal};
						vector rotorTransform[4];
						rotorHitZone.GetPointInfo().GetTransform(rotorTransform);

						float damage = m_HelicopterSimulation.RotorGetRPM(rotorHitZone.GetRotorIndex()) *
							Math.AbsFloat(vector.Distance(rotorPosition[3], contact.Position)) + contact.Impulse;

						characterDamageManager.HandleDamage(EDamageType.KINETIC, damage, hitPosDirNorm, other, characterHitZone, Instigator.CreateInstigator(null), null, -1, -1);
					}
				}*/
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void EnableContactMaskOnHost(bool enabled)
	{
		if (enabled)
			SetEventMask(GetOwner(), EntityEvent.CONTACT);
		else
			ClearEventMask(GetOwner(), EntityEvent.CONTACT);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState state)
	{
		super.OnDamageStateChanged(state);

		//! Disable collision damage calculations for wrecks
		if (m_fMinImpulse > 0)
			EnableContactMaskOnHost(state != EDamageState.DESTROYED);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnContact(IEntity owner, IEntity other, Contact contact)
	{
		SCR_RotorHitZone rotorHitZone = SCR_RotorHitZone.Cast(GetDefaultHitZone());
		if (!rotorHitZone || !rotorHitZone.IsSpinning())
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(other);
		if (character)
			DamageOther(other, contact, rotorHitZone);

		float minImpulse = m_fMinImpulse;
		if (character)
			minImpulse *= m_fMinImpulseMultiplierCharacters;
		else
			minImpulse *= m_fMinImpulseMultiplierOther;

		if (contact.Impulse < minImpulse)
			return false;

		DamageRotors(owner, contact, rotorHitZone);

		if (!character && m_RootDamageManager && contact.Impulse >= m_RootDamageManager.GetMinImpulse())
			m_RootDamageManager.CollisionDamage(owner, other, contact);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		// Do not do anything if the default hitzone is not rotor hitzone
		if (!SCR_RotorHitZone.Cast(GetDefaultHitZone()))
			return;

		// Only meaningful on server
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rpl && rpl.IsProxy())
			return;

		IEntity root = owner.GetRootParent();

		m_HelicopterSimulation = VehicleHelicopterSimulation.Cast(root.FindComponent(VehicleHelicopterSimulation));
		m_RootDamageManager = SCR_VehicleDamageManagerComponent.Cast(root.FindComponent(SCR_VehicleDamageManagerComponent));

		// Set minimum impulse same as vehicle damage manager
		Physics physics = owner.GetPhysics();
		if (physics)
			m_fMinImpulse = physics.GetMass();

		if (m_fMinImpulse > 0)
			EnableContactMaskOnHost(true);
	}
}
