// Copyright Tobias Hermann 2015.
// https://github.com/Dobiasd/FunctionalPlus
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "fplus/fplus.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <ctime>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

void Test_Maybe()
{
    using namespace fplus;
    auto square = [](int x){ return x*x; };
    auto sqrtToMaybe = [](float x) {
        return x < 0.0f ? nothing<float>() :
                just(static_cast<float>(sqrt(static_cast<float>(x))));
    };
    auto sqrtToMaybeInt = [](int x) {
        return x < 0 ? nothing<int>() :
                just(fplus::round<int>(sqrt(static_cast<float>(x))));
    };
    auto IntToFloat = [](const int& x) { return static_cast<float>(x); };

    assert(maybe<int>(4) == just<int>(4));

    auto x = just<int>(2);
    maybe<int> y = nothing<int>();
    auto Or42 = bind_1st_of_2(just_with_default<int>, 42);
    auto SquareAndSquare = compose(square, square);
    assert(Or42(x) == 2);
    assert(Or42(y) == 42);
    auto squareMaybe = lift_maybe(square);
    auto sqrtAndSqrt = and_then_maybe(sqrtToMaybe, sqrtToMaybe);
    auto sqrtIntAndSqrtIntAndSqrtInt = and_then_maybe(sqrtToMaybeInt, sqrtToMaybeInt, sqrtToMaybeInt);
    assert(sqrtIntAndSqrtIntAndSqrtInt(256) == just(2));
    auto sqrtIntAndSqrtIntAndSqrtIntAndSqrtInt = and_then_maybe(sqrtToMaybeInt, sqrtToMaybeInt, sqrtToMaybeInt, sqrtToMaybeInt);
    assert(sqrtIntAndSqrtIntAndSqrtIntAndSqrtInt(65536) == just(2));
    assert(squareMaybe(x) == just(4));
    assert(squareMaybe(y) == nothing<int>());
    assert((lift_maybe(SquareAndSquare))(x) == just(16));
    auto LiftedIntToFloat = lift_maybe(IntToFloat);
    auto JustInt = just<int>;
    auto IntToMaybeFloat = compose(JustInt, LiftedIntToFloat);
    auto IntToFloatAndSqrtAndSqrt = and_then_maybe(IntToMaybeFloat, sqrtAndSqrt);
    assert(is_in_range(1.41f, 1.42f)(unsafe_get_just<float>
            (IntToFloatAndSqrtAndSqrt(4))));
    typedef std::vector<maybe<int>> IntMaybes;
    typedef std::vector<int> Ints;
    IntMaybes maybes = {just(1), nothing<int>(), just(2)};
    assert(justs(maybes) == Ints({ 1,2 }));
    assert(just(1) == just(1));
    assert(just(1) != just(2));
    assert(just(1) != nothing<int>());
    assert(nothing<int>() == nothing<int>());

    Ints wholeNumbers = { -3, 4, 16, -1 };
    assert(transform_and_keep_justs(sqrtToMaybeInt, wholeNumbers)
            == Ints({2,4}));
    assert(transform_and_concat(bind_1st_of_2(replicate<int>, 3), Ints{ 1,2 })
            == Ints({ 1,1,1,2,2,2 }));
    assert(show_maybe(just<int>(42)) == std::string("Just 42"));
    assert(show_maybe(nothing<int>()) == std::string("Nothing"));

    std::string thrown_str;
    try
    {
        throw_on_nothing(std::invalid_argument("raised"), nothing<int>());
    }
    catch (const std::exception& e)
    {
        thrown_str = e.what();
    }
    assert(thrown_str == "raised");

    maybe<int> maybe_4(4);
    maybe<int> maybe_4_copy(maybe_4);
    maybe<int> maybe_4_copy_2;
    maybe_4_copy_2 = maybe_4_copy;
    assert(maybe_4_copy_2 == just<int>(4));

    maybe<int> maybe_int_nothing;
    maybe<int> maybe_int_nothing_copy(maybe_int_nothing);
    maybe<int> maybe_int_nothing_copy_2;
    maybe_int_nothing_copy_2 = maybe_int_nothing_copy;
    assert(maybe_int_nothing_copy_2 == nothing<int>());
    assert(flatten_maybe(maybe<maybe<int>>(maybe<int>(1))) == maybe<int>(1));
    assert(flatten_maybe(maybe<maybe<int>>(maybe<int>())) == nothing<int>());
    assert(flatten_maybe(maybe<maybe<int>>()) == nothing<int>());
}

void Test_Result()
{
    using namespace fplus;
    auto square = [](int x){ return x*x; };
    auto sqrtToResult = [](float x) {
        return x < 0.0f ? error<float>(std::string("no sqrt of negative numbers")) :
                ok<float, std::string>(static_cast<float>(sqrt(static_cast<float>(x))));
    };
    auto sqrtToResultInt = [](int x) {
        return x < 0 ? error<int>(std::string("no sqrt of negative numbers")) :
                ok<int, std::string>(fplus::round<int>(sqrt(static_cast<float>(x))));
    };
    auto IntToFloat = [](const int& x) { return static_cast<float>(x); };

    auto x = ok<int, std::string>(2);
    auto y = error<int, std::string>("an error");
    auto Or42 = bind_1st_of_2(ok_with_default<int, std::string>, 42);
    auto SquareAndSquare = compose(square, square);
    assert(Or42(x) == 2);
    assert(Or42(y) == 42);

    auto squareResult = lift_result<std::string>(square);
    auto sqrtAndSqrt = and_then_result(sqrtToResult, sqrtToResult);
    assert(squareResult(x) == (ok<int, std::string>(4)));
    assert(squareResult(y) == (error<int>(std::string("an error"))));

    auto sqrtIntAndSqrtIntAndSqrtInt = and_then_result(sqrtToResultInt, sqrtToResultInt, sqrtToResultInt);
    assert(sqrtIntAndSqrtIntAndSqrtInt(256) == (ok<int, std::string>(2)));
    auto sqrtIntAndSqrtIntAndSqrtIntAndSqrtInt = and_then_result(sqrtToResultInt, sqrtToResultInt, sqrtToResultInt, sqrtToResultInt);
    assert(sqrtIntAndSqrtIntAndSqrtIntAndSqrtInt(65536) == (ok<int, std::string>(2)));

    assert((lift_result<std::string>(SquareAndSquare))(x) == (ok<int, std::string>(16)));
    auto LiftedIntToFloat = lift_result<std::string>(IntToFloat);
    auto OkInt = ok<int, std::string>;
    auto IntToResultFloat = compose(OkInt, LiftedIntToFloat);
    auto IntToFloatAndSqrtAndSqrt = and_then_result(IntToResultFloat, sqrtAndSqrt);
    assert(is_in_range(1.41f, 1.42f)(unsafe_get_ok<float>
            (IntToFloatAndSqrtAndSqrt(4))));

    typedef std::vector<result<int, std::string>> IntResults;
    typedef std::vector<int> Ints;
    typedef std::vector<std::string> Strings;
    IntResults results = {ok<int, std::string>(1), error<int>(std::string("no sqrt of negative numbers")), ok<int, std::string>(2)};

    assert(oks(results) == Ints({ 1,2 }));
    assert(errors(results) == Strings({std::string("no sqrt of negative numbers")}));

    assert((ok<int, std::string>(1)) == (ok<int, std::string>(1)));
    assert((ok<int, std::string>(1)) != (ok<int, std::string>(2)));
    assert((ok<int, std::string>(1)) != (error<int, std::string>(std::string("fail"))));
    assert(error<int>(std::string("fail")) == (error<int>(std::string("fail"))));
    assert(error<int>(std::string("fail 1")) != (error<int>(std::string("fail 2"))));

    Ints wholeNumbers = { -3, 4, 16, -1 };
    assert(transform_and_keep_oks(sqrtToResultInt, wholeNumbers)
            == Ints({2,4}));

    assert(transform_and_concat(bind_1st_of_2(replicate<int>, 3), Ints{ 1,2 })
            == Ints({ 1,1,1,2,2,2 }));
    assert(show_result(ok<int, std::string>(42)) == std::string("Ok 42"));
    assert(show_result(error<int, std::string>("fail")) == std::string("Error fail"));
    assert((to_maybe<int, std::string>(x)) == just(2));
    assert((from_maybe<int, std::string>(just(2), std::string("no error"))) == x);

    std::string thrown_str;
    try
    {
        throw_on_error(std::invalid_argument("exception string"), error<int, std::string>("failed"));
    }
    catch (const std::exception& e)
    {
        thrown_str = e.what();
    }
    assert(thrown_str == "exception string");
    thrown_str.clear();

    try
    {
        throw_type_on_error<std::invalid_argument>(error<int, std::string>("failed"));
    }
    catch (const std::exception& e)
    {
        thrown_str = e.what();
    }
    assert(thrown_str == "failed");
    thrown_str.clear();

    result<int, std::string> result_4 = ok<int, std::string>(4);
    result<int, std::string> result_4_copy(result_4);
    result<int, std::string> result_4_copy_2 = error<int, std::string>("dummy");
    result_4_copy_2 = result_4_copy;
    assert(result_4_copy_2 == (ok<int, std::string>(4)));

    result<int, std::string> result_int_string_error = error<int, std::string>("error");
    result<int, std::string> result_int_string_error_copy(result_int_string_error);
    result<int, std::string> result_int_string_error_copy_2 = ok<int, std::string>(9999);
    result_int_string_error_copy_2 = result_int_string_error_copy;
    assert(result_int_string_error_copy_2 == (error<int, std::string>("error")));
}


