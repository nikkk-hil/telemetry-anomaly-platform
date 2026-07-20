class MessageFramer {
    constructor(onMessage) {
        this.onMessage = onMessage;

        this.PREFIX_SIZE = 4;
        this.prefixBuffer = Buffer.alloc(this.PREFIX_SIZE);
        this.remainingPrefixBytes = this.PREFIX_SIZE;
        this.remainingMessageBytes = 0;

        this.messageChunks = [];
    }

    feed(chunk) {
        let usedBytes = 0;
        const totalBytes = chunk.length;

        while (usedBytes < totalBytes) {
            if (this.remainingMessageBytes === 0 && this.remainingPrefixBytes > 0) {
                const toCopy = Math.min(this.remainingPrefixBytes, totalBytes - usedBytes);
                const alreadyHave = this.PREFIX_SIZE - this.remainingPrefixBytes;
                chunk.copy(this.prefixBuffer, alreadyHave, usedBytes, usedBytes + toCopy);

                this.remainingPrefixBytes -= toCopy;
                usedBytes += toCopy;

                if (this.remainingPrefixBytes === 0) {
                    const messageLength = this.prefixBuffer.readUInt32LE(0);
                    this.remainingMessageBytes = messageLength;
                }
            }

            else if (this.remainingMessageBytes > 0) {
                const toCopy = Math.min(this.remainingMessageBytes, totalBytes - usedBytes);
                const slice = chunk.subarray(usedBytes, usedBytes + toCopy);
                this.messageChunks.push(slice);

                usedBytes += toCopy;
                this.remainingMessageBytes -= toCopy;
            }

            if (this.remainingPrefixBytes === 0 && this.remainingMessageBytes === 0) {
                const completeMessage = Buffer.concat(this.messageChunks).toString('utf8');
                this.onMessage(completeMessage);

                this.remainingPrefixBytes = this.PREFIX_SIZE;
                this.messageChunks = [];
            }
        }
    }
}

module.exports = { MessageFramer };