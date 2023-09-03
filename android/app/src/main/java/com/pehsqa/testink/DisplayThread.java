package com.peshqa.testink;

import android.view.SurfaceHolder;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;

import android.widget.Toast;

public class DisplayThread extends Thread
{
	volatile boolean isRunning = false;
	SurfaceHolder surfaceHolder;
	
	DisplayThread(SurfaceHolder surfaceHolder, Context context)
	{
		super();
		this.surfaceHolder = surfaceHolder;
		//Toast.makeText(context,"Hello",Toast.LENGTH_SHORT).show();
	}
	
	@Override
	public void run()
	{
		//Looping until the boolean is false
		while (isRunning)
		{
			GameEngine gameEngine = GameEngine.getInstance();
			//Updates the game objects buisiness logic
			gameEngine.Update();
	
			//locking the canvas
			Canvas canvas = surfaceHolder.lockCanvas(null);
	
			if (canvas != null)
			{
				//Clears the screen with black paint and draws
				//object on the canvas
				Paint p = new Paint();
				p.setColor(0xFF00FF00);
				synchronized (surfaceHolder)
				{
					canvas.drawRect(
						0,
						0,
						canvas.getWidth(),
						canvas.getHeight(), p);
			
					gameEngine.Draw(canvas);
				}
				//unlocking the Canvas
				surfaceHolder.unlockCanvasAndPost(canvas);
			}
			//delay time
			try
			{
				Thread.sleep(30); // ms
			}
			catch (InterruptedException ex)
			{
				//TODO: Log
			}
		}
	}
}