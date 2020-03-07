`SurfaceView`相关：

https://blog.csdn.net/jinzhuojun/article/details/44062175





`SurfaceView`为什么不能放在其他`ViewGroup`里面去？

`SurfaceView`大小缩放卡顿的原因是`SurfaceView`是他与宿主窗口是分开的，因此外界的大小改变，他并不能及时响应。（这个是猜测）

