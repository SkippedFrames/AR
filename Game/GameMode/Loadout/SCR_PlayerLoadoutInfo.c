//------------------------------------------------------------------------------------------------
class SCR_PlayerLoadoutInfo
{
	protected int m_iPlayerId;
	protected int m_iLoadoutIndex = -1;

	//------------------------------------------------------------------------------------------------
	int GetLoadoutIndex()
	{
		return m_iLoadoutIndex;
	}

	//------------------------------------------------------------------------------------------------
	void SetLoadoutIndex(int loadoutIndex)
	{
		m_iLoadoutIndex = loadoutIndex;
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerId()
	{
		return m_iPlayerId;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_PlayerLoadoutInfo()
	{
	}

	//------------------------------------------------------------------------------------------------
	static SCR_PlayerLoadoutInfo Create(int playerId)
	{
		SCR_PlayerLoadoutInfo info = new SCR_PlayerLoadoutInfo();
		info.m_iPlayerId = playerId;
		info.m_iLoadoutIndex = -1;
		return info;
	}

	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_PlayerLoadoutInfo instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_iPlayerId);
		snapshot.SerializeInt(instance.m_iLoadoutIndex);
		return true;
	}

	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_PlayerLoadoutInfo instance)
	{
		snapshot.SerializeInt(instance.m_iPlayerId);
		snapshot.SerializeInt(instance.m_iLoadoutIndex);
		return true;
	}

	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeInt(packet);	// m_iPlayerId
		snapshot.EncodeInt(packet);	// m_iLoadoutIndex
	}

	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);	// m_iPlayerId
		snapshot.DecodeInt(packet);	// m_iLoadoutIndex
		return true;
	}

	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 4+4);   // m_iPlayerId, m_iLoadoutIndex
	}

	static bool PropCompare(SCR_PlayerLoadoutInfo instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_iPlayerId)
		    && snapshot.CompareInt(instance.m_iLoadoutIndex);
	}
};
