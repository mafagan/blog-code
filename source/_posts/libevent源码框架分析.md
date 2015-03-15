title: Libevent源码分析
tags: [libevent, unix, signal, ]  
date: 2014-12-26 10:25
category: coding
---
libevent是一个事件触发(**Reactor**)的网络库，适用于windows、linux、bsd等多种平台，内部使用select、epoll、kqueue等系统调用管理事件机制。与其它网络库相比，libevent最大的特点就是很好的整合了I/O、timer以及signal的处理。之前我曾经在实习项目中用到过libevent，由于当时对整个libevent的运行方式不太熟悉，导致设计程序框架的时候多了很多不必要的考虑和编码，所以就在这里对libevent的核心部分进行一次剖析。
<!-- more -->
##libevent的使用
对于libevent来说，I/O, timer以及signal都是一个event，其内部已经封装好了对这些事件的处理，因此用户调用的时候只需要注册关心的事件以及事件的处理函数(即回调函数),然后等待事件的即可。问题的关键是libevent是怎么整合这些事件的？下面我来解释一下。

**注：关于libevent的详细使用方法可以查阅[官方手册](http://www.wangafu.net/~nickm/libevent-book/)(貌似要番嫱。。)**
##libevent的主体框架
libevent的设计比较清晰明了，核心部分就是一个主循环，负责监控各个事件，并维护一个活跃链表(**active list**)，对于就绪的事件，libevent就把它插入到活跃链表中，然后按照优先级顺序分别执行活跃链表中得事件的注册回调函数。

从主循环入口开始，程序要先先检测退出标识，若有则退出循环，否则继续。记录当前时间，获取timer事件中最早到期事件的时间，这里说下timer事件存储的数据结构。timer中得事件根据超时时间的大小使用小根堆得方式存储，因此插入和删除都是O(logn)的时间复杂度，而获取最早超时的事件只需要直接读取堆顶得数据即可。获取最早超时的时间和当前时间的时间差作为epoll等函数的超时时间，这就避免了因为没有I/O操作导致程序错过了执行timer事件的情况。处理完I/O事件之后，就可以及时地继续处理timer事件了。

下面我们来看一下源代码：
``` c
while (!done) {
    base->event_continue = 0;

    /* Terminate the loop if we have been asked to */
    if (base->event_gotterm) {
        break;
    }

    if (base->event_break) {
        break;
    }

    timeout_correct(base, &tv);

    tv_p = &tv;
    if (!N_ACTIVE_CALLBACKS(base) && !(flags & EVLOOP_NONBLOCK)) {
        timeout_next(base, &tv_p);
    } else {
        /*
         * if we have active events, we just poll new events
         * without waiting.
         */
        evutil_timerclear(&tv);
    }

    /* If we have no events, we just exit */
    if (!event_haveevents(base) && !N_ACTIVE_CALLBACKS(base)) {
        event_debug(("%s: no events registered.", __func__));
        retval = 1;
        goto done;
    }

    /* update last old time */
    gettime(base, &base->event_tv);

    clear_time_cache(base);

    /* 等待I/O事件，并设定最大等待时间 */
    res = evsel->dispatch(base, tv_p);

    if (res == -1) {
        event_debug(("%s: dispatch returned unsuccessfully.",
                    __func__));
        retval = -1;
        goto done;
    }

    update_time_cache(base);

    /* 处理到时事件 */
    timeout_process(base);

    if (N_ACTIVE_CALLBACKS(base)) {

        /* 处理活跃链表中得事件 */
        int n = event_process_active(base);
        if ((flags & EVLOOP_ONCE)
                && N_ACTIVE_CALLBACKS(base) == 0
                && n != 0)
            done = 1;
    } else if (flags & EVLOOP_NONBLOCK)
        done = 1;
}
```

主循环的框架如下图:

![libevent_loop](http://ccxcu.img43.wal8.com/img43/507748_20150118041318/142152564418.jpg)

上文只是解释了I/O和timer的工作流程，还有signal部分尚未讲解。signal对于程序来说是完全异步的，你完全不知道注册的回调函数会在什么时候调用，那么libevent是怎么把signal整合进去的呢？这也是我一开始对libevent感到最好奇的地方。

##libevent对signal的处理
细心的朋友可能已经在上面的框架图中找到了答案，没错，libevent把signal信号转换成了I/O数据。具体的方法就是使用socket pair进行数据交流。在实际操作中，生成一对socket, 其中一个为读socket，另一个为写socket，写socket得数据自然是流向读socket。在捕捉到信号之后，libevent就会找到这个写socket，向里面写入一个8位的数据，值就是信号值。当然，在用户注册了监控的信号之后，libevent就已经把读socket列入监控的范围，在下一次epoll_wait操作的时候自然就可以获取信号的相关信息。

signal部分的源代码：
``` c
static void __cdecl evsig_handler(int sig)
{
    int save_errno = errno;
#ifdef WIN32
    int socket_errno = EVUTIL_SOCKET_ERROR();
#endif
    ev_uint8_t msg;

    if (evsig_base == NULL) {
        event_warnx(
                "%s: received signal %d, but have no base configured",
                __func__, sig);
        return;
    }

#ifndef _EVENT_HAVE_SIGACTION
    signal(sig, evsig_handler);
#endif

    /* Wake up our notification mechanism */
    msg = sig;
    send(evsig_base_fd, (char*)&msg, 1, 0);
    errno = save_errno;
#ifdef WIN32
    EVUTIL_SET_SOCKET_ERROR(socket_errno);
#endif
}
```
#总结
libevent的代码放置比较随意，都在根目录下，但是整体设计还是比较简洁明了的。libevent的想法是想提供一套全面跨平台的网络I/O解决方案，因此性能也没有达到极致。当然，我认为libevent在设计上最大的缺点就是使用了全局变量。因此，我更建议使用libev，从名字看就知道，两者的功能相近，设计相似(怎么有点某鹅的赶脚。。)，但是libev有着更少的bug，去掉了该死的全局变量，专注于posix，秉承unix“一个程序只做一件事，并做好它”，因此我在接下来的项目中也会采用libev。
