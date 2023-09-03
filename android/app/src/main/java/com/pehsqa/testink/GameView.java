package com.peshqa.testink;

import android.view.SurfaceView;
import android.content.Context;
import android.view.SurfaceHolder;

//import android.widget.Toast;

public class GameView extends SurfaceView
{
	public Context context;
	
	GameView(Context context)
	{
		super(context);
		this.context = context;
	}
	
}