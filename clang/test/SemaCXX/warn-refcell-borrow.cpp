// RUN: %clang_cc1 -fsyntax-only -std=c++17 -Wrefcell-borrow -verify %s
// RUN: %clang_cc1 -fsyntax-only -std=c++17 -DPRAGMA_ON -verify %s
// RUN: %clang_cc1 -fsyntax-only -std=c++17 -verify=off %s

// off-no-diagnostics

#ifdef PRAGMA_ON
#pragma clang diagnostic warning "-Wrefcell-borrow"
#endif

namespace safecpp {
template <typename T> class Ref {
public:
  Ref() {}
  ~Ref() {}
};
template <typename T> class RefMut {
public:
  RefMut() {}
  ~RefMut() {}
};
template <typename T> class RefCell {
public:
  explicit RefCell(T) {}
  Ref<T> borrow() const { return Ref<T>(); }
  RefMut<T> borrow_mut() { return RefMut<T>(); }
};
} // namespace safecpp

using safecpp::RefCell;

void shared_then_exclusive() {
  RefCell<int> cell(1);
  auto r = cell.borrow();
  auto m = cell.borrow_mut(); // expected-warning {{borrow of RefCell 'cell' conflicts with a borrow that is still alive}}
}

void exclusive_then_shared() {
  RefCell<int> cell(1);
  auto m = cell.borrow_mut();
  auto r = cell.borrow(); // expected-warning {{borrow of RefCell 'cell' conflicts with a borrow that is still alive}}
}

void two_exclusive() {
  RefCell<int> cell(1);
  auto a = cell.borrow_mut();
  auto b = cell.borrow_mut(); // expected-warning {{borrow of RefCell 'cell' conflicts with a borrow that is still alive}}
}

void two_shared_ok() {
  RefCell<int> cell(1);
  auto r1 = cell.borrow();
  auto r2 = cell.borrow();
}

void scoped_ok() {
  RefCell<int> cell(1);
  {
    auto r = cell.borrow();
  }
  auto m = cell.borrow_mut();
}

void different_cells_ok() {
  RefCell<int> a(1), b(2);
  auto r = a.borrow();
  auto m = b.borrow_mut();
}

// Guards from the branch are dead after it, so m2 is fine.
void conflict_in_branch(bool c) {
  RefCell<int> cell(1);
  if (c) {
    auto r = cell.borrow();
    auto m = cell.borrow_mut(); // expected-warning {{borrow of RefCell 'cell' conflicts with a borrow that is still alive}}
  }
  auto m2 = cell.borrow_mut();
}

// Each iteration's guard dies before the back edge.
void loop_ok() {
  RefCell<int> cell(1);
  for (int i = 0; i < 3; ++i) {
    auto m = cell.borrow_mut();
  }
}

void pragma_off() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wrefcell-borrow"
  RefCell<int> cell(1);
  auto r = cell.borrow();
  auto m = cell.borrow_mut();
#pragma clang diagnostic pop
}
