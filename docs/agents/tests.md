# Tests

Enable tests at configure time:
- Set BUILD_TESTING=ON in the CMake configuration.

Run tests:
- cd _build
- ctest -C Release --output-on-failure

Filtering examples:
- By name:
  ctest -C Release --output-on-failure -R examples_medium
- By label:
  ctest -C Release --output-on-failure -L unit

Tip:
- Use ctest -N to list tests without running them.

