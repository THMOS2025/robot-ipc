# TODO
1. Leave an interface for modifying the length of the shared memory queue
2. 思考一下如何解决host_variable需要手动删除的问题
3. 完善logger，读取数据（可以实现命令行显示，像ros一样），存储数据（每个topic存一个或继承存储）
4. server通讯方式
5. 多写一些example（尤其是python的）
6. host_variable可以在read中判断读到的数据是否比上一次读的数据更新