import pg from "pg";
import { addToBlocklist } from "../utils/blocklist.js";

const { Pool } = pg;
const pool = new Pool({ connectionString: process.env.DATABASE_URL });
const blockTtlSeconds = Number.parseInt(process.env.BLOCK_TTL_SECONDS ?? "1800", 10);

async function initializeBlocklist() {
  if (!process.env.DATABASE_URL) {
    throw new Error("DATABASE_URL is required for durable blocklist storage");
  }

  if (!Number.isInteger(blockTtlSeconds) || blockTtlSeconds <= 0) {
    throw new Error("BLOCK_TTL_SECONDS must be a positive integer");
  }
  await pool.query(`
    CREATE TABLE IF NOT EXISTS block_events (
      id BIGSERIAL PRIMARY KEY,
      ip TEXT NOT NULL,
      blocked_at TIMESTAMPTZ NOT NULL DEFAULT now(),
      expires_at TIMESTAMPTZ NOT NULL
    );
    CREATE INDEX IF NOT EXISTS idx_block_events_expires ON block_events (expires_at);
    CREATE INDEX IF NOT EXISTS idx_block_events_ip ON block_events (ip);
    ALTER TABLE block_events DROP COLUMN IF EXISTS trigger_count;
  `);

  const { rows } = await pool.query(`
    SELECT ip, MAX(expires_at) AS expires_at
    FROM block_events
    WHERE expires_at > NOW()
    GROUP BY ip
  `);
  for (const { ip, expires_at } of rows) addToBlocklist(ip, expires_at);
}

// Cache first so a database outage cannot delay blocking; persistence is best-effort.
async function persistBlockedIp(ip) {
  const expiresAt = new Date(Date.now() + blockTtlSeconds * 1000).toISOString();
  addToBlocklist(ip, expiresAt);

  try {
    await pool.query(
      `INSERT INTO block_events (ip, expires_at)
       VALUES ($1, $2)`,
      [ip, expiresAt],
    );
  } catch (err) {
    console.error(`Failed to persist blocked IP ${ip}:`, err.message);
  }
}

async function closeBlocklistStore() {
  await pool.end();
}

export { initializeBlocklist, persistBlockedIp, closeBlocklistStore };
