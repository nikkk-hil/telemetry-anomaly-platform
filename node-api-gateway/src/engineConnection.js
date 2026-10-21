import net from "net";
import { MessageFramer } from "./utils/MessageFramer.js";
import { persistBlockedIp } from "./persistence/blocklistStore.js";

let socket = null;
let retryDelay = 1000;
let isConnected = false;

const connectToEngine = () => {
  socket = new net.Socket();
  const framer = new MessageFramer((message) => {
    handleEngineMessage(message);
  });
  socket.connect(process.env.ENGINE_PORT, process.env.ENGINE_HOST, () => {
    isConnected = true;
    retryDelay = 1000;
    console.log("Socket Connected");
  });

  socket.on("data", (chunk) => {
    framer.feed(chunk);
  });

  socket.on("error", (err) => {
    console.log(err.message);
  });

  socket.on("close", () => {
    isConnected = false;
    retryDelay = Math.min(retryDelay*3, 60000);
    setTimeout(() => {
      connectToEngine();
    }, retryDelay);
  });
}

const sendTelemetry = (ip, endpoint) => {
  if (!isConnected) return;

  const payload = {
    ip,
    endpoint,
    timestamp: Date.now()
  };
  const payloadBytes = Buffer.from(JSON.stringify(payload), "utf8");
  const lengthPrefix = Buffer.alloc(4);
  lengthPrefix.writeUInt32LE(payloadBytes.length, 0);

  const framed = Buffer.concat([lengthPrefix, payloadBytes]);
  socket.write(framed);
};

const handleEngineMessage = async (message) => {
  if (message.startsWith("BLOCK:")) {
    const ip = message.substring(6).trim();
    if (!ip) return;

    await persistBlockedIp(ip);
  }
};

export { connectToEngine, sendTelemetry };