void Test_Compare()
{
    using namespace fplus;
    assert(is_equal_to(2)(2));
    assert(!is_equal_to(2)(3));
    assert(!is_not_equal_to(2)(2));
    assert(is_not_equal_to(2)(3));

    assert(!is_less(2, 2));
    assert(is_less(2, 3));
    assert(!is_less(3, 2));
    assert(is_less_than(3)(2));

    assert(is_less_or_equal(2, 2));
    assert(is_less_or_equal(2, 3));
    assert(!is_less_or_equal(3, 2));
    assert(is_less_or_equal_than(3)(2));

    assert(!is_greater(2, 2));
    assert(!is_greater(2, 3));
    assert(is_greater(3, 2));
    assert(!is_greater_than(3)(2));

    assert(is_greater_or_equal(2, 2));
    assert(!is_greater_or_equal(2, 3));
    assert(is_greater_or_equal(3, 2));
    assert(!is_greater_or_equal_than(3)(2));

    assert(identity(2) == 2);
    assert(always<int>(2)(5) == 2);
    auto square = [](int x){ return x*x; };
    assert(is_equal_by_and_by(square, square)(2, -2));
    assert(is_equal_by(square)(2, -2));
    assert(is_not_equal_by_and_by(square, square)(2, 3));
    assert(is_not_equal_by(square)(2, 3));

    assert(xor_bools(false, false) == false);
    assert(xor_bools(true, false) == true);
    assert(xor_bools(false, true) == true);
    assert(xor_bools(true, true) == false);

    auto int_less = [](int x, int y) { return x < y; };
    auto int_less_eq = [](int x, int y) { return x <= y; };
    assert(ord_to_eq(int_less)(1, 2) == false);
    assert(ord_to_eq(int_less)(2, 2) == true);
    assert(ord_to_eq(int_less)(2, 1) == false);
    assert(ord_to_not_eq(int_less)(1, 2) == true);
    assert(ord_to_not_eq(int_less)(2, 2) == false);
    assert(ord_to_not_eq(int_less)(2, 1) == true);
    assert(ord_eq_to_eq(int_less_eq)(1, 2) == false);
    assert(ord_eq_to_eq(int_less_eq)(2, 2) == true);
    assert(ord_eq_to_eq(int_less_eq)(2, 1) == false);
    assert(ord_eq_to_not_eq(int_less_eq)(1, 2) == true);
    assert(ord_eq_to_not_eq(int_less_eq)(2, 2) == false);
    assert(ord_eq_to_not_eq(int_less_eq)(2, 1) == true);
}

struct ExplicitFromIntStruct
{
    explicit ExplicitFromIntStruct(int x) : x_(x) {}
    int x_;
};
bool operator == (const ExplicitFromIntStruct &lhs, const ExplicitFromIntStruct & rhs) { return lhs.x_ == rhs.x_; }

int squareFunc(int x)
{
    return x * x;
}

struct squareStruct
{
    int operator()(int x) { return x*x; }
};

