# pthread相关文档

## pthread相关基础知识介绍

- 创建线程

  ```c
  int pthread_create(pthread_t *restrict tidp, const pthread_attr_t *restrict attr, 
                    void *(*start_rtn)(void *), void *restrict arg)
  ```

  新创建的线程从`start_rtn`函数的地址开始运行，该函数只有一个无类型指针参数arg，新创建线程的线程ID会被设置到`tidp`指向的内存单元，至于`attr`属性可以暂不关注

- 终止线程

  单个线程可以通过3种方式退出：

  - 线程可以简单的从启动线程中返回
  - 线程可以被同一进程中的其他线程返回
  - 线程调用`pthread_exit`

  `pthread_exit`的参数原型如下：

  ```c
  void pthread_exit(void *rval_ptr)
  ```

  其中，`rval_ptr`可以由进程中的其他线程通过`pthread_join`函数访问到这个指针：

  ```c
  int pthread_join(pthrad_t thread, void **rval_ptr)
  ```

  调用`pthread_join`的线程将一直阻塞，直到指定的线程终止。通过调用`pthread_join`可以自动把线程设置为分离状态，这样线程所占用的资源就可以恢复，而如果线程已经处于分离状态，则调用`pthread_join`就会返回失败。

  ## 问题代码示例

  `OOM`代码出问题的根源就在于断网情况下进入了一个死循环的逻辑中，在该代码逻辑中会循环调用`pthread_create`方法，错误代码类似于如下简化代码：

  ```c
  void * startThread(void *) {
      LOGE("test");
      return NULL;
  }
  
  pthread_t threadId = 0;
  while (true) {
      pthread_create(&threadId, NULL, startThread, NULL);
  }
  ```

  由于`pthread_create`创建的线程资源一直没有释放，导致了`OOM`进而导致了后续的崩溃。

  ## 问题的修复

  一个简单的修复方案就是通过调用`thread_join`来等待线程结束，并自动进行线程资源释放，代码如下：

  ```c
  pthread_t threadId = 0;
  while (true) {
      pthread_create(&threadId, NULL, startThread, NULL);
      sleep(1);
      LOGE("Main Thread");
      pthread_join(threadId, NULL);
  }
  ```

  然而，在项目代码中采用这种调用方式，却会产生另一种崩溃：

  ```java
  2020-03-07 16:27:26.632 18994-18994/? A/DEBUG: *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
  2020-03-07 16:27:26.632 18994-18994/? A/DEBUG: Build fingerprint: 'Xiaomi/chiron/chiron:9/PKQ1.190118.001/V11.0.2.0.PDECNXM:user/release-keys'
  2020-03-07 16:27:26.633 18994-18994/? A/DEBUG: Revision: '0'
  2020-03-07 16:27:26.633 18994-18994/? A/DEBUG: ABI: 'arm'
  2020-03-07 16:27:26.633 18994-18994/? A/DEBUG: pid: 18959, tid: 18959, name: hreadcreatetest  >>> com.example.threadcreatetest <<<
  2020-03-07 16:27:26.633 18994-18994/? A/DEBUG: signal 6 (SIGABRT), code -6 (SI_TKILL), fault addr --------
  2020-03-07 16:27:26.633 18994-18994/? A/DEBUG: Abort message: 'invalid pthread_t 0xcf2bf970 passed to libc'
  2020-03-07 16:27:26.633 18994-18994/? A/DEBUG:     r0  00000000  r1  00004a0f  r2  00000006  r3  00000008
  2020-03-07 16:27:26.633 18994-18994/? A/DEBUG:     r4  00004a0f  r5  00004a0f  r6  ffbf0af4  r7  0000010c
  2020-03-07 16:27:26.633 18994-18994/? A/DEBUG:     r8  00000000  r9  eb548000  r10 ffbf0b90  r11 eb548000
  2020-03-07 16:27:26.633 18994-18994/? A/DEBUG:     ip  edeea3cc  sp  ffbf0ae0  lr  ede55789  pc  ede4cfaa
  2020-03-07 16:27:26.754 30547-30607/? D/NetworkController.MobileSignalController(1): 4G level = 5
  2020-03-07 16:27:26.764 1553-1603/? W/BroadcastQueue: Background execution not allowed: receiving Intent { act=android.intent.action.SIG_STR flg=0x10 (has extras) } to com.douguo.recipe/.BootBroadcastPush
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG: backtrace:
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #00 pc 0001cfaa  /system/lib/libc.so (abort+58)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #01 pc 00071b15  /system/lib/libc.so (__pthread_internal_find(long)+100)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #02 pc 00071b49  /system/lib/libc.so (pthread_join+28)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #03 pc 00006a4b  /data/app/com.example.threadcreatetest-S4d-d9tHEMOkIFvsfBXBeQ==/lib/arm/libnative-lib.so (Java_com_example_threadcreatetest_MainActivity_threadLoopJNI+86)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #04 pc 00417879  /system/lib/libart.so (art_quick_generic_jni_trampoline+40)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #05 pc 00413375  /system/lib/libart.so (art_quick_invoke_stub_internal+68)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #06 pc 003ecca1  /system/lib/libart.so (art_quick_invoke_stub+224)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #07 pc 000a1c8d  /system/lib/libart.so (art::ArtMethod::Invoke(art::Thread*, unsigned int*, unsigned int, art::JValue*, char const*)+136)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #08 pc 001e6e95  /system/lib/libart.so (art::interpreter::ArtInterpreterToCompiledCodeBridge(art::Thread*, art::ArtMethod*, art::ShadowFrame*, unsigned short, art::JValue*)+236)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #09 pc 001e198f  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+814)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #10 pc 003e878d  /system/lib/libart.so (MterpInvokeDirect+196)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #11 pc 00406194  /system/lib/libart.so (ExecuteMterpImpl+14484)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #12 pc 00011a6c  <anonymous:eaba4000> (com.example.threadcreatetest.MainActivity.access$000)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #13 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #14 pc 001ca7f1  /system/lib/libart.so (art::interpreter::ArtInterpreterToInterpreterBridge(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*, art::JValue*)+152)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #15 pc 001e1977  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+790)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #16 pc 003e88db  /system/lib/libart.so (MterpInvokeStatic+130)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #17 pc 00406214  /system/lib/libart.so (ExecuteMterpImpl+14612)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #18 pc 00011a20  <anonymous:eaba4000> (com.example.threadcreatetest.MainActivity$1.onClick+4)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #19 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #20 pc 001ca7f1  /system/lib/libart.so (art::interpreter::ArtInterpreterToInterpreterBridge(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*, art::JValue*)+152)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #21 pc 001e1977  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+790)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #22 pc 003e84cf  /system/lib/libart.so (MterpInvokeInterface+1010)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #23 pc 00406294  /system/lib/libart.so (ExecuteMterpImpl+14740)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #24 pc 00d7c59e  /system/framework/boot-framework.vdex (android.view.View.performClick+34)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #25 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #26 pc 001ca7f1  /system/lib/libart.so (art::interpreter::ArtInterpreterToInterpreterBridge(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*, art::JValue*)+152)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #27 pc 001e1977  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+790)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #28 pc 003e78ff  /system/lib/libart.so (MterpInvokeVirtual+442)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #29 pc 00406094  /system/lib/libart.so (ExecuteMterpImpl+14228)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #30 pc 00d7c5c4  /system/framework/boot-framework.vdex (android.view.View.performClickInternal+6)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #31 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #32 pc 001ca7f1  /system/lib/libart.so (art::interpreter::ArtInterpreterToInterpreterBridge(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*, art::JValue*)+152)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #33 pc 001e1977  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+790)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #34 pc 003e878d  /system/lib/libart.so (MterpInvokeDirect+196)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #35 pc 00406194  /system/lib/libart.so (ExecuteMterpImpl+14484)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #36 pc 00d78d98  /system/framework/boot-framework.vdex (android.view.View.access$3100)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #37 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #38 pc 001ca7f1  /system/lib/libart.so (art::interpreter::ArtInterpreterToInterpreterBridge(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*, art::JValue*)+152)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #39 pc 001e1977  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+790)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #40 pc 003e88db  /system/lib/libart.so (MterpInvokeStatic+130)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #41 pc 00406214  /system/lib/libart.so (ExecuteMterpImpl+14612)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #42 pc 00d5e7b8  /system/framework/boot-framework.vdex (android.view.View$PerformClick.run+4)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #43 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #44 pc 001ca7f1  /system/lib/libart.so (art::interpreter::ArtInterpreterToInterpreterBridge(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*, art::JValue*)+152)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #45 pc 001e1977  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+790)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #46 pc 003e84cf  /system/lib/libart.so (MterpInvokeInterface+1010)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #47 pc 00406294  /system/lib/libart.so (ExecuteMterpImpl+14740)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #48 pc 00bd6c4e  /system/framework/boot-framework.vdex (android.os.Handler.handleCallback+4)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #49 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #50 pc 001ca7f1  /system/lib/libart.so (art::interpreter::ArtInterpreterToInterpreterBridge(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*, art::JValue*)+152)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #51 pc 001e1977  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+790)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #52 pc 003e88db  /system/lib/libart.so (MterpInvokeStatic+130)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #53 pc 00406214  /system/lib/libart.so (ExecuteMterpImpl+14612)
  2020-03-07 16:27:26.994 18994-18994/? A/DEBUG:     #54 pc 00bd6ad8  /system/framework/boot-framework.vdex (android.os.Handler.dispatchMessage+8)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #55 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #56 pc 001ca7f1  /system/lib/libart.so (art::interpreter::ArtInterpreterToInterpreterBridge(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*, art::JValue*)+152)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #57 pc 001e1977  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+790)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #58 pc 003e78ff  /system/lib/libart.so (MterpInvokeVirtual+442)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #59 pc 00406094  /system/lib/libart.so (ExecuteMterpImpl+14228)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #60 pc 00be962a  /system/framework/boot-framework.vdex (android.os.Looper.loop+420)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #61 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #62 pc 001ca7f1  /system/lib/libart.so (art::interpreter::ArtInterpreterToInterpreterBridge(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*, art::JValue*)+152)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #63 pc 001e1977  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+790)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #64 pc 003e88db  /system/lib/libart.so (MterpInvokeStatic+130)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #65 pc 00406214  /system/lib/libart.so (ExecuteMterpImpl+14612)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #66 pc 00434c56  /system/framework/boot-framework.vdex (android.app.ActivityThread.main+214)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #67 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #68 pc 001ca737  /system/lib/libart.so (art::interpreter::EnterInterpreterFromEntryPoint(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*)+82)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #69 pc 003db401  /system/lib/libart.so (artQuickToInterpreterBridge+880)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #70 pc 004178ff  /system/lib/libart.so (art_quick_to_interpreter_bridge+30)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #71 pc 00413375  /system/lib/libart.so (art_quick_invoke_stub_internal+68)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #72 pc 003ecda3  /system/lib/libart.so (art_quick_invoke_static_stub+222)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #73 pc 000a1c9f  /system/lib/libart.so (art::ArtMethod::Invoke(art::Thread*, unsigned int*, unsigned int, art::JValue*, char const*)+154)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #74 pc 00349d4d  /system/lib/libart.so (art::(anonymous namespace)::InvokeWithArgArray(art::ScopedObjectAccessAlreadyRunnable const&, art::ArtMethod*, art::(anonymous namespace)::ArgArray*, art::JValue*, char const*)+52)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #75 pc 0034b19d  /system/lib/libart.so (art::InvokeMethod(art::ScopedObjectAccessAlreadyRunnable const&, _jobject*, _jobject*, _jobject*, unsigned int)+1024)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #76 pc 002fcf75  /system/lib/libart.so (art::Method_invoke(_JNIEnv*, _jobject*, _jobject*, _jobjectArray*)+40)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #77 pc 006aaee7  /system/framework/arm/boot-core-oj.oat (offset 0x2c9000) (java.lang.Class.getDeclaredMethodInternal [DEDUPED]+110)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #78 pc 00413375  /system/lib/libart.so (art_quick_invoke_stub_internal+68)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #79 pc 003ecca1  /system/lib/libart.so (art_quick_invoke_stub+224)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #80 pc 000a1c8d  /system/lib/libart.so (art::ArtMethod::Invoke(art::Thread*, unsigned int*, unsigned int, art::JValue*, char const*)+136)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #81 pc 001e6e95  /system/lib/libart.so (art::interpreter::ArtInterpreterToCompiledCodeBridge(art::Thread*, art::ArtMethod*, art::ShadowFrame*, unsigned short, art::JValue*)+236)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #82 pc 001e198f  /system/lib/libart.so (bool art::interpreter::DoCall<false, false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, art::JValue*)+814)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #83 pc 003e78ff  /system/lib/libart.so (MterpInvokeVirtual+442)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #84 pc 00406094  /system/lib/libart.so (ExecuteMterpImpl+14228)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #85 pc 012a2e40  /system/framework/boot-framework.vdex (com.android.internal.os.RuntimeInit$MethodAndArgsCaller.run+22)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #86 pc 001c610b  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadERKNS_20CodeItemDataAccessorERNS_11ShadowFrameENS_6JValueEb.llvm.974546410+378)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #87 pc 001ca737  /system/lib/libart.so (art::interpreter::EnterInterpreterFromEntryPoint(art::Thread*, art::CodeItemDataAccessor const&, art::ShadowFrame*)+82)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #88 pc 003db401  /system/lib/libart.so (artQuickToInterpreterBridge+880)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #89 pc 004178ff  /system/lib/libart.so (art_quick_to_interpreter_bridge+30)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #90 pc 01efa1c1  /system/framework/arm/boot-framework.oat (offset 0x9ea000) (com.android.internal.os.ZygoteInit.main+1928)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #91 pc 00413375  /system/lib/libart.so (art_quick_invoke_stub_internal+68)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #92 pc 003ecda3  /system/lib/libart.so (art_quick_invoke_static_stub+222)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #93 pc 000a1c9f  /system/lib/libart.so (art::ArtMethod::Invoke(art::Thread*, unsigned int*, unsigned int, art::JValue*, char const*)+154)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #94 pc 00349d4d  /system/lib/libart.so (art::(anonymous namespace)::InvokeWithArgArray(art::ScopedObjectAccessAlreadyRunnable const&, art::ArtMethod*, art::(anonymous namespace)::ArgArray*, art::JValue*, char const*)+52)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #95 pc 00349b77  /system/lib/libart.so (art::InvokeWithVarArgs(art::ScopedObjectAccessAlreadyRunnable const&, _jobject*, _jmethodID*, std::__va_list)+310)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #96 pc 0028ff05  /system/lib/libart.so (art::JNI::CallStaticVoidMethodV(_JNIEnv*, _jclass*, _jmethodID*, std::__va_list)+444)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #97 pc 00077a05  /system/lib/libandroid_runtime.so (_JNIEnv::CallStaticVoidMethod(_jclass*, _jmethodID*, ...)+28)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #98 pc 00079cb1  /system/lib/libandroid_runtime.so (android::AndroidRuntime::start(char const*, android::Vector<android::String8> const&, bool)+520)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #99 pc 00001b1f  /system/bin/app_process32 (main+886)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #100 pc 000a0f65  /system/lib/libc.so (__libc_init+48)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #101 pc 00001767  /system/bin/app_process32 (_start_main+38)
  2020-03-07 16:27:26.995 18994-18994/? A/DEBUG:     #102 pc 000000c4  <unknown>
  ```

  产生该问题的原因是因为`create`的线程已经被释放了，因此系统认为该`threadID`为非法的`threadID`。（为什么代码中的执行逻辑和单纯的`thread_join`的执行结果不一致？）

  ## 一些遗留问题

  虽然崩溃的基本逻辑找到了，但是却产生了一些疑问：为什么创建线程过多会崩溃，而且在底层第一次`OOM`的时候却不会崩溃，而崩溃的地方却是在其他地方调用的时候呢？另外，`OOM`中的`Memory`指得是哪一个内存空间呢？为此需要对`Android`线程创建的流程进行简单的分析：

  我们以预览崩溃的`log`为例，简单跟踪一下`java`线程创建的方式：

  ```java
  // java.lang.Thread
  public synchronized void start() {
      ...
      nativeCreate(this, stackSize, daemon);
      ...
  }
  ```

  `Java Thread`的`start`方法调用了`native`方法，该`native`方法参数解释如下：

  - `this`：即`Thread`对象本身
  - `stackSize`：新创建的线程的栈大小
  - `daemon`：表明新创建的线程是否是`Daemon`线程

  `Native`层的代码分析的是`Android 8.0`的`ART`虚拟机源码，首先`Thread::nativeCreate` 的`native`实现。在`art/runtime/native/java_lang_thread.cc` 中。其主要逻辑会调用到 `art/runtime/thread.cc` 的 `art::Thread::CreateNativeThread` 函数来。

  1. ```c++
      void Thread::CreateNativeThread(JNIEnv* env, jobject java_peer, size_t stack_size, bool is_daemon) {
       ...
       // 创建java.lang.Thread相对应的native层C++对象
       Thread* child_thread = new Thread(is_daemon);
       // Use global JNI ref to hold peer live while child thread starts.
       child_thread->tlsPtr_.jpeer = env->NewGlobalRef(java_peer);
       stack_size = FixStackSize(stack_size);
     
       // Thread.start is synchronized, so we know that nativePeer is 0, and know that we're not racing to
       // assign it.
       env->SetLongField(java_peer, WellKnownClasses::java_lang_Thread_nativePeer,
                         reinterpret_cast<jlong>(child_thread));
     
       // Try to allocate a JNIEnvExt for the thread. We do this here as we might be out of memory and
       // do not have a good way to report this on the child's side.
       // 获取线程的JniEnv结构
       std::string error_msg;
       std::unique_ptr<JNIEnvExt> child_jni_env_ext(
           JNIEnvExt::Create(child_thread, Runtime::Current()->GetJavaVM(), &error_msg));
     
       int pthread_create_result = 0;
       if (child_jni_env_ext.get() != nullptr) {
         pthread_t new_pthread;
         pthread_attr_t attr;
         child_thread->tlsPtr_.tmp_jni_env = child_jni_env_ext.get();
         CHECK_PTHREAD_CALL(pthread_attr_init, (&attr), "new thread");
         CHECK_PTHREAD_CALL(pthread_attr_setdetachstate, (&attr, PTHREAD_CREATE_DETACHED),
                            "PTHREAD_CREATE_DETACHED");
         CHECK_PTHREAD_CALL(pthread_attr_setstacksize, (&attr, stack_size), stack_size);
         pthread_create_result = pthread_create(&new_pthread,
                                                &attr,
                                                Thread::CreateCallback,
                                                child_thread);
         CHECK_PTHREAD_CALL(pthread_attr_destroy, (&attr), "new thread");
     
         // java线程创建成功的条件
         if (pthread_create_result == 0) {
           // pthread_create started the new thread. The child is now responsible for managing the
           // JNIEnvExt we created.
           // Note: we can't check for tmp_jni_env == nullptr, as that would require synchronization
           //       between the threads.
           child_jni_env_ext.release();
           return;
         }
       }
     
       // 线程创建失败的处理
       // Either JNIEnvExt::Create or pthread_create(3) failed, so clean up.
       {
         MutexLock mu(self, *Locks::runtime_shutdown_lock_);
         runtime->EndThreadBirth();
       }
       // Manually delete the global reference since Thread::Init will not have been run.
       env->DeleteGlobalRef(child_thread->tlsPtr_.jpeer);
       child_thread->tlsPtr_.jpeer = nullptr;
       delete child_thread;
       child_thread = nullptr;
       // TODO: remove from thread group?
       env->SetLongField(java_peer, WellKnownClasses::java_lang_Thread_nativePeer, 0);
       {
         std::string msg(child_jni_env_ext.get() == nullptr ?
             StringPrintf("Could not allocate JNI Env: %s", error_msg.c_str()) :
             StringPrintf("pthread_create (%s stack) failed: %s",
                                      PrettySize(stack_size).c_str(), strerror(pthread_create_result)));
         ScopedObjectAccess soa(env);
         soa.Self()->ThrowOutOfMemoryError(msg.c_str());
       }
     }
     ```

     我们可以注意到对应的错误处理函数中抛出的异常即是我们在预览页面所得到的的异常堆栈信息：

     ```c++
     std::string msg(child_jni_env_ext.get() == nullptr ?
             StringPrintf("Could not allocate JNI Env: %s", error_msg.c_str()) :
             StringPrintf("pthread_create (%s stack) failed: %s",
                                      PrettySize(stack_size).c_str(), strerror(pthread_create_result)));
         ScopedObjectAccess soa(env);
         soa.Self()->ThrowOutOfMemoryError(msg.c_str());
     ```

     因此，我们可以知道预览页面的崩溃根源所在了，即`pthread_create`创建失败后抛出异常，导致崩溃。那么`pthread_create`为什么会失败呢？看一下`pthread_create`的实现逻辑：

     ```c++
     // bionic/lib/bionic/pthread_create.cpp
     int pthread_create(pthread_t* thread_out, pthread_attr_t const* attr,
                        void* (*start_routine)(void*), void* arg) {
       ...
       // 栈分配
       pthread_internal_t* thread = NULL;
       void* child_stack = NULL;
       int result = __allocate_thread(&thread_attr, &thread, &child_stack);
       if (result != 0) {
         return result;
       }
       ...
     
       return 0;
     }
     ```

     而`__allocae_thread`的逻辑则如下所示：

     ```c++
     // bionic/lib/bionic/pthread_create.cpp
     static int __allocate_thread(pthread_attr_t* attr, pthread_internal_t** threadp, void** child_stack) {
       size_t mmap_size;
       uint8_t* stack_top;
     
       if (attr->stack_base == NULL) {
         // The caller didn't provide a stack, so allocate one.
         // Make sure the stack size and guard size are multiples of PAGE_SIZE.
         mmap_size = BIONIC_ALIGN(attr->stack_size + sizeof(pthread_internal_t), PAGE_SIZE);
         attr->guard_size = BIONIC_ALIGN(attr->guard_size, PAGE_SIZE);
         attr->stack_base = __create_thread_mapped_space(mmap_size, attr->guard_size);
         if (attr->stack_base == NULL) {
           return EAGAIN;
         }
         stack_top = reinterpret_cast<uint8_t*>(attr->stack_base) + mmap_size;
       } else {
         // Remember the mmap size is zero and we don't need to free it.
         mmap_size = 0;
         stack_top = reinterpret_cast<uint8_t*>(attr->stack_base) + attr->stack_size;
       }
       ...
       return 0;
     }
     ```

     再看一下`__create_thread_mapped_space`的实现原理：

     ```c++
     static void* __create_thread_mapped_space(size_t mmap_size, size_t stack_guard_size) {
       // Create a new private anonymous map.
       int prot = PROT_READ | PROT_WRITE;
       int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
       void* space = mmap(NULL, mmap_size, prot, flags, -1, 0);
       if (space == MAP_FAILED) {
         async_safe_format_log(ANDROID_LOG_WARN,
                           "libc",
                           "pthread_create failed: couldn't allocate %zu-bytes mapped space: %s",
                           mmap_size, strerror(errno));
         return NULL;
       }
     
       // Stack is at the lower end of mapped space, stack guard region is at the lower end of stack.
       // Set the stack guard region to PROT_NONE, so we can detect thread stack overflow.
       if (mprotect(space, stack_guard_size, PROT_NONE) == -1) {
         async_safe_format_log(ANDROID_LOG_WARN, "libc",
                               "pthread_create failed: couldn't mprotect PROT_NONE %zu-byte stack guard region: %s",
                               stack_guard_size, strerror(errno));
         munmap(space, mmap_size);
         return NULL;
       }
       prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, space, stack_guard_size, "thread stack guard page");
     
       return space;
     }
     ```

     可以看出这段代码的逻辑是调用`mmap`分配栈内存。这里`mmap flag`中指定了 `MAP_ANONYMOUS`，即**匿名内存映射（mapping anonymous)**。这是在Linux中分配大块内存的常用方式。其分配的是虚拟内存，对应页的物理内存并不会立即分配，而是在用到的时候，触发内核的缺页中断，然后中断处理函数再分配物理内存。

     因此，`pthread_create`失败的原因是`mmap`失败了，即是虚拟内存分配失败了。因此，`pthread_create`崩溃的原因就是

     > 创建线程过程中进程内的虚拟内存地址空间耗尽了

     那么，什么情况下虚拟地址空间才会耗尽呢？一般而言，对于32位的Linux系统而言，可用的虚拟地址空间大小为4G，其中高位的1G空间是留给内核使用的，剩下的3G空间是分配给用户使用的，假设创建一个线程需要`1M`的虚拟内存（实际往往会更大），那么对于一个什么事情都不做，只是创建线程的进程而言，最多也不过能够创建3000个线程而已。当我们的代码逻辑出现问题时，其实很容易爆掉的。

     还有最后一个问题，为什么底层提示`pthread_create failed`的时候没有直接崩溃呢？

     我们再回头看一下`__create_thread_mapped_space`的代码逻辑，其中有这样一段代码：

     ```c++
       if (space == MAP_FAILED) {
         async_safe_format_log(ANDROID_LOG_WARN,
                           "libc",
                           "pthread_create failed: couldn't allocate %zu-bytes mapped space: %s",
                           mmap_size, strerror(errno));
         return NULL;
       }  
     ```

     可以看出`mmap`失败后，直接打印log就退出了，因此并不会导致应用的崩溃。

     ## 总结

     通过对`pthread_create`创建过程的简单分析，我们可以知道不断创建新的线程而不去释放对应的资源，将会导致虚拟地址空间的耗尽而产生`OOM`，而且和`native`层内存耗尽后只会提示失败不同，由`java`层发起的线程创建则会抛出对应的`OOM`异常，导致应用崩溃。

     针对此类问题，除了在代码中需要注意及时释放线程资源以外，还需要考虑合理的执行逻辑，避免写出循环新建线程的代码来。