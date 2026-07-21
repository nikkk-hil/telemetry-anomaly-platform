import { isBlocked } from "../utils/blocklist.js";
import { sendTelemetry } from "../engineConnection.js";

function blockMiddleware(req, res, next){
    const clientIp = req.ip;
    console.log(clientIp);

    if (isBlocked(clientIp))
        return res.status(403).json({ error: 'Forbidden: IP blocked due to suspicious activity.' });

    sendTelemetry(clientIp, req.originalUrl);

    next();
}

export {blockMiddleware}