void Test_ContainerTools()
{
    using namespace fplus;

    auto squareLambda = [](int x) { return x*x; };

    auto is_even = [](int x){ return x % 2 == 0; };
    auto is_odd = [](int x){ return x % 2 == 1; };
    typedef std::pair<int, int> IntPair;
    typedef std::vector<int> IntVector;
    typedef std::vector<IntVector> IntVectors;
    typedef std::vector<bool> BoolVector;
    typedef std::vector<std::size_t> IdxVector;
    IntVector xs = {1,2,2,3,2};
    std::array<int, 5> xs_array = {{1,2,2,3,2}};
    IntVector xsSorted = {1,2,2,2,3};
    std::string xsShown("[1, 2, 2, 3, 2]");
    IntVector xs2Times = {1,2,2,3,2,1,2,2,3,2};

    typedef std::list<int> IntList;
    typedef std::vector<IntList> IntLists;
    IntList intList = { 1,2,2,3,2 };
    IntList intListSorted = { 1,2,2,2,3 };
    IntLists intLists = { { 1 },{ 2, 2 },{ 3 },{ 2 } };
    assert(group(intList) == intLists);

    auto int_mod_10 = [](int x) -> int { return x % 10; };

    // The following works with C++14.
    // assert(transform([](auto x) { return x*x; }, xs) == IntVector({1,4,4,9,4}));

    std::initializer_list<int> initListInts = { 1,2,2,3,2 };
    assert(transform(squareLambda, std::vector<int>(initListInts)) == IntVector({1,4,4,9,4}));
    assert(transform_convert<std::vector<int>>(squareLambda, initListInts) == IntVector({1,4,4,9,4}));
    assert(transform_convert<std::vector<int>>(squareLambda, xs_array) == IntVector({1,4,4,9,4}));

    assert(transform(squareLambda, xs) == IntVector({1,4,4,9,4}));
    assert(keep_if(is_even, xs) == IntVector({2,2,2}));
    assert(drop_if(is_even, xs) == IntVector({1,3}));
    assert(transform(squareLambda, intList) == IntList({ 1,4,4,9,4 }));
    assert(keep_if(is_even, intList) == IntList({ 2,2,2 }));
    assert(drop_if(is_even, intList) == IntList({ 1,3 }));
    assert(group(xs) == IntVectors({IntVector({1}),IntVector({2,2}),IntVector({3}),IntVector({2})}));
    assert(group_on(int_mod_10, IntVector({12,22,34})) == IntVectors({IntVector({12,22}),IntVector({34})}));
    assert(group_globally(xs) == IntVectors({IntVector({1}),IntVector({2,2,2}),IntVector({3})}));
    assert(group_globally_on(int_mod_10, IntVector({12,34,22})) == IntVectors({IntVector({12,22}),IntVector({34})}));
    assert(trim_left(1, intList) == IntList({2,2,3,2}));
    assert(trim_right(2, intList) == IntList({1,2,2,3}));
    assert(trim(0, IntVector({0,2,4,5,6,7,8,0,0})) == IntVector({2,4,5,6,7,8}));
    assert(trim_token_left(IntVector({1,2}), xs) == IntVector({2,3,2}));
    assert(trim_token_left(IntVector({0,1,2}), IntVector({0,1,2,0,1,2,7,5,9})) == IntVector({7,5,9}));
    assert(trim_token_right(IntVector({3,2}), xs) == IntVector({1,2,2}));
    assert(trim_token(IntVector({0,1}), IntVector({0,1,7,8,9,0,1})) == IntVector({7,8,9}));

    auto abs_diff_less_or_equal_3 = [](int a, int b) { return fplus::abs(b - a) <= 3; };
    assert(group_by(abs_diff_less_or_equal_3, IntVector({2,3,6,4,22,21,8,5})) == IntVectors({{2,3,6,4},{22,21},{8,5}}));

    assert(cluster_by(abs_diff_less_or_equal_3, IntVector({2,3,6,4,12,11,20,23,8,4})) == IntVectors({{2,3,6,4,12,11,8,4},{20,23}}));

    typedef std::pair<std::size_t, int> rle_pair_int;
    typedef std::vector<rle_pair_int> rle_list_int;
    IntVector rle_input = {1,2,2,2,2,3,3,2};
    rle_list_int rle_result = {
            std::make_pair(1, 1),
            std::make_pair(4, 2),
            std::make_pair(2, 3),
            std::make_pair(1, 2)};
    assert(run_length_encode(rle_input) == rle_result);
    assert(run_length_decode(rle_result) == rle_input);

    assert(without(2, intList) == IntList({ 1,3 }));

    assert(keep_idxs(IdxVector({1, 3}), xs) == IntVector({2,3}));
    assert(keep_idxs(IdxVector({3, 1}), xs) == IntVector({2,3}));
    assert(keep_idxs(IdxVector({1, 1, 3}), xs) == IntVector({2,3}));
    assert(keep_idxs(IdxVector({1, 3, 7}), xs) == IntVector({2,3}));

    assert(drop_idxs(IdxVector({1, 3}), xs) == IntVector({1,2,2}));

    assert(transform_convert<IntList>(squareLambda, xs) == IntList({ 1,4,4,9,4 }));

    assert(is_equal_by_and_by(is_even, is_even)(2, 4) == true);
    assert(is_equal_by_and_by(is_even, is_even)(1, 2) == false);
    assert(is_equal_by_and_by(is_odd, is_even)(1, 2) == true);
    assert(is_equal_to(2)(2) == true);
    assert(is_equal_to(1)(2) == false);

    assert(is_empty(xs) == false);
    assert(is_empty(IntVector()) == true);
    assert(is_not_empty(xs) == true);
    assert(is_not_empty(IntVector()) == false);
    assert(convert_container<IntList>(xs) == intList);
    assert(append(xs, xs) == xs2Times);
    assert(interweave(IntVector({1,3}), IntVector({2,4})) == IntVector({1,2,3,4}));
    assert(interweave(IntVector({1,3,5,7}), IntVector({2,4})) == IntVector({1,2,3,4,5,7}));
    assert(unweave(IntVector({0,1,2,3})) == std::make_pair(IntVector({0,2}), IntVector({1,3})));
    assert(unweave(IntVector({0,1,2,3,4})) == std::make_pair(IntVector({0,2,4}), IntVector({1,3})));

    typedef std::vector<float> FloatVector;
    assert(convert_elems<float>(xs) == FloatVector({1.0f,2.0f,2.0f,3.0f,2.0f}));

    assert(concat(intLists) == intList);
    assert(concat(IntVectors(2, xs)) == xs2Times);
    assert(repeat(2, xs) == xs2Times);
    assert(replicate(2, xs) == IntVectors({xs, xs}));
    assert(infixes(3, xs) == IntVectors({
        IntVector({1, 2, 2}),
        IntVector({2, 2, 3}),
        IntVector({2, 3, 2})}));
    typedef std::vector<std::string> string_vec;
    typedef std::pair<std::string::value_type, std::string::value_type> char_pair;
    typedef std::vector<char_pair> char_pair_vec;
    IntVector vec0123({0,1,2,3});
    IntList list0123({0,1,2,3});
    std::string ABC("ABC");
    std::string XY("XY");
    auto twoCharsToString = [](std::string::value_type x, std::string::value_type y) { std::string result; result += x; result += y; return result; };
    auto alwaysTrueCharAndChar = [](std::string::value_type, std::string::value_type) { return true; };
    assert(carthesian_product_with(twoCharsToString, ABC, XY) == string_vec({"AX", "AY", "BX", "BY", "CX", "CY"}));
    assert(carthesian_product_where(alwaysTrueCharAndChar, ABC, XY) == char_pair_vec({{'A','X'}, {'A','Y'}, {'B','X'}, {'B','Y'}, {'C','X'}, {'C','Y'}}));
    auto charAndCharSumIsEven = [is_even](std::string::value_type x, std::string::value_type y) { return is_even(x + y); };
    assert(carthesian_product_with_where(twoCharsToString, charAndCharSumIsEven, ABC, XY) == string_vec({"AY", "BX", "CY"}));
    assert(carthesian_product_where(charAndCharSumIsEven, ABC, XY) == char_pair_vec({{'A','Y'}, {'B','X'}, {'C','Y'}}));
    assert(carthesian_product(ABC, XY) == char_pair_vec({{'A','X'}, {'A','Y'}, {'B','X'}, {'B','Y'}, {'C','X'}, {'C','Y'}}));
    std::string ABCD("ABCD");
    typedef std::vector<std::list<int>> intListVec;
    assert(carthesian_product_n(2, ABCD) == string_vec({"AA", "AB", "AC", "AD", "BA", "BB", "BC", "BD", "CA", "CB", "CC", "CD", "DA", "DB", "DC", "DD"}));
    assert(carthesian_product_n(2, vec0123) == IntVectors({{0,0}, {0,1}, {0,2}, {0,3}, {1,0}, {1,1}, {1,2}, {1,3}, {2,0}, {2,1}, {2,2}, {2,3}, {3,0}, {3,1}, {3,2}, {3,3}}));
    assert(combinations(2, ABCD) == string_vec({"AB", "AC", "AD", "BC", "BD", "CD"}));
    assert(combinations(1, ABCD) == string_vec({"A", "B", "C", "D"}));
    assert(combinations(3, ABCD) == string_vec({"ABC", "ABD", "ACD", "BCD"}));
    assert(combinations(2, vec0123) == IntVectors({{0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3}}));
    assert(combinations(2, list0123) == intListVec({{0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3}}));
    assert(permutations(2, ABCD) == string_vec({"AB", "AC", "AD", "BA", "BC", "BD", "CA", "CB", "CD", "DA", "DB", "DC"}));
    assert(combinations_with_replacement(2, ABCD) == string_vec({"AA", "AB", "AC", "AD", "BB", "BC", "BD", "CC", "CD", "DD"}));
    assert(carthesian_product_n(0, vec0123) == IntVectors({IntVector()}));
    assert(permutations(0, vec0123) == IntVectors({IntVector()}));
    assert(combinations(0, IntVector()) == IntVectors({IntVector()}));
    assert(combinations(0, vec0123) == IntVectors({IntVector()}));
    assert(combinations(0, ABCD) == string_vec({""}));
    assert(combinations_with_replacement(0, vec0123) == IntVectors({IntVector()}));
    assert(power_set(std::string("xyz")) == string_vec({"", "x", "y", "z", "xy", "xz", "yz", "xyz"}));
    assert(rotations_left(std::string("abcd")) == string_vec({"abcd", "bcda", "cdab", "dabc"}));
    assert(rotations_right(std::string("abcd")) == string_vec({"abcd", "dabc", "cdab", "bcda"}));
    assert(fill_left(0, 6, IntVector({1,2,3,4})) == IntVector({0,0,1,2,3,4}));
    assert(fill_right(0, 6, IntList({1,2,3,4})) == IntList({1,2,3,4,0,0}));
    assert(fill_left(' ', 6, std::string("12")) == std::string("    12"));
    assert(intersperse(0, xs) == IntVector({1,0,2,0,2,0,3,0,2}));
    assert(fold_left(std::plus<int>(), 100, xs) == 110);
    assert(fold_left_1(std::plus<int>(), xs) == 10);
    assert(fold_right(std::plus<int>(), 100, xs) == 110);
    assert(fold_right_1(std::plus<int>(), xs) == 10);
    auto appendXToStrForFoldL = [](const std::string& str, int x) { return str + std::to_string(x); };
    auto appendXToStrForFoldR = [](int x, const std::string& str) { return str + std::to_string(x); };
    std::string emptyString;
    assert(fold_left(appendXToStrForFoldL, emptyString, xs) == "12232");
    assert(fold_right(appendXToStrForFoldR, emptyString, xs) == "23221");

    assert(scan_left(std::plus<int>(), 20, xs) == IntVector({ 20,21,23,25,28,30 }));
    assert(scan_right(std::plus<int>(), 20, xs) == IntVector({ 30,29,27,25,22,20 }));
    assert(scan_left_1(std::plus<int>(), xs) == IntVector({ 1,3,5,8,10 }));
    assert(scan_right_1(std::plus<int>(), xs) == IntVector({ 10,9,7,5,2 }));

    assert(join(IntList({0}), intLists) == IntList({1,0,2,2,0,3,0,2}));
    assert(join(std::string(", "), std::vector<std::string>({"a", "b", "sea"})) == std::string("a, b, sea"));
    std::map<int, std::string> mapToShow = {{1, "one"}, {2, "two"}};
    assert(show_cont(mapToShow) == "[(1, one), (2, two)]");
    assert(show_cont(xs) == xsShown);
    assert(show_cont_with(", ", xs) == xsShown);
    std::string xsShownNLs = "(1,2,\n"
                             " 2,3,\n"
                             " 2)";
    assert(show_cont_with_frame_and_newlines(",", "(", ")", xs, 2) == xsShownNLs);
    assert(show<int>(1) == "1");
    auto multiply = [](int x, int y){ return x * y; };
    assert(zip_with(multiply, xs, xs)
            == transform(squareLambda, xs));

    const double pi = 3.14159;
    assert(show_float<double>(0, 3, '0')(pi) == "3.142");
    assert(show_float<double>(1, 3, '0')(pi) == "3.142");
    assert(show_float<double>(2, 3, '0')(pi) == "03.142");
    assert(show_float<double>(3, 3, '0')(pi) == "003.142");
    assert(show_float<double>(3, 3, ' ')(pi) == "  3.142");
    assert(show_float<double>(1, 2, '0')(pi) == "3.14");
    assert(show_float<double>(1, 4, '0')(pi) == "3.1416");
    assert(show_float<double>(1, 7, '0')(pi) == "3.1415900");

    auto xsZippedWithXs = zip(xs, xs);
    assert(unzip(xsZippedWithXs).first == xs);
    assert(all(BoolVector()) == true);
    assert(all(BoolVector({true})) == true);
    assert(all(BoolVector({false})) == false);
    assert(all(BoolVector({true, true})) == true);
    assert(all(BoolVector({true, false})) == false);

    assert(all_by(is_even, IntVector()) == true);
    assert(all_by(is_even, IntVector({2})) == true);
    assert(all_by(is_even, IntVector({1})) == false);
    assert(all_by(is_even, IntVector({2, 2})) == true);
    assert(all_by(is_even, IntVector({2, 1})) == false);

    assert(any(BoolVector()) == false);
    assert(any(BoolVector({true})) == true);
    assert(any(BoolVector({false})) == false);
    assert(any(BoolVector({false, false})) == false);
    assert(any(BoolVector({true, false})) == true);

    assert(any_by(is_even, IntVector()) == false);
    assert(any_by(is_even, IntVector({2})) == true);
    assert(any_by(is_even, IntVector({1})) == false);
    assert(any_by(is_even, IntVector({1, 1})) == false);
    assert(any_by(is_even, IntVector({2, 1})) == true);

    assert(none(BoolVector()) == true);
    assert(none(BoolVector({true})) == false);
    assert(none(BoolVector({false})) == true);
    assert(none(BoolVector({false, false})) == true);
    assert(none(BoolVector({true, false})) == false);

    assert(none_by(is_even, IntVector()) == true);
    assert(none_by(is_even, IntVector({2})) == false);
    assert(none_by(is_even, IntVector({1})) == true);
    assert(none_by(is_even, IntVector({1, 1})) == true);
    assert(none_by(is_even, IntVector({2, 1})) == false);

    auto negateInt = [](int i) -> int { return -i; };

    assert(minimum(xs) == 1);
    assert(maximum(xs) == 3);

    assert(minimum_by(std::greater<int>(), xs) == 3);
    assert(maximum_by(std::greater<int>(), xs) == 1);

    assert(minimum_on(negateInt, xs) == 3);
    assert(maximum_on(negateInt, xs) == 1);

    assert(minimum_idx(xs) == 0);
    assert(maximum_idx(xs) == 3);

    assert(minimum_idx_by(std::greater<int>(), xs) == 3);
    assert(maximum_idx_by(std::greater<int>(), xs) == 0);

    assert(minimum_idx_on(negateInt, xs) == 3);
    assert(maximum_idx_on(negateInt, xs) == 0);

    assert(fplus::size_of_cont(xs) == 5);
    assert(fplus::size_of_cont(IntVector()) == 0);
    assert(is_not_empty(xs) == true);


    std::vector<unsigned char> uchars = {200, 202};
    typedef std::vector<double> DoubleVector;
    assert(sum(xs) == 10);
    assert(mean<int>(xs) == 2);
    assert(mean_using_doubles<unsigned char>(uchars) == 201);
    assert(median(IntVector({ 3 })) == 3);
    assert(median(IntVector({ 3, 5 })) == 4);
    assert(is_in_range(3.49f, 3.51f)(median<IntVector, float>(IntVector({ 3, 4 }))));
    assert(is_in_range(3.49, 3.51)(mean<double>(DoubleVector({ 3, 4 }))));
    assert(median(IntVector({ 3, 9, 5 })) == 5);
    assert(median(xs) == 2);
    assert(sort(reverse(xs)) == xsSorted);
    assert(sort(reverse(intList)) == intListSorted);
    assert(sort_by(std::greater<int>(), xs) == reverse(xsSorted));

    assert(sort_on(int_mod_10, IntVector({26,3,14})) == IntVector({3,14,26}));

    assert(unique(xs) == IntVector({1,2,3,2}));
    auto IsEqualByis_even = [&](int a, int b)
            { return is_even(a) == is_even(b); };
    assert(unique_by(IsEqualByis_even, xs) == IntVector({1,2,3,2}));

    assert(unique_on(int_mod_10, IntVector({2,22,3})) == IntVector({2, 3}));

    assert(all_the_same(IntVector()) == true);
    assert(all_the_same(IntVector({1})) == true);
    assert(all_the_same(IntVector({1,1,1})) == true);
    assert(all_the_same(IntVector({1,2,1})) == false);

    assert(all_the_same_on(int_mod_10, IntVector({3,13,33})) == true);
    assert(all_the_same_on(int_mod_10, IntVector({3,14,33})) == false);

    assert(all_unique(IntVector()) == true);
    assert(all_unique(IntVector({1})) == true);
    assert(all_unique(IntVector({1,2,1})) == false);
    assert(all_unique(IntVector({1,2,3})) == true);

    assert(all_unique_less(IntVector()) == true);
    assert(all_unique_less(IntVector({1})) == true);
    assert(all_unique_less(IntVector({1,2,1})) == false);
    assert(all_unique_less(IntVector({1,2,3})) == true);

    assert(all_unique_on(int_mod_10, IntVector({3,14,35})) == true);
    assert(all_unique_on(int_mod_10, IntVector({3,14,33})) == false);

    assert(is_sorted(IntVector()) == true);
    assert(is_sorted(IntVector({1})) == true);
    assert(is_sorted(IntVector({1,2,3})) == true);
    assert(is_sorted(IntVector({1,2,2})) == true);
    assert(is_sorted(IntVector({1,2,1})) == false);
    assert(is_sorted_on(int_mod_10, IntVector({51,32,43})) == true);
    assert(is_sorted_on(int_mod_10, IntVector({15,23})) == false);
    assert(is_strictly_sorted(IntVector({1,2,3})) == true);
    assert(is_strictly_sorted(IntVector({1,2,2})) == false);
    assert(is_strictly_sorted(IntVector({1,2,1})) == false);
    assert(is_strictly_sorted_on(int_mod_10, IntVector({51,32,43})) == true);
    assert(is_strictly_sorted_on(int_mod_10, IntVector({51,32,43,63})) == false);
    assert(is_strictly_sorted_on(int_mod_10, IntVector({15,23})) == false);

    auto is2 = is_equal_to(2);
    auto is3 = is_equal_to(3);
    auto is4 = is_equal_to(4);

    assert(find_first_by(is3, xs) == just(3));
    assert(find_first_by(is4, xs) == nothing<int>());
    assert(find_first_idx_by(is2, xs) == just<size_t>(1));
    assert(find_first_idx_by(is4, xs) == nothing<size_t>());
    assert(find_first_idx(2, xs) == just<size_t>(1));
    assert(find_first_idx(4, xs) == nothing<size_t>());

    assert(find_last_by(is3, xs) == just(3));
    assert(find_last_by(is4, xs) == nothing<int>());
    assert(find_last_idx_by(is2, xs) == just<size_t>(4));
    assert(find_last_idx_by(is4, xs) == nothing<size_t>());
    assert(find_last_idx(2, xs) == just<size_t>(4));
    assert(find_last_idx(4, xs) == nothing<size_t>());

    assert(nth_element<IntVector>(2)(xs) == 2);
    assert(nth_element_flipped(xs)(2) == 2);

    IntPair intPair = std::make_pair(2, 3);
    assert(fst(intPair) == 2);
    assert(snd(intPair) == 3);
    assert(swap_pair_elems(intPair) == std::make_pair(3, 2));
    assert(transform_fst(squareLambda, intPair) == std::make_pair(4, 3));
    assert(transform_snd(squareLambda, intPair) == std::make_pair(2, 9));

    assert(is_elem_of(2, xs) == true);
    assert(is_elem_of(4, xs) == false);

    assert(find_all_instances_of_token(std::string("Plus"),
        std::string("C Plus Plus is a nice language,") +
        std::string(" and FunctionalPlus makes it even nicer."))
        == std::vector<std::size_t>({ 2, 7, 46 }));
    assert(find_all_instances_of_token(std::string("xx"), std::string("bxxxxc"))
        == std::vector<std::size_t>({ 1, 2, 3 }));
    assert(find_all_instances_of_token(std::string("xy"), std::string("xyaaa"))
        == std::vector<std::size_t>({ 0 }));
    assert(find_all_instances_of_token(std::string("xy"), std::string("aaaxy"))
        == std::vector<std::size_t>({ 3 }));
    assert(find_all_instances_of_token(std::string("xx"), std::string("xxxxx"))
        == std::vector<std::size_t>({ 0, 1, 2, 3 }));
    assert(find_all_instances_of_token_non_overlapping(std::string("xx"), std::string("xxxx"))
        == std::vector<std::size_t>({ 0, 2 }));
    assert(find_all_instances_of_token(IntVector({}), IntVector({})) == IdxVector({0}));
    assert(find_all_instances_of_token(IntVector({}), IntVector({1})) == IdxVector({0,1}));
    assert(find_all_instances_of_token_non_overlapping(IntVector({}), IntVector({})) == IdxVector({0}));
    assert(find_all_instances_of_token_non_overlapping(IntVector({}), IntVector({1})) == IdxVector({0,1}));
    assert(find_first_instance_of_token(IntVector({}), IntVector({})) == just<std::size_t>(0));
    assert(find_first_instance_of_token(IntVector({}), IntVector({1,2})) == just<std::size_t>(0));
    assert(split_by_token(IntVector({}), false, IntVector({1,2})) == IntVectors({{1},{2}}));
    assert(split_by_token(IntVector({}), true, IntVector()) == IntVectors({{},{}}));
    assert(split_by_token(IntVector({}), true, IntVector({1,2})) == IntVectors({{},{1},{2},{}}));
    assert(find_all_idxs_of('h', std::string("oh, ha!"))
        == std::vector<std::size_t>({ 1, 4 }));
    assert(find_first_instance_of_token(std::string("haha"), std::string("oh, hahaha!"))
        == just<std::size_t>(4));
    assert(enumerate(xs)
        ==(std::vector<std::pair<std::size_t, int>>({{0,1}, {1,2}, {2,2}, {3,3}, {4,2}})));

    IntList v789 = { 7,8,9 };
    assert(set_range(1, v789, intList) == IntList({ 1,7,8,9,2 }));
    assert(get_range(1, 4, intList) == IntList({ 2,2,3 }));
    assert(replace_elems(2, 5, xs) == IntVector({1,5,5,3,5}));
    assert(replace_tokens(std::string("123"), std::string("_"),
            std::string("--123----123123")) == std::string("--_----__"));
    assert(take(2, xs) == IntVector({ 1,2 }));
    assert(drop(2, xs) == IntVector({ 2,3,2 }));
    assert(take(999, xs) == xs);
    assert(drop(999, xs) == IntVector());
    assert(take_while(is_odd, xs) == IntVector({ 1 }));
    assert(drop_while(is_odd, xs) == IntVector({ 2,2,3,2 }));
    assert(span(is_odd, xs) == std::make_pair(IntVector({ 1 }), IntVector({ 2,2,3,2 })));
    assert(keep_if(is2, xs) == IntVector({ 2,2,2 }));
    assert(keep_if(is3, xs) == IntVector({ 3 }));
    assert(keep_if(is4, xs) == IntVector());
    assert(find_all_idxs_of(2, xs) == IdxVector({ 1,2,4 }));
    assert(count(2, xs) == 3);
    assert(is_infix_of(IntVector({}), IntVector({})) == true);
    assert(is_infix_of(IntVector({}), IntVector({1,2})) == true);
    assert(is_infix_of(IntVector({2,3}), xs) == true);
    assert(is_infix_of(IntVector({2,3}), xs) == true);
    assert(is_infix_of(IntVector({2,1}), xs) == false);
    assert(is_prefix_of(IntVector({ 1,2 }), xs) == true);
    assert(is_prefix_of(IntVector({ 2,2 }), xs) == false);
    assert(is_suffix_of(IntVector({ 3,2 }), xs) == true);
    assert(is_suffix_of(IntVector({ 2,2 }), xs) == false);
    assert(is_subsequence_of(IntVector(), IntVector()) == true);
    assert(is_subsequence_of(IntVector(), xs) == true);
    assert(is_subsequence_of(IntVector({ 1,3 }), xs) == true);
    assert(is_subsequence_of(IntVector({ 3,1 }), xs) == false);
    typedef std::vector<IntVector> IntGrid2d;
    assert(transpose(IntGrid2d({})) == IntGrid2d({}));
    assert(transpose(IntGrid2d({ { 1, 2 } }))
            == IntGrid2d({ { 1 }, { 2 } }));
    assert(transpose(IntGrid2d({ { 1, 2 }, { 3, 4 } }))
            == IntGrid2d({ { 1, 3 }, { 2, 4 } }));
    assert(transpose(IntGrid2d({ { 1, 2, 3 }, { 4, 5, 6 } }))
            == IntGrid2d({ { 1, 4 }, { 2, 5 }, { 3, 6 } }));

    assert(sample(3, xs).size() == 3);

    typedef std::vector<ExplicitFromIntStruct> ExplicitFromIntStructs;
    ExplicitFromIntStructs explicitFromIntStructs = {
        ExplicitFromIntStruct(1),
        ExplicitFromIntStruct(2),
        ExplicitFromIntStruct(2),
        ExplicitFromIntStruct(3),
        ExplicitFromIntStruct(2)
    };

    assert(convert_elems<ExplicitFromIntStruct>(xs) == explicitFromIntStructs);

    assert(transform_with_idx(std::plus<int>(), xs) == IntVector({1+0,2+1,2+2,3+3,2+4}));

    int countUpCounter = 0;
    auto countUp = [countUpCounter]() mutable { return countUpCounter++; };
    assert(generate<IntVector>(countUp, 3) == IntVector({ 0,1,2 }));
    assert(generate_by_idx<IntVector>(squareLambda, 3) == IntVector({ 0,1,4 }));

    auto sumis_even = [&](std::size_t x, int y) { return is_even(x + y); };
    assert(keep_by_idx(is_even, xs) == IntVector({ 1,2,2 }));
    assert(keep_if_with_idx(sumis_even, xs) == IntVector({ 2,3,2 }));
    assert(drop_if_with_idx(sumis_even, xs) == IntVector({ 1,2 }));

    assert(nub(xs) == IntVector({ 1,2,3 }));
    auto bothEven = is_equal_by(is_even);
    assert(nub_by(bothEven, xs) == IntVector({ 1,2 }));
    assert(nub_on(int_mod_10, IntVector({12,32,15})) == IntVector({12,15}));

    typedef std::map<int, std::string> IntStringMap;
    typedef std::map<std::string, int> StringIntMap;
    IntStringMap intStringMap = {{1, "2"}, {4, "53"}, {7, "21"}};
    StringIntMap stringIntMap = {{ "2", 1}, { "53", 4}, { "21", 7}};
    assert(swap_keys_and_values(intStringMap) == stringIntMap);

    typedef std::vector<std::string> StringVector;
    assert(get_map_keys(intStringMap) == IntVector({1, 4, 7}));
    assert(get_map_values(intStringMap) == StringVector({"2", "53", "21"}));

    typedef std::unordered_map<int, std::string> IntStringUnorderedMap;
    typedef std::unordered_map<std::string, int> StringIntUnorderedMap;
    IntStringUnorderedMap intStringUnorderedMap = { { 1, "2" },{ 4, "53" },{ 7, "21" } };
    StringIntUnorderedMap stringIntUnorderedMapSwapped = { { "2", 1 },{ "53", 4 },{ "21", 7 } };
    assert(swap_keys_and_values(intStringUnorderedMap) == stringIntUnorderedMapSwapped);
    assert(convert_container<IntStringUnorderedMap>(intStringMap) == intStringUnorderedMap);
    assert(convert_container<IntStringMap>(intStringUnorderedMap) == intStringMap);

    std::vector<int> mapInts = { 1, 4, 7 };
    std::vector<std::string> mapStrings = { "2", "53", "21" };
    assert(create_map(mapInts, mapStrings) == intStringMap);
    assert(create_unordered_map(mapInts, mapStrings) == intStringUnorderedMap);

    assert(get_from_map(intStringMap, 1) == just<std::string>("2"));
    assert(get_from_map(intStringMap, 9) == nothing<std::string>());
    assert(get_from_map_with_def(intStringMap, std::string("n/a"), 1) == "2");
    assert(get_from_map_with_def(intStringMap, std::string("n/a"), 9) == "n/a");
    assert(map_contains(intStringMap, 1) == true);
    assert(map_contains(intStringMap, 9) == false);

    IntStringMap union_map_1 = {{0, "a"}, {1, "b"}};
    IntStringMap union_map_2 = {{0, "c"}, {2, "d"}};
    IntStringMap union_map_res = {{0, "a"}, {1, "b"}, {2, "d"}};
    IntStringMap union_map_with_res = {{0, "ac"}, {1, "b"}, {2, "d"}};
    assert(map_union(union_map_1, union_map_2) == union_map_res);
    assert(map_union_with(concat<std::vector<std::string>>, union_map_1, union_map_2) == union_map_with_res);

    typedef std::set<int> IntSet;
    IntSet intSet1 = {0,1,2,3};
    IntSet intSet2 = {2,3,4,5};
    IntSet intSet3 = {0,2};
    IntSet intSet4 = {2,3};
    IntSet intSet5 = {0,1};
    IntSet intSet6 = {0,1,2,3,4,5};
    IntSet intSet7 = {0,1,4,5};
    assert(set_includes(intSet1, intSet3) == true);
    assert(set_includes(intSet3, intSet1) == false);
    assert(set_includes(intSet1, intSet2) == false);
    assert(set_merge(intSet1, intSet2) == intSet6);
    assert(set_merge(intSet1, intSet3) == intSet1);
    assert(set_intersection(intSet1, intSet2) == intSet4);
    assert(set_difference(intSet1, intSet2) == intSet5);
    assert(set_symmetric_difference(intSet1, intSet2) == intSet7);

    typedef std::vector<std::pair<std::string, int>> StringIntPairs;
    StringIntPairs stringIntPairs = {{"a", 1}, {"a", 2}, {"b", 6}, {"a", 4}};
    auto stringIntPairsAsMapGrouped = pairs_to_map_grouped(stringIntPairs);
    auto groupNameToMedianMap = transform_map_values(median<std::vector<int>>, stringIntPairsAsMapGrouped);
    auto groupNames = transform(fst<std::string, int>, stringIntPairs);
    auto getMedianValue = bind_1st_and_2nd_of_3(get_from_map_with_def<std::map<std::string, int>>, groupNameToMedianMap, 0);
    auto groupMendianValues = transform(getMedianValue, groupNames);
    auto stringIntPairsSndReplacedWithGroupMedian = zip(groupNames, groupMendianValues);
    assert(stringIntPairsSndReplacedWithGroupMedian == StringIntPairs({{"a", 2}, {"a", 2}, {"b", 6}, {"a", 2}}));

    assert(split_at_idx(2, xs) == std::make_pair(IntVector({1,2}), IntVector({2,3,2})));
    assert(partition(is_even, xs) == std::make_pair(IntVector({2,2,2}), IntVector({1,3})));
    assert(split_every(2, xs) == IntVectors({{1,2}, {2, 3}, {2}}));
    auto splittedAt1And3 = split_at_idxs(IdxVector({1,3}), xs);
    IntVectors splittedAt1And3Dest = {IntVector({1}), IntVector({2,2}), IntVector({3,2})};
    assert(splittedAt1And3 == splittedAt1And3Dest);
    assert(split(3, true, xs) == IntVectors({IntVector({1, 2, 2}), IntVector({2})}));
    assert(split(1, true, IntVector{0,1,2}) == IntVectors({{0},{2}}));
    assert(split(2, true, IntVector{5,2,0,3}) == IntVectors({{5},{0,3}}));
    assert(split(2, true, IntVector{2}) == IntVectors({{},{}}));
    assert(split_by(is_even, true, IntList({1,3,2,2,5,5,3,6,7,9})) == IntLists({{1,3},{},{5,5,3},{7,9}}));
    assert(split_by_keep_separators(is_even, IntList({})) == IntLists());
    assert(split_by_keep_separators(is_even, IntList({2})) == IntLists({IntList({2})}));
    assert(split_by_keep_separators(is_even, IntList({2,2,3})) == IntLists({{2},{2,3}}));
    assert(split_by_keep_separators(is_even, IntList({1,3,2})) == IntLists({{1,3},{2}}));
    assert(split_by_keep_separators(is_even, IntList({1,3,2,2,5,5,3,6,7,9})) == IntLists({{1,3},{2},{2,5,5,3},{6,7,9}}));
    typedef std::map<int, std::size_t> IntSizeTMap;
    IntSizeTMap OccurrencesResult = {{1, 1}, {2, 3}, {3, 1}};
    assert(count_occurrences(xs) == OccurrencesResult);

    assert(replace_range(2, IntVector({8,9}), xs) == IntVector({1,2,8,9,2}));
    assert(insert_at(2, IntVector({8,9}), xs) == IntVector({1,2,8,9,2,3,2}));

    assert(sum(convert_container_and_elems<std::vector<int>>(std::string("hello"))) == 532);

    assert(init(xs) == IntVector({1,2,2,3}));
    assert(tail(xs) == IntVector({2,2,3,2}));

    assert(inits(xs) == IntVectors({{},{1},{1,2},{1,2,2},{1,2,2,3},{1,2,2,3,2}}));
    assert(tails(xs) == IntVectors({{1,2,2,3,2},{2,2,3,2},{2,3,2},{3,2},{2},{}}));

    auto times_two = [](int x) { return 2*x; };
    assert(iterate(times_two, 0, 3) == IntVector({}));
    assert(iterate(times_two, 1, 3) == IntVector({3}));
    assert(iterate(times_two, 2, 3) == IntVector({3,6}));
    assert(iterate(times_two, 5, 3) == IntVector({3,6,12,24,48}));
}

