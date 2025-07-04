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

function copyToClipboard(text) {
  if (navigator.clipboard) {
    navigator.clipboard.writeText(text);
  } else {
    const textArea = document.createElement("textarea");
    textArea.value = text;
    document.body.appendChild(textArea);
    textArea.focus();
    textArea.select();
    try {
      document.execCommand("copy");
      console.log("Fallback: Copied to clipboard");
    } catch (err) {
      console.error("Fallback: Unable to copy", err);
    }
    document.body.removeChild(textArea);
  }
}

function addUsername(username) {
  let usernames = getUsernames();
  if (!usernames.includes(username)) {
    usernames.push(username);
    localStorage.setItem("usernames", JSON.stringify(usernames));
    localStorage.setItem(username, "[]");
    return true;
  }
  return false;
}

function getUsernames() {
  const stored = localStorage.getItem("usernames");
  return stored ? JSON.parse(stored) : [];
}

function hasUsername(username) {
  return getUsernames().includes(username);
}

function removeUsername(username) {
  let usernames = getUsernames();
  if (!usernames.includes(username)) {
    return;
  }
  usernames = usernames.filter((u) => u !== username);
  localStorage.setItem("usernames", JSON.stringify(usernames));
  localStorage.removeItem(username);
}

function getUserSites(username) {
  return hasUsername(username)
    ? JSON.parse(localStorage.getItem(username))
    : [];
}

function addUserSite(username, site) {
  let sites = getUserSites(username);
  sites.push(site);
  localStorage.setItem(username, JSON.stringify(sites));
}

function removeUserSite(username, site) {
  let sites = getUserSites(username);
  sites = sites.filter((s) => s !== site);
  localStorage.setItem(username, JSON.stringify(sites));
}

function hasUserSite(username, site) {
  return getUserSites(username).includes(site);
}

var State = {
  username: undefined,
  password: undefined,
};

function updateUsernamesList() {
  var body = document.getElementById("inner-body");
  body.innerHTML = "";
  var users = getUsernames();
  if (users.length === 0) {
    body.innerHTML +=
      "<p id='create-username'>No usernames found. Please create a username:</p>";
  } else {
    var msg = document.getElementById("create-username");
    if (msg != null && msg != undefined) {
      msg.remove();
    }
  }

  var html = "<ul>";
  users.forEach((user) => {
    html += `<li><button class="mini-list username">${user}</button><button class="remove-username mini-button mini-mini" data-username="${user}">x</button></li>`;
  });
  html += "</ul>";
  body.innerHTML += html;
}

function updatePasswordsList() {
  var body = document.getElementById("inner-body");
  body.innerHTML = "";
  var sites = getUserSites(State.username);
  if (sites.length === 0) {
    body.innerHTML += "<p>No sites found. Please add a site:</p>";
  } else {
    var html = "<ul>";
    sites.forEach((site) => {
      html += `<li><button class="mini-list site">${site}</button><button class="remove-site mini-button mini-mini" data-site="${site}">x</button></li>`;
    });
    html += "</ul>";
    body.innerHTML += html;
  }
  document.querySelectorAll(".site").forEach((site) => {
    site.addEventListener("contextmenu", function (e) {
      e.preventDefault();
      copyToClipboard(
        spectre(State.username, State.password, site.textContent),
      );
    });
  });
}

function moveToPasswords() {
  if (State.username === undefined) {
    return;
  }
  document.getElementById("add-username-body").style.display = "none";
  document.getElementById("add-password-body").style.display = "none";
  document.getElementById("add-site-body").style.display = "block";
  updatePasswordsList();
}

function moveBackToUsername() {
  if (State.username === undefined) {
    return;
  }
  State.username = undefined;
  State.password = undefined;
  document.getElementById("add-username-body").style.display = "block";
  document.getElementById("add-password-body").style.display = "none";
  document.getElementById("add-site-body").style.display = "none";
  var box = document.getElementById("password-box");
  box.style.display = "none";
  box.innerHTML = "";
  updateUsernamesList();
}

document.addEventListener("DOMContentLoaded", function () {
  updateUsernamesList();

  document
    .getElementById("add-username")
    .addEventListener("submit", function (e) {
      e.preventDefault();
      const input = document.getElementById("username-input");
      const username = input.value.trim();
      input.value = "";
      if (username === "") {
        return;
      }
      var err = document.getElementById("username-already-exists");
      if (hasUsername(username)) {
        err.style.display = "table";
        return;
      }
      err.style.display = "none";
      addUsername(username);
      updateUsernamesList();
    });

  document
    .getElementById("add-password")
    .addEventListener("submit", function (e) {
      e.preventDefault();
      const input = document.getElementById("password-input");
      const password = input.value.trim();
      input.value = "";
      if (State.username === undefined || password === "") {
        return;
      }
      State.password = password;
      document.getElementById("add-username-body").style.display = "block";
      document.getElementById("add-password-body").style.display = "none";
      moveToPasswords();
    });

  document
    .getElementById("cancel-password")
    .addEventListener("submit", function (e) {
      e.preventDefault();
      document.getElementById("add-username-body").style.display = "block";
      document.getElementById("add-password-body").style.display = "none";
    });

  document.getElementById("add-site").addEventListener("submit", function (e) {
    e.preventDefault();
    const input = document.getElementById("site-input");
    const site = input.value.trim();
    input.value = "";
    if (site === "") {
      return;
    }
    var err = document.getElementById("site-already-exists");
    if (hasUserSite(State.username, site)) {
      err.style.display = "table";
      return;
    }
    document.getElementById("password-box").style.display = "none";
    err.style.display = "none";
    addUserSite(State.username, site);
    updatePasswordsList();
    console.log(site);
  });

  document.addEventListener("click", function (e) {
    if (e.target.classList.contains("remove-username")) {
      const username = e.target.dataset.username;
      removeUsername(username);
      updateUsernamesList();
    } else if (e.target.classList.contains("username")) {
      const username = e.target.textContent;
      State.username = username;
      document.getElementById("add-username-body").style.display = "none";
      document.getElementById("add-password-body").style.display = "block";
    } else if (e.target.classList.contains("remove-site")) {
      const site = e.target.dataset.site;
      document.getElementById("password-box").style.display = "none";
      removeUserSite(State.username, site);
      updatePasswordsList();
    } else if (e.target.classList.contains("site")) {
      const site = e.target.textContent;
      var box = document.getElementById("password-box");
      box.style.display = "block";
      box.innerHTML = spectre(State.username, State.password, site);
    } else if (e.target.classList.contains("return")) {
      moveBackToUsername();
    }
  });
});
