# 路由API设置
本部分内容是http服务端与客户端交互的路由设置约定。

如果没有特殊说明，默认为**json**格式。

默认开启主机任意网卡的**8000**号端口。

## 用户账号相关
**更新内容：添加token功能，将会在登录成功时response header中附带token信息，格式如下：**

```json
"Authorization": token
```

其中的token实际为"Bearer " + token串，客户端不必关心，直接存储转发即可。

在改密等用户操作时需要header中携带token，现在**约定ret=-2表示token错误**

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
    ret: 0/-1/-2  --- 成功/失败/token验证失败
    msg: 提示信息
}
```

## 文件相关
写在前面： 文件相关的请求均需要携带token，-2表示token验证失败

路径使用'/'连接，不要连续如'//'

### 文件列表
```
post /api/file/list
send
{
    path: 绝对路径
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
    files: [
        {
            name: 名称
            type: folder/file
            size: 大小(byte)
            time: 上次修改时间
        },
        ....
    ] // files 参数在成功时才具有意义
}
```

### 新建文件夹
```
post /api/file/newfolder
send
{
    cwd: 当前工作目录（末尾不加'/'符号）
    foldername: 新建文件夹名称
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```


### 文件重命名
```
post /api/file/rename
send
{
    cwd: 当前工作目录（末尾不加'/'符号）
    oldname: 文件旧名称
    newname: 文件新名称
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```

### 文件删除
```
post /api/file/delete
send
{
    paths: [, ,]    // 文件绝对路径，支持批量删除，对于父目录和子目录都含有的情形（以删除父目录所有内容为准）
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```

### 文件移动
```post /api/file/move
send
{
    oldcwd: 旧当前目录（末尾不加'/'符号）
    newcwd: 新当前目录（末尾不加'/'符号）
    files: [, ,]      需要移动的文件名称
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```

### 文件复制
- 考虑到文件复制会出现同名情况出现，目前暂时以覆盖的方式进行处理

```post /api/file/copy
send
{
    oldcwd: 旧当前目录（末尾不加'/'符号）
    newcwd: 新当前目录（末尾不加'/'符号）
    files: [, ,]      需要移动的文件名称
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```

### 文件上传
- 文件上传和下载为核心模块

### 文件下载