void Test_StringTools()
{
    using namespace fplus;
    std::string untrimmed = "  \n \t   foo  ";
    assert(trim_whitespace_left(untrimmed) == "foo  ");
    assert(trim_whitespace_right(untrimmed) == "  \n \t   foo");
    assert(trim_whitespace(untrimmed) == "foo");
    std::string text = "Hi,\nI am a\r\n***strange***\n\rstring.";
    std::vector<std::string> textAsLinesWithEmty = {
        std::string("Hi,"),
        std::string("I am a"),
        std::string("***strange***"),
        std::string(""),
        std::string("string.") };
    std::vector<std::string> textAsLinesWithoutEmpty = {
        std::string("Hi,"),
        std::string("I am a"),
        std::string("***strange***"),
        std::string("string.") };
    std::vector<std::string> textAsWords = {
        std::string("Hi"),
        std::string("I"),
        std::string("am"),
        std::string("a"),
        std::string("strange"),
        std::string("string") };
    std::vector<std::string> textSplitBySpaceOnly = {
        std::string("Hi,\nI"),
        std::string("am"),
        std::string("a\r\n***strange***\n\rstring.") };
    std::vector<std::string> textSplitBySpaceAndCommaAndLine = {
        std::string("Hi"),
        std::string("I"),
        std::string("am"),
        std::string("a"),
        std::string("***strange***"),
        std::string("string.") };

    assert(split_lines(text, true)
            == textAsLinesWithEmty);
    assert(split_lines(text, false)
            == textAsLinesWithoutEmpty);
    assert(split_words(text, false) == textAsWords);
    assert(split_words_by(text, ' ', false) == textSplitBySpaceOnly);
    assert(split_words_by_many(text, std::string{ " ,\r\n" }, false) == textSplitBySpaceAndCommaAndLine);

    assert(to_string_fill_left('0', 5, 42) == std::string("00042") );
    assert(to_string_fill_right(' ', 5, 42) == std::string("42   ") );
}

