<<<<<<< HEAD
# Shootalter
2019.12.01,在kp.lua里的toTarget函数计算速度，返回值是一个函数。

在TestShootAlter.lua里调用的speed函数多加一个参数，target_point。

在task.lua里照着zgoalie函数的写法写speed函数，接受传入的vtarget_point并赋值给itarget_point，当itarget_point非空时调用kp.toTarget函数，计算ipower，赋值kick_power传入Speed中。

在Speed.lua里接受传入的kick_power并赋值给mkick_power，然后当kick_power为函数的时候调用函数kick_power(runner)，注意这里要传入参数。

计算得到mkick_power传入CSpeed里，这就进入了C++内了。

在LuaModule.cpp里的Skill_Speed函数里增加参数，接受传入的第五个参数为kick_power，传入makeItRun中。

然后在Factory.cpp里的makeItRun函数中增加player.rotdir = kick_power，因为这里没有power接口，就用已有的rotdir接口了。

然后把rotdir传入单例Speed中。在Speed.cpp中接收传入的player.rotdir并赋值给power，传入单例函数setKick中就行了。

如此方实现踢球速度的随机变化。
=======
# Shootalter
2019.12.01,在kp.lua里的toTarget函数计算速度，返回值是一个函数。

在TestShootAlter.lua里调用的speed函数多加一个参数，target_point。

在task.lua里照着zgoalie函数的写法写speed函数，接受传入的vtarget_point并赋值给itarget_point，当itarget_point非空时调用kp.toTarget函数，计算ipower，赋值kick_power传入Speed中。

在Speed.lua里接受传入的kick_power并赋值给mkick_power，然后当kick_power为函数的时候调用函数kick_power(runner)，注意这里要传入参数。

计算得到mkick_power传入CSpeed里，这就进入了C++内了。

在LuaModule.cpp里的Skill_Speed函数里增加参数，接受传入的第五个参数为kick_power，传入makeItRun中。

然后在Factory.cpp里的makeItRun函数中增加player.rotdir = kick_power，因为这里没有power接口，就用已有的rotdir接口了。

然后把rotdir传入单例Speed中。在Speed.cpp中接收传入的player.rotdir并赋值给power，传入单例函数setKick中就行了。如此方实现踢球速度的随机变化。
>>>>>>> 整理实验室电脑上的代码
