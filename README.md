# Assembly

This project is creating an assembly simulator in C++, starting with basic Y86
commands.

## References

Y86 assembly
* [CMU.edu](http://csapp.cs.cmu.edu/public/ch4-preview.pdf)
* [UTEXAS.edu](https://www.cs.utexas.edu/users/witchel/429/lectures/ISA_1.pdf)

## Running

Simply run the following command:
```
> make
```

Then on Windows you can execute `main.exe`, and on other operating systems, execute
`main.out`, or run the following:
```
> make run
```

## Requirements

gcc:
* Linux usually comes with this
* [Mac OS X](https://www.mkyong.com/mac/how-to-install-gcc-compiler-on-mac-os-x/)
* [Windows](http://preshing.com/20141108/how-to-install-the-latest-gcc-on-windows/)

make:
* Linux usually comes with this
* Mac OS X: Have the XCode command line tools installed from gcc link above
* [Windows](http://gnuwin32.sourceforge.net/packages/make.htm)
