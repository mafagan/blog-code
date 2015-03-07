title: leetcode pow(x,n)解题报告
tags: 
- leetcode
- algorithm
- C++
category: coding
date: 2015-03-08 01:52
---

题目很简洁，就是实现pow(x,n)。

##解题思路
直接乘n遍x，时间复杂度O(n)，超时。

二分法，这个比较容易想到，网上也有不少类似的解法，时间复杂度O(logn)，可以通过。

还有一种方法：

对比下这条式子:

![pow_x_n](http://mafagan-img.stor.sinaapp.com/CodeCogsEqn.gif)

用二进制角度来看n，第i个比特位上的1等价于x^(2^i)，因此可以先打表，保存`x^(2^0)...x(2^(sizeof(int)*8))`的值，然后遍历n的每个比特位进行计算，时间复杂度O(1)。

注意事项：
* n小于0的情况。
* n为int可表示的最小值的情况。

Accept代码如下：

```
class Solution {
public:
    double pow(double x, int n) {
        bool flag = false;
        
        if(n==0) return 1;
        
        if(n<0){
            x = 1/x;
            n = -1 * (n + 1); //INT_MIN不可以直接乘-1
            flag = true;
        }
        
        vector<double> res;
        double tmp = x;
        
        res.push_back(1);
        res.push_back(x);
        
        for(int i=2; i<sizeof(int)*8; i++) res.push_back(res[i-1]*res[i-1]);

        int bits = n;
        double retvalue = 1;

        for(int i=0; i<sizeof(int)*8; i++){
            if((bits&0x01) != 0) retvalue *= res[i+1];
            bits = bits >> 1;
        }

        if(flag) return retvalue*x;
        else return retvalue;
    }
};
```

差点忘了，三八妇女节快乐^_^！
