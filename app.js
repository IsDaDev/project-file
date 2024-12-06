const { match } = require("assert");
const express = require("express");
const { request } = require("http");
const path = require("path");
const { Database } = require("sqlite3");
const port = 3000;
const serverIP = "http://127.0.0.1:8000";

const fetchData = async (path) => {
  let fullPath = serverIP + path;
  let data = await fetch(fullPath);
  return data.text();
};

const app = express();

app.set("view engine", "ejs");

app.use(express.static(path.join(__dirname, "public")));

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});

app.get("/", (req, res) => {
  res.render("index");
});

app.get("/list", async (req, res) => {
  try {
    let data = await fetchData(req.path);
    let dataCount = data.substring(data.length - 7);
    let matches = data.match(/{.*?}/g);

    matches = matches.map((dataset) => {
      let cl = dataset.replaceAll("'", '"');
      return JSON.parse(cl);
    });

    matches.sort((a, b) => {
      if (a.type < b.type) return -1;
      if (a.type > b.type) return 1;
      return 0;
    });

    console.log(matches);

    res.render("find", { entries: matches, dataCount: dataCount });
  } catch (error) {
    console.log("Error: ", error);
    res.send("Error retrieving data");
  }
});
