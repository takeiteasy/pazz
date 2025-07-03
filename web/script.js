/* script.js -- https://github.com/takeiteasy/pazz

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

function addUsername(username) {
  let usernames = getUsernames();
  if (!usernames.includes(username)) {
    usernames.push(username);
    localStorage.setItem("usernames", JSON.stringify(usernames));
  }
}

function getUsernames() {
  const stored = localStorage.getItem("usernames");
  return stored ? JSON.parse(stored) : [];
}

function removeUsername(username) {
  let usernames = getUsernames();
  usernames = usernames.filter((u) => u !== username);
  localStorage.setItem("usernames", JSON.stringify(usernames));
}

function getMainUsername() {
  return localStorage.getItem("mainUsername");
}

function setMainUsername(username) {
  localStorage.setItem("mainUsername", username);
}

var State = {
  no_usernames: false,
  username: undefined,
};

document.addEventListener("DOMContentLoaded", function () {
  var users = getUsernames();
  if (users.length === 0) {
    State.no_usernames = true;
  } else if (users.length === 1) {
    State.username = users[0];
    setMainUsername(State.username);
  } else {
    State.username = getMainUsername();
  }

  var body = document.getElementsByClassName("body")[0];
  if (State.no_usernames) {
    body.innerHTML += "<p>Please create a username:</p>";
  } else {
    body.innerHTML += "<p>Select a username:</p>";
  }
});
