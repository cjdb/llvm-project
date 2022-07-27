// RUN: %clang_cc1 -fsyntax-only -std=c++17 %s

namespace std {
template <class F, class... Args>
void invoke(F &&, Args &&...);

// Slightly different to the real deal to simplify test.
template <class T>
class reference_wrapper {
public:
  constexpr reference_wrapper(T &t) : data(&t) {}

  constexpr operator T &() const noexcept { return *data; }

private:
  T *data;
};
} // namespace std

#define assert(...)   \
  if (!(__VA_ARGS__)) \
    __builtin_unreachable();

template <class Returns, bool IsNoexcept, int ExpectedResult, class F, class T, class... Args>
constexpr void bullet_1(F f, T &&t, Args... args) {
  assert(std::invoke(f, static_cast<T &&>(t), args...) == ExpectedResult);
  static_assert(__is_same(decltype(std::invoke(f, static_cast<T &&>(t), args...)), Returns));
  static_assert(noexcept(std::invoke(f, static_cast<T &&>(t), args...)) == IsNoexcept);
}

template <class Returns, bool IsNoexcept, int ExpectedResult, class F, class T, class... Args>
constexpr void bullet_2(F f, T &t, Args... args) {
  std::reference_wrapper<T> rw(t);
  assert(std::invoke(f, rw, args...) == ExpectedResult);
  static_assert(__is_same(decltype(std::invoke(f, rw, args...)), Returns));
  static_assert(noexcept(std::invoke(f, rw, args...)) == IsNoexcept);
}

template <class Returns, bool IsNoexcept, int ExpectedResult, class F, class T, class... Args>
constexpr void bullet_3(F f, T &t, Args... args) {
  assert(std::invoke(f, &t, args...) == ExpectedResult);
  static_assert(__is_same(decltype(std::invoke(f, &t, args...)), Returns));
  static_assert(noexcept(std::invoke(f, &t, args...)) == IsNoexcept);
}

template <class T>
constexpr bool bullets_1_through_3(T t) {
  bullet_1</*Returns=*/int &, /*IsNoexcept=*/true, /*ExpectedResult=*/0>(&T::plus, t, 3, -3);
  bullet_1</*Returns=*/const int &, /*IsNoexcept=*/true, /*ExpectedResult=*/-4>(&T::minus, static_cast<const T &>(t), 1, 2, 3);
  bullet_1</*Returns=*/int &&, /*IsNoexcept=*/false, /*ExpectedResult=*/49>(&T::square, static_cast<__remove_reference(T) &&>(t), 7);
  bullet_1</*Returns=*/const int &&, /*IsNoexcept=*/false, /*ExpectedResult=*/-63>(&T::sum, static_cast<const T &&>(t), -1, -2, -4, -8, -16, -32);

  bullet_2</*Returns=*/int &, /*IsNoexcept=*/true, /*ExpectedResult=*/0>(&T::plus, t, 3, -3);
  bullet_2</*Returns=*/const int &, /*IsNoexcept=*/true, /*ExpectedResult=*/-4>(&T::minus, static_cast<const T &>(t), 1, 2, 3);

  bullet_3</*Returns=*/int &, /*IsNoexcept=*/true, /*ExpectedResult=*/0>(&T::plus, t, 3, -3);
  bullet_3</*Returns=*/const int &, /*IsNoexcept=*/true, /*ExpectedResult=*/-4>(&T::minus, static_cast<const T &>(t), 1, 2, 3);

  return true;
}

template <class Returns, class F, class T, class U>
constexpr void bullet_4(F f, T &&t, U ExpectedResult) {
  assert(std::invoke(f, static_cast<T &&>(t)) == ExpectedResult);
  static_assert(__is_same(decltype(std::invoke(f, static_cast<T &&>(t))), Returns));
  static_assert(noexcept(std::invoke(f, static_cast<T &&>(t))));
}

template <class Returns, class F, class T, class U>
constexpr void bullet_5(F f, T &t, U ExpectedResult) {
  std::reference_wrapper<T> rw(t);
  assert(std::invoke(f, rw) == ExpectedResult);
  static_assert(__is_same(decltype(std::invoke(f, rw)), Returns));
  static_assert(noexcept(std::invoke(f, rw)));
}

template <class Returns, class F, class T, class U>
constexpr void bullet_6(F f, T &t, U ExpectedResult) {
  assert(std::invoke(f, &t) == ExpectedResult);
  static_assert(__is_same(decltype(std::invoke(f, &t)), Returns));
  static_assert(noexcept(std::invoke(f, &t)));
}

