/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 devilogic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef NFA_HPP
#define NFA_HPP

#include <algorithm>
#include <iostream>
#include <string>
#include <map>
#include <vector>

struct NFA_Config {
    bool config_skip_self;              // 跳过链接自身状态
    bool config_reverse;                // 逆向得满足条件
};

typedef bool (*fptr_test)();
typedef int (*fptr_run)(bool);

class NFA_Status {
public:
    NFA_Status() : _func_test(nullptr), _func_run(nullptr), _target(true) {};
    NFA_Status(const char* name, fptr_test ftest, fptr_run frun, double weight=0.0, bool target=true) : 
        _func_test(ftest), _func_run(frun), _weight(weight), _target(target) {
        _name = name;
    };
    virtual ~NFA_Status() {};

    virtual bool test(bool reverse=false) {
        if(_func_test) {
            bool meet = _func_test();
            meet = reverse == false ? meet == _target : meet != _target;
            return meet;
        }
        throw;      // FIXME: define some exceptions
    };

    virtual int run(bool meet, bool reverse=false) {
        meet = reverse == false ? meet : !meet;
        if (_func_run)
            return _func_run(meet);
        return 0;
    };

    std::string name() const { return _name; };
    bool target() const { return _target; };
    double weight() const { return _weight; };

    void set_name(const char* name) { _name = name; };
    void set_test_func(fptr_test test_f) { _func_test = test_f; };
    void set_run_func(fptr_run run_f) { _func_run = run_f; };
    void set_target(bool target) { _target = target; };
    void set_weight(double weight) { _weight = weight; };

private:
    std::string _name;
    fptr_test _func_test;
    fptr_run _func_run;
    bool _target;
    double _weight;
};

typedef std::vector<int> nfa_way_t;
typedef std::vector<nfa_way_t> nfa_ways_t;

class NFA {
public:
    NFA() {
        _config.config_skip_self = true;
        _config.config_reverse = false;
    };
    virtual ~NFA() {};

    NFA_Status* get_status(int id) {
        if (!find_status(id))
            throw;  // FIXME
        return _status[id];
    };

    bool find_status(int id) {
        return ((id < 0) || (id >= _status.size())) ? false : true;
    };

    int find_status_id(std::string name) {
        auto iter = _hashtbl.find(name);
        if (iter == _hashtbl.end())
            return -1;
        return _hashtbl[name];
    };

    std::string find_status_name(int id) {
        auto iter = _hashtbl.begin();
        while(iter != _hashtbl.end()) {
            if(id == iter->second)
                return iter->first;
            iter++;
        }
        return "";
    };

    int is_accept_status_by_name(std::string name) {
        int id = find_status_id(name);
        if (id == -1)
            return -1;
        return is_accept_status(id);
    };

    /*
     * 判断一个状态是否是接受状态，但是这里跳过了自循环状态，接受状态也可以是自循环的
     * 
     * id: 状态的id
     */
    int is_accept_status(int id) {
        std::vector<bool> outs = _adjacency_graph[id];
        for(size_t dst_id = 0; dst_id < size(); dst_id++) {
            // FIXME: 类型转换
            if (dst_id == id) continue; // 跳过自循环状态
            if (outs[dst_id] == true)
                return false;
        }
        return true;
    };

    int connect(NFA_Status* src, NFA_Status* dst) {
        int src_id = 0, dst_id = 0;
        auto iter = _hashtbl.find(src->name());
        if (iter == _hashtbl.end()) {
            _hashtbl[src->name()] = _status.size();

            _status.push_back(src);
            _adjacency_graph.push_back(std::vector<bool>(size(), false));
            _reverse_adjacency_graph.push_back(std::vector<bool>(size(), false));
        }
        src_id = _hashtbl[src->name()];

        iter = _hashtbl.find(dst->name());
        if (iter == _hashtbl.end()) {
            _hashtbl[dst->name()] = _status.size();

            _status.push_back(dst);
            _adjacency_graph.push_back(std::vector<bool>(size(), false));
            _reverse_adjacency_graph.push_back(std::vector<bool>(size(), false));
        }
        dst_id = _hashtbl[dst->name()];

        update();
        _adjacency_graph[src_id][dst_id] = true;
        _reverse_adjacency_graph[dst_id][src_id] = true;
        update_all_ways(_config.config_skip_self);

        return 0;
    };

    /*
     * 从源找到目的地路径是否存在并返回所有可能达到的路径
     * 
     * src_id: 源状态id
     * dst_id: 目的状态id
     */
    nfa_ways_t find_ways(int src_id, int dst_id) {
        nfa_ways_t ways = _way_pointers[src_id];
        nfa_ways_t founds;
        for (size_t way_id = 0; way_id < ways.size(); way_id++) {
            if (find_status_in_way(ways[way_id], dst_id))
                founds.push_back(ways[way_id]);
        }
        return founds;
    };

