import "dotenv/config";
import express from "express";
import { connectToEngine } from "./src/engineConnection.js";
import { blockMiddleware } from "./src/middlewares/blockMiddleware.js";
import { getBlocklistSize, removeExpiredBlocklistEntries } from "./src/utils/blocklist.js";
import { closeBlocklistStore, initializeBlocklist } from "./src/persistence/blocklistStore.js";

const app = express();
const PORT = process.env.PORT_1;
const blocklistSweep = setInterval(removeExpiredBlocklistEntries, 60_000);
blocklistSweep.unref();

app.use(express.json());
app.use(blockMiddleware);

app.get("/api/test", (req, res) => {
  res.json({ message: "Request allowed.", ip: req.ip });
});

app.get("/api/status", (req, res) => {
  res.json({ blockedIpCount: getBlocklistSize() });
});

async function start() {
  try {
    await Promise.race([
      initializeBlocklist(),
      new Promise((_, reject) => setTimeout(() => reject(new Error("DB restore timed out")), 5_000)),
    ]);
  } catch (err) {
    console.error("Starting without restored DB blocklist:", err.message);
  }
  connectToEngine();
  app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
  });
}

for (const signal of ["SIGINT", "SIGTERM"]) {
  process.once(signal, async () => {
    clearInterval(blocklistSweep);
    await closeBlocklistStore();
    process.exit(0);
  });
}

start().catch((err) => {
  console.error("Gateway failed to start:", err.message);
  process.exit(1);
});