template <class F, class T, class U>
constexpr bool bullets_4_through_6(F f, T t, U ExpectedResult) {
  bullet_4</*Returns=*/U &>(f, t, ExpectedResult);
  bullet_4</*Returns=*/const U &>(f, static_cast<const T &>(t), ExpectedResult);
  bullet_4</*Returns=*/U &&>(f, static_cast<T &&>(t), ExpectedResult);
  bullet_4</*Returns=*/const U &&>(f, static_cast<const T &&>(t), ExpectedResult);

  bullet_5</*Returns=*/U &>(f, t, ExpectedResult);
  bullet_5</*Returns=*/const U &>(f, static_cast<const T &>(t), ExpectedResult);

  bullet_6</*Returns=*/U &>(f, t, ExpectedResult);
  bullet_6</*Returns=*/const U &>(f, static_cast<const T &>(t), ExpectedResult);

  return true;
}

constexpr int zero() { return 0; }
constexpr int square(int x) { return x * x; }
constexpr double product(double x, double y) { return x * y; }

struct summation {
  template <class... Ts>
  constexpr auto operator()(Ts... ts) {
    return (ts + ...);
  }
};

struct callable {
  int x;

  constexpr int &operator()() &noexcept { return x; }
  constexpr const int &operator()() const &noexcept { return x; }
  constexpr int &&operator()() &&noexcept { return static_cast<int &&>(x); }
  constexpr const int &&operator()() const &&noexcept { return static_cast<const int &&>(x); }
};

template <class T, auto expected, bool IsNoexcept, class F, class... Args>
constexpr bool bullet_7(F &&f, Args &&...args) {
  return std::invoke(static_cast<F>(f), static_cast<Args>(args)...) == expected &&
         __is_same(decltype(std::invoke(static_cast<F>(f), static_cast<Args>(args)...)), T) &&noexcept(std::invoke(static_cast<F>(f), static_cast<Args>(args)...)) == IsNoexcept;
}

template <class T, class Expected>
constexpr bool bullet_7_1() {
  callable c{21};
  return std::invoke(static_cast<T>(c)) == 21 &&
         __is_same(decltype(std::invoke(static_cast<T>(c))), Expected) &&noexcept(std::invoke(static_cast<T>(c)));
}

struct Base {
  mutable int data;

  constexpr int &plus(int x, int y) &noexcept {
    data = x + y;
    return data;
  }

  constexpr const int &minus(int x, int y, int z) const &noexcept {
    data = x - y - z;
    return data;
  }

  constexpr int &&square(int x) && {
    data = x * x;
    return static_cast<int &&>(data);
  }

  constexpr const int &&sum(int a, int b, int c, int d, int e, int f) const && {
    data = a + b + c + d + e + f;
    return static_cast<const int &&>(data);
  }
};

struct Derived : Base {
  double data2;
};

void test_invoke() {
  static_assert(bullets_1_through_3(Base{}));
  static_assert(bullets_1_through_3(Derived{}));

  static_assert(bullets_4_through_6(&Base::data, Base{21}, 21));
  static_assert(bullets_4_through_6(&Base::data, Derived{-96, 18}, -96));
  static_assert(bullets_4_through_6(&Derived::data2, Derived{21, 34}, 34.0));

  static_assert(bullet_7<int, 0, false>(zero));
  static_assert(bullet_7<int, 25, false>(square, 5.0));
  static_assert(bullet_7<double, 9, false>(product, 9, 1));
  static_assert(bullet_7<int, 0, false>(&zero));
  static_assert(bullet_7<long, 55L, false>(summation{}, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10L));
  static_assert(bullet_7<int, 18, false>([] { return 18; }));

  static_assert(bullet_7_1<callable &, int &>());
  static_assert(bullet_7_1<const callable &, const int &>());
  static_assert(bullet_7_1<callable &&, int &&>());
  static_assert(bullet_7_1<const callable &&, const int &&>());
}

struct ptr {
  Base &operator*() const;
};

void test_errors() {
  // std::invoke(&Base::data);
  // expected-error@-1{{too few arguments for pointer-to-member invocation; 'std::invoke' requires at least 2 (got 1)}}
  // std::invoke(&Base::plus);
  // expected-error@-1{{too few arguments for pointer-to-member invocation; 'std::invoke' requires at least 2 (got 1)}}
  // std::invoke(&Base::data, Base{}, Base{});
  // expected-error@-1{{too many arguments for pointer-to-data-member invocation; 'std::invoke' requires exactly 2 arguments (got 3)}}
  // std::invoke(&Base::sum, Base{}, 1);
  // expected-error@-1{{too few arguments to function call, expected 6, have 1}}
  std::invoke(&Base::sum, Base{}, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  // expected-error@-1{{too many arguments to function call, expected 6, have 10}}
  // std::invoke(&Base::plus, ptr{});
  // expected-error@-1{{second argument to 'std::invoke' must be a class compatible with the first argument, but it is 'ptr'}}
}
