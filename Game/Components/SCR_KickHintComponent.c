[ComponentEditorProps(category: "GameScripted/KickHintComponent", description: "")]
class SCR_KickHintComponentClass: ScriptComponentClass
{
};
class SCR_KickHintComponent: ScriptComponent
{
	
	protected ref ScriptInvoker m_OnCriminalScoreIncreased = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_DoIncreaseCriminalScore(float points)
	{
		m_OnCriminalScoreIncreased.Invoke(points);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowUI(float points)
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-FriendlyFire_Text", "#AR-FriendlyFire_Title", 15);
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyClientCriminalScoreIncreased(SCR_ECrimeNotification crime, float points)
	{
		Rpc(RPC_DoIncreaseCriminalScore, points);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_OnCriminalScoreIncreased.Insert(ShowUI);
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_ECrimeNotification
{
	TEAMKILL
}
