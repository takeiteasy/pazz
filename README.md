# libspectre

An implementation of the password hasing process featured by [spectre.app](https://spectre.app/spectre-algorithm.pdf) (formerly "Master Password")

To use as a library, you can include ```spectre.h``` and ```spectre.c``` in your project or run ```make lib```. Or build the cli app with ```make cli```.

```
usage: spectre --name username --site www.example.com [arg value...]

  Arguments:
    * --name/-n     -- Name of new user [required]
    * --password/-p -- Master password of user [optional]
                       If this is not set, you will be required
                       to enter it through a masked prompt
    * --site/-s     -- Site password identifier [required]
    * --counter/-c  -- Modify to generate a different password
                       for a site identifier [optional]
    * --scope/-S    -- Scecify password purpose [optional]
    * --template/-t -- Password output format [optional]

  Scopes:
    * authentication (default)
    * identification
    * recovery

  Templates:
    * max (20 characters)
    * long (14 characters) (default)
    * medium (8 characters)
    * short (4 characters)
    * basic (8 characters, A-Z + 0-9)
    * pin (4 characters, 0-9)
    * name (9 characters)
    * phrase (20 characters with spaces)

```

## Dependencies

- [technion/libscrypt](https://github.com/technion/libscrypt) [BSD-2-Clause]
    - src/spectre.c -- scrypt + sha256 implementation
- [skandhurkat/Getopt-for-Visual-Studio](https://github.com/skandhurkat/Getopt-for-Visual-Studio) [ZPL-2.1]
    - src/getopt_win32.h (only needed for cli)

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
