const { base64decode } = require("nodejs-base64");

const fetchData = async (path, serverIP) => {
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

const handleData = async (fpath, serverIP) => {
  let data = await fetchData(fpath, serverIP);

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

  for (let i = 0; i < matches.length; i++) {
    matches[i]["content"] = base64decode(matches[i]["content"]);
  }

  return { entries: matches, dataCount: dataCount, path: fpath };
};

const dataplaneFormatting = (data) => {
  let name = data[0]["name"];
  let size = data[0]["size"];
  let content = data[0]["content"];

  if (content == base64decode("binary file")) {
    content = "This is a binary file";
  }

  return { name, size, content };
};

module.exports = { handleData, dataplaneFormatting };
