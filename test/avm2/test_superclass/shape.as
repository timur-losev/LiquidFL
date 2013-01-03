package
{
	public class shape extends box
	{
		private var i : Number;

		public function shape(param)
		{
			super(10 * param);
			i = param;
			trace("shape constructor is called with " + i);
		}
		
		override public function draw()
		{
			trace("SHAPE: draw is called: i=" + i);
			super.draw();
		}

	}
}