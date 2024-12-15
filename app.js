const express = require("express");
const path = require("path");
const port = 3000;
const serverIP = "http://127.0.0.1:8000";

const fetchData = async (path) => {
  let fullPath = serverIP + path;
  console.log("Fetching from: ", fullPath);
  let response = await fetch(fullPath);

  if (!response.ok) {
    console.error(`Failed to fetch: ${response.statusText}`);
    throw new Error(`HTTP Error ${response.status}`);
  }

  return response.text();
};

const sortArray = (array) => {
  array.sort((a, b) => {
    if (a.type < b.type) return -1;
    if (a.type > b.type) return 1;
    return 0;
  });
};

const handleData = async (fpath) => {
  let data = await fetchData(fpath);

  if (!data) {
    throw new Error("Error fetching data");
  }

  let dataCount = data.substring(data.length - 7);
  let matches = data.match(/{.*?}/g);

  if (!matches) {
    return { entries: matches, dataCount: dataCount, path: fpath };
  }

  matches = matches.map((dataset) => {
    let cl = dataset.replaceAll("'", '"');
    return JSON.parse(cl);
  });

  sortArray(matches);

  return { entries: matches, dataCount: dataCount, path: fpath };
};

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
    const { entries, dataCount, path } = await handleData(req.path);
    console.log(entries);
    res.render("find", { entries, dataCount, path });
  } catch (error) {
    console.log("Error: ", error);
    res.send("Error retrieving data");
  }
});

app.get("/list/:path*", async (req, res) => {
  try {
    const fullPath = `/list/${req.params.path}${req.params[0] || ""}`;
    const { entries, dataCount, path } = await handleData(fullPath);

    if (entries[0]["name"] == "not found") {
      res.render("404", { dir: path });
    }

    res.render("find", { entries, dataCount, path });
  } catch (error) {
    let errorMsg = "Error retrieving data for path: " + req.params.path;
    console.error(errorMsg, error);
    res.status(500).send(errorMsg);
  }
});
