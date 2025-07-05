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

var State = {
  username: undefined,
  password: undefined,
  site: undefined,
  showAdvanced: false,
};

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

function addUserSite(username, site, scope, counter, template) {
  let sites = getUserSites(username);
  sites.push({
    identifier: site,
    scope: scope,
    counter: counter,
    template: template,
  });
  localStorage.setItem(username, JSON.stringify(sites));
}

function removeUserSite(username, site) {
  let sites = getUserSites(username);
  sites = sites.filter((s) => s.identifier !== site);
  localStorage.setItem(username, JSON.stringify(sites));
}

function hasUserSite(username, site) {
  var sites = getUserSites(username);
  var result = false;
  sites.every((s) => {
    if (s.identifier === site) {
      result = true;
      return true;
    }
    return false;
  });
  return result;
}

function setLastUser(username) {
  localStorage.setItem("lastUser", username);
}

function getLastUser() {
  var last_user = localStorage.getItem("lastUser");
  if (!hasUsername(last_user)) {
    localStorage.removeItem("lastUser");
    return null;
  }
  return last_user;
}

function hasLastUser() {
  return getLastUser() !== null;
}

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
    var selected =
      State.username != undefined && user === State.username ? "selected" : "";
    html += `<li><button class="mini-list username ${selected}">${user}</button><button class="remove-username mini-button mini-mini" data-username="${user}">x</button></li>`;
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
      var selected =
        State.site != undefined && site.identifier === State.site
          ? "selected"
          : "";
      html += `<li><button class="mini-list site ${selected}">${site.identifier}</button><button class="remove-site mini-button mini-mini" data-site="${site.identifier}">x</button></li>`;
    });
    html += "</ul>";
    body.innerHTML += html;
  }
  document.querySelectorAll(".site").forEach((btn) => {
    btn.addEventListener("contextmenu", function (e) {
      e.preventDefault();
      const sites = getUserSites(State.username);
      sites.every((site) => {
        if (site.identifier === btn.textContent) {
          copyToClipboard(
            spectre(
              State.username,
              State.password,
              site.identifier,
              site.counter,
              site.scope,
              site.template,
            ),
          );
          return false;
        }
        return true;
      });
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
  State.username = undefined;
  State.password = undefined;
  State.site = undefined;
  updateUsernamesList();
}

function showAdvanced() {
  var elems = document.getElementsByClassName("advanced-password");
  State.showAdvanced = !State.showAdvanced;
  for (var i = 0; i < elems.length; i++) {
    elems[i].style.display = State.showAdvanced ? "block" : "none";
  }
  var txt = document.getElementById("toggle-advanced");
  txt.innerText = State.showAdvanced ? "Hide Advanced" : "Show Advanced";
}

class FileReader {
  static async openJsonFile() {
    if ("showOpenFilePicker" in window) {
      try {
        return await this.openWithDialog();
      } catch (err) {
        if (err.name !== "AbortError") {
          console.warn("File System Access API failed, falling back to input");
          return await this.openWithInput();
        }
        return null;
      }
    } else {
      return await this.openWithInput();
    }
  }

  static async openWithDialog() {
    const [fileHandle] = await window.showOpenFilePicker({
      types: [
        {
          description: "JSON files",
          accept: { "application/json": [".json"] },
        },
      ],
      multiple: false,
    });
    const file = await fileHandle.getFile();
    const content = await file.text();
    let jsonData;
    try {
      jsonData = JSON.parse(content);
    } catch (parseError) {
      throw new Error(`Invalid JSON file: ${parseError.message}`);
    }
    return { file, content, jsonData, fileHandle };
  }

  static openWithInput() {
    return new Promise((resolve, reject) => {
      const input = document.createElement("input");
      input.type = "file";
      input.accept = ".json,application/json";

      input.onchange = async (event) => {
        const file = event.target.files[0];
        if (!file) {
          resolve(null);
          return;
        }

        // Additional validation for file extension
        if (!file.name.toLowerCase().endsWith(".json")) {
          reject(new Error("Please select a JSON file"));
          return;
        }

        try {
          const content = await file.text();
          const jsonData = JSON.parse(content);

          resolve({ file, content, jsonData });
        } catch (error) {
          if (error instanceof SyntaxError) {
            reject(new Error(`Invalid JSON file: ${error.message}`));
          } else {
            reject(error);
          }
        }
      };

      input.oncancel = () => resolve(null);
      input.click();
    });
  }
}

async function importUsers() {
  const result = await FileReader.openJsonFile();
  if (!result) {
    return;
  }
  var data = JSON.parse(result.content);
  if (State.password === undefined) {
    if (data !== "object") {
      alert("Invalid JSON data, expected object");
      return;
    }
    for (var key in data) {
      if (addUsername(key)) {
        var value = data[key];
        var identifier = value.identifier;
        var scope = value.scope;
        var counter = value.counter;
        var template = value.template;
        if (!identifier || !scope || !counter || !template) {
          continue;
        }
        addUserSite(
          key,
          value.identifier,
          value.scope,
          value.counter,
          value.template,
        );
      }
    }
  } else {
    if (!data.isArray()) {
      alert("Invalid JSON data, expected array");
      return;
    }
    for (var value in data) {
      var identifier = value.identifier;
      var scope = value.scope;
      var counter = value.counter;
      var template = value.template;
      if (!identifier || !scope || !counter || !template) {
        continue;
      }
      addUserSite(
        State.username,
        value.identifier,
        value.scope,
        value.counter,
        value.template,
      );
    }
  }
}

class FileSaver {
  static async saveFile(content, filename, contentType = "text/plain") {
    if ("showSaveFilePicker" in window) {
      try {
        return await this.saveWithDialog(content, filename, contentType);
      } catch (err) {
        if (err.name !== "AbortError") {
          console.warn(
            "File System Access API failed, falling back to download",
          );
        }
        return this.saveWithBlob(content, filename, contentType);
      }
    } else {
      return this.saveWithBlob(content, filename, contentType);
    }
  }

  static async saveWithDialog(content, filename, contentType) {
    const fileHandle = await window.showSaveFilePicker({
      suggestedName: filename,
      types: [
        {
          description: "Files",
          accept: { [contentType]: [`.${filename.split(".").pop()}`] },
        },
      ],
    });

    const writable = await fileHandle.createWritable();
    await writable.write(content);
    await writable.close();

    return true;
  }

  static saveWithBlob(content, filename, contentType) {
    const blob = new Blob([content], { type: contentType });
    const url = URL.createObjectURL(blob);

    const link = document.createElement("a");
    link.href = url;
    link.download = filename;

    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);

    URL.revokeObjectURL(url);
    return true;
  }
}

function exportUsers() {
  if (State.password === undefined) {
    var users = getUsernames();
    if (users.length === 0) {
      alert("Nothing to export...");
      return;
    }
    var result = {};
    users.forEach((user) => {
      result[user] = getUserSites(user);
    });
    FileSaver.saveFile(JSON.stringify(result), "pazz.json");
  } else {
    var sites = getUserSites(State.username);
    if (sites.length === 0) {
      alert("Nothing to export...");
      return;
    }
    var result = [];
    sites.forEach((site) => {
      result.push(site);
    });
    FileSaver.saveFile(JSON.stringify(result), `pazz_${State.username}.json`);
  }
}

function clearUsers() {
  if (State.password != undefined) {
    var sites = getUserSites(State.username);
    if (sites.length === 0) {
      return;
    }
    if (!confirm(`Do you want to remove all ${sites.length} site(s)?`)) {
      return;
    }
    sites.forEach((site) => {
      removeUserSite(State.username, site.identifier);
    });
    updatePasswordsList();
  } else {
    var users = getUsernames();
    if (users.length === 0) {
      return;
    }
    var sites_count = 0;
    users.forEach((user) => {
      sites_count += getUserSites(user).length;
    });
    if (
      !confirm(
        `Are you sure you want to clear all users? There are ${users.length} user(s) and ${sites_count} site(s).`,
      )
    ) {
      return;
    }
    localStorage.clear();
    updateUsernamesList();
  }
}

document.addEventListener("DOMContentLoaded", function () {
  if (hasLastUser()) {
    var last_user = getLastUser();
    State.username = last_user;
    document.getElementById("add-username-body").style.display = "none";
    document.getElementById("add-password-body").style.display = "block";
  } else {
    var users = getUsernames();
    if (users.length === 1) {
      State.username = users[0];
      document.getElementById("add-username-body").style.display = "none";
      document.getElementById("add-password-body").style.display = "block";
    }
  }
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
      State.username = username;
      updateUsernamesList();
      document.getElementById("add-username-body").style.display = "none";
      document.getElementById("add-password-body").style.display = "block";
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
      setLastUser(State.username);
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
      State.username = undefined;
      updateUsernamesList();
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
    // Get + store values for scope, counter + template
    const scope = document.getElementById("scope-input").value.trim();
    const counter = parseInt(document.getElementById("counter-input").value);
    const template = parseInt(
      document.getElementById("template-input").value.trim(),
    );
    addUserSite(State.username, site, scope, counter, template);
    State.site = undefined;
    updatePasswordsList();
  });

  document.addEventListener("click", function (e) {
    if (e.target.classList.contains("remove-username")) {
      const username = e.target.dataset.username;
      var sites = getUserSites(username);
      var extra =
        sites.length > 0 ? ` and all their sites (${sites.length})` : "";
      if (!confirm(`Do you want to remove user "${username}"${extra}?`)) {
        return;
      }
      removeUsername(username);
      State.username = undefined;
      updateUsernamesList();
      document.getElementById("add-username-body").style.display = "block";
      document.getElementById("add-password-body").style.display = "none";
    } else if (e.target.classList.contains("username")) {
      const username = e.target.textContent;
      if (State.username === username) {
        State.username = undefined;
        document.getElementById("add-username-body").style.display = "block";
        document.getElementById("add-password-body").style.display = "none";
      } else {
        State.username = username;
        document.getElementById("add-username-body").style.display = "none";
        document.getElementById("add-password-body").style.display = "block";
      }
      updateUsernamesList();
    } else if (e.target.classList.contains("remove-site")) {
      const site = e.target.dataset.site;
      if (!confirm(`Do you want to remove site "${site}"?`)) {
        return;
      }
      document.getElementById("password-box").style.display = "none";
      removeUserSite(State.username, site);
      State.site = undefined;
      updatePasswordsList();
    } else if (e.target.classList.contains("site")) {
      const site = e.target.textContent;
      var box = document.getElementById("password-box");
      if (State.site === site) {
        State.site = undefined;
        box.style.display = "none";
        box.innerHTML = "";
      } else {
        const sites = getUserSites(State.username);
        sites.every((_site) => {
          if (_site.identifier === site) {
            box.innerHTML = spectre(
              State.username,
              State.password,
              _site.identifier,
              _site.counter,
              _site.scope,
              _site.template,
            );
            box.style.display = "block";
            State.site = site;
            return false;
          }
          return true;
        });
      }
      updatePasswordsList();
    } else if (e.target.classList.contains("return")) {
      State.site = undefined;
      moveBackToUsername();
    }
  });
});
