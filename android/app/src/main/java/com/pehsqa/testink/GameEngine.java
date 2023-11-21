package com.peshqa.testink;

import android.view.SurfaceHolder;
import android.graphics.Canvas;
import android.content.Context;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.util.DisplayMetrics;
import android.content.ContextWrapper;
import android.graphics.Rect;

import android.view.View;

public class GameEngine
{
	public native void updateBitmap(Bitmap bitmap);
	public native long initData();
	public native void updateAndRenderGame(long data, Bitmap bitmap);
	public native void itHappen(long data, char c);
	public native void updateMouse(long data, int x, int y, int is_down);
	
	public static GameEngine instance = null;
	public long data;
	
	public void itH(int x, int y, int w, int h)
	{
		char dir = 'd';
		if (x < w*0.3) {
			dir = 'l';
		} else if (x > w*0.7) {
			dir = 'r';
		} else if (y < h*0.5) {
			dir = 'u';
		}
		itHappen(data, dir);
	}
	
	public void imitateMouse(int x, int y, int is_down)
	{
		updateMouse(data, x, y, is_down);
	}
	
	public native void onScreenResize(long data, int format, int width, int height);
	public void screenResize(int format, int width, int height)
	{
		onScreenResize(data, format, width, height);
	}
	
	public static GameEngine getInstance()
	{
        if (instance == null) {
            instance = new GameEngine();
        }
        return instance;
    }
	
	GameEngine()
	{
		data = initData();
	}
	
	public boolean UpdateAndRender(SurfaceHolder surfaceHolder, Context context)
	{
		//locking the canvas
		Canvas canvas = surfaceHolder.lockCanvas(null);

		if (canvas != null)
		{
			synchronized (surfaceHolder)
			{
				canvas.drawColor(0xFFFFFFFF);
				ContextWrapper wra = new ContextWrapper(context);
				DisplayMetrics metrics = wra.getBaseContext().getResources().getDisplayMetrics();         
				int w = metrics.widthPixels;
				int h = metrics.heightPixels;


				Bitmap.Config conf = Bitmap.Config.ARGB_8888; // see other conf types
				Bitmap bmp = Bitmap.createBitmap(w, h, conf); // this creates a MUTABLE bitmap0
				//updateBitmap(bmp);
				updateAndRenderGame(data, bmp);
				Matrix mat = new Matrix();
				canvas.drawBitmap(bmp, null, new Rect(0,0,w,h), null);
				
			}
			//unlocking the Canvas
			
			surfaceHolder.unlockCanvasAndPost(canvas);
		}
		
		return true;
	}
	
}