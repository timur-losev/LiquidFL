package Game
{
	import flash.display.*;
	import flash.events.*;
	
	/**
	 * ...
	 * @author Timur Losev
	 */
	public class Main extends MovieClip 
	{
		
		public function Main():void 
		{
			if (stage) init();
			else addEventListener(Event.ADDED_TO_STAGE, init);
		}
		
		private function init(e:Event = null):void 
		{
			removeEventListener(Event.ADDED_TO_STAGE, init);
			// entry point
			
			while (true)
			{
				trace("Just a trace...");
			}
		}
		
	}
	
}