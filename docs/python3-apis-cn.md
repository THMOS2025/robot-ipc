# Python API

Python API 与 C/C++ 不同，因为 PyObject 在 Python 描述符中的内存布局不同。对于相同的 host_variable/host_function，两端都必须是 Python 的，除非：a. 你使 pickle 后的内存布局与 C 结构体完全相同；b. 你在 C 代码中处理它。

Python 非常灵活，即使使用 pickle 也没有固定的内存布局长度。然而，底层 C 库是为具有固定大小的 C 结构体设计的。因此，在 Python API 中，你可以为每个组件设置最大大小，默认情况下为 4096 字节（4KB）。如果 pickle 后的数据长度超过了你设置的最大大小，库会抛出异常。但请记住，对于特定的共享对象，所有使用该共享对象的用户都必须设置相同的 max_size 参数（不同的 max_size 可以不同）。

### 安装

1. 克隆此项目
2. `mkdir build && cd build`
3. `cmake .. && sudo make install -j4`
4. 将文件 python3/robot_ipc.py 复制（或符号链接）到你的项目中
5. 确保 `ctypes` 和 `pickle` 已在你的 Python 环境中安装。它们默认已安装。

* * *

### 主机变量

- `class HostVariable(name: str, max_size: int = 4096)`
- name：用于标识变量的字符串
- max_size：变量的大小。变量越小，执行速度越快。但你的 Python 对象在 pickle 后必须小于最大大小。
- `HostVariable.data`
- 将其视为一个简单的本地 Python 变量。每当你写入它时，数据都会在整个主机范围内同步。

> ### 注意 - 锁
>
> 您应该避免在共享变量正在读写时终止程序（ctrl + c 或 sigkill 9）。由于“锁”是通过手动设置的字节实现的，因此在获取“锁”但未释放时终止程序会导致插槽死锁。虽然默认有 14 个插槽，并且所有插槽都死锁的可能性非常小，但一旦发生这种情况，变量就会卡住，直到您移除共享内存并重新启动每个使用它的进程。通常情况下，这个过程非常快，在读写时终止的可能性非常小。但是，如果您以极高的频率（至少 10kHz，取决于您的硬件）更新变量，并因此终止并重新启动它，则很有可能导致一个插槽死锁。因此，请尽可能优雅地处理 ctrl + c 信号。
>
> 如果您处于调试阶段，并且不太在意延迟，则可以在 DEBUG 模式下构建（并安装）动态库。在该模式下，ctrl-C 信号在锁定前被禁用，并在释放锁定后启用。

### 主机函数调用者

- `class HostFunctionCaller(name: str, max_sz_args: int = 4096, max_sz_ret: int = 4096)`
- name：用于标识变量的字符串
- max_sz_args：传递参数的最大大小（pickle 后）
- max_sz_ret：返回响应的最大大小（pickle 后）
- `__call__(*args, **kwargs)`：像普通函数一样调用实例
- `.get_response()`：等待响应并返回。

> ### 注意 - 响应
>
> 如果调用 `get_response()`，请确保远程函数确实返回了某些内容（而不是 `None`）。否则，您的进程将卡在此处。如果您在回调函数中确实返回了某些内容，请记住在缓冲区已满后调用 `get_response()`，否则隧道将卡住。

>
> 以下情况下没有响应：

>
> - `max_sz_ret` 设置为 0。
> - 您的回调函数明确返回 `None`。

> - 您的回调函数未返回

### 主机函数调度器

- `class HostFunctionDispatcher(max_func_count: int)`
- max_func_count：您要添加到此调度器的函数数量。
- `.add(name: str, foo: function, max_sz_args = 4096, max_sz_ret = 4096)`
- name：宿主函数的标识符。
- foo：回调函数。
- max_sz_arg：传递参数的最大大小（pickled 后）
- max_sz_ret：返回响应的最大大小（pickled 后）

> ### 警告
>
> 请确保 max_sz_arg 和 max_sz_ret 与调用方设置的值一致。否则此隧道将停止工作。

- `.start()`
- 将调度器作为守护线程启动。

- 启动后可以添加其他函数（`.add()`），但尚未经过全面测试，可能会失败。

- `__del()__`
- 当实例被垃圾回收时，调度器将停止工作。
