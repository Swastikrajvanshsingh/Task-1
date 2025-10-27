# TaskScheduler

A C++ multi-threaded task scheduler library with support for various scheduling strategies.

## Features

- Thread pool with configurable worker count
- Task queue with FIFO scheduling
- Extensible task system

## Building

### Local Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run Tests

```bash
./scripts/run_tests.sh
```

Or manually:
```bash
cd build
ctest --output-on-failure
```

### Docker Build

```bash
docker build -t taskscheduler -f docker/Dockerfile .
docker run taskscheduler
```

## Requirements

- C++17 compatible compiler
- CMake 3.14 or higher
- GoogleTest (automatically fetched)
