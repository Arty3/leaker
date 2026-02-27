# Leaker

A simple program whose whole purpose is to leak memory.

## Why?

I was bored and here we are

## Usage

```sh
./leaker --size=...
```

The `--size` argument is required, all others are optional

> [!CAUTION]  
> By default RAM usage wont rise unless you force commit the memory
> This enables the program to write to the memory (forcefully committing it)
> Once the program terminates, the memory is returned to the kernel, reverting the leak

### Arguments

| Argument | Description |
|:---------|:------------|
| `--size=<value>[suffix]` | Size of each allocation, suffixes: b, kb, mb, gb (default: b) |
| `--iters=<count>` | Number of iterations, -1 for infinite (default: -1) |
| `--interval=<seconds>` | Sleep interval between allocations (default: 0) |
| `--force-commit` | Forces the OS to commit the memory (default: false) |
| `--silent` | Suppress output (default: false) |
| `--stop-on-fail` | Stop when an allocation fails (default: false) |
| `--help` | Shows a help message |

## Build

### Posix

Please make sure you have [GCC](https://gcc.gnu.org/) and [make](https://www.gnu.org/software/make/) installed on your machine.

Then simply run:

```sh
make all
```

If you wish to clean:

```sh
make clean
```

If you wish to full clean:

```sh
make fclean
```

If you wish to rebuild:

```sh
make re
```

### Windows

Please make sure you have [Visual Studio](https://visualstudio.microsoft.com/) installed with the **Desktop development with C++** workload, which includes MSVC and the Windows SDK.

Then, from a **Developer Command Prompt for VS** (found in Start Menu under Visual Studio), navigate to the project directory and run:

```bat
build_msvc.bat
```

Cleaning and similar must be done manually

> [!TIP]
> You may also compile using [MinGW](https://www.mingw-w64.org/)

## License

This project uses the MIT License. For more info, please find the `LICENSE` file here: [License](LICENSE)

