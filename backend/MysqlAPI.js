var express = require("express");
var bodyParser = require("body-parser");
var mysql = require("mysql2");
const cors = require("cors");
var app = express();
app.use(cors());
app.use(express.static("public"));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
// step 1: rong workbench mở cửa sổ sql và chạy lệnh
//ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'your_password';
//step 2:
var con = mysql.createConnection({
  host: "thcntt3-database-test",
  port: "3306",
  user: "root",
  password: "123456",
  //insecureAuth : true,
  database: "thcntt",
});
//step 1
con.connect(function (err) {
  if (err) {
    console.log(err);
    throw err;
  }
  console.log("Connected!!!");
  var sql = "SELECT * FROM devices";
  con.query(sql, function (err, results) {
    if (err) throw err;
    console.log(results);
  });
});
//RESTFull API
app.get("/devices", function (req, res) {
  var sql = "SELECT * FROM devices";
  con.query(sql, function (err, results) {
    if (err) throw err;
    res.send(results);
  });
});
app.post("/devices", function (req, res) {
  const { name, status } = req.body;
  var sql =
    "insert devices(name,status) values('" + name + "','" + status + "');";
  con.query(sql, function (err, results) {
    if (err) throw err;
    res.send("add device successfully");
  });
});
var server = app.listen(5555, function () {
  var host = server.address().address;
  var port = server.address().port;
  console.log("Example app listening at http://%s:%s", host, port);
});
