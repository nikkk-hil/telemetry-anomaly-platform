import "dotenv/config";
import express from "express";
import { connectToEngine } from "./src/engineConnection.js";
import { blockMiddleware } from "./src/middlewares/blockMiddleware.js";
import { getBlocklistSize } from "./src/utils/Blocklist.js";

const app = express();
const PORT = process.env.PORT;

connectToEngine();

app.use(express.json());
app.use(blockMiddleware);

app.get("/api/test", (req, res) => {
  res.json({ message: "Request allowed.", ip: req.ip });
});

app.get("/api/status", (req, res) => {
  res.json({ blockedIpCount: getBlocklistSize() });
});

app.listen(PORT, () => {
      console.log("Server is running on http://localhost:3000");
});
