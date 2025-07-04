/* pazz_helper.js -- https://github.com/takeiteasy/pazz

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
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

function spectre(
  name,
  pass,
  site,
  counter = 0,
  scope = "com.lyndir.masterpassword",
  type = 0,
) {
  if (!Module || !Module._spectre) {
    console.log("Emscripten module not yet ready or function not found.");
    return;
  }
  try {
    let namePtr = Module._malloc(Module.lengthBytesUTF8(name) + 1);
    Module.stringToUTF8(name, namePtr, Module.lengthBytesUTF8(name) + 1);
    let passPtr = Module._malloc(Module.lengthBytesUTF8(pass) + 1);
    Module.stringToUTF8(pass, passPtr, Module.lengthBytesUTF8(pass) + 1);
    let sitePtr = Module._malloc(Module.lengthBytesUTF8(site) + 1);
    Module.stringToUTF8(site, sitePtr, Module.lengthBytesUTF8(site) + 1);
    let keyScopePtr = Module._malloc(Module.lengthBytesUTF8(keyScope) + 1);
    Module.stringToUTF8(scope, keyScopePtr, Module.lengthBytesUTF8(scope) + 1);
    let resultPtr = Module._spectre(
      namePtr,
      passPtr,
      sitePtr,
      counter,
      keyScopePtr,
      type,
    );
    return Module.UTF8ToString(resultPtr);
  } catch (e) {
    console.error("Error calling C function (spectre):", e);
  } finally {
    if (namePtr) Module._free(namePtr);
    if (passPtr) Module._free(passPtr);
    if (sitePtr) Module._free(sitePtr);
    if (keyScopePtr) Module._free(keyScopePtr);
    if (resultPtr) Module._free(resultPtr);
  }
}
