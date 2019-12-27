# GetActiveWindow
在内核中获取当前系统激活的窗口和进程


思路：

在内核中使用
NtUserGetForegroundWindow 获取当前窗口句柄

NtUserQueryWindow通过窗口句柄获取进程id

ZwQuerySystemInfomation遍历所有进程找到对应的进程
