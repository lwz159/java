package com.example.videostudy.custom;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

import com.example.videostudy.R;

public class PictureShowView extends View {
    private Paint mPaint;
    private Bitmap mBitmap;
    private Rect mSrcRect;
    private RectF mDestRect;

    public PictureShowView(Context context) {
        super(context);
        init();
    }

    public PictureShowView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public PictureShowView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    public PictureShowView(Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    private void init() {
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mBitmap = BitmapFactory.decodeResource(getContext().getResources(), R.drawable.img_20160706_192545);
        mSrcRect = new Rect(0, 0, mBitmap.getHeight(), mBitmap.getWidth());
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (mDestRect == null) {
            mDestRect = getBitmapRect(mBitmap);
        }
        canvas.drawBitmap(mBitmap, mSrcRect, mDestRect, mPaint);
    }

    private RectF getBitmapRect(Bitmap bitmap) {
        int bitmapHeight = bitmap.getHeight();
        int bitmapWidth = bitmap.getWidth();
        int viewWidth = getWidth();
        int viewHeight = getHeight();
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
