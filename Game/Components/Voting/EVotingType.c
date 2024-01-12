enum EVotingType
{
	KICK = 0, ///< Kick a player, value is player ID
	ADMIN, ///< Give a player admin rights, value is player ID
	EDITOR_IN, ///< Give a player Game Master rights, value is player ID
	EDITOR_OUT, ///< Give a player Game Master rights, value is player ID
	EDITOR_WITHDRAW, ///< As a GM, withdraw your rights, value is player ID
	RESTART, ///< Restart the world, no value
	WORLD, ///< Choose the next world, value TBD
	AUTO_LIGHTBAN, ///< Lightban voting raised by the WarCrimesModule if enabled in the world
	AUTO_HEAVYBAN, ///< Heavyban voting raised by the WarCrimesModule if enabled in the world
	AUTO_KICK, ///< Kick voting raised by the WarCrimesModule if enabled in the world
};
