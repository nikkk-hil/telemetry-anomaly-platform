const blockedIps = new Map();

function addToBlocklist(ip, expiresAt) {
    const expiryMs = new Date(expiresAt).getTime();
    const currentExpiry = blockedIps.get(ip) ?? 0;
    if (expiryMs > currentExpiry) blockedIps.set(ip, expiryMs);
}

function isBlocked(ip){
    const expiryMs = blockedIps.get(ip);
    if (!expiryMs) return false;
    console.log(`Expiry: ${expiryMs}`);
    console.log(`Now: ${Date.now()}`);
    if (expiryMs <= Date.now()) {
        blockedIps.delete(ip);
        return false;
    }
    return true;
}

function getBlocklistSize(){
    return blockedIps.size;
}

function removeExpiredBlocklistEntries() {
    const now = Date.now();
    for (const [ip, expiryMs] of blockedIps) {
        if (expiryMs <= now) blockedIps.delete(ip);
    }
}

export { isBlocked, addToBlocklist, getBlocklistSize, removeExpiredBlocklistEntries };
