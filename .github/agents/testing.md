# Testing Guide

This document covers testing patterns for Antares-Xpansion.

## Building Tests

Enable tests during CMake configuration:

```bash
cmake -B build -S . -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Running Tests

### All Tests

```bash
ctest --test-dir build
```

### C++ Tests Only

```bash
ctest --test-dir build -R cpp
```

### Python Tests Only

```bash
pytest tests/python/
```

### With Coverage

```bash
cmake -B build -S . -DBUILD_TESTING=ON -DCODE_COVERAGE=ON
cmake --build build
ctest --test-dir build
# Coverage report in build/coverage/
```

## C++ Tests (Google Test)

### Location

```
tests/cpp/
├── benders/
├── logger/
├── merge_mps/
├── multisolver_interface/
├── solvers_interface/
├── outer_loop/
├── restart_benders/
└── zip_mps/
```

### Naming

- Test files: `*_test.cpp`
- Test executables: automatically built

### Writing Tests

```cpp
#include <gtest/gtest.h>

class MyClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }
};

TEST_F(MyClassTest, ShouldDoSomething) {
    MyClass obj;
    EXPECT_EQ(obj.compute(), expected_value);
}
```

### Test Fixtures

- Use `SetUp()` / `TearDown()` for resource management
- Prefer fixtures over global state

### Assertions

- Use `EXPECT_*` for non-fatal failures (test continues)
- Use `ASSERT_*` for fatal failures (stops test)
- Prefer clear failure messages:

```cpp
EXPECT_EQ(actual, expected) << "Failed for input: " << input;
```

### Mocking

- Use GoogleMock for mocking
- Keep mocks simple and focused

## Python Tests (pytest)

### Location

```
tests/python/
```

### Naming

- Test files: `test_*.py`
- Test functions: `test_*`
- Test classes: `Test*`

### Writing Tests

```python
import pytest

def test_load_candidates(study_path):
    reader = CandidatesReader(study_path)
    candidates = reader.load()
    assert len(candidates) == 5
    assert "candidate1" in candidates

@pytest.fixture
def study_path(tmp_path):
    # Create test study
    return tmp_path / "test_study"
```

### Fixtures

Use pytest fixtures for setup:

```python
@pytest.fixture
def mock_logger():
    return MockLogger()
```

### Parameterized Tests

```python
@pytest.mark.parametrize("input,expected", [
    (1, 2),
    (2, 4),
])
def test_double(input, expected):
    assert double(input) == expected
```

## Test Data

### C++ Test Data

Located in `data_test/` and test directories. Copy required data in `SetUp()`:

```cpp
void SetUp() override {
    fs::copy("test_data/input.mps", temp_dir_, true);
}
```

### Python Test Data

Use `tmp_path` fixture for temporary test files.

## Integration Tests

Some tests require full Antares solver. Mark these:

```cpp
// Mark slow tests
TEST(SolverTest, DISABLED_FullIntegration) {
    // Only run manually
}
```

## Debugging Failed Tests

1. Run single test:
   ```bash
   ctest --test-dir build -R "test_name" -V
   ```

2. Run with verbose output:
   ```bash
   ctest --test-dir build -V
   ```

## CI Requirements

- All tests must pass before merging
- Code coverage must not decrease significantly
- Run full test suite locally before pushing
