# chex
chex is a tool to quickly view .hex files in terminal

If you add the exe into your paths, you should be able to use it just like any other terminal command

![console_preview](docs/console_preview.png "console_preview")

## what is intel .hex file

You can read something about intel hex file [here](https://en.wikipedia.org/wiki/Intel_HEX).

## how to run and build

I use gcc, so I do it like this:

> gcc .\src\main.c -o .\bin\chex

> .\bin\chex.exe -f demo.hex


## todo:
- create help -h 
- create option to add start and end address to display only part of hex file
- polish coloring of output
- check if checksum is correct
- print "metadata" of the hexfile
- add header to describe colums in the hexfile