    bool check_connect(int src_id, int dst_id) {
        nfa_ways_t ways = find_ways(src_id, dst_id);
        if (ways.size() == 0)
            return false;
        return true;
    };

    bool status_test_run(int id) {
        if ((id < 0) || (id >= _status.size()))
            throw;  // FIXME
        
        NFA_Status* status = _status[id];
        bool meet = status->test(_config.config_reverse);
        try {
            status->run(meet, _config.config_reverse);
        } catch (...) {
            // FIXME...
        }
        return meet;
    };

    /*
     * 遍历src的所有到达dst_id的路径，尝试在最优路径上测试状态
     * 如果全部状态都测试成功则执行目的状态
     * 
     * src_id: 源id
     */
    bool test(int src_id) {
        bool found = find_status(src_id);
        if (!found)
            return false;

        nfa_ways_t ways = _way_pointers[src_id];
        if (ways.size() == 0)
            return false;

        // 这里首先测试源本身是否测试通过
        if (!status_test_run(src_id))
            return false;

        // 找到每条都通的路
        for (auto iter = ways.begin(); iter < ways.end(); iter++) {
            auto way = *iter;
            for (auto iter_status = way.begin(); iter_status < way.end(); iter_status++) {
                int id = *iter_status;

                // 找到接受节点
                if (iter_status == way.end()-1) {
                    if (status_test_run(id) == false)
                        break;
                    else
                        return true;
                }
                // 非最终目的状态，则测试路径上的所有状态是否满足条件
                if (status_test_run(id) == false)
                    break;
            }/* end for */
        }
        return false;
    };

    bool test(const char* src) {
        int id = find_status_id(src);
        if (id == -1)
            return false;
        return test(id);
    };

    /*
     * 遍历src的所有到达dst_id的路径，尝试在最优路径上测试状态
     * 如果全部状态都测试成功则执行目的状态
     * 
     * src_id: 源id
     * dst_id: 目的id
     */
    bool test(int src_id, int dst_id) {
        bool found = find_status(src_id);
        if (!found)
            return false;

        // 在src_id的路点中搜索dst_id
        nfa_ways_t ways = find_ways(src_id, dst_id);
        if (ways.size() == 0)
            return false;

        // 这里首先测试源本身是否测试通过
        if (status_test_run(src_id) == false)
            return false;

        // 找到每条都通的路
        for (auto iter = ways.begin(); iter < ways.end(); iter++) {
            auto way = *iter;
            for (auto iter_status = way.begin(); iter_status < way.end(); iter_status++) {
                int id = *iter_status;

                // 找到目的节点
                if (id == dst_id) {
                    if (status_test_run(id) == false)
                        break;
                    else
                        return true;
                }
                // 非最终目的状态，则测试路径上的所有状态是否满足条件
                if (status_test_run(id) == false)
                    break;
            }
        }

        // 所有路线的所有状态都不满足
        return false;
    };

    bool test(const char* src, const char* dst) {
        auto iter = _hashtbl.find(src);
        if (iter == _hashtbl.end())
            return false;

        iter = _hashtbl.find(dst);
        if (iter == _hashtbl.end())
            return false;
        
        int src_id = _hashtbl[src];
        int dst_id = _hashtbl[dst];

        return test(src_id, dst_id);
    };

    /*
     * 从一个源开始搜索此源到达所有接受状态的所有路径
     * 当不跳过自循环节点时，虽然不是接受状态，但是自循环状态本身当作接受状态。
     * 在搜索过程中使用searched记录所有遍历的路点，防止二次进入形成死循环
     * 
     * src_id: 源id
     * searched: 曾今所有过的路径
     * skip_self: 是否记录自循环路径,默认是记录
     * 
     * 返回一个'nfa_ways_t'的结构，其中保存了所有路点。
     */
    nfa_ways_t search_from_source(int src_id, nfa_way_t& searched, bool skip_self=false) {
        nfa_ways_t ways;
        std::vector<bool> outs = _adjacency_graph[src_id];
        for(size_t dst_id = 0; dst_id < size(); dst_id++) {
            // 这里跳过所有的曾今搜索过的点，防止进入死循环
            if (find_status_in_way(searched, dst_id))
                continue;

            if (outs[dst_id] == true) {
                // FIXME 这里src_id是int类型，需要做类型转换
                if (src_id != dst_id) {
                    if (is_accept_status(dst_id)) {
                        nfa_way_t w;
                        w.push_back(dst_id);
                        ways.push_back(w);
                    } else {
                        searched.push_back(src_id);
                        nfa_ways_t next_ways = search_from_source(dst_id, searched, skip_self);
                        searched.pop_back();    // 弹出曾今的遍历点，保证每条路径的路点信息正确

                        // 这里存在n条路径, 将下一个节点的路点追加到当前的状态后边
                        nfa_way_t w;
                        for (size_t way_id = 0; way_id < next_ways.size(); way_id++) {
                            w.clear();
                            w.push_back(dst_id);
                            for (size_t next_id = 0; next_id < next_ways[way_id].size(); next_id++)
                                w.push_back(next_ways[way_id][next_id]);
                            ways.push_back(w);
                        }/* end for */
                    }
                } else {
                    if (skip_self == false) {
                        nfa_way_t w;
                        w.push_back(dst_id);
                        ways.push_back(w);
                    }
                }/* end else */
            }/* end if */
        }
        return ways;
    };

