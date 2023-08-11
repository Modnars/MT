# MT

> _A project of the third MoreFun Young Talent Traing Program._

## 亮点

使用 `C++20` 原生协程来实现一些常用的功能组件，并将其应用至实际的 `RPC` 场景，给出了一种具体的实现应用方案。

本项目兼顾了业务开发者（库应用端）的 **易用性** 与库性能的 **高效性**。是一种将 `C++20` 协程投入实际生产落地的具体方案，对于细节拓展，也为后续开发者尽可能预留了功能拓展的可能性。

对于实际业务场景来说，注册新的 `Service` 类型族，新的 `Method` 业务功能，都变得非常简单。在本库及相应的插件支持下，业务代码只需要实现相应的 `Method` 内部逻辑即可：

```cpp
mt::Task<int> protocol::DemoServiceImpl::Echo(::google::protobuf::RpcController *controller,
                                              const ::protocol::EchoReq &req, ::protocol::EchoRsp &rsp,
                                              ::google::protobuf::Closure *done) {
    LLOG_TRACE("req: %s", req.ShortDebugString().c_str());
    if (req.msg().size() > 0UL && req.msg()[0UL] != 'A') {
        protocol::EchoReq inner_req;
        protocol::EchoRsp inner_rsp;
        inner_req.set_msg(std::string("ACK: ") + req.msg());
        std::uint64_t uid = 1UL;  // 路由到另一个服务进程（用 uid % channels_num 来路由）
        LLOG_INFO("send req|req:%s", inner_req.ShortDebugString().c_str());
        auto ret = co_await DemoServiceStub::Echo(uid, inner_req, &inner_rsp);
        CO_COND_RET_WLOG(ret != 0, ret, "call DemoServiceStub::Echo failed|ret:%d", ret);
        LLOG_TRACE("get inner rsp: %s", inner_rsp.ShortDebugString().c_str());
        rsp.set_msg(inner_rsp.msg() + " @2");
        co_return 0;
    }
    rsp.set_msg(req.msg() + " @1");
    LLOG_TRACE("rsp: %s", rsp.ShortDebugString().c_str());
    co_return 0;
}
```

对于业务中需要向其他服务请求并获得回包数据的场景，会自动挂起当前协程，发送一个 RPC 请求，等到收到相应 RPC 应答回包时，自动恢复当前协程并继续执行。使用协程来编写异步代码，直观性、高效性在此得到充分展现。值得注意的是，`C++20` 下，对于协程调用，这里需要使用 `co_await` 关键字来等待协程执行，相较于以往栈式协程（比如 libco 协程库）而言，可能多了一个语言的关键字，同时要求逻辑本身写在一个协程中（比如本项目会**自动生成代码**，但注意生成的代码体本身是一个协程）。这本身并不能算是一个缺点，而是 `C++20` 语言标准下的一个特点——协程调用就用 `co_await` 来标识（或者说得具体一些，是需要等待协程执行返回值的场景下）。相应的，对于协程内的返回值返回，使用 `co_return` 而非传统函数的 `return`。

对于 `C++` 标准的发展趋势来看，未来 `execution` 进入标准可能会极大弱化这样的协程关键字使用。但（可能）处于过渡阶段的当下，熟悉并使用这样的机制还是很必要的。

## 构建

```shell
$ git clone https://github.com/Modnars/MT --recursive
```

```shell
$ cd MT/ && ./mt_build.sh
```

## 仓库目录

```shell
$ tree . -F -L 1
.
├── 3rd/
├── CMakeLists.txt
├── docs/
├── include/
├── mt_build.sh*
├── README.md
├── rpc/
├── src/
└── test/
```

- `3rd` - 第三方仓库，MT 用到的第三方仓库包括：
    - `Catch2` - 用于单元测试
    - `fmt` - 用于格式化输出
    - `llbc` - 用于 `rpc` 底层网络包管理
    - `nanobench` - 用于压力测试
    - `protobuf-3.20.3` - 用于网络传输协议支持
- `CMakeLists.txt` - cmake 构建文件
- `docs` - 说明文档目录
- `include` - `mt` 库代码头文件目录
- `mt_build.sh` - 便于构建使用的构建脚本
    - 执行时自动创建构建目录 `build`，默认使用构建过程中的缓存文件继续构建，接参数 `all` 时清除 `build` 目录重新构建。
- `README.md` - 此文档
- `rpc` - `mt` 协程库与 `RPC` 框架结合的应用
- `src` - `mt` 库实现源码
- `test` - `mt` 库的测试代码目录

