title: 《STL源码剖析》读书笔记(一)——迭代器的相应类型 
tags: 
- C++
- STL
- iterator
category: coding
date: 2015-02-16 18:53
---
迭代器是一种智能指针，主要完成的工作无非就是提取它所指向的内容和成员的访问。
##迭代器的相应类型(台译作『型别』)
问题：现在有一个迭代器，在算法的实现中需要获取它的相应类型，也就是迭代器所指向对象的类型，应该怎么做？

其中的一种解决方法就是使用function template的参数推导(argument deducation)机制。
```c++

template <typename T>
void real_func(T instance)
{
        cout << instance << endl;

}

template <typename T>
void func(T iter)
{
        real_func(*iter);

}

int main()
{
    int a = 2;
    func(&a);
}
```
这种方法可以解决大多数应用的情况，巧妙地获取了迭代器所指向对象的类型。