    bool find_status_in_way(nfa_way_t& way, int id) {
        if (way.size() == 0)
            return false;
        
        auto iter = find (way.begin(), way.end(), id);
        if (iter != way.end())
            return true;
        return false;
    };

    /*
     * 对路点进行排序,默认状态下是最短路径优先
     * ways: 当前所有的路点
     * reverse: 设置为true表示使用最长路径进行排序
     */
    void sort_ways(nfa_ways_t& ways, bool reverse=false) {
        if (reverse == false)
            sort(ways.begin(), ways.end(), [](nfa_way_t& w1, nfa_way_t& w2)->bool {return w1.size() < w2.size(); });
        else
            sort(ways.begin(), ways.end(), [](nfa_way_t& w1, nfa_way_t& w2)->bool {return w1.size() >= w2.size(); });
    };

    /*
     * 从每个源状态开始搜索，每个状态到达接受状态的路径
     * 1) 搜索从源状态开始到接受状态的路径
     * 2) 对找到的所有路径进行排序(最短路径优先)
     * 3) 根据源名称保存路径
     * 
     * skip_self: 跳过自循环链接
     */
    void update_all_ways(bool skip_self=false) {
        nfa_way_t searched;
        _way_pointers.clear();
        for(size_t src_id = 0; src_id < _adjacency_graph.size(); src_id++) {
            std::string name = find_status_name(src_id);
            if (name == "") {
                // FIXME: 这里需要定义异常类
                throw "can't found status";
            }
            
            nfa_ways_t ways = search_from_source(src_id, searched, skip_self);
            sort_ways(ways);
            _way_pointers[src_id] = ways;
        }
    };

    void _update(std::vector<std::vector<bool> > &vec) {
        for(size_t i = 0; i < vec.size(); i++) {
            size_t r = size() - vec[i].size();
            for(size_t j = 0; j < r; j++)
                vec[i].push_back(false);
        }
    };

    void update() {
        _update(_adjacency_graph);
        _update(_reverse_adjacency_graph);
    };

    size_t size() const { return _status.size(); };

    void show_ways() {
        for (auto s = _way_pointers.begin(); s != _way_pointers.end(); s++) {
            std::cout << "status: " << find_status_name(s->first) << std::endl;
            nfa_ways_t ways = s->second;
            for (auto way = ways.begin(); way < ways.end(); way++) {
                for (size_t i = 0; i < way->size(); i++) {
                    std::cout << find_status_name((*way)[i]);
                    if (i != way->size()-1)
                        std::cout << "=>";
                }
                std::cout << std::endl;
            }
        }/* end for */
    };

    void show() {
        for (auto iter_src = _adjacency_graph.begin(); iter_src < _adjacency_graph.end(); iter_src++) {
            for (auto iter_dst = iter_src->begin(); iter_dst < iter_src->end(); iter_dst++) {
                std::cout << *iter_dst << " ";
            }
            std::cout << std::endl;
        }
    };

    /*
     * 按照权重链接状态
     * 权重是一个浮点数，整数部分表示级别，小数部分表示分数
     * 级别高的链接级别低的
     * 分数高的链接分数低的
     * 上一个级别链接下一个级别
     * 禁止跨级别链接
     */
    static void combination_by_weight(NFA& nfa, std::vector<NFA_Status*>& status_set) {
        std::sort(status_set.begin(), status_set.end(), 
            [](NFA_Status* s1, NFA_Status* s2)->bool {return s1->weight() > s2->weight(); });
        for (auto status : status_set) {
            _combination_by_weight(nfa, status, status_set);
        }
    };

    static void _combination_by_weight(NFA& nfa, NFA_Status* status, 
        std::vector<NFA_Status*>& status_set) {
        for (auto test : status_set) {
            if (status->name() == test->name())
                continue;

            if ((int)status->weight() - (int)test->weight() == 1) {
                nfa.connect(status, test);
            } else if ((int)status->weight() == (int)test->weight()) {
                if (status->weight() > test->weight()) {
                    nfa.connect(status, test);
                }
            } else {
                // ...
            }
        }/* end for */
    };

public:
    /*
     * NFA的相关配置变量
     */
    NFA_Config _config;

protected:
    /*
     * 下列变量存放了所有关于状态存储以及路径链接的变量
     */
    std::map<std::string, int> _hashtbl;
    std::vector<NFA_Status*> _status;
    std::vector<std::vector<bool> > _adjacency_graph;
    std::vector<std::vector<bool> > _reverse_adjacency_graph;
    std::map<int, nfa_ways_t> _way_pointers;
};

#endif