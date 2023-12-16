package com.peshqa.testink;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.view.SurfaceView;
import android.view.SurfaceHolder;

import android.content.Context;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

import android.view.Window;
import android.view.WindowManager;

import android.widget.Toast;

public class MainActivity extends Activity implements SensorEventListener
{
	// Used to load the 'testink' library on application startup.
    static {
        System.loadLibrary("testink");
    }
	
	/**
     * A native method that is implemented by the 'testink' native library,
     * which is packaged with this application.
     */
	public native String stringFromJNI();
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// TODO: get fullscreen to work!
		//requestWindowFeature(Window.FEATURE_NO_TITLE);
        //getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
        //        WindowManager.LayoutParams.FLAG_FULLSCREEN);
		
		SurfaceView view = new GameView(this/*, AppConstants.GetEngine()*/);

		setContentView(view);
		getActionBar().hide();
        /*TextView label = new TextView(this);
        label.setText(stringFromJNI());

        setContentView(label);*/
    }
	
	@Override
    public void onResume() {
        super.onResume();
		// Register sensor
		SensorManager manager = (SensorManager)
			getSystemService(Context.SENSOR_SERVICE);
		//if (manager.getSensorList(Sensor.TYPE_ACCELEROMETER).size() == 0) {
		if (manager.getSensorList(Sensor.TYPE_ROTATION_VECTOR).size() == 0) {
			//Toast.makeText(this,"No accelerometer installed",Toast.LENGTH_SHORT).show();
		} else {
			Sensor accelerometer = manager.getSensorList(
					//Sensor.TYPE_ACCELEROMETER).get(0);
					Sensor.TYPE_ROTATION_VECTOR).get(0);
			if (!manager.registerListener(this, accelerometer,
					SensorManager.SENSOR_DELAY_GAME)) {
				//Toast.makeText(this,"Couldn't register sensor listener",Toast.LENGTH_SHORT).show();
			} else {
				//Toast.makeText(this,"now!",Toast.LENGTH_SHORT).show();
			}
		}
	}
	@Override
    public void onPause() {
        super.onPause();
		SensorManager manager = (SensorManager)
			getSystemService(Context.SENSOR_SERVICE);
		manager.unregisterListener(this);
	}
	
	@Override
    public void onSensorChanged(SensorEvent event) {
		GameEngine ge = GameEngine.getInstance(this);
		
		if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR)
		{
			// TODO
		} else if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
			ge.updateRotationVector(ge.data, event.values[0], event.values[1], event.values[2], event.values[3]);
			Toast.makeText(this,"now",Toast.LENGTH_SHORT).show();
		}
		ge.updateRotationVector(ge.data, event.values[0], event.values[1], event.values[2], event.values[3]);
		//Toast.makeText(this,"now",Toast.LENGTH_SHORT).show();
    }
	@Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // do nothing
    }
	
}