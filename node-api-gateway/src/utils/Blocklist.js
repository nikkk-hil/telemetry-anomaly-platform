const blockedIps = new Set();

function addToBlocklist(ip) {
    blockedIps.add(ip);
}

function isBlocked(ip){
    return blockedIps.has(ip);
}

function getBlocklistSize(){
    return blockedIps.size;
}

export { isBlocked, addToBlocklist, getBlocklistSize };