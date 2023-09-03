package com.peshqa.testink;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.view.SurfaceView;
import android.view.SurfaceHolder;

//import android.widget.Toast;

public class MainActivity extends Activity
{
	public DisplayThread displayThread = null;
	
	// Used to load the 'testink' library on application startup.
    static {
        System.loadLibrary("testink");
    }
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		SurfaceView view = new GameView(this/*, AppConstants.GetEngine()*/);
		
		view.getHolder().addCallback(new SurfaceHolder.Callback() {
			//@Override
			public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3)
			{
				/*DO NOTHING*/
				//Toast.makeText(context,"one",Toast.LENGTH_SHORT).show();
			}
		
			//@Override
			public void surfaceCreated(SurfaceHolder arg0)
			{
				//Toast.makeText(context,"Hello Javatpoint",Toast.LENGTH_SHORT).show();
				//Starts the display thread
				if(displayThread == null || !displayThread.isRunning)
				{
					displayThread = new DisplayThread(view.getHolder(), MainActivity.this);
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

		setContentView(view);
		getActionBar().hide();
        /*TextView label = new TextView(this);
        label.setText(stringFromJNI());

        setContentView(label);*/
    }
	
	/**
     * A native method that is implemented by the 'testink' native library,
     * which is packaged with this application.
     */
	public native String stringFromJNI();
}