const express = require("express");
const path = require("path");
const port = 3000;
const serverIP = "http://127.0.0.1:8000";
const { handleData, dataplaneFormatting } = require("./functions.js");

const app = express();

app.set("view engine", "ejs");

app.use(express.static(path.join(__dirname, "public")));

app.use((req, res, next) => {
  if (req.path.length > 1 && req.path.endsWith("/")) {
    const newPath = req.path.slice(0, -1);
    return res.redirect(301, newPath);
  }
  next();
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});

app.get("/", (req, res) => {
  res.render("index");
});

app.get("/list", async (req, res) => {
  try {
    const { entries, dataCount, path } = await handleData(req.path, serverIP);
    res.render("find", { entries, dataCount, path });
  } catch (error) {
    console.log("Error: ", error);
    res.send("Error retrieving data");
  }
});

app.get("/list/:path*", async (req, res) => {
  try {
    const fullPath = `/list/${req.params.path}${req.params[0] || ""}`;
    const { entries, dataCount, path } = await handleData(fullPath, serverIP);

    if (entries[0]["name"] == "not found") {
      res.render("404", { dir: path });
    } else if (entries[0].flag == 1) {
      const { name, size, content } = dataplaneFormatting(entries);
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
