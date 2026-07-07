package com.openworld.sos.quest;

import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.app.Activity;

public class MainActivity extends Activity implements SurfaceHolder.Callback {
  static {
    System.loadLibrary("sos_quest");
  }

  private SurfaceView surfaceView;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    surfaceView = new SurfaceView(this);
    surfaceView.getHolder().addCallback(this);
    setContentView(surfaceView);

    String storeRoot = getFilesDir().getAbsolutePath() + "/SOSLocal";
    nativeInit(storeRoot, "");
  }

  @Override
  public void surfaceCreated(SurfaceHolder holder) {
    nativeSetSurface(holder.getSurface());
  }

  @Override
  public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    nativeDrawFrame();
  }

  @Override
  public void surfaceDestroyed(SurfaceHolder holder) {}

  private native void nativeInit(String storeRoot, String splatRoomId);
  private native void nativeSetSurface(Object surface);
  private native void nativeDrawFrame();
}
