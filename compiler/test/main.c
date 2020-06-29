// assert prints out reason and exits with code 1 if the given assertion is false (= 0).
int assert(int assertion, char *reason) {
    if (assertion != 0) {
        return 0;
    }
    printf(reason);
    printf("\n");
    exit(1);
}

int assertEquals(int got, int want, char *reason) {
    if (got == want) {
        return 0;
    }
    printf(reason);
    printf("\n");
    printf("want: %d, but got: %d\n", want, got);
    exit(1);
}

// assert test_1 returns 5
int test_1() {
    return 5;
    return 8;
}

// assert test_2 returns 1
int test_2() {
    if (1 == 1)
        return 1;
    else
        return 2;
}

// assert test_3 returns 5
int test_3() {
    int i;
    i = 5;
    if (i == 4)
        return 3;
    return 5;
}

// assert test_4 returns 5
int test_4() {
    int i;
    i = 0;
    for (; i < 5; )
        i = i + 1;
    return i;
}

// assert test_5 returns 16
int test_5() {
    int ans;
    ans = 1;
    int i;
    for (i = 0; i < 4; i = i + 1)
        ans = ans * 2;
    return ans;
}

// assert test_6 returns 17
int test_6() {
    int test;
    test = 20;
    while (test / 3 != 5)
        test = test - 1;
    return test;
}

// assert test_7 returns 7
int test_7() {
    int i;
    i = 2;
    if (i == 2) {
        i = i + 1;
        i = i * 2 + 1;
    }
    return i;
}

// for test: genuine fibonacci recursive function
int fib(int x) {
    if (x <= 1) return x;
    return fib(x - 1) + fib(x - 2);
}

// Takeuchi function
int tarai(int x, int y, int z) {
    if (x <= y) return y;
    return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y));
}

// assert test_8 returns 3
int test_8() {
    int x;
    x = 3;
    int *y;
    y = &x;
    return *y;
}

// assert test_9 returns 3
int test_9() {
    int x;
    int *y;
    y = &x;
    *y = 3;
    return x;
}

// assert test_10 returns 3
int test_10() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    return *p + *(p + 1);
}

// assert test_11 returns 8
int test_11() {
    int a[2];
    a[0] = 5;
    a[1] = 3;
    return a[0] + a[1];
}

// global variables test
int a;

// assert test_12 returns 110
int test_12() {
    int *p;
    p = &a;
    *p = 110;
    return *p;
}

int arr[2];

// assert test_13 returns 8
int test_13() {
    int x;
    x = 5;
    arr[0] = 1;
    arr[1] = 2;
    int *p;
    p = arr;
    return p[0] + arr[1] + x;
}

int y[3];

// assert test_14 returns 18
int test_14() {
    int x;
    x = 5;
    int i;
    for (i = 0; i < 3; i = i + 1) {
        y[i] = i * 3 + 3;
    }
    if (x == 5) {
        return y[0] + y[1] + y[2];
    } else {
        return y[0] + y[1];
    }
}

// assert test_15 returns 3
int test_15() {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[0] + y;
}

// assert test_16 returns 97
int test_16() {
    char *x;
    x = "abc";
    return x[0];
}

// global variable initializers test

int gv_17 = 5;
char gv_18[7] = "foobar";
int *gv_19 = &gv_17;
char *gv_20 = gv_18 + 3;

// assert test_17 returns 5
int test_17() {
    return gv_17;
}

// assert test_18 returns 102
int test_18() {
    return gv_18[0];
}

// assert test_19 returns 5
int test_19() {
    return *gv_19;
}

// assert test_20 returns 97
int test_20() {
 return gv_20[1];
}

// assert test_20


