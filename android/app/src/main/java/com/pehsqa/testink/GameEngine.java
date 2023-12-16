package com.peshqa.testink;

import android.view.SurfaceHolder;
import android.graphics.Canvas;
import android.content.Context;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.util.DisplayMetrics;
import android.content.ContextWrapper;
import android.graphics.Rect;

import android.content.res.AssetManager;

/*import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;*/

import android.view.View;

public class GameEngine
{
	public native void updateBitmap(Bitmap bitmap);
	public native long initData(AssetManager mgr);
	public native void updateAndRenderGame(long data, Bitmap bitmap);
	public native void itHappen(long data, char c);
	public native void updateMouse(long data, int x, int y, int is_down);
	public native void updateRotationVector(long data, float x, float y, float z, float w);
	
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
	
	public static GameEngine getInstance(Context context)
	{
        if (instance == null) {
            instance = new GameEngine(context);
        }
        return instance;
    }
	
	GameEngine(Context context)
	{
		AssetManager assetManager = context.getAssets();
		data = initData(assetManager);
		
		/*SensorManager manager = (SensorManager)
                getSystemService(Context.SENSOR_SERVICE);
        //if (manager.getSensorList(Sensor.TYPE_ACCELEROMETER).size() == 0) {
		if (manager.getSensorList(Sensor.TYPE_ROTATION_VECTOR).size() == 0) {
            // No accelerometer installed
        } else {
            Sensor accelerometer = manager.getSensorList(
                    Sensor.TYPE_ACCELEROMETER).get(0);
            if (!manager.registerListener(this, accelerometer,
                    SensorManager.SENSOR_DELAY_GAME)) {
                // Couldn't register sensor listener
            }
        }*/
		
	}
	
	/*@Override
    public void onSensorChanged(SensorEvent event) {
		if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR)
		{
			
		} else if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
			updateRotationVector(data, event.values[0], event.values[1], event.values[2], event.values[3]);
		}
    }
	@Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // do nothing
    }*/
	
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