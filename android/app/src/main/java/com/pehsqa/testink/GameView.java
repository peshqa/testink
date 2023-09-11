package com.peshqa.testink;

import android.view.SurfaceView;
import android.content.Context;
import android.view.SurfaceHolder;

import android.view.View;
import android.view.MotionEvent;

import android.util.DisplayMetrics;
import android.content.ContextWrapper;
import android.graphics.Rect;

//import android.widget.Toast;

public class GameView extends SurfaceView
{
	public Context context;
	public DisplayThread displayThread = null;
	
	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		int x = (int) event.getX();
		int y = (int) event.getY();

		switch (event.getAction()) {
			case MotionEvent.ACTION_DOWN: {
				GameEngine ge = GameEngine.getInstance();
				ContextWrapper wra = new ContextWrapper(context);
				DisplayMetrics metrics = wra.getBaseContext().getResources().getDisplayMetrics();         
				int w = metrics.widthPixels;
				int h = metrics.heightPixels;
				ge.itH(x, y, w, h);
				return true;
				//break;
			}
			case MotionEvent.ACTION_MOVE: {

				break;
			}
		}

		return false;
	}
	
	GameView(Context context)
	{
		super(context);
		this.context = context;
		
		super.getHolder().addCallback(new SurfaceHolder.Callback() {
			//@Override
			public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3)
			{
				/*DO NOTHING*/
				//Toast.makeText(context,"one",Toast.LENGTH_SHORT).show();
			}
		
			//@Override
			public void surfaceCreated(SurfaceHolder holder)
			{
				//Toast.makeText(context,"Hello Javatpoint",Toast.LENGTH_SHORT).show();
				//Starts the display thread
				if(displayThread == null || !displayThread.isRunning)
				{
					displayThread = new DisplayThread(holder, context);
					displayThread.isRunning = true;
					displayThread.start();
				}
				else
				{
					displayThread.start();
				}
			}
			
			//@Override
			public void surfaceDestroyed(SurfaceHolder arg0)
			{
				//Toast.makeText(context,"Hello Javatpoint",Toast.LENGTH_SHORT).show();
				//Stop the display thread
				displayThread.isRunning = false;
				//AppConstants.StopThread(_displayThread);
			}
		});
	}
	
}