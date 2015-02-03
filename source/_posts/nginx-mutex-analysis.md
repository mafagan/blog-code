title: nginx中的accept-mutex源码分析(一)
tags: 
- nginx
- mutex
category: coding
date: 2015-01-31 14:26
---
最近的项目遇到了些麻烦，于是就到各家项目的源码里东逛逛西逛逛，看看有没有什么值得借鉴(chao)的东西，一来二去总算是找(chao)到了一些有用的东西，同时还无意间了解了下nginx的源码结构，其中nginx对于accept的处理倒是让我有种别开生面的赶脚(菜鸟本质暴露了...)，所以这里就写下来做个小小的记录。
<!-- more -->
###nginx架构
nginx是典型的多进程模式，其中一个是master进程，其它是worker进程，顾名思义，master就是负责管理的，管理各个worker进程，worker进程自然就是干活的，具体点就是处理数据、处理请求的。

nginx所有的worker进程都会监听端口，实现方法比较简单，首先让父进程监听80端口，获得fd，fork之后子进程自然会继承监听fd。细心的童鞋会注意到，这里面至少有两个问题，一个是当请求过来的时候哪个进程能获得这个请求的处理权，另一个问题就是如何做到负载均衡。

###惊群
第一个问题涉及到一个比较经典的现象，名叫『惊群』。有时候我们会使用多个进程对一个fd进行accept，在早期的linux版本中，请求到来的时候所有的accept都会惊醒，其中一个返回成功，其它都会返回accept失败，这就是所谓的『惊群』现象。不过linux-2.6之后的版本中，这个问题得到了修复，也就是请求到达的时候，只有可以成功accept的那个进程会惊醒，而其它会继续阻塞。

这个貌似解决了问题，其实问题还没有结束。在现代的服务器设计中，很少还会用一个进程或者一个线程进行accept操作，更多地是使用多路I/O复用技术，这里以epoll为例，当把listen fd加入到epoll监听中，请求到来的时候依旧会惊醒所有的进程，真是蛋疼...好吧，我们来看看nginx的应对惊群的经典解决方案。

###nginx的accept-mutex

nginx在监听listen fd(或者说把listen fd加入到epoll中)之前首先需要去竞争一把锁，只有在获得了这把锁之后才对listen fd进行监听。获取锁是马上返回的，当获取失败的时候，nginx会结合timer事件设置最大等待时间，然后再去获取监听锁。如果进程获取到监听锁，则...还是直接看代码吧，套用linus的名言：
> Read the fucking source code.

翻译成中文就是，方便的话请阅读源代码^_^.

```
void
ngx_process_events_and_timers(ngx_cycle_t *cycle)
{
    ngx_uint_t  flags;
    ngx_msec_t  timer, delta;

    if (ngx_timer_resolution) {
        timer = NGX_TIMER_INFINITE;
        flags = 0;

    } else {
        timer = ngx_event_find_timer();
        flags = NGX_UPDATE_TIME;

#if (NGX_THREADS)

    if (timer == NGX_TIMER_INFINITE || timer > 500) {
        timer = 500;
    }

#endif
    }
	/* 检测是否启用mutex，多worker进程下一般都会启用 */
    if (ngx_use_accept_mutex) {
        if (ngx_accept_disabled > 0) {
            ngx_accept_disabled--;

        } else {
            if (ngx_trylock_accept_mutex(cycle) == NGX_ERROR) {
            /* 尝试获取锁，不管成功还是失败都会立即返回 */
                return;
            }

            if (ngx_accept_mutex_held) {
                /* 获取到锁之后添加flag */
                flags |= NGX_POST_EVENTS;

            } else {
                /* 如果获取不到锁需要结合timer事件设置下一次抢锁的时间 */
                if (timer == NGX_TIMER_INFINITE
                    || timer > ngx_accept_mutex_delay)
                {
                    timer = ngx_accept_mutex_delay;
                }
            }
        }
    }

    delta = ngx_current_msec;
	
	/* 开始epoll收集处理事件 */
    (void) ngx_process_events(cycle, timer, flags);
	
	/* delta就是epoll_wait消耗掉的时间 */
    delta = ngx_current_msec - delta;
	
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                   "timer delta: %M", delta);
	/* accept事件已经被加入到单独的任务队列并会被优先处理 */
    ngx_event_process_posted(cycle, &ngx_posted_accept_events);
	
	/* accept事件处理完之后先释放accept锁，因为其它事件的处理可能耗时较长，不要占着茅坑不睡觉 */
    if (ngx_accept_mutex_held) {
        ngx_shmtx_unlock(&ngx_accept_mutex);
    }

    if (delta) {
        ngx_event_expire_timers();
    }
	
	/* 之后可以放心处理其它事件了 */
    ngx_event_process_posted(cycle, &ngx_posted_events);
}

```

###结语
这里只是解释了nginx惊群问题的解决，负载均衡部分留到下文分解。