void Test_Read()
{
    using namespace fplus;

    assert(read_value<std::size_t>("42") == just<std::size_t>(42));
    assert(read_value<unsigned long>("42") == just<unsigned long>(42));
    assert(read_value<unsigned long long>("42") == just<unsigned long long>(42));
    assert(read_value<int>("42") == just<int>(42));
    assert(read_value<long>("42") == just<long>(42));
    assert(read_value<long long>("42") == just<long long>(42));
    assert(read_value<int>("-3") == just<int>(-3));
    assert(read_value<int>("twenty") == nothing<int>());
    assert(read_value<int>("3 thousand") == nothing<int>());

    assert(read_value_result<int>("42") == (ok<int, std::string>(42)));
    assert(read_value_result<int>("-3") == (ok<int, std::string>(-3)));
    assert(is_error(read_value_result<int>("twenty")));
    assert(is_error(read_value_result<int>("3 thousand")));

    assert(is_in_range(-42.4f, -42.2f)(unsafe_get_just(read_value<float>("-42.3"))));
    assert(is_in_range(-42.4 , -42.2 )(unsafe_get_just(read_value<double>("-42.3"))));
    assert(is_in_range(-42.4L, -42.2L )(unsafe_get_just(read_value<long double>("-42.3"))));
}

void Test_SideEffects()
{
    using namespace fplus;
    typedef std::vector<int> Ints;
    Ints buffer;
    //auto push_one = [&](){ buffer.push_back(1); };
    auto push_one_return_true = [&]() -> bool { buffer.push_back(1); return true; };
    auto push_one_return_false = [&]() -> bool { buffer.push_back(1); return false; };
    auto push_3_ones = execute_serially(replicate(3, push_one_return_true))();
    assert(buffer == Ints({1,1,1}));

    auto return_one = [&](){ return 1; };
    assert(execute_parallelly(replicate(4, return_one))() == Ints({1,1,1,1}));
    //execute_fire_and_forget([](){std::cout << "Fired and forgotten." << std::endl;})();

    buffer.clear();
    execute_max_n_times_until_success(5, push_one_return_false)();
    execute_max_n_times_until_success(3, push_one_return_false, 1)();
    assert(buffer == Ints({1,1,1,1,1,1,1,1}));

    buffer.clear();
    typedef std::function<bool()> BoolReturningFunction;
    typedef std::vector<BoolReturningFunction> BoolReturningFunctions;
    execute_serially_until_success(BoolReturningFunctions({push_one_return_false, push_one_return_false, push_one_return_true, push_one_return_true}))();
    assert(buffer == Ints({1,1,1}));

    buffer.clear();
    execute_serially_until_failure(BoolReturningFunctions({push_one_return_true, push_one_return_false, push_one_return_false}))();
    assert(buffer == Ints({1,1}));

    std::atomic<int> atomic_int(0);
    auto inc_atomic_int_return_true = [&]() -> bool { ++atomic_int; return true;};
    execute_parallelly(replicate(4, inc_atomic_int_return_true))();
    assert(atomic_int.load() == 4);
}

