import { isBlocked } from "../utils/blocklist.js";
import { sendTelemetry } from "../engineConnection.js";

function blockMiddleware(req, res, next){
    const clientIp = req.ip;
    // console.log(req.ip);
    if (isBlocked(clientIp))
        return res.status(429).json({ error: 'Too many requests.' });

    sendTelemetry(clientIp, req.originalUrl);

    next();
}

export {blockMiddleware}
