# Teditor
## A simple text/code editor


### About

Teditor is a text editor made in one file, based, but not direct fork of [kilo](https://github.com/antirez/kilo), with some others features like `auto tab`, `soft tab`, `center screen on cursor`, and some near insignificant others.

If you want to contribute you can read the next section.

### Contribute

You can contribute doing pull requests. Contribute in the way that you want. If you do not know how to do a pull request, read this:

1. Fork this repository on your account (in github)
2. Clone your fork with `git clone https://github.com/[your username]/Teditor.git`
3. Do the changes and run `git add ted` on the source directory
4. Do a commit with `git commit -m "[your commit message]"`
5. Push to your fork with `git push`
6. And do the pull request in the github button on your fork

Thank you for contributing.

To compile, run in the source directory: `make`, and to run `./ted`

### Run

```
make
./ted
```

If you want to edit a text file:

```
touch [filename]
./ted [filename]
```


### To install globally on your system (Linux only)
#### In the source directory:

```
make
sudo cp ted /usr/bin/ted
```

And you can run ted with `ted [filename]`

##### If you want to add to applications menu

```
sudo cp ./content/ted.png /usr/share/icons/hicolor/48x48/apps/ted.png
sudo cp ./content/ted.desktop /usr/share/applications/ted.desktop
```

### Usage

The simple usage: `ted [filename]`

If you run `ted [filename]` and filename does not exists, ted will return a error, but you can run `ted` and when you press `Ctrl+S` it will ask you for the filename, if the file does not exists, it will be created.

Commands:

```
Ctrl+C: Exit (if the file is not saved, it will prompt you to type Ctrl+C N more times)
Ctrl+S: Save
Ctrl+F: Search
Ctrl+E: Center the screen with the cursor
```

Keys:

```
Arrow Up: Move the cursor up
Arrow Down: Move the cursor down
Arrow Left: Move the cursor left
Arrow Right: Move the cursor right
Page Up: Move 1 page up
Page Down: Move 1 page down
Ctrl + Arrow Up: Move the cursor up N times (default = 5)
Ctrl + Arrow Down: Move the cursor down N times (default = 5)

if wrapcursor == 2 (default)
{
Arrow Left: If the cursor is in the line start, it will NOT come back to the other line
Arrow Right: The same thing but diferrent
Ctrl + Arrow Left: If the cursor is in the line start, it will come back to the other line
Ctrl + Arrow Right: The same thing but different
}
else if wrapcursor == 1
{
If the cursor is in the line start/end, it will come back to the other line
}
else if wrapcursor == 0
{
If the cursor is in the line start/end, it will NOT come back to the other line
}
```

### Things to do

1. UTF-8 support
2. More syntax highlighting
3. Create translation files and replace the in code strings
4. Create a `.tedrc`
5. Fragment the files
6. Windows support (not too important)
