package
{

	public class circle extends shape
	{
		var i : Number;

		public function circle(param)
		{
			super(10 * param);
			i = param;
			trace("circle constructor is called with " + i);
		}

		override public function draw()
		{
			trace("CIRCLE: draw is called: i=" + i);
			super.draw();
		}
	}
}