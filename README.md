# Teditor
## A simple text/code editor

<image src="teditor.gif">

A simple text and code editor with (partial) UTF-8 support made using ncurses

## Installing

### Manual installation

Go to your programs directory and do: 

```sh
git clone https://github.com/ArthurBacci64/Teditor.git
cd Teditor
sudo make PREFIX=/usr install
```

Installed.

If you want to install locally, do `make PREFIX=~/.local install` instead of `sudo make install`.

## Uninstalling

Just do:

```sh
rm ~/.config/ted -rf
sudo rm $(which ted) -rf
```

If you installed locally, you do not need to use `sudo`.

## Help

Press `Ctrl-G`, type `manual`, press `ENTER` and press `ENTER` again.

## Contributing

Please open a Issue if you found some bug or if you have a suggestion or a question.

## Goals

- [ ] Implement grapheme clusters and replace the wchar code points. Also try to make it use a normal `char` buffer.
- [ ] Fix the bad code and try to make it stable enough for real usage
- [ ] Fix all current issues
