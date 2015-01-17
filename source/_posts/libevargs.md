title: liev回调函数的参数传递问题
tags: libev
dategory: coding
date: 2015-01-15 18:29
---
libev是一个类似于libevent的事件驱动型的网络库，之前用它来做项目的时候对它由很高的期望，谁知一上手就遇到了挺无语的问题，先来看下libev的官方示例：

###Libev调用示例
首先定义一个I/O变量：
``` c
ev_io stdin_readable;
```
然后初始化I/O实例：
``` c
ev_io_init (&stdin_readable, stdin_readable_cb, STDIN_FILENO, EV_READ);
```
然后开始监听：
``` c
ev_io_start (loop, &stdin_readable);
```
最后生成处理事件的回调函数：
``` c
static void stdin_readable_cb (struct ev_loop *loop, ev_io *w, int revents)
```
蛋不舒服的地方就是这个回调函数，三个参数都是libev留给自己调用的，想传个参数进去都没有办法，这个倒是显得libevent比较人性化，一个void*就把所有问题解决了。后来我猜想libev会不会在ev_io这个结构体里面给我留个小小的位置，于是就到libev的头文件里面找，然后就看到了这段代码:
``` c
typedef struct ev_io                                                                                                               
{                                                                                                                                  
    EV_WATCHER_LIST (ev_io)                                                                                                                            
    int fd;     /* ro */
    int events; /* ro */
} ev_io;  
```
好吧，遇到了个宏，我跳不就行了，跳完之后发现又有宏，好吧，我再跳，然后，还有宏！受不了了，直接出大招把所有宏替换掉：
``` bash	
gcc -E /usr/include/ev.h -o Eev.h
```
到Eev.h里面找到下面这段代码:
``` c
typedef struct ev_io                                                                                                              
{                                                                                                                                 
    int active; 
    int pending; 
    int priority; 
    void *data; 
    void (*cb)(struct ev_loop *loop, struct ev_io *w, int revents); 
    struct ev_watcher_list *next;
    int fd;
    int events;                                                                                                                     
} ev_io;
```	
不用猜就知道data变量就是我们要找的东东了。

###后记
后来我又去官网逛了一圈，发现下面这段话。。
> Each watcher has, by default, a void *data member that you can read or modify at any time: libev will completely ignore it. This can be used to associate arbitrary data with your watcher. 

官网还提供了一种很巧妙的方法，把ev_io结构放在自定义结构的首位，这样子ev_io实例的地址恰好就是自定义结构体实例的地址：
> If you need more data and don't want to allocate memory separately and store a pointer to it in that data member, you can also "subclass" the watcher type and provide your own data:

```
struct my_io
{
    ev_io io;
    int otherfd;
    void *somedata;
    struct whatever *mostinteresting;
};
struct my_io w;
ev_io_init (&w.io, my_cb, fd, EV_READ);
```
> And since your callback will be called with a pointer to the watcher, you can cast it back to your own type:

```
static void my_cb (struct ev_loop *loop, ev_io *w_, int revents)
{
    struct my_io *w = (struct my_io *)w_;
}
```
果然还是应了那句话，多看文档少作死。
