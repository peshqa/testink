package com.peshqa.testink;

import android.view.SurfaceHolder;
import android.graphics.Canvas;
import android.content.Context;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.util.DisplayMetrics;
import android.content.ContextWrapper;
import android.graphics.Rect;

public class GameEngine
{
	public static GameEngine instance = null;
	
	public static GameEngine getInstance() {
        if (instance == null) {
            instance = new GameEngine();
        }
        return instance;
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
				updateBitmap(bmp);
				Matrix mat = new Matrix();
				canvas.drawBitmap(bmp, null, new Rect(0,0,w,h), null);
				
			}
			//unlocking the Canvas
			surfaceHolder.unlockCanvasAndPost(canvas);
		}
		
		return true;
	}
	
	public native void updateBitmap(Bitmap bitmap);
}