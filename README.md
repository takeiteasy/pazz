# pazz

Totally offline, reproducible password generation + management. Passwords are not stored anywhere and can be reproduced anywhere.

You can use pazz at [takeiteasy.github.io/pazz](https://takeiteasy.github.io/pazz/). Or you can simple clone this repo or download the [zip file](https://github.com/takeiteasy/pazz/archive/refs/heads/master.zip). Simply open index.html in your browser and you're good to go.

If you're paranoid, or interested in the cli tool, please see the [build](#build) section.

## Usage

Pazz needs a few things to work: a username, an identifier, a master password, a scope, a counter and a template.

<p align="center">
   <img src="https://raw.githubusercontent.com/takeiteasy/pazz/master/static/1.png">
</p>

First you need to add a username. This will be stored using `localStorage`. Once you have made a username you will be prompted for a master password. This is not stored and is not checked against anything. If you enter the wrong password your generated password will not be the same.

<p align="center">
   <img src="https://raw.githubusercontent.com/takeiteasy/pazz/master/static/2.png">
</p>

Once you've entered your password you will be presented by the sites screen. You may now add a site identifier to your user. A good idea would be using the web address as the identifier. Click the add button and the list of sites will be updated.

<p align="center">
   <img src="https://raw.githubusercontent.com/takeiteasy/pazz/master/static/3.png">
</p>

If you want to change advanced settings (scope, counter, template), you can click the 'Show Advanced' button. The scope is just another string that affects the password generation. Don't change this unless you know what you're doing. The counter is useful if you have been asked to change your password. And the template is the format your password will be generated in.

<p align="center">
   <img src="https://raw.githubusercontent.com/takeiteasy/pazz/master/static/4.png">
</p>

Now when you left click on the site identifier you will be presented with a generated password. Right click on the site identifier it will be copied to your clipboard.

<p align="center">
   <img src="https://raw.githubusercontent.com/takeiteasy/pazz/master/static/5.png">
</p>

## Build

If you want to build pazz yourself you can run the following commands:

```bash
# make javascript, cli program and library
make all
# make javascript
make web
# make cli program
make cli
# make library
make library
```

 To build the javascript library you will need emscripten installed. The cli program and library just need a C compiler.

## TODO

- [ ] Modify existing password settings

## Acknowledgements

- Based on [this](https://spectre.app/spectre-algorithm.pdf) paper by [spectre.app](https://spectre.app/) (formerly "Master Password")
- scrypt+sha256 implementation taken from [technion/libscrypt](https://github.com/technion/libscrypt) (BSD-2-License)

## License
```
The MIT License (MIT)

Copyright (c) 2022 George Watson

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```
