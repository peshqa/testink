package com.peshqa.testink;

import android.view.SurfaceHolder;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;

import android.widget.Toast;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.util.DisplayMetrics;
import android.content.ContextWrapper;
import android.graphics.Rect;

import android.util.Log;

public class DisplayThread extends Thread
{
	volatile boolean isRunning = false;
	SurfaceHolder surfaceHolder;
	Context context;
	
	DisplayThread(SurfaceHolder surfaceHolder, Context context)
	{
		super();
		this.surfaceHolder = surfaceHolder;
		this.context = context;
		//Toast.makeText(context,"Hello",Toast.LENGTH_SHORT).show();
	}
	
	@Override
	public void run()
	{
		//Looping until the boolean is false
		while (isRunning)
		{
			GameEngine gameEngine = GameEngine.getInstance(context);
			gameEngine.UpdateAndRender(surfaceHolder, context);
			
			//delay time
			try
			{
				Thread.sleep(10); // ms
			}
			catch (InterruptedException ex)
			{
				//TODO: Log
			}
		}
	}
	
	
}