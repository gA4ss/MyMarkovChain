# yMarkov
马尔科夫链


## NFA模块
不确定有限状态机(_Nondeterministic Finite Automata_)

本程序通过设定的链接后，将不确定有限状态机(_NFA_)，转换为确定性有限状态机(_DFA_)．并按照节点链接的数量从小到大依次排列．优先
测试最少的路经．每次链接后程序会**更新路经列表并排序**，所以在链接时如果状态非常多则可能会慢一些，但是这样的好处是，在测试时直接
遍历路经，速度会很快．

### 测试
|名称|说明|
|----|---|
|`nfa.test(int src_id)`|测试从`src_id`开始是否有一条到达**接受状态**的路经|
|`nfa.test(int src_id, int dst_id)`|测试从`src_id`开始是否有一条到达`dst_id`的路经|

**以上的`int src_id`，可以换成`char*`使用状态名称来使用．**


### 显示状态

|名称|说明|
|----|---|
|`nfa.show()`|显示状态的**邻接图**|
|`nfa.show_ways()`|显示状态的每条路径|

### 配置
`NFA_Config`,两个选项:

1. `config_skip_self`     跳过链接自身状态
2. `config_reverse`       当条件满足取逆在返回结果

### 定义测试与完成函数
一个**状态**，有一个**测试函数**,原型为`bool fptr_test()`,返回`true`满足当前测试，反之不满足．有一个**运行函数**，
原型为`int fptr_run(bool)`，参数布尔值传递的是测试函数返回的结果．此参数可以通过配置选项`config_reverse`来反转测试结果．
当到达一个节点后，会先运行`test`然后根据返回结果传递给`run`．每次验证状态都会调用这两个函数，所以即便不满足条件也可以触发`run`
中的一些代码．
```c++
bool A_test() {
    cout << "A test" << endl;
    return false;
}

int A_run(bool meet) {
    if (meet)
        cout << "A meet" << endl;
    else
        cout << "A not meet" << endl;
    return 0;
}

bool B_test() {
    cout << "B test" << endl;
    return true;
}

int B_run(bool meet) {
    if (meet)
        cout << "B meet" << endl;
    else
        cout << "B not meet" << endl;
    return 0;
}

bool C_test() {
    cout << "C test" << endl;
    return false;
}

int C_run(bool meet) {
    if (meet)
        cout << "C meet" << endl;
    else
        cout << "C not meet" << endl;
    return 0;
}

bool D_test() {
    cout << "D test" << endl;
    return true;
}

int D_run(bool meet) {
    if (meet)
        cout << "D meet" << endl;
    else
        cout << "D not meet" << endl;
    return 0;
}

bool E_test() {
    cout << "E test" << endl;
    return true;
}

int E_run(bool meet) {
    if (meet)
        cout << "E meet" << endl;
    else
        cout << "E not meet" << endl;
    return 0;
}

bool F_test() {
    cout << "F test" << endl;
    return true;
}

int F_run(bool meet) {
    if (meet)
        cout << "F meet" << endl;
    else
        cout << "F not meet" << endl;
    return 0;
}

bool G_test() {
    cout << "G test" << endl;
    return true;
}

int G_run(bool meet) {
    if (meet)
        cout << "G meet" << endl;
    else
        cout << "G not meet" << endl;
    return 0;
}
```

### 链接
```c++
NFA_Status A("A", A_test, A_run);
NFA_Status B("B", B_test, B_run);
NFA_Status C("C", C_test, C_run);
NFA_Status D("D", D_test, D_run);
NFA_Status E("E", E_test, E_run);
NFA_Status F("F", F_test, F_run);
NFA_Status G("G", G_test, G_run);

NFA nfa;
//nfa._config.config_reverse = true;

nfa.connect(&A, &A);        // 自身链接自身
nfa.connect(&A, &C);
nfa.connect(&A, &D);
nfa.connect(&D, &B);

nfa.connect(&C, &D);
nfa.connect(&D, &F);

nfa.connect(&C, &F);
nfa.connect(&C, &B);

nfa.connect(&B, &E);
nfa.connect(&E, &G);
nfa.connect(&A, &C);
nfa.connect(&A, &D);

// 测试是否有路径可以到达A
if (nfa.test("A"))
    cout << "connected" << endl;
else
    cout << "it has not way" << endl;

cout << "-----" << endl;

// 测试是否从A能到达B
if (nfa.test("A", "B"))
    cout << "A => B connected" << endl;
else
    cout << "A => B it has not way" << endl;
```

### 编译

`g++ test_nfa.cpp -std=c++11 -o ./test_nfa.out`

这里必须使用`-std=c++11`标准进行编译．