bool is_odd(int x) { return x % 2 == 1; }
void Test_example_KeepIf()
{
    typedef std::vector<int> Ints;
    Ints numbers = { 24, 11, 65, 44, 80, 18, 73, 90, 69, 18 };

    { // Version 1: hand written range based for loop
    Ints odds;
    for (int x : numbers)
        if (is_odd(x))
            odds.push_back(x);
    }

    { // Version 2: STL
    Ints odds;
    std::copy_if(std::begin(numbers), std::end(numbers),
            std::back_inserter(odds), is_odd);
    }

    { // Version : FunctionalPlus
    auto odds = fplus::keep_if(is_odd, numbers);
    }
}

void run_n_times(std::function<std::vector<int>(std::vector<int>)> f,
    std::size_t n, const std::string& name, const std::vector<int>& inList)
{
    typedef std::chrono::time_point<std::chrono::system_clock> Time;
    Time startTime = std::chrono::system_clock::now();
    size_t lengthSum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        lengthSum += f(inList).size();
    }
    Time endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    std::cout << name << "(check: " << lengthSum << "), elapsed time: " << elapsed_seconds.count() << "s\n";
}

void Test_example_KeepIf_performance()
{
    using namespace fplus;

    typedef std::vector<int> Ints;
    auto run_loop = [&](const Ints numbers)
    {
        Ints odds;
        for (int x : numbers)
            if (is_odd(x))
                odds.push_back(x);
        return odds;
    };
    auto run_stl = [&](const Ints numbers)
    {
        Ints odds;
            std::copy_if(std::begin(numbers), std::end(numbers),
                    std::back_inserter(odds), is_odd);
        return odds;
    };
    auto run_FunctionalPlus = [&](const Ints numbers)
        { return keep_if(is_odd, numbers); };

    // make debug runs faster
#if defined NDEBUG || defined _DEBUG
    std::size_t numRuns = 10;
#else
    std::size_t numRuns = 20000;
#endif

    Ints numbers = generate<Ints>(rand, 5000);
    run_n_times(run_loop, numRuns, "Hand-written for loop", numbers);
    run_n_times(run_stl, numRuns, "std::copy_if", numbers);
    run_n_times(run_FunctionalPlus, numRuns, "FunctionalPlus::keep_if", numbers);
}


