# ChargePort_Schedule
智能充电桩调度系统小组作业

## DDL
6月7日23:30：作业3 [https://docs.qq.com/doc/DSHlpWEJxUnBGWU9R?u=8a51e56e7cee484a9ced038766512017](https://docs.qq.com/doc/DSHlpWEJxUnBGWU9R?u=8a51e56e7cee484a9ced038766512017)

6月12日23:30：个人代码任务完成 + 整体模块间能够跑通

## 成员分工

### User:（YY PL）
with server
登录、注册、注销
充值、扣费
查看排队结果（查看前车等待数量、查看排队号码）
With ChargePort
提交充电申请、修改充电申请
查看充电详情


### ChargePort:(YL)
with user:
接收充电申请
接收充电申请的修改
with admin:
接收开启、关闭充电桩命令；
接收充电桩状态信息查询申请；


### Admin：(LZ)
with ChargePort：
查询充电桩信息（查询充电桩状态信息、查询车辆等候信息）
开启充电桩
关闭充电桩


### Server：(ZSY)
with User：
提供充电服务（调度策略生成、车辆排队号码生成、计费）
with Admin
用户信息维护
充电桩监控
数据统计


## 组内评分标准
### 基础分数（90分）

### 加分项（加分上限为10分）
按时完成指定任务（5分）+ 积极沟通互帮互助（5分）

### 扣分项（减分上限为10分）：
酌情扣分：
任务未完成/任务进度滞后/个人负责任务参与比例低于70%/超过3次未提交每日进度小结

 
