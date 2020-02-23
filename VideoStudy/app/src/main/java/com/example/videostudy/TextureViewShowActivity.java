package com.example.videostudy;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.view.TextureView;

import androidx.appcompat.app.AppCompatActivity;

public class TextureViewShowActivity extends AppCompatActivity implements TextureView.SurfaceTextureListener {
    private TextureView mTextureView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mTextureView = findViewById(R.id.textureView);
        mTextureView.setSurfaceTextureListener(this);
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        new Thread(new TextureViewShowActivity.DrawRunnable()).start();
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {

    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {

    }

    private class DrawRunnable implements Runnable {
        @Override
        public void run() {
            Bitmap bitmap = BitmapFactory.decodeResource(TextureViewShowActivity.this.getResources(), R.drawable.img_20160706_192545);
            Canvas canvas = mTextureView.lockCanvas();
            Paint paint = new Paint();
            Rect srcRect = new Rect(0, 0, bitmap.getHeight(), bitmap.getWidth());
            RectF destRect = getBitmapRect(bitmap);
            canvas.drawBitmap(bitmap, srcRect, destRect, paint);
            mTextureView.unlockCanvasAndPost(canvas);
        }
    }

    private RectF getBitmapRect(Bitmap bitmap) {
        int bitmapHeight = bitmap.getHeight();
        int bitmapWidth = bitmap.getWidth();
        int viewWidth = mTextureView.getWidth();
        int viewHeight = mTextureView.getHeight();
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