void Test_example_SameOldSameOld()
{
    std::list<std::string> things = {"same old", "same old"};
    if (fplus::all_the_same(things))
        std::cout << "All things being equal." << std::endl;
}

void Test_example_IInTeam()
{
    std::string team = "Our team is great. I love everybody I work with.";
    std::cout << "There actually are this many 'I's in team: " <<
        fplus::count("I", fplus::split_words(team, false)) << std::endl;
}

struct Entity
{
    Entity() : calm_(true), bright_(true) {}
    bool calm_;
    bool bright_;
};

void Test_example_AllIsCalmAndBright()
{
    auto isCalm = [](const Entity& e) { return e.calm_; };
    auto isBright = [](const Entity& e) { return e.bright_; };
    std::vector<Entity> entities(4);
    if (fplus::all_by(fplus::logical_and(isCalm, isBright), entities))
        std::cout << "Silent night." << std::endl;
}

struct cat
{
    std::string name_;
    double softness_;
    double temperature_;
    double size_;
    double roundness_;
    double fur_amount_;
};

void Test_example_TheCutestCat()
{
    auto cuteness = [](const cat& c) -> double
    {
        return c.softness_ * c.temperature_ * c.roundness_ *
            c.fur_amount_ - c.size_;
    };
    std::vector<cat> cats = {
        {"Tigger",   5, 5, 5, 5, 5},
        {"Simba",    2, 9, 9, 2, 7},
        {"Muffin",   9, 4, 2, 8, 6},
        {"Garfield", 6, 5, 7, 9, 5}};

    auto cutest_cat = fplus::maximum_on(cuteness, cats);

    std::cout << cutest_cat.name_ <<
        " is happy and sleepy. *purr* *purr* *purr*" << std::endl;
}

