# Teditor
## A simple text/code editor

<image src="teditor.gif">

A simple text and code editor made in ncurses with UTF-8 support, mouse events (including scroll and clicks), autotab and a permanent stratch buffer (just run `ted`, without any argument). All this with only less than 1000 lines of code (counting with cloc).

### Compiling

```
git clone https//github.com/ArthurBacci64/Teditor.git
cd Teditor
make
```

to run:

```
./ted
```

to recompile:

```
make clean
make
```

If you want to develop/contribute, use `make dev` instead of `make`

```
make clean
make dev
./ted
```

## Shortcuts
**Ctrl+H** Opens the config dialog.

**Ctrl+S** Saves the file.
<br>  
If you open the config dialog, type the command you want
and press Enter. Commands may require an argument.

Config dialog commands:

**tablen** Set tab length (expects argument)
**use_spaces** Tab or Space?
<br>  
<br>  
## Contributing
Please open a Pull Request if you have a solution or a new feature.

Please open a Issue if you find some bug or if you have a suggestion.

Pages in the Wiki also are appreciated

Thanks for reading.
