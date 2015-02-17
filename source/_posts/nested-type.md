title: 《STL源码剖析》读书笔记(二)——typename与嵌套依赖类型
tags: 
- C++
- STL
- nested type
category: coding
date: 2015-02-17 00:56
---

在C++泛型编程中，template中的参数主要有两种声明方法，一种是typename，另一种是class。在这种情况下，typename和class是没有区别的，也就是说是使用typename还是class可以根据自己的喜好选择。

##typename在嵌套类型中的使用
typename在template中的作用可能是最为大家所熟知的，但是反倒是typename最主要的作用大家(其实就是我，不然也不会有这篇笔记啦。。)用得比较少，先看一下下面这段代码
```C++
template <class Parm>
Parm minus( Parm* instance )
{
    Parm::name * p; // 这是一个指针声明还是乘法?
}
```
假设传入instance的类型如下：
```C++
struct Real
{
    typedef int name;
    ...
};
```
像这种情况，name会被编译器视作Parm类的静态变量，但是很明显，`Parm::name *p`的本意是想声明一个指针的，这时候就需要在前面加一个`typename`，这样编译器就知道`Parm::name`是一个类型了。
```C++
template <class Parm>
Parm minus( Parm* instance )
{
    typename Parm::name * p; // 这是一个指针声明
}
```
