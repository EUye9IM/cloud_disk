# 路由API设置
本部分内容是http服务端与客户端交互的路由设置约定。

如果没有特殊说明，默认为**json**格式。

默认开启主机任意网卡的**8000**号端口。

## 用户账号相关
**更新内容：添加token功能，将会在登录成功时response header中附带token信息，格式如下：**

```json
"Authorization": token
```

其中的token实际为"Bearer " + <token>，客户端不必关心，直接存储转发即可。

在改密等用户操作时需要header中携带token，现在**约定ret=-2表示缺少token**

用户账号相关，包括登录、登出、改密等相关
### 登录
```
post /api/login
send
{
    user: 账号
    password: 密码
}
return 
{
    ret: 0/-1  --- 成功/失败
    msg: 提示信息
}
```

### 注册
```
post /api/signup
send
{
    user: 账号
    password: 密码
    confirmpassword: 确认密码(前端比较确认后)
}
return
{
    ret: 0/-1  --- 成功/失败
    msg: 提示信息
}
```

### 登出
```
post /api/logout
send
{
    user: 账号
}
return 
{
    ret: 0/-1  --- 成功/失败
    msg: 提示信息
}
```

### 改密
```
post /api/changepass
send
{
    user: 账号
    oldpassword: 旧密码
    newpassword: 新密码
    confirmpassword: 新密码的确认密码
}
return 
{
    ret: 0/-1/-2  --- 成功/失败/缺少token
    msg: 提示信息
}
```

## 文件相关
