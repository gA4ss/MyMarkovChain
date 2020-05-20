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

#include "nfa.hpp"
#include <iostream>
#include <map>
using namespace std;

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

int main(int argc, char const *argv[]) {
    NFA_Status A("A", A_test, A_run);
    NFA_Status B("B", B_test, B_run);
    NFA_Status C("C", C_test, C_run);
    NFA_Status D("D", D_test, D_run);
    NFA_Status E("E", E_test, E_run);
    NFA_Status F("F", F_test, F_run);
    NFA_Status G("G", G_test, G_run);
    
    //NFA_Config config;
    NFA nfa;
    //nfa._config.config_reverse = true;
    //nfa._config = config;

    nfa.connect(&A, &A);
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

    nfa.show();
    cout << endl;
    nfa.show_ways();

    if (nfa.test("A"))
        cout << "connected" << endl;
    else
        cout << "it has not way" << endl;

    cout << "-----" << endl;

    if (nfa.test("A", "B"))
        cout << "A => B connected" << endl;
    else
        cout << "A => B it has not way" << endl;
    return 0;
}
