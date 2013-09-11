package org.coolreader.newui;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;

public class CoolReader extends Activity {
	
	public CRView crview;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		crview = new CRView(this);
		setContentView(crview);
	}

    @Override
    protected void onPause() {
        super.onPause();
        crview.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        crview.onResume();
    }	
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.cool_reader, menu);
		return true;
	}

}
