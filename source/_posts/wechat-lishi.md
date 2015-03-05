title: 微信红包算法探讨 
tags: 
- 微信
- 红包
- algorithm
- C++
category: coding
date: 2015-02-21 16:29
---
今年过年微信红包成了全民焦点，虽然大多数的红包就一块八角的样子，还是搞得大家乐此不彼地，蛋爷我年三十晚什么都没干就守在手机旁边不是摇手机红包就是抢群红包。作为一名程序猿，自然会想了解下红包的实现细节。我在网上谷歌了下，微信目前是没有公布红包的实现细节的，所以这里就提出一个自己的方案。
##微信红包规则
红包领了不少，据观察红包主要有以下几个限制条件：
1. 所有人都能分到红包，也就是不会出现红包数值为0的情况。
2. 所有人的红包数值加起来等于支付的金额。
3. 红包波动范围比较大，约5%~8%的红包数值在平均值的两倍以上，同时数额0.01出现的概率比较高。
4. 红包的数值是随机的，并且数值的分布近似于正态分布。
<!-- more -->

这里假设红包的总金额为T，红包个数为k，第i个红包的金额为ai，红包金额生成函数为rand(之后会讨论这个函数)。

因为每个红包的最小值为0.01，所以在初始的时候为每个红包预留0.01元，那么剩余金额总数为`T-0.01*k`。

为了让每个红包金额都是随机的，红包将会由系统逐一生成，金额为当前剩余金额范围内的随机数。算法如下：
> ai = rand(T - 0.01 * k - a0 - ... - ai-1)

##正态分布的实现
由于C++等语言提供的随机函数是平均分布的，因此如果需要使红包金额近似正态分布，需要对平均分布进行[Box–Muller](http://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform)转换操作，C++实现代码如下：
```c++
#define TWO_PI 6.2831853071795864769252866
#include <cmath>
#include <cstdlib>
double generateGaussianNoise(const double mu, const double sigma)
{
    using namespace std;
    static bool haveSpare = false;
    static double rand1, rand2;
 
    if(haveSpare)
    {
        haveSpare = false;
        return (sigma * sqrt(rand1) * sin(rand2)) + mu;
    }
 
    haveSpare = true;
 
    rand1 = rand() / ((double) RAND_MAX);
    if(rand1 < 1e-100) rand1 = 1e-100;
    rand1 = -2 * log(rand1);
    rand2 = (rand() / ((double) RAND_MAX)) * TWO_PI;
 
    return (sigma * sqrt(rand1) * cos(rand2)) + mu;
}
```
函数`generateGaussianNoise`的两个参数为期望值mu和标准差sigma，显然，mu的值为当前红包的均值，令分配第i个红包时所剩总金额为Ti，所以：
> Ti = T - 0.01 * k - a0 - ... - ai-1 

易得：

> mu = Ti / (k - i)

###sigma的值
红包数额的分布并不完全符合正太分布，因为每个红包的数额都有上限和下限，所以准确地说应该是截尾正态分布，在这里红包金额范围为[0, Ti]。

剩下要做的就是确定sigma的数值，sigma的值会直接影响红包数额的分布曲线。

根据正态分布的三个sigma定理, 生成的随机数值有95.449974%几率落在(mu-2\*sigma,mu+2\*sigma)内，为了使得mu-2\*sigma = 0，sigma = mu/2。对于生成的随机数落在[0, Ti]以外区间的情况，采用截断处理，统一返回0或者Ti。也就是说，最后生成的随机数值分别有大约6%的几率为0或者大于2*mu，加上保留的0.01，符合条件3列出的情况。最后给出这部分C++的代码：

```
#include <vector>
vector<double> generateMoneyVector(const double mon, const int pics)
{
    vector<double> valueVec;
    double moneyLeft = mon - pics * 0.01;
    double mu, sigma;
    double noiseValue;

    for(int i = 0; i < pics - 1; i++)
    {
        mu = moneyLeft / (pics - i);
        sigma = mu / 2;
        noiseValue = generateGaussianNoise(mu, sigma);
        
        if(noiseValue < 0) noiseValue = 0;
        if(noiseValue > moneyLeft) noiseValue = moneyLeft;
        
        valueVec.push_back(noiseValue + 0.01);
        moneyLeft -= noiseValue;
    }

    return valueVec;
}
```
对于收到抢红包的请求的时候，只需要进行pop操作并返回即可。
##结语
这里还有一些细节没有处理，例如对返回值进行小数位数的处理等，就不做细致说明了。以上只是我对微信红包算法的一种个人猜想，有不足的地方望多指教。
##References
1. http://en.wikipedia.org/wiki/Normal_distribution
2. http://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform

###转载请注明
原文地址：http://mafagan.sinaapp.com/2015/02/21/wechat-lishi/
