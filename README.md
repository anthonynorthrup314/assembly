# Assembly

This project is an assembly simulator in C++, starting with basic Y86
commands.

Note: `main.c` is currently the working version. I was in the process of
writing the interpreter/compiler under `main.cpp`, but got distracted.

## References

Y86 assembly
- [CMU.edu](http://csapp.cs.cmu.edu/public/ch4-preview.pdf)
- [UTEXAS.edu](https://www.cs.utexas.edu/users/witchel/429/lectures/ISA_1.pdf)

## Running

Simply run the following command:
```bash
> make
```

Once the program has compiled, run it with a source file as a parameter, or use
the following to run the sample file
```bash
> make test
```

Example output:
```
./main.out test.src
[!] TODO: Compile 'test.src'
Stopped in 52 steps at PC = 0x11.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%eax:   0x00000000      0x0000abcd
%ecx:   0x00000000      0x00000024
%ebx:   0x00000000      0xffffffff
%esp:   0x00000000      0x00000100
%ebp:   0x00000000      0x00000100
%esi:   0x00000000      0x0000a000

Changes to memory:
0x00e8: 0x00000000      0x000000f8
0x00ec: 0x00000000      0x0000003d
0x00f0: 0x00000000      0x00000014
0x00f4: 0x00000000      0x00000004
0x00f8: 0x00000000      0x00000100
0x00fc: 0x00000000      0x00000011
```

Or use the `FILE` flag to specify a file to test
```bash
> make test FILE=something.src
```

## Requirements

gcc:
- Linux usually comes with this
- [Mac OS X](https://www.mkyong.com/mac/how-to-install-gcc-compiler-on-mac-os-x/)
- [Windows](http://preshing.com/20141108/how-to-install-the-latest-gcc-on-windows/)
  - Only steps 1 and 2 are required, no need to manually download/build GCC

make:
- Linux usually comes with this
- Mac OS X: Have the XCode command line tools installed from gcc link above
- Windows: Should have using the GCC instructions above
