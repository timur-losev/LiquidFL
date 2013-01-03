
class rectangle extends MovieClip
{

	//This will keep track of the ship's speed 
	var velocity;
	//This will create a buffer between shooting missiles.
	var shootLimiter;
	//This variable will keep track of how often to create enemies
	var enemyTimer;
	//This variable will keep track of how oftern to create a miniboss
	var miniBossTimer;
	//This variable will keep track of how often to create a powerup
	var powerUpTimer;
	//This variable will keep track of how many minibosses before the boss
	var bossCountdown;
	//This variable will be an array and will contain a list of all enemies on stage.
	var enemies;
	//This will keep track of the ship's current score.
	var score;
	//This will store the final score of a game after bonuses have been tallied.
	var finalScore;
	//This variable keeps track of the ship's current health
	var health;
	
	//This will keep track of how many kills the ship has made
	var kills;
	//This will keep track of how many times a ship missile fails to hit a target before flying off screen.
	var misses;
	//This will keep track of whether the ship is currently shaking from taking damage
	var shaking;
	//this will store how many frames the ship should shake for when it takes damage
	var shakeDuration;
	//this variable is actually a reference to the shield movie clip that is nested inside the ship movie clip.
	var shield:MovieClip;

	function onLoad()
	{
		trace("onLoad");
	}
	
	function onEnterFrame()
	{
		
	}
}