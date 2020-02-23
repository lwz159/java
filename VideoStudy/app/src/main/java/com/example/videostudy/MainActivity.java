package com.example.videostudy;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.media.MediaMetadataRetriever;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    private SurfaceView mSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //mSurfaceView = findViewById(R.id.surfaceView);
        //mSurfaceView.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        new Thread(new DrawRunnable()).start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    private class DrawRunnable implements Runnable {
        @Override
        public void run() {
            Bitmap bitmap = BitmapFactory.decodeResource(MainActivity.this.getResources(), R.drawable.img_20160706_192545);
            SurfaceHolder surfaceHolder = mSurfaceView.getHolder();
            Canvas canvas = surfaceHolder.lockCanvas();
            Paint paint = new Paint();
            Rect srcRect = new Rect(0, 0, bitmap.getHeight(), bitmap.getWidth());
            RectF destRect = getBitmapRect(bitmap);
            canvas.drawBitmap(bitmap, srcRect, destRect, paint);
            surfaceHolder.unlockCanvasAndPost(canvas);
        }
    }

    private RectF getBitmapRect(Bitmap bitmap) {
        int bitmapHeight = bitmap.getHeight();
        int bitmapWidth = bitmap.getWidth();
        int viewWidth = mSurfaceView.getWidth();
        int viewHeight = mSurfaceView.getHeight();
        float bitmapRatio = (float) bitmapWidth / (float) bitmapHeight;
        float screenRatio = (float) viewWidth / (float) viewHeight;
        int factWidth;
        int factHeight;
        int x1, y1, x2, y2;
        if (bitmapRatio > screenRatio) {
            factWidth = viewWidth;
            factHeight = (int)(factWidth / bitmapRatio);
            x1 = 0;
            y1 = (viewHeight - factHeight) / 2;
        } else if (bitmapRatio < screenRatio) {
            factHeight = viewHeight;
            factWidth = (int)(factHeight * bitmapRatio);
            x1 = (viewWidth - factWidth) / 2;
            y1 = 0;
        } else {
            factWidth = bitmapWidth;
            factHeight = bitmapHeight;
            x1 = 0;
            y1 = 0;
        }
        x2 = x1 + factWidth;
        y2 = y1 + factHeight;
        return new RectF(x1, y1, x2, y2);
    }
}