std::list<std::uint64_t> collatz_seq(std::uint64_t x)
{
    std::list<std::uint64_t> result;
    while (x > 1)
    {
        result.push_back(x);
        if (x % 2 == 0)
            x = x / 2;
        else
            x = 3 * x + 1;
    }
    result.push_back(x);
    return result;
}

void Test_example_CollatzSequence()
{
    typedef std::list<uint64_t> Ints;

    // [1, 2, 3 ... 29]
    auto numbers = fplus::generate_range<Ints>(1, 30);

    // A function that does [1, 2, 3, 4, 5] -> "[1 => 2 => 3 => 4 => 5]"
    auto show_ints = fplus::bind_1st_of_2(fplus::show_cont_with<Ints>, " => ");

    // A composed function that calculates a Collatz sequence and shows it.
    auto show_collats_seq = fplus::compose(collatz_seq, show_ints);

    // Apply it to all our numbers.
    auto seq_strs = fplus::transform(show_collats_seq, numbers);

    // Combine the numbers and their sequence representations into a map.
    auto collatz_dict = fplus::create_map(numbers, seq_strs);

    // Print some of the sequences.
    std::cout << collatz_dict[13] << std::endl;
    std::cout << collatz_dict[17] << std::endl;
}

void Test_VariadicNumericTools()
{
    using namespace fplus;

    assert(min(1,2,3,4,5) == 1);
    assert(min(1.01,1.02,1.03,1.04,1.05) == 1.01);
    assert(min(-54,2,3,54,5) == -54);
    assert(min(-54.2,2.7,3,54,5) == -54.2);
    assert(min(123,123,123,124) == 123);
    assert(min(123) == 123);
    assert(min(123,123) == 123);
    assert(min(123,123,123) == 123);
    assert(min(-1) == -1);
    assert(min(-1,-2) == -2);
    assert(min(-1,-2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == -2);
    assert(min('a','b','c') == 'a');

    assert(max(1,2,3,4,5) == 5);
    assert(max(1.01,1.02,1.03,1.04,1.05) == 1.05);
    assert(max(-54,2,3,54,5) == 54);
    assert(max(-54.2,2.7,3,54.85,5) == 54.85);
    assert(max(123,123,123,124) == 124);
    assert(max(123) == 123);
    assert(max(123,123) == 123);
    assert(max(123,123,123) == 123);
    assert(max(123,123,123,123) == 123);
    assert(max(123,123,123,123,123) == 123);
    assert(max(123,123,123,123,123,123) == 123);
    assert(max(-1) == -1);
    assert(max(-1,-2) == -1);
    assert(max(-1,-2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == 0);
    assert(max('a','b','c') == 'c');
}

int main()
{
    std::cout << "Running all tests." << std::endl;

    std::cout << "Testing Maybe." << std::endl;
    Test_Maybe();
    std::cout << "Maybe OK." << std::endl;

    std::cout << "Testing Result." << std::endl;
    Test_Result();
    std::cout << "Result OK." << std::endl;

    std::cout << "Testing Compare." << std::endl;
    Test_Compare();
    std::cout << "Compare OK." << std::endl;

    std::cout << "Testing ContainerTools." << std::endl;
    Test_ContainerTools();
    std::cout << "ContainerTools OK." << std::endl;

    std::cout << "Testing StringTools." << std::endl;
    Test_StringTools();
    std::cout << "StringTools OK." << std::endl;

    std::cout << "Testing Read." << std::endl;
    Test_Read();
    std::cout << "Read OK." << std::endl;

    std::cout << "Testing Side Effects." << std::endl;
    Test_SideEffects();
    std::cout << "Side Effects OK." << std::endl;

    std::cout << "Testing Variadic Numeric Tools." << std::endl;
    Test_VariadicNumericTools();
    std::cout << "Variadic Numeric Tools OK." << std::endl;

    std::cout << "Testing Applications." << std::endl;
    Test_example_KeepIf();
    Test_example_KeepIf_performance();
    Test_example_SameOldSameOld();
    Test_example_IInTeam();
    Test_example_AllIsCalmAndBright();
    Test_example_TheCutestCat();
    Test_example_CollatzSequence();
    std::cout << "Applications OK." << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << "All OK." << std::endl;
    std::cout << "=======" << std::endl;
}
