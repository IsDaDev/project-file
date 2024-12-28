const express = require("express");
const dotenv = require("dotenv");
dotenv.config();

const func = require("./scripts/functions.js");
const app = express();
const port = 3000;
const dataServer = "http://127.0.0.1:8000";

require("./scripts/config.js")(app);

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});

app.get("/", (req, res) => {
  if (!req.session.user) {
    return res.redirect("/login");
  } else {
    res.render("home");
  }
});

app.get("/login", (req, res) => {
  if (req.session.user) {
    return res.redirect("/");
  } else {
    res.render("login");
  }
});

app.post("/login", async (req, res) => {
  const username = req.body.username;
  const password = req.body.password;

  if (await func.loginCheck(username, password)) {
    req.session.user = username;
    return res.redirect("/");
  } else {
    res.send("invalid credentials");
  }
});

app.get("/list", async (req, res) => {
  if (!req.session.user) {
    return res.redirect("/login");
  }

  try {
    const { entries, dataCount, path } = await func.handleData(
      req.path,
      dataServer
    );
    res.render("find", { entries, dataCount, path });
  } catch (error) {
    console.log("Error: ", error);
    res.send("Error retrieving data");
  }
});

app.get("/list/:path*", async (req, res) => {
  if (!req.session.user) {
    return res.redirect("/login");
  }

  try {
    const fullPath = `/list/${req.params.path}${req.params[0] || ""}`;
    const { entries, dataCount, path } = await func.handleData(
      fullPath,
      dataServer
    );

    if (entries[0]["name"] == "not found") {
      res.render("404", { dir: path });
    } else if (entries[0].flag == 1) {
      const { name, size, content } = func.displayTextAsCode(entries);
      res.render("dataplane", { name, size, content, path });
    } else {
      res.render("find", { entries, dataCount, path });
    }
  } catch (error) {
    let errorMsg = "Error retrieving data for path: " + req.params.path;
    console.error(errorMsg, error);
    res.status(500).send(errorMsg);
  }
});