int main() {
    assert(1 == 1, "1 == 1 assertion failure");
    assert(0 != 1, "0 != 0 assertion failure");

    assertEquals(55 + 5 - 3, 57, "55 + 5 - 3 does not equal to 57");
    assertEquals(5 + 6 * 7, 47, "5 + 6 * 7 does not equal to 47");
    assertEquals(8 - 3 - 2, 3, "8 - 3 - 2 does not equal to 3");
    assertEquals(5 * (9 - 6), 15, "5 * (9 - 6) does not equal to 15");
    assertEquals((5 + 4) / 2, 4, "(5 + 4) / 2 does not equal to 4");

    assertEquals(4 - 5, -1, "4 - 5 does not equal to -1");
    assertEquals(-10 + 20, 10, "-10 + 20 does not equal to 10");
    assertEquals(5 + -10, -5, "5 + -10 does not equal to -5");

    assertEquals(10 == 2 + 2 * 4, 1, "10 == 2 + 2 * 4 does not equal to 1");
    assertEquals(10 != 10, 0, "10 != 10 does not equal to 0");
    assertEquals(10 > 5, 1, "10 > 5 does not equal to 1");
    assertEquals(2 * 5 >= 11, 0, "2 * 5 >= 11 does not equal to 0");
    assertEquals(10 <= 5, 0, "10 <= 5 does not equal to 0");
    assertEquals(5 < 2 * 3, 1, "5 < 2 * 3 does not equal to 1");

    int a;
    a = 10;
    assertEquals(a, 10, "variable a does not equal to 10 after substitution");
    assertEquals(a + 5, 15, "a + 5 does not equal to 15");
    int b;
    b = 4 + a;
    assertEquals(a * b, 140, "a * b does not equal to 140");

    assertEquals(test_1(), 5, "return value of test_1 does not equal to 5");
    assertEquals(test_2(), 1, "return value of test_2 does not equal to 1");
    assertEquals(test_3(), 5, "return value of test_3 does not equal to 5");
    assertEquals(test_4(), 5, "return value of test_4 does not equal to 5");
    assertEquals(test_5(), 16, "return value of test_5 does not equal to 16");
    assertEquals(test_6(), 17, "return value of test_6 does not equal to 17");
    assertEquals(test_7(), 7, "return value of test_7 does not equal to 7");

    assertEquals(fib(7), 13, "fib(7) does not equal to 13");
    assertEquals(tarai(10, 6, 0), 10, "tarai(10, 6, 0) does not equal to 10");

    assertEquals(test_8(), 3, "return value of test_8 does not equal to 3");
    assertEquals(test_9(), 3, "return value of test_9 does not equal to 3");

    int x;
    int *y;
    char ch;
    assertEquals(sizeof(x), 4, "sizeof(x) does not equal to 4");
    assertEquals(sizeof(y), 8, "sizeof(y) does not equal to 8");
    assertEquals(sizeof(ch), 1, "sizeof(ch) does not equal to 1");
    assertEquals(sizeof(x + 3), 4, "sizeof(x + 3) does not equal to 4");
    assertEquals(sizeof(y + 3), 8, "sizeof(y + 3) does not equal to 8");
    assertEquals(sizeof(*y), 4, "sizeof(*y) does not equal to 4");
    assertEquals(sizeof(1), 4, "sizeof(1) does not equal to 4");
    assertEquals(sizeof(sizeof(1)), 4, "sizeof(sizeof(1)) does not equal to 4");

    assertEquals(test_10(), 3, "return value of test_10 does not equal to 3");
    assertEquals(test_11(), 8, "return value of test_11 does not equal to 8");

    assertEquals(test_12(), 110, "return value of test_12 does not equal to 110");
    assertEquals(test_13(), 8, "return value of test_13 does not equal to 8");
    assertEquals(test_14(), 18, "return value of test_14 does not equal to 18");

    assertEquals(test_15(), 3, "return value of test_15 does not equal to 3");
    assertEquals(test_16(), 97, "return value of test_16 does not equal to 97");

    assertEquals(test_17(), 5, "return value of test_17 does not equal to 5");
    assertEquals(test_18(), 102, "return value of test_17 does not equal to 5");
    assertEquals(test_19(), 5, "return value of test_17 does not equal to 5");
    assertEquals(test_20(), 97, "return value of test_17 does not equal to 5");

    /*
    This is a block comment
    */

    return 0;
}